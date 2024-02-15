// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2007, 2008, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
//     http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
// 

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Server";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Server/Transaction.h"

#include "DServer/AutoBranch.h"
#include "DServer/Branch.h"

#include "Schema/Object.h"
#include "Schema/Database.h"

#include "Server/FakeError.h"
#include "Server/InstanceManager.h"
#include "Server/SQLDispatchEntry.h"
#include "Server/Session.h"

#include "Common/Assert.h"
#include "Common/IntegerData.h"
#include "Common/Message.h"
#include "Common/ResultSetMetaData.h"
#include "Common/SQLData.h"
#include "Communication/ServerConnection.h"
#include "DServer/Cascade.h"
#include "Exception/InvalidExplicitTransaction.h"
#include "Exception/NotBeginTransaction.h"
#include "Exception/NotSupported.h"
#include "Exception/ReadOnlyDatabase.h"
#include "Exception/XA_ProtocolError.h"
#include "Statement/StartTransactionStatement.h"
#include "Statement/SetTransactionStatement.h"
#include "Statement/TransactionMode.h"
#include "Statement/XA_CommitStatement.h"
#include "Statement/XA_EndStatement.h"
#include "Statement/XA_ForgetStatement.h"
#include "Statement/XA_Identifier.h"
#include "Statement/XA_PrepareStatement.h"
#include "Statement/XA_RollbackStatement.h"
#include "Statement/XA_StartStatement.h"
#include "Trans/AutoBranch.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace
{

namespace _Transaction
{
	// SQL 文で指定されたトランザクションモードを得る
	Trans::Transaction::Mode
	getMode(const Statement::TransactionMode* stmt);
	// SQL 文で指定されたトランザクションブランチ識別子を得る
	Trans::Branch::ID
	getXID(const Statement::XA_Identifier* stmt);
}

//	FUNCTION
//	$$::_Transaction::getMode --
//		SQL 文で指定されたトランザクションモードを得る
//
//	NOTES
//
//	ARGUMENTS
//		Statement::TransactionMode*	stmt
//			SQL 文のトランザクションモードの部分
//
//	RETURN
//		得られたトランザクションモード
//
//	EXCEPTIONS

Trans::Transaction::Mode
_Transaction::getMode(const Statement::TransactionMode* stmt)
{
	Trans::Transaction::Mode mode;

	if (stmt) {
		switch (stmt->getAccMode()) {
		case Statement::TransactionMode::ReadOnly:
			mode._category = Trans::Transaction::Category::ReadOnly;
			break;
		case Statement::TransactionMode::ReadWrite:
			mode._category = Trans::Transaction::Category::ReadWrite;
			break;
		}

		switch (stmt->getIsoLevel()) {
		case Statement::TransactionMode::ReadUncommitted:
			mode._isoLevel =
				Trans::Transaction::IsolationLevel::ReadUncommitted;
			break;
		case Statement::TransactionMode::ReadCommitted:
			mode._isoLevel = Trans::Transaction::IsolationLevel::ReadCommitted;
			break;
		case Statement::TransactionMode::RepeatableRead:
			mode._isoLevel = Trans::Transaction::IsolationLevel::RepeatableRead;
			break;
		case Statement::TransactionMode::Serializable:
			mode._isoLevel = Trans::Transaction::IsolationLevel::Serializable;
			break;
		}

		if (stmt->isUsingSnapshot())
			mode._snapshot = Boolean::True;
	}

	return mode;
}

//	FUNCTION
//	$$::_Transaction::getXID --
//		SQL 文で指定されたトランザクションブランチ識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		Statement::XA_Identifier* stmt
//			SQL 文のトランザクションブランチ識別子の部分
//
//	RETURN
//		得られたトランザクションブランチ識別子
//
//	EXCEPTIONS

Trans::Branch::ID
_Transaction::getXID(const Statement::XA_Identifier* stmt)
{
	; _SYDNEY_ASSERT(stmt);

	return Trans::Branch::ID(stmt->getGlobalTransactionIdentifier(),
							 stmt->getBranchQualifier(),
							 stmt->getFormatIdentifier());
}

}

//	FUNCTION public
//	Server::AutoSession::AutoSession -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Server::InstanceManager&	manager
//			セッションオブジェクトの生成・破棄を管理するマネージャ
//		Server::ID			id
//			セッション識別子
//		int	statementType
//			実行するSQL文のタイプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoSession::AutoSession(InstanceManager& manager, ID id, int statementType)
	: _manager(manager),
	  _session(0)
{
	_session = manager.lockSession(id, statementType);
	; _SYDNEY_ASSERT(_session);
}

