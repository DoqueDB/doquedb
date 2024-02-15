// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Branch.cpp -- 
// 
// Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "DServer";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"
#include "DServer/Branch.h"

#include "Checkpoint/Database.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Configuration.h"
#include "Common/DoubleLinkedList.h"
#include "Common/HashTable.h"
#include "Common/Message.h"

#include "DServer/AutoBranch.h"
#include "DServer/Cascade.h"
#include "DServer/OpenMPExecutor.h"
#include "DServer/ResultSet.h"
#include "DServer/Session.h"

#include "Exception/AlreadyBeginTransaction.h"
#include "Exception/BadArgument.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/ModLibraryError.h"
#include "Exception/NotSupported.h"
#include "Exception/XA_InvalidIdentifier.h"

#include "Os/AutoCriticalSection.h"

#include "Server/Session.h"

#include "Schema/Database.h"
#include "Schema/ObjectID.h"

#include "Trans/AutoLatch.h"
#include "Trans/TimeStamp.h"
#include "Trans/LogData.h"
#include "Trans/LogInfo.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_DSERVER_USING

namespace {

	typedef	Common::HashTable<
		Common::DoubleLinkedList<Branch>, Branch>	HashTable;

	namespace _Configuration
	{
		Common::Configuration::ParameterInteger
		_tableSize("DServer_BranchTableSize", 97, false);
	}

	namespace _Branch
	{
		// 生成済のトランザクションブランチ記述子を探す
		Branch*
		find(HashTable::Bucket& bucket, const Trans::XID& id);

		// すべてのトランザクションブランチ記述子を管理する
		// ハッシュ表に登録するためのハッシュ値を計算する
		unsigned int
		branchTableHash(const Trans::XID& id);

		// 以下の情報を保護するためのラッチ
		Os::CriticalSection		_latch;

		// すべてのトランザクションブランチ記述子を管理するハッシュ表
		HashTable*				_branchTable = 0;
	}

	namespace _Impl
	{
		// メンバー関数とstatic関数から呼び出されるので、
		// 実装はここで行う
		
		// 子サーバのトランザクションブランチを開始する
		void _XAStart(const Trans::XID& cXId_,
					  Common::LargeVector<DServer::Session*>& vecSession_,
					  const Trans::Transaction::Mode& cMode_,
					  Schema::ObjectID::Value databaseID_);
		
		// 子サーバのトランザクションブランチを終了する
		void _XAEnd(const Trans::XID& cXId_,
					Common::LargeVector<DServer::Session*>& vecSession_);

		// 子サーバのトランザクションブランチのコミット準備をする
		void _XAPrepare(const Trans::XID& cXId_,
						Common::LargeVector<DServer::Session*>& vecSession_);
		
		// 子サーバのトランザクションブランチをコミットする
		void _XACommit(const Trans::XID& cXId_,
					   Common::LargeVector<DServer::Session*>& vecSession_);
		
		// 子サーバのトランザクションブランチをロールバックする
		void _XARollback(const Trans::XID& cXId_,
						 Common::LargeVector<DServer::Session*>& vecSession_);
					 
	}
}

//
//	FUNCTION local
//	_$$::_Branch::find -- 生成済のトランザクションブランチ記述子を探す
//
//	NOTES
//
//	ARGUMENTS
//		HashTable::Bucket&	bucket
//			トランザクションブランチ記述子が格納されるべきハッシュ表のバケット
//		Trans::XID&	xid
//			トランザクションブランチ識別子
//
//	RETURN
//		0 以外の値
//			得られたトランザクションブランチ記述子を格納する領域の先頭アドレス
//		0
//			見つからなかった
//
//	EXCEPTIONS
//		なし
//
Branch*
_Branch::find(HashTable::Bucket& bucket, const Trans::XID& xid)
{
	switch (bucket.getSize()) {
	default:
	{
		// バケットに登録されているトランザクションブランチ記述子のうち、
		// 与えられたトランザクションブランチ識別子のものを探す

		HashTable::Bucket::Iterator			begin(bucket.begin());
		HashTable::Bucket::Iterator			ite(begin);
		const HashTable::Bucket::Iterator&	end = bucket.end();

		do {
			Branch& branch = *ite;

			if (branch.getID() == xid) {

				// 見つかったトランザクションブランチ記述子を
				// バケットの先頭に移動して、
				// 最近に参照されたものほど、見つかりやすくする

				bucket.splice(begin, bucket, ite);

				return &branch;
			}
		} while (++ite != end) ;

		break;
	}
	case 1:
	{
		Branch& branch = bucket.getFront();

		if (branch.getID() == xid)

			// 見つかった

			return &branch;

		break;
	}
	case 0:
		break;
	}

	// 見つからなかった

	return 0;
}

