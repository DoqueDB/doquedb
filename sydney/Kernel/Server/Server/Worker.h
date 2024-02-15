// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Worker.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2007, 2011, 2012, 2013, 2014, 2015, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_WORKER_H
#define __SYDNEY_SERVER_WORKER_H

#include "Server/Module.h"
#include "Server/Type.h"
#include "Server/Thread.h"
#include "Server/Transaction.h"
#include "Common/Common.h"
#include "Common/DataArrayData.h"
#include "Common/Status.h"
#include "Communication/ServerConnection.h"
#include "Statement/Object.h"
#include "Schema/Database.h"
#include "Opt/Planner.h"
#include "Os/CriticalSection.h"

#include "ModVector.h"
#include "ModAutoPointer.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

class InstanceManager;
class Transaction;
class Session;
class SQLDispatchEntry;
class Manager;

//
//	CLASS
//	Server::Worker --
//
//	NOTES
//
//
class Worker : public Thread
{
public:
	//コンストラクタ
	Worker(InstanceManager& cInstanceManager_,
		   int iConnectionSlaveID_);
	//デストラクタ
	virtual ~Worker();

	//
	//	FUNCTION public
	//	Server::Worker::getID -- WorkerIDを得る
	//
	ID getID() const
	{
		return m_iWorkerID;
	}


	//
	//	FUNCTION public
	//	Server::Worker::getSessionID -- 実行中セッションのIDを得る
	//
	ID getSessionID() const
	{
		return m_iSessionID;
	}
		

	//Workerの終了を通知する
	void reportEndWorker();

	//実行を停止する
	void stop();

	// synclockを取得する
	bool trylockSync();

	// synclockを解除する
	void unlockSync();

	// データベースに対して実行可能な操作であるか確認する
	SYD_SERVER_FUNCTION
	static void isOperationApplicable(Trans::Transaction& cTransaction_,
									  const Schema::Database& cDatabase_,
									  int iStatementType_);
	SYD_SERVER_FUNCTION
	static void isOperationApplicable(Trans::Transaction& cTransaction_,
									  const Schema::Database& cDatabase_,
									  const Statement::Object* pStatement_,
									  Session* pSession_);



private:

	//スレッドとして起動されるメイン関数
	void runnable();

	//セッションを開始する
	void beginSession();
	//セッションを開始する
	void beginSession2();
	//セッションを終了する
	void endSession();
	//セッションを終了する
	void endSession2();
	//SQL文を実行する
	void executeStatement();
	//SQL文をコンパイルする
	void prepareStatement();
	//SQL文をコンパイルする
	void prepareStatement2();
	//コンパイル結果を実行する
	void executePrepare();
	
	//コンパイル結果を実行する
	void doExecutePrepare(int iStatementType_,
						  AutoSession& cSession_,
						  Opt::Planner* pPlanner_,
						  Common::DataArrayData* pParameter_,
						  int iID_,
						  const ModUnicodeString* pSQL_);
		
	
	//コンパイル結果を削除する
	void erasePrepareStatement();

	// create new user
	void createUser();
	// drop a user
	void dropUser();
	// change own password
	void changeOwnPassword();
	// change password for a user
	void changePassword();
	// レプリケーションできるか確認する
	void checkReplication();
	// スレーブに論理ログを転送する
	void transferLogicalLog();
	// スレーブへの論理ログの転送を停止する
	void stopTransferLogicalLog();
	// レプリケーションを開始する
	void startReplication();

	// query product version
	void queryProductVersion();

	// execute statement
	void executeStatement(ID iSessionID_, Statement::Object* pStatement_,
						  Common::DataArrayData* pParameter_,
						  const ModUnicodeString* pSQL_,
						  Common::DataArrayData* pCurrentParameter_);

	// トランザクション関連の SQL 文を実行する
	void
	executeTransactionStatement(ID sessionID,
								const Statement::Object& stmt,
								const ModUnicodeString* pSQL_);

	// start transaction
	void startTransaction(AutoSession& cSession_,
						  const Statement::Object& stmt);
	// xa_start
	void xa_start(AutoSession& cSession_,
				  const Statement::Object& stmt);

	//同期処理を実行する
	void sync(ID iSessionID_,
			  Statement::Object* pStatement_,
			  const ModUnicodeString* pSQL_);
	void sync(int iCount_);

	// セッションまたは、コネクションを強制終了する
	void disconnect(ID iSessionID_,
					Statement::Object* pStatement_,
					const ModUnicodeString* pSQL_);

	void disconnectSession(ID iClientID_, ID iSessionID_);

	void disconnectSession(InstanceManager& cInstanceManager_, ID iSessionID_);

	void disconnectClient(ID iClientID_);

	void declareVariable(ID iSessionID_,
						 Statement::Object* pStatement_,
						 const ModUnicodeString* pSQL_);

	//チェックポイント処理を実行する
	void checkpoint(ID iSessionID_,
					Statement::Object* pStatement_,
					const ModUnicodeString* pSQL_);
	void checkpoint(int iCount_);

	// explain one statement
	void explain(ID iSessionID_,
				 Statement::Object* pStatement_,
				 Common::DataArrayData* pParameter_,
				 const ModUnicodeString* pSQL_,
				 Common::DataArrayData* pCurrentParameter_);
	// start explain
	void startExplain(ID iSessionID_,
					  Statement::Object* pStatement_,
					  const ModUnicodeString* pSQL_);
	// end explain
	void endExplain(ID iSessionID_,
					Statement::Object* pStatement_,
					const ModUnicodeString* pSQL_);

	//その他のSQL文を実行する
	void executeOperation(ID iSessionID_,
						  Statement::Object* pStatement_,
						  Common::DataArrayData* pParameter_,
						  const ModUnicodeString* pSQL_,
						  Common::DataArrayData* pCurrentParameter_);

	//オプティマイザを実行する
	void executeOptimizer(Session* pSession_,
						  Statement::Object* pStatement_,
						  Common::DataArrayData* pParameter_,
						  const SQLDispatchEntry& cEntry_,
						  Transaction& cTransaction_);

	//クライアントに実行ステータスを送る
	void sendStatus(const Common::Status& cStatus_,
					const Exception::Object& eException_ = Exception::Object());

	//クライアントからSessionIDを受け取る
	ID recvSessionID();

	// recieve password
	void recvPassword(ModUnicodeString& cstrPassword_);

	// 論理ログを送る
	void sendLogicalLog(Trans::Log::File::Queue* pQueue_,
						Schema::ObjectID::Value uiDatabaseID_,
						const Trans::Log::Data* pData_,
						Trans::Log::LSN lsn_);

	//InstanceManager
	InstanceManager& m_cInstanceManager;
	//コネクションスレーブID
	int m_iConnectionSlaveID;

	//WorkerID
	ID m_iWorkerID;

	// SessionID
	ID m_iSessionID;

	//クライアントとのコネクション
	ModAutoPointer<Communication::ServerConnection> m_pConnection;

	//コネクション排他制御用
	Os::CriticalSection m_cCriticalSection;


	// sync排他制御用
	Os::CriticalSection m_cSyncSection;

#ifdef OBSOLETE
	//実行許可が出たかどうか
	bool m_bExecution;
#endif
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_WORKER_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2007, 2011, 2012, 2013, 2014, 2015, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
