// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.h --
// 
// Copyright (c) 2002, 2003, 2007, 2008, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_TRANSACTION_H
#define __SYDNEY_SERVER_TRANSACTION_H

#include "Server/Module.h"
#include "Server/Type.h"

#include "Schema/ObjectID.h"

#include "Trans/AutoTransaction.h"
#include "Trans/Branch.h"
#include "Trans/XID.h"

#include "Common/LargeVector.h"
#include "Common/DateTimeData.h"

_SYDNEY_BEGIN

namespace Communication
{
class ServerConnection;
}
namespace Schema
{
class Database;
}
namespace Statement
{
class Object;
}
namespace Trans
{
class Branch;
}
namespace DServer
{
class Branch;
class Session;
}

_SYDNEY_SERVER_BEGIN

class InstanceManager;
class Session;
class SQLDispatchEntry;

//	CLASS
//	Server::AutoSession --
//		生成中に自動的にセッションオブジェクトをロックする自動セッションクラス
//
//	NOTES
//		オブジェクトが存在する間に、同じセッションに対しアクセスされた場合、
//		Exception::SessionBusy の例外が発生する

class AutoSession
{
public:
	// コンストラクタ
	AutoSession(InstanceManager& manager, ID id, int iStatementType_);
	// デストラクタ
	~AutoSession();

	// * 前置演算子
	Session&
	operator *() const;
	// -> 演算子
	Session*
	operator ->() const;

	// 保持するセッションを表すオブジェクトを格納する領域の先頭アドレスを得る
	Session*
	get() const;

	// 保持するセッションの実行文タイプを変更する
	void changeStatementType(int iStatementType_);

private:
	// セッションオブジェクトの生成・破棄を管理するマネージャ
	InstanceManager&	_manager;
	// セッションを表すオブジェクト
	Session*			_session;
};

//	CLASS
//	Server::Transaction --
//		あるセッションで実行するトランザクションを扱うためのクラス
//
//	NOTES

class Transaction
{
public:
	// コンストラクタ
	Transaction(ID sessionID);
	// デストラクタ
	~Transaction();

	// トランザクション記述子を得る
	Trans::Transaction&
	getDescriptor();

	// SQL文の指示に従い、トランザクションを明示的に開始する
	void
	start(Schema::ObjectID::Value databaseID, const Statement::Object& stmt);
	// SQL文の指示に従い、トランザクションを明示的にコミットする
	void
	commit(const Statement::Object& dummy);
	// このセッションで開始しているトランザクションをコミットする
	void
	commit(bool bNeedReCache_ = false);

	// SQL文の指示に従い、トランザクションを明示的にロールバックする
	void
	rollback(const Statement::Object& dummy);
	// このセッションで開始しているトランザクションをロールバックする
	void
	rollback();

	// SQL文の指示に従い、トランザクションのデフォルトモードを設定する
	void
	set(const Statement::Object& stmt);

	// トランザクションが実行中か調べる
	bool
	isInProgress() const;

	// SQL文の指示に従い、トランザクションブランチを開始する
	Trans::XID
	xa_start(Schema::ObjectID::Value databaseID, const Statement::Object& stmt);

	// SQL文の指示に従い、トランザクションブランチを待機・中断する
	void
	xa_end(const Statement::Object& stmt);
	void
	xa_end(const Trans::XID& xid, Trans::Branch::EndOption::Value option);
	// このセッションと連係している(『データ操作中』・『中断中』の)
	// トランザクションブランチをすべて待機する
	void
	xa_end();

	// SQL文の指示に従い、トランザクションブランチのコミットを準備する
	void
	xa_prepare(const Statement::Object& stmt);
	// SQL文の指示に従い、トランザクションブランチをコミットする
	void
	xa_commit(const Statement::Object& stmt);
	// SQL文の指示に従い、トランザクションブランチをロールバックする
	void
	xa_rollback(const Statement::Object& stmt);
	void
	xa_rollback(const Trans::XID& xid);

	// SQL文の指示に従い、コミット準備済トランザクションブランチを確認する
	void
	xa_recover(const Statement::Object& dummy,
			   Communication::ServerConnection& connection);
	// SQL文の指示に従い、
	// ヒューリスティックに解決済のトランザクションブランチを抹消する
	void
	xa_forget(const Statement::Object& stmt);

	// このオブジェクトを保持するセッションと
	// トランザクションブランチが連携しているか
	bool
	xa_isActive() const;