//
//	FUNCTION local
//	_$$::_Branch::branchTableHash --
//		すべてのトランザクションブランチ記述子を管理するハッシュ表に
//		登録するためのハッシュ値を計算する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::XID&	id
//			ハッシュ表に登録するトランザクションブランチ記述子の
//			トランザクションブランチ識別子
//
//	RETURN
//		得られたハッシュ値
//
//	EXCEPTIONS
//		なし

unsigned int
_Branch::branchTableHash(const Trans::XID& id)
{
	; _SYDNEY_ASSERT(!(id.isNull() || id.isIllegal()));

	const Common::BinaryData& gtrID = id.getGlobalTransactionID();

	const unsigned char* p =
		syd_reinterpret_cast<const unsigned char*>(gtrID.getValue());
	const unsigned char* q = p + gtrID.getSize();

	unsigned int i = 0;
	for (; p < q; ++p)
		i += static_cast<unsigned int>(*p);

	return i;
}

//
//	FUNCTION local
//	_$$::_Impl::_XAStart
//		-- 子サーバのトランザクションブランチを開始する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::XID& cXid_
//		トランザクションブランチ識別子
//	Common::LargeVector<DServer::Session*>& vecSession_
//		子サーバのセッション
//	const Trans::Transaction::Mode& cMode_
//	   	トランザクションモード
//	Schema::ObjectID::Value databaseID_
//		データベースID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
_Impl::_XAStart(const Trans::XID& cXid_,
				Common::LargeVector<DServer::Session*>& vecSession_,
				const Trans::Transaction::Mode& cMode_,
				Schema::ObjectID::Value databaseID_)
{
	ModUnicodeOstrStream s;	// SQL文
	
	s << "xa start X'"
	  << cXid_.getGlobalTransactionID().encodeString()
	  << "'";
	
	bool prev = false;

	if (cMode_._category != Trans::Transaction::Category::Unknown)
	{
		switch (cMode_._category)
		{
		case Trans::Transaction::Category::ReadWrite:
			s << " read write";
			break;
		case Trans::Transaction::Category::ReadOnly:
			s << " read only";
			break;
		}
		
		prev = true;
	}

	if (cMode_._isoLevel != Trans::Transaction::IsolationLevel::Unknown)
	{
		if (prev) s << ",";
		
		switch (cMode_._isoLevel)
		{
		case Trans::Transaction::IsolationLevel::ReadUncommitted:
			s << " isolation level read uncommitted";
			break;
		case Trans::Transaction::IsolationLevel::ReadCommitted:
			s << " isolation level read committed";
			break;
		case Trans::Transaction::IsolationLevel::RepeatableRead:
			s << " isolation level repeatble read";
			break;
		case Trans::Transaction::IsolationLevel::Serializable:
			s << " isolation level serializable";
			break;
		}

		prev = true;
	}

	if (cMode_._snapshot == Boolean::True)
	{
		if (prev) s << ",";
		s << " using snapshot";
	}

	// 子サーバにSQL文を投げる

	ModUnicodeString sql(s.getString());
	Common::LargeVector<OpenMPExecutor::ExecuteStatus::Value> vecStatus;
	OpenMPExecutor executor(vecSession_, vecStatus, sql, true /*force*/);

	try
	{
		executor.run();
	}
	catch (...)
	{
		// 成功した分散トランザクションをロールバックする

		// SQL文を作成する
		
		s.clear();
		s << "xa end X'"
		  << cXid_.getGlobalTransactionID().encodeString()
		  << "'";
		ModUnicodeString xaend(s.getString());

		s.clear();
		s << "xa rollback X'"
		  << cXid_.getGlobalTransactionID().encodeString()
		  << "'";
		ModUnicodeString xarollback(s.getString());
		
		Common::LargeVector<DServer::Session*>::Iterator s
			= vecSession_.begin();
		Common::LargeVector<OpenMPExecutor::ExecuteStatus::Value>::Iterator i
			= vecStatus.begin();

		for (; i != vecStatus.end(); ++i, ++s)
		{
			if ((*i) == OpenMPExecutor::ExecuteStatus::Succeeded)
			{
				// 成功したトランザクションをロールバックする

				bool succeeded = false;

				try
				{
					ModAutoPointer<ResultSet> r
						= (*s)->executeStatement(xaend);
					r->getStatus();
					succeeded = true;
				}
				catch (Exception::Object& e)
				{
					SydErrorMessage << e << ModEndl;
				}
				catch (ModException& e)
				{
					SydErrorMessage
						<< Exception::ModLibraryError(
							moduleName, srcFile, __LINE__, e)
						<< ModEndl;
				}
				catch (...)
				{
					SydErrorMessage << "Unexpected Exception" << ModEndl;
				}

				try
				{
					if (succeeded)
					{
						succeeded = false;
						
						ModAutoPointer<ResultSet> r
							= (*s)->executeStatement(xarollback);
						r->getStatus();

						succeeded = true;
					}
				}
				catch (Exception::Object& e)
				{
					SydErrorMessage << e << ModEndl;
				}
				catch (ModException& e)
				{
					SydErrorMessage
						<< Exception::ModLibraryError(
							moduleName, srcFile, __LINE__, e)
						<< ModEndl;
				}
				catch (...)
				{
					SydErrorMessage << "Unexpected Exception" << ModEndl;
				}

				if (succeeded == false)
				{
					// 成功していないので、データベースを利用不可にする

					(void) Checkpoint::Database::setAvailability(
						databaseID_, false);
				}
			}
		}

		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION local
//	_$$::_Impl::_XAEnd
//		-- 子サーバのトランザクションブランチを終了する
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::XID& cXid_
//		トランザクションブランチ識別子
//	Common::LargeVector<DServer::Session*>& vecSession_
//		子サーバのセッション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
_Impl::_XAEnd(const Trans::XID& cXid_,
			  Common::LargeVector<DServer::Session*>& vecSession_)
{
	ModUnicodeOstrStream s;	// SQL文

	s << "xa end X'"
	  << cXid_.getGlobalTransactionID().encodeString()
	  << "'";

	// 子サーバにSQL文を投げる

	ModUnicodeString sql(s.getString());
	Common::LargeVector<OpenMPExecutor::ExecuteStatus::Value> vecStatus;
	OpenMPExecutor executor(vecSession_, vecStatus, sql, true /*force*/);

	try
	{
		executor.run();
	}
	catch (...)
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION local
//	_$$::_Impl::_XAPrepare
//		-- 子サーバのトランザクションブランチのコミット準備を行う
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::XID& cXid_
//		トランザクションブランチ識別子
//	Common::LargeVector<DServer::Session*>& vecSession_
//		子サーバのセッション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
_Impl::_XAPrepare(const Trans::XID& cXid_,
				  Common::LargeVector<DServer::Session*>& vecSession_)
{
	ModUnicodeOstrStream s;	// SQL文

	s << "xa prepare X'"
	  << cXid_.getGlobalTransactionID().encodeString()
	  << "'";

	// 子サーバにSQL文を投げる

	ModUnicodeString sql(s.getString());
	Common::LargeVector<OpenMPExecutor::ExecuteStatus::Value> vecStatus;
	OpenMPExecutor executor(vecSession_, vecStatus, sql, true /*force*/);

	try
	{
		executor.run();
	}
	catch (...)
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION local
//	_$$::_Impl::_XACommit
//		-- 子サーバのトランザクションブランチをコミットする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::XID& cXid_
//		トランザクションブランチ識別子
//	Common::LargeVector<DServer::Session*>& vecSession_
//		子サーバのセッション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
_Impl::_XACommit(const Trans::XID& cXid_,
				 Common::LargeVector<DServer::Session*>& vecSession_)
{
	ModUnicodeOstrStream s;	// SQL文

	s << "xa commit X'"
	  << cXid_.getGlobalTransactionID().encodeString()
	  << "'";

	// 子サーバにSQL文を投げる

	ModUnicodeString sql(s.getString());
	Common::LargeVector<OpenMPExecutor::ExecuteStatus::Value> vecStatus;
	OpenMPExecutor executor(vecSession_, vecStatus, sql, true /*force*/);

	try
	{
		executor.run();
	}
	catch (...)
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION local
//	_$$::_Impl::_XARollback
//		-- 子サーバのトランザクションブランチをロールバックする
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::XID& cXid_
//		トランザクションブランチ識別子
//	Common::LargeVector<DServer::Session*>& vecSession_
//		子サーバのセッション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
_Impl::_XARollback(const Trans::XID& cXid_,
				   Common::LargeVector<DServer::Session*>& vecSession_)
{
	ModUnicodeOstrStream s;	// SQL文

	s << "xa rollback X'"
	  << cXid_.getGlobalTransactionID().encodeString()
	  << "'";

	// 子サーバにSQL文を投げる

	ModUnicodeString sql(s.getString());
	Common::LargeVector<OpenMPExecutor::ExecuteStatus::Value> vecStatus;
	OpenMPExecutor executor(vecSession_, vecStatus, sql, true /*force*/);

	try
	{
		executor.run();
	}
	catch (...)
	{
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION public
//	Trans::Branch::attach -- トランザクションブランチ記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//	Trans::XID& id
//		生成するトランザクションブランチ記述子の表す
//		トランザクションブランチのトランザクションブランチ識別子
//
//	RETURN
//	得られたトランザクションブランチ記述子を格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Branch*
Branch::attach()
{
	// トランザクションブランチ識別子を生成する
	// タイムスタンプ値を利用する

	Trans::TimeStamp::Value t
		= static_cast<Trans::TimeStamp::Value>(Trans::TimeStamp::assign());
	Common::BinaryData d(static_cast<const void*>(&t),
						 sizeof(Trans::TimeStamp::Value));
	Trans::XID xid(d);

	return attach(xid);
}

// static
Branch*
Branch::attach(const Trans::XID& id)
{
	if (id.isNull() || id.isIllegal())

		// 不正なトランザクションブランチ識別子が与えられた

		_SYDNEY_THROW0(Exception::XA_InvalidIdentifier);

	// トランザクションブランチ記述子の生成・破棄に関する情報を
	// 保護するためのラッチをかける

	Os::AutoCriticalSection latch(_Branch::_latch);

	// 与えられたトランザクションブランチ識別子の
	// トランザクションブランチ記述子を
	// 格納すべきハッシュ表のバケットを求める

	; _SYDNEY_ASSERT(_Branch::_branchTable);

	const unsigned int addr =
		_Branch::branchTableHash(id) % _Branch::_branchTable->getLength();
	HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(addr);

	// 与えられたトランザクションブランチ識別子の表す
	// トランザクションブランチのトランザクションブランチ記述子が
	// 求めたバケットに登録されていれば、それを得る

	Branch* branch = _Branch::find(bucket, id);
	if (branch)

		// 見つかったので、参照回数を 1 増やす

		++branch->m_iRefCount;
	else {

		// 見つからなかったので、生成する

		branch = new Branch(id);
		; _SYDNEY_ASSERT(branch);

		// 参照回数を 1 にする

		branch->m_iRefCount = 1;

		// ハッシュ表のバケットの先頭に挿入して、
		// 最近に参照されたものほど、見つかりやすくする

		bucket.pushFront(*branch);
	}

	return branch;
}

//
//	FUNCTION public
//	Trans::Branch::detach -- トランザクションブランチ記述子の参照をやめる
//
//	NOTES
//	トランザクションブランチ記述子の参照をやめても、
//	他のどこかで参照されているか、トランザクションブランチが
//	存在しない状態でなければ、トランザクションブランチ記述子は破棄されない
//	逆にどこからも参照されておらず、トランザクションブランチが
//	存在しない状態であれば、トランザクションブランチ記述子は直ちに破棄される
//
//	ARGUMENTS
//	Trans::Branch*		branch
//		参照をやめるトランザクションブランチ記述子を
//		格納する領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Branch::detach(Branch* branch)
{
	if (branch)
	{
		Branch* p = 0;

		// トランザクションブランチ記述子の生成・破棄に関する情報を
		// 保護するためのラッチをかける

		Os::AutoCriticalSection latch(_Branch::_latch);

		; _SYDNEY_ASSERT(branch->m_iRefCount);

		// 参照回数を 1 減らす

		if (!--branch->m_iRefCount) {

			// どこからも参照されなくなる

			Os::AutoCriticalSection latch(branch->m_cLatch);

			if (branch->m_eStatus == Status::NonExistent) {

				// 与えられたトランザクションブランチ記述子を格納する
				// ハッシュ表のバケットを求め、
				// トランザクションブランチ記述子を取り除く

				; _SYDNEY_ASSERT(_Branch::_branchTable);

				const unsigned int addr =
					_Branch::branchTableHash(branch->getID()) %
						_Branch::_branchTable->getLength();
				_Branch::_branchTable->getBucket(addr).erase(*branch);

				// アンラッチした後、破棄するようにする

				p = branch;
			}
		}

		if (p)

			// トランザクションブランチ記述子を破棄する

			delete p;

	}
}

//
//	FUNCTION public
//	DServer::Branch::start
//		-- 子サーバのトランザクションブランチを開始する
//
//	NOTES
//
//	ARGUMENTS
//	Server::SessionID uiSessionID_
//		セッションID
//	Trans::Transaction& cTrans_
//		トランザクション
//	const Trans::Transaction::Mode& cMode_
//		トランザクションモード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::start(Server::SessionID uiSessionID_,
			  Trans::Transaction& cTrans_,
			  const Trans::Transaction::Mode& cMode_)
{
	Os::AutoCriticalSection latch(m_cLatch);

	// 論理ログファイル関連情報クラスを得る
	
	const Trans::Log::Info& logInfo
		= cTrans_.getLogInfo(Trans::Log::File::Category::Database);

	// トランザクションの開始を表す論理ログの LSN を確認し、
	// トランザクション開始から何もログが書かれていないことを確認する

	if (logInfo.getBeginTransactionLSN() != Trans::Log::IllegalLSN)

		// すでに何か書かれているので、
		// 子サーバで分散トランザクションは開始できない

		_SYDNEY_THROW0(Exception::AlreadyBeginTransaction);

	// 子サーバでトランザクションブランチを開始するには、
	// 親側に対応するセッションが必要

	if (uiSessionID_ == Server::IllegalSessionID)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);

	// 子サーバのセッションを得る
	
	Server::Session* pSession
		= Server::Session::getSession(uiSessionID_);
	
	if (pSession == 0)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	
	Common::LargeVector<DServer::Session*>& vecSession
		= pSession->getCascadeSession();
	
	// 子サーバでトランザクションブランチを開始する
	
	_Impl::_XAStart(m_cXid, vecSession, cMode_, logInfo.getDatabaseID());

	try
	{
		// 成功したので、ログを書く
		
		Trans::Log::XATransactionData log(m_cXid);
		
		Trans::AutoLatch latch(cTrans_, logInfo.getLockName());
		cTrans_.storeLog(Trans::Log::File::Category::Database, log);
	}
	catch (...)
	{
		// データベースを利用不可にする

		(void) Checkpoint::Database::setAvailability(
			logInfo.getDatabaseID(), false);
		
		_SYDNEY_RETHROW;
	}

	// 状態

	m_eStatus = Status::Active;
}

//
//	FUNCTION public
//	DServer::Branch::end
//		-- 子サーバのトランザクションブランチを終了する
//
//	NOTES
//
//	ARGUMENTS
//	Server::SessionID uiSessionID_
//		セッションID
//	Trans::Transaction& cTrans_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::end(Server::SessionID uiSessionID_, Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection latch(m_cLatch);

	// 状態を確認する

	if (m_eStatus != Status::Active)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);

	// 子サーバでトランザクションブランチを終了するには、
	// 親側に対応するセッションが必要

	if (uiSessionID_ == Server::IllegalSessionID)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);

	// 子サーバのセッションを得る
	
	Server::Session* pSession
		= Server::Session::getSession(uiSessionID_);
	
	if (pSession == 0)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	
	Common::LargeVector<DServer::Session*>& vecSession
		= pSession->getCascadeSession();

	const Trans::Log::Info& logInfo
		= cTrans_.getLogInfo(Trans::Log::File::Category::Database);
	
	try
	{
		// 子サーバでトランザクションブランチを終了する
	
		_Impl::_XAEnd(m_cXid, vecSession);
	}
	catch (...)
	{
		// データベースを利用不可にする

		(void) Checkpoint::Database::setAvailability(
			logInfo.getDatabaseID(), false);
		
		_SYDNEY_RETHROW;
	}

	// 状態

	m_eStatus = Status::Idle;
}
	
//
//	FUNCTION public
//	DServer::Branch::commitOnePhase
//		-- 子サーバのトランザクションブランチを一気にコミットする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::commitOnePhase(Server::SessionID uiSessionID_,
					   Trans::Transaction& cTrans_)
{
	//
	//	子サーバのトランザクションブランチに対して、
	//	end -> prepare -> commit を一気に実行する
	//
	//	親サーバ側のトランザクションがが通常のトランザクションの場合に
	//	利用されることを想定している
	//
	
	Os::AutoCriticalSection latch(m_cLatch);

	// エラーが発生していないか？

	if (m_bErrorOccurred)
		// コミットできない
		_SYDNEY_THROW0(Exception::NotSupported);

	// 状態を確認する

	if (m_eStatus != Status::Active)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);

	// 子サーバでトランザクションブランチを終了するには、
	// 親側に対応するセッションが必要

	if (uiSessionID_ == Server::IllegalSessionID)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);

	// 子サーバのセッションを得る
	
	Server::Session* pSession
		= Server::Session::getSession(uiSessionID_);
	
	if (pSession == 0)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	
	Common::LargeVector<DServer::Session*>& vecSession
		= pSession->getCascadeSession();

	const Trans::Log::Info& logInfo
		= cTrans_.getLogInfo(Trans::Log::File::Category::Database);
	
	try
	{
		// 子サーバでトランザクションブランチを終了する
	
		_Impl::_XAEnd(m_cXid, vecSession);
	}
	catch (...)
	{
		// データベースを利用不可にする

		(void) Checkpoint::Database::setAvailability(
			logInfo.getDatabaseID(), false);
		
		_SYDNEY_RETHROW;
	}

	// 状態

	m_eStatus = Status::Idle;

	try
	{
		// 子サーバでトランザクションブランチをコミット準備する
	
		_Impl::_XAPrepare(m_cXid, vecSession);
	}
	catch (...)
	{
		_SYDNEY_RETHROW;
	}

	// 状態

	m_eStatus = Status::Prepared;
	
	try
	{
		// 子サーバでトランザクションブランチをコミットする
	
		_Impl::_XACommit(m_cXid, vecSession);
	}
	catch (...)
	{
		// データベースを利用不可にする

		(void) Checkpoint::Database::setAvailability(
			logInfo.getDatabaseID(), false);
		
		_SYDNEY_RETHROW;
	}

	// 状態

	m_eStatus = Status::NonExistent;
}
	