//	FUNCTION public
//	Server::AutoSession::~AutoSession -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoSession::~AutoSession()
{
	_session->setCurrentSQL(0,0);	// 実行SQL文をクリアする
	_manager.unlockSession(_session);
}

//
//	FUNCTION public
//	Server::AutoSession::changeStatementType
//		-- 保持するセッションの実行文タイプを変更する
//
//	NOTES
//
//	ARGUMENTS
//	int iStatementType_
//		実行文タイプ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
AutoSession::changeStatementType(int iStatementType_)
{
	_manager.lock();
	try
	{
		_session->changeStatementType(iStatementType_);
	}
	catch (...)
	{
		_manager.unlock();
		_SYDNEY_RETHROW;
	}
	_manager.unlock();
}

//	FUNCTION public
//	Server::Transaction::Transaction -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Server::ID		sessionID
//			トランザクションを実行するセッションのセッション識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Transaction::Transaction(ID sessionID)
	: _sessionID(sessionID),
	  _transaction(Trans::Transaction::attach(sessionID)),
	  _branch(0),
	  _status(Status::Unknown),
	  _databaseID(Schema::Object::ID::Invalid),
	  m_bUsingSnapshot(false),
	  m_pXABranch(0)
{
	; _SYDNEY_ASSERT(_transaction.get());
	m_cStartTime.setNull();
}

//	FUNCTION public
//	Server::Transaction::~Transaction -- デストラクタ
//
//	NOTES
//		実行中のトランザクションがあれば、ロールバックされる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Transaction::~Transaction()
{}

//	FUNCTION public
//	Server::Transaction::getDescriptor -- トランザクション記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたトランザクション記述子
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			トランザクションが実行中でないし、
//			『データ操作中』のトランザクションブランチと連係していない

Trans::Transaction&
Transaction::getDescriptor()
{
	switch (_status) {
	case Status::Explicit:
	case Status::Implicit:
		; _SYDNEY_ASSERT(_transaction.get());
		return *_transaction;
	case Status::Branch:
		; _SYDNEY_ASSERT(_branch);
		return _branch->getTransaction();
	default:
		; _SYDNEY_THROW0(Exception::NotBeginTransaction);
	}
}

//	FUNCTION public
//	Server::Transaction::start --
//		SQL文の指示に従い、トランザクションを明示的に開始する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションを明示的に開始する SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::start(Schema::ObjectID::Value databaseID,
				   const Statement::Object& stmt)
{
	// SQL 文に指定されているトランザクションモードを得る

	const Statement::StartTransactionStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::StartTransactionStatement&, stmt);

	const Trans::Transaction::Mode mode =
		_Transaction::getMode(tmp.getTransactMode());

	// 求めたモードのトランザクションを開始する

	; _SERVER_FAKE_ERROR(Worker::startTransaction);
	; _SYDNEY_ASSERT(_transaction.get());
	_transaction->begin(databaseID, mode, Status::Explicit);

	// 明示的にトランザクションが開始されていることを覚えておく

	_status = Status::Explicit;
	m_cStartTime.setCurrent();

	// 後で分散トランザクションを開始するかもしれないので、
	// 必ず版管理を利用するトランザクションかどうかを確認しておく

	m_bUsingSnapshot =
		(((mode._snapshot != Boolean::Unknown) ?
		  mode._snapshot : _transaction->getDefaultMode()._snapshot)
		 == Boolean::True);
}

//	FUNCTION public
//	Server::Transaction::commit --
//		SQL文の指示に従い、トランザクションを明示的にコミットする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションを明示的にコミットする SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::commit(const Statement::Object& dummy)
{
	// 現状、引数を指定しない Server::Transaction::commit と動作は同じである

	; _SERVER_FAKE_ERROR(Worker::transactionCommit);
	commit();
}

