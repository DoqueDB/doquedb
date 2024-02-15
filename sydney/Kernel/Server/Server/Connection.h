// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Connection.h --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_CONNECTION_H
#define __SYDNEY_SERVER_CONNECTION_H

#include "Server/Module.h"
#include "Server/Type.h"
#include "Server/InstanceManager.h"

#include "Common/ExecutableObject.h"
#include "Common/Status.h"
#include "Communication/ServerConnection.h"
#include "Os/CriticalSection.h"
#include "Exception/Object.h"

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

class Manager;

//
//	CLASS
//	Server::Connection -- サーバコネクション
//
//	NOTES
//	クライアントとのコネクションを管理する
//
class Connection : public Thread
{
public:
	//Pointer
	typedef InstanceManagerPointer Pointer;

	//コンストラクタ(Server::Manager用)
	Connection(Manager& cServerManager_,
			   Communication::ServerConnection* pConnection_,
			   const ModUnicodeString& cstrHostName_);
	//コンストラクタ(Server::Connection用)
	Connection(Pointer pInstanceManager_,
			   Communication::ServerConnection* pConnection_);
	//デストラクタ
	virtual ~Connection();

	//
	//	FUNCTION public
	//	Server::Connection::getInstanceManager -- InstanceManagerを得る
	//
	InstanceManager& getInstanceManager()
	{
		return *m_pInstanceManager;
	}

	//
	//	FUNCTION public
	//	Server::Connection::getID -- ConnectionIDを得る
	//
	ID getID() const
	{
		return m_iConnectionID;
	}

	//
	//	FUNCTION public
	//	Server::Connection::stop -- 実行を停止する
	//
	void stop()
	{
		abort();
	}

	//クライアントに実行ステータスを送る
	static void sendStatus(Communication::ServerConnection* pConnection_,
							const Common::Status& cStatus_,
							const Exception::Object& eException_ = Exception::Object());

	// 終了処理
	void terminate();
	
private:
	
	//スレッドとして起動される
	void runnable();
	//クライアントからの要求を処理する
	void loop();

	//新しいサーバコネクションを作成する
	void beginConnection();
	//Workerを起動する
	void beginWorker();
	//Workerに中断要求を行う
	void cancelWorker();
	//最適化結果を削除する
	void erasePrepareStatement();
	//再利用しないコネクションを切断する
	void disconnectConnection();
	//利用可能性をチェックする
	void checkAvailability();

	//メインのコネクション
	Communication::ServerConnection* m_pConnection;

	//ID
	ID m_iConnectionID;

	//InstanceManager
	Pointer m_pInstanceManager;
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_CONNECTION_H

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