//
//	FUNCTION public
//	DServer::Branch::prepare
//		-- 子サーバのトランザクションブランチをコミット準備する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::prepare(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection latch(m_cLatch);

	// 状態を確認する

	// エラーが発生していないか？

	if (m_bErrorOccurred)
		// コミットできない
		_SYDNEY_THROW0(Exception::NotSupported);
	
	if (m_eStatus != Status::Idle)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);

	const Trans::Log::Info& logInfo
		= cTrans_.getLogInfo(Trans::Log::File::Category::Database);

	// 操作の対象であるデータベースのスキーマ情報を取得する

	Schema::Database* pDatabase
		= Schema::Database::get(logInfo.getDatabaseID(), cTrans_);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	pDatabase->open();
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	// 子サーバのセッションを得る

	Common::LargeVector<Session*> vecSession
		= Cascade::getSession(*pDatabase, cTrans_);

	try
	{
		// 子サーバのトランザクションブランチをコミット準備する
	
		_Impl::_XAPrepare(m_cXid, vecSession);
	}
	catch (...)
	{
		// 子サーバのセッションを破棄する
		Cascade::eraseSession(vecSession);
		
		_SYDNEY_RETHROW;
	}

	// 子サーバのセッションを破棄する
	Cascade::eraseSession(vecSession);
		
	m_eStatus = Status::Prepared;	// 状態
}