	// SQL 文の実行の開始を宣言する
	void
	startStatement(Session* pSession_,
				   Communication::ServerConnection& connection,
				   const SQLDispatchEntry& entry,
				   Trans::Transaction::IsolationLevel::Value isolation);
	// SQL 文の実行が成功したことを宣言する
	void
	commitStatement(bool bNeedReCache_ = false);
	// SQL 文の実行をなかったことにする
	void
	rollbackStatement();

	// トランザクションが操作するデータベースを設定する
	void
	setLog(Schema::Database& database);

	// セッション識別子を得る
	ID
	getSessionID() const;

	// データベースIDを得る
	ID
	getDatabaseID() const;

	// 状態を取得する
	const ModUnicodeString&
	getState();

	// 開始時間を取得する
	const Common::DateTimeData& getStartTime() const
		{ return m_cStartTime; }

	// 分散トランザクションを開始する
	// ロールバック、コミットは自動的に実行される
	void startXATransaction(const SQLDispatchEntry& cEntry_,
							const Schema::Database& cDatabase_);

	// カスケード先のセッションを得る
	Common::LargeVector<DServer::Session*>& getCascadeSession()
		{ return m_vecpDSession; }

	// トランザクションを続行できるかどうか
	bool isNormal() const;

private:
	
	//	TYPEDEF
	//	Server::Transaction::Status --
	//		実行中のトランザクション・トランザクションブランチがなにかを表す
	//
	//	NOTES

	typedef Trans::Transaction::Type	Status;

	struct State
	{
		//	TYPEDEF
		//	Server::Transaction::Status::Value --
		//		トランザクションの状態を表す
		//
		//	NOTES
		
		enum Value
		{
			// 未実行
			NotInProgress = 0,
			// 参照のみのトランザクション
			ReadOnly,
			// 更新を伴うトランザクション
			ReadWrite,
			// 版を使用したトランザクション
			VersionUse,
			// 値の数
			MAX_NUMBER
		};
		static const ModUnicodeString str[State::MAX_NUMBER];
	};

	// セッション識別子
	ID								_sessionID;
	// トランザクション記述子
	Trans::AutoTransaction			_transaction;
	// セッションと連係した『データ操作中』の
	// トランザクションブランチのトランザクションブランチ記述子
	Trans::Branch*					_branch;
	// 実行中のトランザクション・トランザクションブランチがなにか
	Status::Value					_status;
	// データベースID
	int								_databaseID;
	// 開始日時
	Common::DateTimeData			m_cStartTime;

	// 必ず版管理を利用するモードか
	bool							m_bUsingSnapshot;

	// 分散トランザクション
	DServer::Branch*				m_pXABranch;

	// カスケード時の子セッション
	Common::LargeVector<DServer::Session*> m_vecpDSession;
};

//	FUNCTION public
//	Server::AutoSession::operator * -- * 前置演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		保持するセッションを表すオブジェクト
//
//	EXCEPTIONS
//		なし

inline
Session&
AutoSession::operator *() const
{
	return *get();
}

//	FUNCTION public
//	Server::AutoSession::operator -> -- -> 演算子
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		保持するセッションを表すクラスを格納する領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Session*
AutoSession::operator ->() const
{
	return get();
}

//	FUNCTION public
//	Server::AutoSession::get --
//		保持するセッションを表すクラスを格納する領域の先頭アドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた先頭アドレス
//
//	EXCEPTIONS
//		なし

inline
Session*
AutoSession::get() const
{
	return _session;
}



//	FUNCTION public
//	Server::Transaction::isInProgress --
//		トランザクションが実行中か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			実行中である
//		false
//			実行中でない
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::isInProgress() const
{
	return _status == Status::Explicit || _status == Status::Implicit;
}

//	FUNCTION public
//	Server::Transaction::xa_isActive --
//		トランザクションブランチが開始され、
//		このオブジェクトを保持するセッションと連係しているか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			実行中である
//		false
//			実行中でない
//
//	EXCEPTIONS
//		なし

inline
bool
Transaction::xa_isActive() const
{
	return _status == Status::Branch;
}

//	FUNCTION public
//	Server::Transaction::getSessionID -- セッション識別子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたセッション識別子
//
//	EXCEPTIONS
//		なし

inline
ID
Transaction::getSessionID() const
{
	return _sessionID;
}

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_TRANSACTION_H

//
//	Copyright (c) 2002, 2003, 2007, 2008, 2011, 2012, 2013, 2014, 2015, 2016, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
