// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InstanceManager.h --
// 
// Copyright (c) 2002, 2003, 2004, 2006, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_INSTANCEMANAGER_H
#define __SYDNEY_SERVER_INSTANCEMANAGER_H

#include "Server/Module.h"
#include "Server/Type.h"
#include "Server/Thread.h"
#include "Server/UserList.h"

#include "Common/DateTimeData.h"
#include "Common/SafeExecutableObject.h"
#include "Common/ObjectPointer.h"
#include "Common/VectorMap.h"
#include "Communication/ServerConnection.h"

#include "Os/CriticalSection.h"
#include "Os/Event.h"

#include "ModHasher.h"
#include "ModHashMap.h"
#include "ModMap.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
// 暗号化対応
namespace Communication
{
	class CryptKey;
}
namespace Statement
{
	class Object;
	class ExplainOption;
}
_SYDNEY_SERVER_BEGIN

class Manager;
class Connection;
class Session;
class Worker;
class InstanceManagerPointer;

//
//	CLASS
//	Server::InstanceManager -- 
//
//	NOTES
//	サーバコネクション以下のリソースを管理する
//	サーバコネクションのインスタンスは、クライアントのセッション数に応じて、
//	増減するが、このResourceManagerはそれらのサーバコネクションの
//	インスタンス間で共有される。
//	最後のサーバコネクションが終了するときに、自動的に消滅する。
//
class InstanceManager : public Common::SafeExecutableObject
{
public:
	//
	//	CLASS
	//	Server::Connection::PrepareTable
	//
	//	NOTES
	//	PrepareStatementを管理する
	//
	class PrepareTable : public Common::Object
	{
	public:
		//コンストラクタ
		PrepareTable();
		//デストラクタ
		virtual ~PrepareTable();

		//存在をチェックする
		bool checkPrepareID(int iPrepareID_);
		//最適化結果IDを追加
		void pushPrepareID(int iPrepareID_);
		//最適化結果IDを削除
		void popPrepareID(int iPrepareID_);

		//すべての最適化結果をOptimizerから削除する
		void eraseAllPrepareID();

	private:
		//PrepareIDの配列
		ModVector<int> m_veciPrepareID;
	};


	static InstanceManagerPointer create(Manager& cServerManager_,
					const ModUnicodeString& cstrHostName_,
					int iProtocolVersion_);
	//デストラクタ
	virtual ~InstanceManager();

	// 終了処理を行う
	void terminate();

	//セッション
	Session* getSession(ID iSessionID_);	//参照
	Session* lockSession(ID iSessionID_,
						 int iStatementType_);	//ロック
	void unlockSession(Session* cSession_);	//アンロック
	void pushSession(Session* pSession_);	//追加
	Session* popSession(ID iSessionID_);	//削除
	bool isLockedSession(ID iSessionID_);	//ロック中か

	// データベース名を得る
	const ModUnicodeString& getDatabaseName(ID iSessionID_);
	// Get user name with a session ID
	const ModUnicodeString& getUserName(ID iSessionID_);
	// Get session user is superuser or not
	bool isSuperUser(ID iSessionID_);

	//コネクション
	void pushConnection(
		Communication::ServerConnection* pConnection_);		//追加
	Communication::ServerConnection* popConnection(
		int iConnectionSlaveID_);	//削除

	// サーバコネクション
	void pushServerConnection(Server::Connection* pConnection_);
	void removeServerConnection(Server::Connection* pConnection_);
	void abortServerConnections();

		
	//最適化ID
	bool checkPrepareID(const ModUnicodeString& cstrDatabaseName_,
						int iPrepareID_);		//存在チェック
	void pushPrepareID(const ModUnicodeString& cstrDatabaseName_,
						int iPrepareID_);		//追加
	void popPrepareID(const ModUnicodeString& cstrDatabaseName_,
						int iPrepareID_);		//削除

	//Worker
	void pushWorker(Worker* pWorker_);		//追加
	void cancelWorker(ID iWorkerID_);		//中断

	//ワーカが終了したことをマネージャに知らせる
	void reportEndWorker(ID iWorkerID_);
#ifdef OBSOLETE
	//ワーカをキューに登録する
	void pushWorkerQueue(Worker* pWorker_);
#endif

	// Workerが終了するまで待つ
	bool waitWorker();
	
	//
	//	FUNCTION public
	//	Server::InstanceManager::getServerManager -- サーバマネージャーを得る
	//
	Manager& getServerManager()
	{
		return m_cServerManager;
	}

	// 共通鍵を取得(暗号化対応)
	const Communication::CryptKey::Pointer& getKey();
	// 共通鍵を設定(暗号化対応)
	void setKey(const Communication::CryptKey::Pointer& pKey_);

	// 以下はSystem_Session用のインターフェース

	// マップをロックする
	static void lockMap();
	// マップのロックを解除する
	static void unlockMap();
	// 取得する
	static InstanceManagerPointer lowerBound(Server::ID iID_);

	static void  terminateOtherSessionsOfAllInstanceManagers(
		const ModUnicodeString& cstrDatabaseName_, const ID iSessionID_);
	
	
	// ロックする
	void lock() { m_cLatch.lock(); }
	// ロックを解除する
	void unlock() { m_cLatch.unlock(); }
	
	// Sessionを取得する
	Session* lowerBoundSession(Server::ID iID_);