//
//	FUNCTION public
//	DServer::Branch::commit
//		-- 子サーバのトランザクションブランチをコミットする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		トランザクション
//	bool isOnePhase_ (default false)
//		トランザクションブランチが終了した状態のものをコミットするかどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::commit(Trans::Transaction& cTrans_,
			   bool isOnePhase_)
{
	Os::AutoCriticalSection latch(m_cLatch);

	// エラーが発生していないか？

	if (m_bErrorOccurred)
		// コミットできない
		_SYDNEY_THROW0(Exception::NotSupported);
	
	// 状態を確認する

	switch (m_eStatus)
	{
	case Status::Prepared:
		break;
	case Status::Idle:
		if (isOnePhase_)
			break;
	default:
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);
	}

	const Trans::Log::Info& logInfo
		= cTrans_.getLogInfo(Trans::Log::File::Category::Database);

	// 操作の対象であるデータベースのスキーマ情報を取得する

	Schema::Database* pDatabase
		= Schema::Database::get(logInfo.getDatabaseID(), cTrans_);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	pDatabase->open();
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	// 子サーバのセッションを得る

	Common::LargeVector<Session*> vecSession
		= Cascade::getSession(*pDatabase, cTrans_);

	if (m_eStatus == Status::Idle)
	{
		// トランザクションブランチ終了状態のものをコミットするが、
		// 子サーバには二相コミットする

		_Impl::_XAPrepare(m_cXid, vecSession);

		m_eStatus = Status::Prepared;
	}
	
	try
	{
		// 子サーバのトランザクションブランチをコミットする
	
		_Impl::_XACommit(m_cXid, vecSession);
	}
	catch (...)
	{
		// 子サーバのセッションを破棄する
		Cascade::eraseSession(vecSession);
		
		// データベースを利用不可にする

		(void) Checkpoint::Database::setAvailability(
			logInfo.getDatabaseID(), false);
		
		_SYDNEY_RETHROW;
	}

	// 子サーバのセッションを破棄する
	Cascade::eraseSession(vecSession);
		
	m_eStatus = Status::NonExistent;	// 状態
}