//	FUNCTION public
//	Server::Transaction::commit --
//		SQL文の指示に従い、トランザクションを明示的にコミットする
//
//	NOTES
//
//	ARGUMENTS
//		bool bNeedReCache_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::commit(bool bNeedReCache_)
{
	try {

		// 分散トランザクションがあればコミットする

		if (m_pXABranch)
		{
			m_pXABranch->commitOnePhase(_sessionID, getDescriptor());
			DServer::Branch::detach(m_pXABranch);
			m_pXABranch = 0;
		}
		
		// トランザクションをコミットする

		; _SYDNEY_ASSERT(_transaction.get());
		_transaction->commit(bNeedReCache_);

		// カスケード先のセッションを解放する

		DServer::Cascade::eraseSession(m_vecpDSession);
		m_vecpDSession.clear();
		
	} catch (...) {

		if (!_transaction->isInProgress())
		{
			// カスケード先のセッションを解放する

			DServer::Cascade::eraseSession(m_vecpDSession);
			m_vecpDSession.clear();
			
			// 例外が発生してもトランザクションが終了していれば、
			// 終了したことを覚えておく

			_status = Status::Unknown;
			m_cStartTime.setNull();
		}

		_SYDNEY_RETHROW;
	}

	// 明示的なトランザクションが終了したことを覚えておく

	_status = Status::Unknown;
	m_cStartTime.setNull();
}

//	FUNCTION public
//	Server::Transaction::rollback --
//		SQL文の指示に従い、トランザクションを明示的にロールバックする
//
//	NOTES
//		現状、引数を指定しない Server::Transaction::rollback と動作は同じである
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションを明示的にロールバックする SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::rollback(const Statement::Object& dummy)
{
	// 現状、引数を指定しない Server::Transaction::rollback と動作は同じである

	; _SERVER_FAKE_ERROR(Worker::transactionRollback);
	rollback();
}

//	FUNCTION public
//	Server::Transaction::rollback --
//		自身の開始したトランザクションをロールバックする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::rollback()
{
	try {

		// 分散トランザクションがあればブランチを終了する

		if (m_pXABranch)
		{
			m_pXABranch->end(_sessionID, getDescriptor());
			DServer::Branch::detach(m_pXABranch);
			m_pXABranch = 0;
		}
		
		// トランザクションをロールバックする

		; _SYDNEY_ASSERT(_transaction.get());
		_transaction->rollback();

		// カスケード先のセッションを解放する

		DServer::Cascade::eraseSession(m_vecpDSession);
		m_vecpDSession.clear();

	} catch (...) {

		if (!_transaction->isInProgress())
		{
			// カスケード先のセッションを解放する

			DServer::Cascade::eraseSession(m_vecpDSession);
			m_vecpDSession.clear();
			
			// 例外が発生してもトランザクションが終了していれば、
			// 終了したことを覚えておく

			_status = Status::Unknown;
			m_cStartTime.setNull();
		}

		_SYDNEY_RETHROW;
	}

	// 明示的なトランザクションが終了したことを覚えておく

	_status = Status::Unknown;
	m_cStartTime.setNull();
}

//	FUNCTION public
//	Server::Transaction::set --
//		SQL文の指示に従い、トランザクションのデフォルトモードを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションのデフォルトモードを設定する SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::set(const Statement::Object& stmt)
{
	// SQL 文に指定されているトランザクションモードを得る

	const Statement::SetTransactionStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::SetTransactionStatement&, stmt);

	const Trans::Transaction::Mode mode =
		_Transaction::getMode(tmp.getTransactMode());

	// 求めたモードをトランザクションのデフォルトモードにする

	; _SERVER_FAKE_ERROR(Worker::setTransaction);
	; _SYDNEY_ASSERT(_transaction.get());
	_transaction->setDefaultMode(mode);
}

//	FUNCTION public
//	Server::Transaction::startStatement -- SQL 文の実行の開始を宣言する
//
//	NOTES
//
//	ARGUMENTS
//		Server::Session*	pSession_
//			セッション
//		Communication::ServerConnection&	connection
//			SQL 文の実行の開始を宣言するセッションと
//			この SQL 文の実行を要求したクライアントとのコネクション
//		Server::SQLDispatchEntry&	entry
//			実行の開始を宣言する SQL 文に関する取扱情報
//		Trans::Transaction::IsolationLevel::Value	isolation
//			暗黙のトランザクションを開始する場合のそのアイソレーションレベル
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::InvalidExplicitTransaction
//			明示的に開始されたトランザクション中で
//			実行できない SQL 文を実行しようとした
//		Exception::NotBeginTransaction
//			暗黙に開始されたトランザクション中で
//			実行できない SQL 文を実行しようとした

