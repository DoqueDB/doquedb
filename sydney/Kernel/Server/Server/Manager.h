// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h --
// 
// Copyright (c) 2002, 2003, 2005, 2006, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_MANAGER_H
#define __SYDNEY_SERVER_MANAGER_H

#include "Server/Module.h"
#include "Server/Type.h"
#include "Server/ThreadManager.h"
#include "Server/UserList.h"

#include "Common/Thread.h"

#include "Communication/ServerConnection.h"

#include "Os/CriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

class Connection;
class Worker;

//
//	CLASS
//	Server::Manager -- Serverモジュールのメインスレッド
//
//	NOTES
//
//
class Manager : public Common::Thread
{
public:
	//コンストラクタ
	Manager(Server::Type eType_);
	//デストラクタ
	virtual ~Manager();

	//初期化
	void initialize(bool bInstall_ = false,
					const ModUnicodeString& regPath = "");
	//後処理
	void terminate();

	//スレッドに停止を要求する
	void stop();

	//新しいコネクションを登録する
	void pushConnection(Connection* pServerConnection_);
	//コネクションが終了したことをコネクションマネージャーに知らせる
	void reportEndConnection(ID iConnectionID_);

	//新しいワーカを登録する
	void pushWorker(Worker* pWorker_);
	//ワーカを中断する
	void cancelWorker(ID iWorkerID_);
	//ワーカが終了したことをワーカマネージャに知らせる
	void reportEndWorker(ID iWorkerID_);
	// Workerを管理対象から外す
	Worker* popWorker(ID iWorkerID_);

#ifdef OBSOLETE
	//待機ワーカを登録する
	void pushWorkerQueue(Worker* pWorker_);
#endif

	//Sydney全体が利用可能かどうかを設定する
	SYD_SERVER_FUNCTION static bool setAvailability(bool bFlag_);
	//Sydney全体が利用可能か調べる
	SYD_SERVER_FUNCTION static bool isAvailable();
	//not availableのログを出したことを設定する
	SYD_SERVER_FUNCTION static bool setNotAvailableLogged(bool bFlag_);
	//not availableのログを出したか調べる
	SYD_SERVER_FUNCTION static bool isNotAvailableLogged();

	// マスターIDを得る
	static int getMasterID();

	// get user list
	static UserList* getUserList();

	// add user
	static void addUser(const ModUnicodeString& cstrUserName_,
						const ModUnicodeString& cstrPassword_,
						int iUserID_);
	// delete user
	static void deleteUser(const ModUnicodeString& cstrUserName_, int iDropBehavior_);
	// change password
	static void changePassword(const ModUnicodeString& cstrUserName_,
							   const ModUnicodeString& cstrPassword_);
	// verify password
	static UserList::Entry::Pointer verifyPassword(const ModUnicodeString& cstrUserName_,
												   const ModUnicodeString& cstrPassword_);

	// get userID from user name
	static int getUserID(const ModUnicodeString& cstrUserName_);

	// check superuser from user name
	static bool isSuperUser(const ModUnicodeString& cstrUserName_);

	// recover password file
	static void recoverPasswordFile();

private:
	//スレッドとして起動されるメソッド
	void runnable();
	//クライアントからの要求を処理する
	void loop();

	//コネクションを開始する
	void beginConnection(Communication::ServerConnection* pConnection_);

	// 認証する
	void checkUser(Communication::ServerConnection* pConnection_);

	//利用可能性をクリアする
	static void clearAvailability();

	//revoke all the privileges from all the databases for one user
	static void revokeAll(int iUserID_);

	//サーバのタイプ
	Type m_eType;

	//初期化カウンタ
	int m_iInitialized;

	//ConnectionManager
	ThreadManager m_cConnectionManager;
	//WorkerManager
	ThreadManager m_cWorkerManager;

	//Shutdown要求が来たクライアントとのコネクション
	Communication::ServerConnection* m_pShutdownConnection;

	//Sydney全体が利用可能かどうか
	static bool m_bAvailable;
	//Availabilityのログを出したか
	static bool m_bNotAvailableLogged;
	//それの排他制御用のクリティカルセクション
	static Os::CriticalSection m_cAvailabilityCriticalSection;

	//外部から実行される可能性のあるメソッドを保護するため
	Os::CriticalSection m_cLatch;
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_MANAGER_H

//
//	Copyright (c) 2002, 2003, 2005, 2006, 2007, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