//
//	FUNCTION public
//	DServer::Branch::rollback
//		-- 子サーバのトランザクションブランチをロールバックする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::rollback(Trans::Transaction& cTrans_)
{
	Os::AutoCriticalSection latch(m_cLatch);

	// 状態を確認する

	if (m_eStatus != Status::Idle && m_eStatus != Status::Prepared)
		// 通常ありえない
		_SYDNEY_THROW0(Exception::BadArgument);

	const Trans::Log::Info& logInfo
		= cTrans_.getLogInfo(Trans::Log::File::Category::Database);

	// 操作の対象であるデータベースのスキーマ情報を取得する

	Schema::Database* pDatabase
		= Schema::Database::get(logInfo.getDatabaseID(), cTrans_);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	pDatabase->open();
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	// 子サーバのセッションを得る

	Common::LargeVector<Session*> vecSession
		= Cascade::getSession(*pDatabase, cTrans_);

	try
	{
		// 子サーバのトランザクションブランチをロールバックする
	
		_Impl::_XARollback(m_cXid, vecSession);
	}
	catch (...)
	{
		// 子サーバのセッションを破棄する
		Cascade::eraseSession(vecSession);
		
		// データベースを利用不可にする

		(void) Checkpoint::Database::setAvailability(
			logInfo.getDatabaseID(), false);
		
		_SYDNEY_RETHROW;
	}

	// 子サーバのセッションを破棄する
	Cascade::eraseSession(vecSession);
		
	m_eStatus = Status::NonExistent;	// 状態
}