void
Transaction::startStatement(
	Session* pSession_,
	Communication::ServerConnection& connection,
	const SQLDispatchEntry& entry,
	Trans::Transaction::IsolationLevel::Value isolation)
{
	if (!isNormal())
		// ロールバックしかできない
		_SYDNEY_THROW0(Exception::NotSupported);

	if (pSession_->isSlaveDatabase()
		&& entry.isExecutableOnSlaveDatabase() == false)
		// スレーブでは実行できない文
		_SYDNEY_THROW1(Exception::ReadOnlyDatabase,
					   pSession_->getDatabaseName());
	
	switch (_status) {
	case Status::Explicit:
	case Status::Branch:

		// 明示的にトランザクションまたは
		// トランザクションブランチが開始されている

		if (!(entry.getPermissionType() & Permission::Explicitly))

			// 明示的に開始されたトランザクション中で
			// 実行できない SQL 文を実行しようとした

			_SYDNEY_THROW0(Exception::InvalidExplicitTransaction);
		break;

	case Status::Unknown:
	{
		// 明示的にトランザクション・トランザクションブランチが開始されていない

		if (!(entry.getPermissionType() & Permission::Implicitly))

			// 暗黙に開始されたトランザクション中で
			// 実行できない SQL 文を実行しようとした

			_SYDNEY_THROW0(Exception::NotBeginTransaction);

		// 暗黙に開始するトランザクションのモードを得る

		; _SYDNEY_ASSERT(_transaction.get());
		Trans::Transaction::Mode mode(
			_transaction->getDefaultMode()._category,
			isolation, Boolean::Unknown);

		if (!(entry.getPermissionType() & Permission::Explicitly) ||
			mode._category == Trans::Transaction::Category::Unknown)

			// 明示的に開始されたトランザクション中で実行できない
			// SQL 文を実行しようとしているか、
			// トランザクションの種別がまだ不明の場合、
			// トランザクションの種別を
			// 読取専用トランザクションで実行可能な SQL 文に対しては
			// 読取専用トランザクションを、そうでないものに対しては
			// 更新トランザクションとする

			mode._category =
			((entry.isExecutableInsideReadOnlyTransaction() == Boolean::True) ?
			 Trans::Transaction::Category::ReadOnly :
			 Trans::Transaction::Category::ReadWrite);

		if (pSession_->isSlaveDatabase() &&
			mode._category == Trans::Transaction::Category::ReadOnly)

			// スレーブデータベースで、
			// 読取専用トランザクションの場合は、
			// 必ず、版管理を使った読み取り専用トランザクションとする

			mode._snapshot = Boolean::True;

		// 暗黙のトランザクションを開始する

		_transaction->begin(pSession_->getDatabaseID(), mode, Status::Implicit);

		// 暗黙のトランザクションを開始したことを覚えておく

		_status = Status::Implicit;
		m_cStartTime.setCurrent();
		
		// 後で分散トランザクションを開始するかもしれないので、
		// 必ず版管理を利用するトランザクションかどうかを確認しておく

		m_bUsingSnapshot =
			(((mode._snapshot != Boolean::Unknown) ?
			  mode._snapshot : _transaction->getDefaultMode()._snapshot)
			 == Boolean::True);
		
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
	}

	try {

		// SQL 文の実行を開始する

		getDescriptor().beginStatement(&connection);

	} catch (...) {

		if (_status == Status::Implicit)

			// 開始した暗黙のトランザクションを終了する

			rollback();

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Server::Transaction::commitStatement -- SQL 文の実行が成功したことを宣言する
//
//	NOTES
//
//	ARGUMENTS
//		bool bNeedReCache_ = false
//			trueのときスキーマオブジェクトの切り替えを行う
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::commitStatement(bool bNeedReCache_)
{
	; _SYDNEY_ASSERT(_transaction.get());

	switch (_status) {
	case Status::Explicit:
		_transaction->commitStatement();				break;
	case Status::Branch:
		; _SYDNEY_ASSERT(_branch);
		_branch->getTransaction().commitStatement();	break;
	case Status::Implicit:
		commit(bNeedReCache_);							break;
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//	FUNCTION public
//	Server::Transaction::rollbackStatement -- SQL 文の実行がなかったことにする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::rollbackStatement()
{
	; _SYDNEY_ASSERT(_transaction.get());

	if (m_pXABranch)
		// エラーが発生したのでフラグを設定
		m_pXABranch->setErrorFlag();

	switch (_status) {
	case Status::Explicit:
		_transaction->rollbackStatement();				break;
	case Status::Branch:
		; _SYDNEY_ASSERT(_branch);
		_branch->getTransaction().rollbackStatement();	break;
	case Status::Implicit:
		rollback();										break;
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//	FUNCTION public
//	Server::Transaction::xa_start --
//		SQL文の指示に従い、トランザクションブランチを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションブランチを開始する SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Trans::XID
Transaction::xa_start(Schema::ObjectID::Value databaseID,
					  const Statement::Object& stmt)
{
	const Statement::XA_StartStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::XA_StartStatement&, stmt);

	// 指定されたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子を生成する

	Trans::XID xid(_Transaction::getXID(tmp.getIdentifier()));
	Trans::AutoBranch branch(Trans::Branch::attach(xid));
	; _SYDNEY_ASSERT(branch.get());

	const Trans::Branch::StartOption::Value option =
		(tmp.getJoin()) ? Trans::Branch::StartOption::Join :
		(tmp.getResume()) ? Trans::Branch::StartOption::Resume :
		Trans::Branch::StartOption::Unknown;

	if (option == Trans::Branch::StartOption::Unknown)

		// 新たにトランザクションブランチを開始する

		branch->start(databaseID, _sessionID,
					  _Transaction::getMode(tmp.getTransactionMode()));
	else

		// 既存のトランザクションブランチを再開・結合する

		branch->start(databaseID, _sessionID, option);

	// 開始・再開・結合されたトランザクションブランチの
	// トランザクションブランチ記述子を覚えておく

	; _SYDNEY_ASSERT(!_branch);
	_branch = branch.release();

	if (!_branch->getChildID().isNull() && !_branch->getChildID().isIllegal())
	{
		// 子サーバのトランザクション記述子を得る

		m_pXABranch = DServer::Branch::attach(_branch->getChildID());
	}

	// トランザクションブランチが開始されていることを覚えておく

	_status = Status::Branch;
	m_cStartTime.setCurrent();

	return xid;
}

//	FUNCTION public
//	Server::Transaction::xa_end --
//		SQL文の指示に従い、トランザクションブランチを待機・中断する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションブランチを待機・中断する SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_end(const Statement::Object& stmt)
{
	const Statement::XA_EndStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::XA_EndStatement&, stmt);

	Trans::Branch::EndOption::Value option;
	switch (tmp.getSuspensionMode()) {
	case Statement::XA_EndStatement::SuspensionMode::Normal:
		option = Trans::Branch::EndOption::Suspend;	break;
	case Statement::XA_EndStatement::SuspensionMode::ForMigrate:
		option = Trans::Branch::EndOption::Migrate;	break;
	default:
		option = Trans::Branch::EndOption::Unknown;
	}

	xa_end(_Transaction::getXID(tmp.getIdentifier()), option);
}

//	FUNCTION public
//	Server::Transaction::xa_end --
//		SQL文の指示に従い、トランザクションブランチを待機・中断する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID& xid
//			トランザクションブランチを待機・中断する
//			トランザクションブランチ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_end(const Trans::XID& xid,
					Trans::Branch::EndOption::Value option)
{
	// 指定されたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子を生成する

	Trans::AutoBranch branch(Trans::Branch::attach(xid));
	; _SYDNEY_ASSERT(branch.get());

	//【注意】	Branch::end でトランザクションブランチと連携している
	//			セッションと本関数を呼び出したセッションが同じであることを
	//			チェックするため、ここで操作するデータベースが
	//			同じであることを確認する必要はない

	// トランザクションブランチを待機・中断する

	branch->end(_sessionID, option);

	// 子サーバのトランザクションブランチがあれば、
	// 子サーバに対してブランチ終了を行う
	
	if (m_pXABranch)
	{
		// 子サーバのトランザクションブランチを終了する

		m_pXABranch->end(_sessionID, getDescriptor());

		// 成功したので子サーバのトランザクションブランチ記述子を破棄する

		DServer::Branch::detach(m_pXABranch);
		m_pXABranch = 0;
	}

	// トランザクションブランチを待機・中断できたので、
	// トランザクションブランチ記述子を破棄する

	; _SYDNEY_ASSERT(_branch);
	Trans::Branch::detach(_branch);

	// カスケード先のセッションを解放する

	DServer::Cascade::eraseSession(m_vecpDSession);
	m_vecpDSession.clear();
			
	// トランザクションブランチとこのオブジェクトを保持する
	// セッションとの連係がなくなったことを覚えておく

	_status = Status::Unknown;
	m_cStartTime.setNull();
}

//	FUNCTION public
//	Server::Transaction::xa_end --
//		このセッションと連係している(『データ操作中』・『中断中』の)
//		トランザクションブランチをすべて待機する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_end()
{
	if (_status == Status::Branch) {

		// 覚えているトランザクション記述子の表す
		// トランザクションブランチを待機する

		; _SYDNEY_ASSERT(_branch);
		_branch->end(_sessionID, Trans::Branch::EndOption::Unknown);

		// 子サーバのトランザクションブランチがあれば、
		// 子サーバに対してブランチ終了を行う
	
		if (m_pXABranch)
		{
			// 子サーバのトランザクションブランチを終了する

			m_pXABranch->end(_sessionID, getDescriptor());

			// 成功したので子サーバのトランザクションブランチ記述子を破棄する

			DServer::Branch::detach(m_pXABranch);
			m_pXABranch = 0;
		}

		// トランザクションブランチを待機できたので、
		// トランザクションブランチ記述子を破棄する

		Trans::Branch::detach(_branch);

		// カスケード先のセッションを解放する

		DServer::Cascade::eraseSession(m_vecpDSession);
		m_vecpDSession.clear();
			
		// トランザクションブランチとこのオブジェクトを保持する
		// セッションとの連係がなくなったことを覚えておく

		_status = Status::Unknown;
		m_cStartTime.setNull();
	}

#ifdef OBSOLETE
	
	//【注意】	中断中のトランザクションブランチはサポート外だから
	//			このコードでいいが、サポートする場合、
	//			カスケード先のセッションの扱いを変更する必要がある
	
	// このセッションで中断中のトランザクションブランチがあるか調べる

	ModVector<Trans::Branch::ID> result;
	Trans::Branch::getID(
		_sessionID, Trans::Branch::Status::Suspended, result);
	Trans::Branch::getID(
		_sessionID, Trans::Branch::Status::SuspendedForMigrate, result);

	if (result.getSize()) {

		// あれば、すべて待機する

		ModVector<Trans::Branch::ID>::Iterator ite(result.begin());
		const ModVector<Trans::Branch::ID>::Iterator& end = result.end();

		for (; ite != end; ++ite) {
			Trans::AutoBranch pAuto(Trans::Branch::attach(*ite));
			pAuto->end(_sessionID);
		}
	}
#endif
}

//	FUNCTION public
//	Server::Transaction::xa_prepare --
//		SQL文の指示に従い、トランザクションブランチのコミットを準備する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションブランチのコミットを準備する SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_prepare(const Statement::Object& stmt)
{
	const Statement::XA_PrepareStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::XA_PrepareStatement&, stmt);

	// 指定されたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子を生成する

	Trans::AutoBranch branch(
		Trans::Branch::attach(_Transaction::getXID(tmp.getIdentifier())));
	; _SYDNEY_ASSERT(branch.get());

	// 子サーバのトランザクションブランチがあれば、
	// 子サーバに対してコミット準備を行う
	
	if (!branch->getChildID().isNull() && !branch->getChildID().isIllegal())
	{
		// 子サーバのトランザクションブランチ記述子を得る
		
		DServer::AutoBranch cbranch(
			DServer::Branch::attach(branch->getChildID()));

		// 子サーバのトランザクションブランチのコミット準備をする

		cbranch->prepare(branch->getTransaction());
	}

	// トランザクションブランチのコミットを準備する

	branch->prepare();
}

//	FUNCTION public
//	Server::Transaction::xa_commit --
//		SQL文の指示に従い、トランザクションブランチをコミットする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションブランチをコミットする SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_commit(const Statement::Object& stmt)
{
	const Statement::XA_CommitStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::XA_CommitStatement&, stmt);

	// 指定されたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子を生成する

	Trans::AutoBranch branch(
		Trans::Branch::attach(_Transaction::getXID(tmp.getIdentifier())));
	; _SYDNEY_ASSERT(branch.get());

	// 子サーバのトランザクションブランチがあれば、
	// 子サーバに対してコミットを行う
	
	if (!branch->getChildID().isNull() && !branch->getChildID().isIllegal())
	{
		// 子サーバのトランザクションブランチ記述子を得る
		
		DServer::AutoBranch cbranch(
			DServer::Branch::attach(branch->getChildID()));

		// 子サーバのトランザクションブランチのコミットする

		cbranch->commit(branch->getTransaction(), tmp.getOnePhase());
	}

	// トランザクションブランチをコミットする

	branch->commit(tmp.getOnePhase());
}

//	FUNCTION public
//	Server::Transaction::xa_rollback --
//		SQL文の指示に従い、トランザクションブランチをロールバックする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	stmt
//			トランザクションブランチをロールバックする SQL 文
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_rollback(const Statement::Object& stmt)
{
	const Statement::XA_RollbackStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::XA_RollbackStatement&, stmt);

	xa_rollback(_Transaction::getXID(tmp.getIdentifier()));
}

//	FUNCTION public
//	Server::Transaction::xa_rollback --
//		SQL文の指示に従い、トランザクションブランチをロールバックする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID& xid
//			トランザクションブランチを待機・中断する
//			トランザクションブランチ識別子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_rollback(const Trans::XID& xid)
{
	// 指定されたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子を生成する

	Trans::AutoBranch branch(Trans::Branch::attach(xid));
	; _SYDNEY_ASSERT(branch.get());

	//【注意】	子サーバのトランザクションブランチのロールバックも
	//			親のトランザクションのロールバック内で実行されるので、
	//			ここで実行していはいけない

	// トランザクションブランチをロールバックする

	branch->rollback();
}

//	FUNCTION public
//	Server::Transaction::xa_recover --
//		SQL文の指示に従い、コミット準備済トランザクションブランチを確認する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	dummy
//			コミット準備済トランザクションブランチを確認する SQL 文
//		Communication::ServerConnection&	connection
//			実行結果を設定するコネクションを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::xa_recover(const Statement::Object& dummy,
						Communication::ServerConnection& connection)
{
	// コミット準備済およびヒューリスティックに解決済の
	// トランザクションブランチのトランザクションブランチ識別子をすべて求める

	ModVector<Trans::Branch::ID> xids;
	Trans::Branch::getID(Trans::Branch::Status::Prepared, xids);
	Trans::Branch::getID(Trans::Branch::Status::HeuristicallyCompleted, xids);

	// 最初に結果を表すタプルのメタデータを返す

	const ModUnicodeString tableName = _TRMEISTER_U_STRING("XA_RecoverResult");
	const Common::SQLData columnType[] =
	{
		Common::SQLData(Common::SQLData::Type::Binary,
						Common::SQLData::Flag::Variable,
						Trans::XID::GlobalTransactionID::SizeMax, 0),
		Common::SQLData(Common::SQLData::Type::Binary,
						Common::SQLData::Flag::Variable,
						Trans::XID::BranchQualifier::SizeMax, 0),
		Common::SQLData(Common::SQLData::Type::Int,
						Common::SQLData::Flag::Fixed,
						sizeof(ModInt32), 0)
	};
	const ModUnicodeString columnName[] =
	{
		_TRMEISTER_U_STRING("global_transaction_identifier"),
		_TRMEISTER_U_STRING("branch_qualifier"),
		_TRMEISTER_U_STRING("format_identifier")
	};
	const int columnCount = sizeof(columnType) / sizeof(Common::SQLData);

	Common::ResultSetMetaData meta;
	meta.reserve(columnCount);

	for (int i = 0; i < columnCount; ++i) {
		Common::ColumnMetaData tmp(columnType[i]);
		tmp.setColumnName(columnName[i]);
		tmp.setColumnAliasName(columnName[i]);
		tmp.setTableName(tableName);
		tmp.setTableAliasName(tableName);
		tmp.setNotSearchable();
		tmp.setReadOnly();

		meta.pushBack(tmp);
	}

	connection.writeObject(&meta);

	// 次に求めたトランザクションブランチ識別子を返す

	if (const int n = xids.getSize())
		for (int i = 0; i < n; ++i) {
			Common::DataArrayData tmp;
			tmp.reserve(columnCount);

			tmp.pushBack(Common::DataArrayData::Pointer(
							 &xids[i].getGlobalTransactionID()));
			tmp.pushBack(Common::DataArrayData::Pointer(
							 &xids[i].getBranchQualifier()));
			tmp.pushBack(Common::DataArrayData::Pointer(
							 new Common::IntegerData(xids[i].getFormatID())));

			connection.writeObject(&tmp);
		}

	// 最後に EOD を返す

	connection.writeObject(0);
}