	// IDを得る
	const ID getID() const
		{ return m_iID; }
	// クライアントホスト名を得る
	const ModUnicodeString& getHostName() const
		{ return m_cstrHostName; }
	// クライアント接続時間を得る
	const Common::DateTimeData& getConnectedTime() const
		{ return m_cConnectedTime; }
	// プロトコルバージョンを得る
	int getProtocolVersion() const
		{ return m_iProtocolVersion; }
	// 暗号化モードを得る
	const ModUnicodeString* getCryptMode() const;

	// セッションを動作させているWorkerを止める
	void terminateWorkers(const ModList<Server::ID>& listSessionID_);
	
	//HashMap
	typedef Common::VectorMap<ID, InstanceManager*, ModLess<ID> > Map;
	typedef ModMap<int, Communication::ServerConnection*, ModLess<int> > ConnectionMap;
	typedef Common::VectorMap<ID, Session*, ModLess<ID> > SessionMap;
	typedef ModHashMap<ModUnicodeString, PrepareTable*, ModUnicodeStringHasher> PrepareMap;
	typedef ModHashMap<ID, Worker*, ModHasher<ID> > WorkerMap;
	typedef ModList<Server::Connection*> ServerConnectionList;

private:

	//排他用
	Os::CriticalSection m_cLatch;

	//コネクションを保持するマップ
	ConnectionMap m_mapConnection;
	//セッションを保持するマップ
	SessionMap m_mapSession;
	//最適化結果IDを保持するマップ
	PrepareMap m_mapPrepare;
	//Workerを保持するマップ
	WorkerMap m_mapWorker;
	// サーバコネクションを保持するリスト
	ServerConnectionList m_listServerConnection;

	//ID
	ID m_iID;
	//クライアントホスト名
	ModUnicodeString m_cstrHostName;
	//接続時間
	Common::DateTimeData m_cConnectedTime;
	//プロトコルパージョン
	int m_iProtocolVersion;

	//サーバマネージャー
	Manager& m_cServerManager;

	// すべてのWorkerの終了を待っているかどうか
	bool m_bWaitWorker;
	Os::Event m_cWorkerEvent;

	// 共通鍵(暗号化対応)
	Communication::CryptKey::Pointer m_pCommonKey;

	//コンストラクタ
	InstanceManager(Manager& cServerManager_,
					const ModUnicodeString& cstrHostName_,
					int iProtocolVersion_);
	
	// データベース名が一致し、セッションIDが一致しないセッションを終了する
	void terminateOtherSessions(const ModUnicodeString& cstrDatabaseName_, const Server::ID iSessionID_);

	// 終了するセッションのリストを作成する
	void createTerminateSessionList(const ModUnicodeString& cstrDatabaseName_,
									const Server::ID iSessionID_, ModList<Server::ID>& listSessionID_);



	// セッションを動作させているそれぞれのWorkerにstopを要求する
	void stopWorkers(const ModList<Server::ID>& listSessionID_, ModList<Server::Worker*>& listWorker_);

	// セッションを動作させているWorkerにstopを要求する
	Server::Worker* stopWorker(const Server::ID iSessionID_);

	// Workerをjoin()し、終了を待つ
	void joinWorkers(const ModList<Server::Worker*>& listWorker_);

	// セッションをロールバックする
	void rollbackSessions(const ModList<Server::ID>& listSessionID_);
	
};

//
//	CLASS
//	InstanceManagerPointer -- 
//
class InstanceManagerPointer
{
public:
	// コンストラクタ
	InstanceManagerPointer(InstanceManager* pObject_ = 0)
		: m_pObject(pObject_)
	{
		addReference();
	}

	// デストラクタ
	~InstanceManagerPointer()
	{
		release();
	}

	// コピーコンストラクタ
	InstanceManagerPointer(const InstanceManagerPointer& r_)
	{
		m_pObject = r_.get();
		addReference();
	}

	// 代入オペレータ
	InstanceManagerPointer& operator =(const InstanceManagerPointer& r_)
	{
		if (get() != r_.get())
		{
			//自身のオブジェクトの参照を解放する
			release();
			//代入元のオブジェクトをメンバへ代入する
			m_pObject = r_.get();
			addReference();
		}
		return *this;
	}

	// 代入オペレータ
	InstanceManagerPointer& operator =(InstanceManager* r)
	{
		if (get() != r) {
			release();

			m_pObject = r;
			addReference();
		}

		return *this;
	}

	// 参照を得る
	InstanceManager& operator* () const
	{
		return *get();
	}

	// ポインタを得る
	InstanceManager* operator-> () const
	{
		return get();
	}

	// ポインタを得る
	InstanceManager* get() const
	{
		return const_cast<InstanceManager*>(m_pObject);
	}

	// オブジェクトの参照を解放する
	void release()
	{
		if (m_pObject)
		{
			if (m_pObject->decrementReferenceCounter() == 0)
			{
				m_pObject->terminate();
				delete m_pObject;
			}
			m_pObject = 0;
		}
	}

	// オブジェクトの参照を加える
	void addReference()
	{
		if (m_pObject)
		{
			m_pObject->incrementReferenceCounter();
		}
	}

private:
	//オブジェクトへのポインタ
	InstanceManager* m_pObject;
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_INSTANCEMANAGER_H

//
//	Copyright (c) 2002, 2003, 2004, 2006, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