//
//	FUNCTION public static
//	DServer::Branch::rollback
//		-- トランザクションのロールバック時のUNDO処理時のロールバック
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		トランザクション
//	const Trans::Log::Data& cData_
//		ログデータ
//	const ModUnicodeString& cstrDatabase_
//		データベース名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::rollback(Trans::Transaction& cTrans_,
				 const Trans::Log::Data& cData_,
				 const ModUnicodeString& cstrDatabase_)
{
	// UNDO する操作の対象であるデータベースのスキーマ情報を取得する

	Schema::Database* pDatabase
		= Schema::Database::get(
			Schema::Database::getID(cstrDatabase_, cTrans_), cTrans_);
	; _SYDNEY_ASSERT(pDatabase);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	pDatabase->open();
	Common::AutoCaller1<Schema::Database, bool>
		autoCloser(pDatabase, &Schema::Database::close, false);

	rollback(cTrans_, cData_, *pDatabase);
}

//
//	FUNCTION public static
//	DServer::Branch::rollback
//		-- 障害回復時のUNDO・REDO処理時のロールバック
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		トランザクション
//	const Trans::Log::Data& cData_
//		ログデータ
//	Schema::Database& cDatabase_
//		データベース
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::rollback(Trans::Transaction& cTrans_,
				 const Trans::Log::Data& cData_,
				 Schema::Database& cDatabase_)
{
	// 必要な情報を得るためにサブクラスにキャストする
	
	if (cData_.getCategory() != Trans::Log::Data::Category::XATransaction)
		_SYDNEY_THROW0(Exception::LogItemCorrupted);

	const Trans::Log::XATransactionData& cData
		= _SYDNEY_DYNAMIC_CAST(const Trans::Log::XATransactionData&, cData_);
	
	// トランザクションブランチ記述子を得る
	
	AutoBranch branch(Branch::attach(cData.getXID()));

	// 状態を確認する
	// 自動リカバリで呼ばれる場合、状態が Status::NonExistent となるので、
	// ここで確認し、Status::NonExistent なら Stauts::Idle にする

	if (branch->m_eStatus == Status::NonExistent)
		branch->m_eStatus = Status::Idle;

	// 子サーバのセッションを得る

	Common::LargeVector<DServer::Session*> vecSession
		= Cascade::getSession(cDatabase_, cTrans_);

	try
	{
		// 子サーバのトランザクションブランチをロールバックする

		_Impl::_XARollback(cData.getXID(), vecSession);
	}
	catch (...)
	{
		// 子サーバのセッションを解放する

		Cascade::eraseSession(vecSession);
		
		// 必要なら、呼び出し側でデータベースを利用不可にするので、
		// ここでは再送するのみ
		
		_SYDNEY_RETHROW;
	}

	// 子サーバのセッションを解放する

	Cascade::eraseSession(vecSession);
}