//	FUNCTION public
//	Server::Transaction::xa_forget --
//		SQL文の指示に従い、
//		ヒューリスティックに解決済のトランザクションブランチを抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object&	dummy
//			ヒューリスティックに解決済の
//			トランザクションブランチを抹消する SQL 文
//
//	RETURN
//		なし
//

void
Transaction::xa_forget(const Statement::Object& stmt)
{
	const Statement::XA_ForgetStatement& tmp =
		_SYDNEY_DYNAMIC_CAST(const Statement::XA_ForgetStatement&, stmt);

	// 指定されたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子を生成する

	Trans::AutoBranch branch(
		Trans::Branch::attach(_Transaction::getXID(tmp.getIdentifier())));
	; _SYDNEY_ASSERT(branch.get());

	// ヒューリスティックに解決済のトランザクションブランチを抹消する

	branch->forget();
}

//	FUNCTION public
//	Server::Transaction::setLog --
//		トランザクションが操作するデータベースを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			このデータベースを操作するように設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Transaction::setLog(Schema::Database& database)
{
	_databaseID = database.getDatabaseID();
	getDescriptor().setLog(database);
}


//	FUNCTION public
//	Server::Transaction::getDatabaseID --
//		トランザクションが操作するデータベースのIDを取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		データベースID
//
//	EXCEPTIONS

ID
Transaction::getDatabaseID() const
{
	
	return _databaseID;
}

const ModUnicodeString Transaction::State::str[State::MAX_NUMBER]
= {"NotInProgress", "ReadOnly", "ReadWrite", "VersionUse"};

//	FUNCTION public
//	Server::Transaction::getState --
//		トランザクションの状態を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//
//	RETURN
//		State::Value          
//
//	EXCEPTIONS
const ModUnicodeString&
Transaction::getState()
{
	State::Value result = State::NotInProgress;
	
	if (isInProgress())
	{
		if (getDescriptor().getCategory()
			== Trans::Transaction::Category::ReadWrite)
		{
			result = State::ReadWrite;
		}
		else if (getDescriptor().getCategory()
				 == Trans::Transaction::Category::ReadOnly)
		{
			if (getDescriptor().isNoVersion())
			{
				result = State::ReadOnly;
			}
			else
			{
				result = State::VersionUse;
			}
		}
	}

	return State::str[result];
}

//
//	FUNCTION public
//	Server::Transaction::startXATransaction
//		-- 分散トランザクションを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Server::SQLDispatchEntry& cEntry_
//		実行の開始を宣言する SQL 文に関する取扱情報
//	const Schema::Database& cDatabase_
//		データベース
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Transaction::startXATransaction(const SQLDispatchEntry& cEntry_,
								const Schema::Database& cDatabase_)
{
	// セッションの寿命とトランザクションの寿命をそろえたので、
	// セッションが存在している場合は、トランザクションを起動すう必要はない
	
	if (m_vecpDSession.getSize() != 0)
		// すでに開始済み
		return;
	
	// カスケード先のセッションを得る
	m_vecpDSession
		= DServer::Cascade::getSession(cDatabase_, getDescriptor());

	if (_status == Status::Implicit &&
		cEntry_.isXATransactionNeeded() == false)
		// 分散トランザクションを開始する必要なし
		return;

	// 分散トランザクションを開始する必要あり

	Trans::Transaction::Mode mode;

	mode._category = getDescriptor().getCategory();
	mode._isoLevel = getDescriptor().getIsolationLevel();
	mode._snapshot = (m_bUsingSnapshot ? Boolean::True : Boolean::Unknown);

	// 分散トランザクションマネージャを得る
	
	m_pXABranch = DServer::Branch::attach();

	try
	{
		// 分散トランザクションを開始する

		m_pXABranch->start(_sessionID, getDescriptor(), mode);
	}
	catch (...)
	{
		// 分散トランザクションの開始に失敗したので、
		// そのまま detach する

		DServer::Branch::detach(m_pXABranch);
		m_pXABranch = 0;

		// カスケード先のセッションを解放する

		DServer::Cascade::eraseSession(m_vecpDSession);
		m_vecpDSession.clear();
		
		_SYDNEY_RETHROW;
	}

	if (_status == Status::Branch)
		
		// 親もトランザクションブランチの場合、
		// 子のトランザクションブランチ識別子を
		// 親のトランザクションブランチに設定する

		_branch->setChildID(m_pXABranch->getID());
}

//
//	FUNCTION public
//	Server::Transaction::isNormal
//		-- トランザクションが正常な状態か否か
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		正常な状態の場合は true 、それ以外の場合は false
//
//	EXCEPTIONS
//
bool
Transaction::isNormal() const
{
	bool r = true;
	
	if (m_pXABranch)
	{
		// 子サーバに対してトランザクションブランチを実行している場合、
		// エラーが発生したら、それ以降は rollback しか実行できない
		// そのような状態かどうか確認する
		
		r = (m_pXABranch->isErrorOccurred() ? false : true);
	}

	return r;
}

//
//	Copyright (c) 2002, 2003, 2004, 2007, 2008, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