//
//	FUNCTION private
//	DServer::Branch::Branch -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const Trans::XID& cXid_
//		トランザクションブランチ識別子
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Branch::Branch(const Trans::XID& cXid_)
	: m_cXid(cXid_), m_eStatus(Status::NonExistent), m_iRefCount(0),
	  m_bErrorOccurred(false)
{
}

//
//	FUNCTION private
//	DServer::Branch::~Branch -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Branch::~Branch()
{
}

//
//	FUNCTION private static
//	DServer::Branch::initialize -- 初期化
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::initialize()
{
	try
	{
		// すべてのトランザクションブランチ記述子を管理するハッシュ表を生成する

		_Branch::_branchTable =
			new HashTable(_Configuration::_tableSize.get(),
						  &Branch::m_pPrev,
						  &Branch::m_pNext);
		; _SYDNEY_ASSERT(_Branch::_branchTable);
	}
	catch (...)
	{
		Branch::terminate();
		_SYDNEY_RETHROW;
	}
}

//
//	FUNCTION private static
//	DServer::Branch::terminate -- 後処理
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Branch::terminate()
{
	//【注意】	Trans::Branch::prepareTerminate で DServer::Branch もすべて
	//			ロールバックされるので、トランザクションブランチ記述子は
	//			存在しないはす

	if (_Branch::_branchTable) {

		// すべてのトランザクションブランチ記述子を管理するハッシュ表に
		// 登録されているトランザクションブランチ記述子があれば、破棄する

		unsigned int i = 0;
		do {
			HashTable::Bucket& bucket = _Branch::_branchTable->getBucket(i);

			while (bucket.getSize()) {
				Branch* branch = &bucket.getFront();
				bucket.popFront();
				delete branch;
			}
		} while (++i < _Branch::_branchTable->getLength()) ;

		// すべてのトランザクションブランチ記述子を管理するハッシュ表を破棄する

		delete _Branch::_branchTable, _Branch::_branchTable = 0;
	}
}

//
// Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
