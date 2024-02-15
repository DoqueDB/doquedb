// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InstanceManager.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Server/InstanceManager.h"
#include "Server/Manager.h"
#include "Server/Session.h"
#include "Server/UserList.h"
#include "Server/Worker.h"
#include "Server/FakeError.h"
#include "Server/Connection.h"

#include "Checkpoint/Daemon.h"
#include "Checkpoint/Database.h"

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/StringData.h"
#include "Communication/Crypt.h"
#include "Exception/ConnectionRanOut.h"
#include "Exception/ConnectionNotExist.h"
#include "Exception/SessionBusy.h"
#include "Exception/SessionNotAvailable.h"
#include "Exception/SessionNotExist.h"
#include "Exception/BadArgument.h"
#include "Opt/Optimizer.h"

#include "Os/AutoCriticalSection.h"
#include "Statement/Identifier.h"
#include "Statement/Literal.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

namespace {

	//
	//	VARIABLE local
	//	_$$::_InstanceManagerMap
	//
	//	NOTES
	//	すべてのInstanceManagerのインスタンスを保存するマップ
	//	本当はsetでいいけど、ModにないのでModMapを使用
	//
	InstanceManager::Map _cInstanceManagerMap;

	//
	//	VARIABLE local
	//	_$$::_InstanceManagerLatch
	//
	//	NOTES
	//	_$$::_InstanceManagerMapなどを操作を排他するためのCriticalSection
	//
	Os::CriticalSection _cInstanceManagerLatch;

	//
	//	VARIABLE local
	//	_$$::_iNewInstanceID -- 新しいインスタンスマネージャーID
	//
	ID _iNewInstanceID = 0;

	//
	//	FUNCTION local
	//	_$$::_getCryptMode -- 暗号化モードを表す文字列を得る
	//
	
	const ModUnicodeString* _getCryptMode(int mode)
	{
		return 0;
	}

	class _AutoLockMap
	{	
	public:
		_AutoLockMap() { InstanceManager::lockMap(); }
		~_AutoLockMap() {InstanceManager::unlockMap(); }
	};
}

//
//	FUNCTION public
//	Server::InstanceManager::PrepareTable::PrepareTable -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Server::InstanceManager& cInstanceManager_
//		サーバコネクション
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
InstanceManager::PrepareTable::PrepareTable()
{
}

//
//	FUNCTION public
//	Server::InstanceManager::PrepareTable::~PrepareTable -- デストラクタ
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
InstanceManager::PrepareTable::~PrepareTable()
{
}

//
//	FUNCTION public
//	Server::InstanceManager::PrepareTable::checkPrepareID -- 最適化結果IDをがあるかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InstanceManager::PrepareTable::checkPrepareID(int iPrepareID_)
{
	bool bResult = false;

	//削除する
	ModVector<int>::Iterator i = m_veciPrepareID.begin();
	for (; i != m_veciPrepareID.end(); ++i)
	{
		if ((*i) == iPrepareID_)
		{
			//ひとつしかない
			bResult = true;
			break;
		}
	}

	return bResult;
}

//
//	FUNCTION public
//	Server::InstanceManager::PrepareTable::pushPrepareID -- 最適化結果IDを追加する
//
//	NOTES
//
//	ARGUMENTS
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::PrepareTable::pushPrepareID(int iPrepareID_)
{
	//配列に追加する
	m_veciPrepareID.pushBack(iPrepareID_);
}

//
//	FUNCTION public
//	Server::InstanceManager::PrepareTable::popPrepareID -- 最適化結果IDを削除する
//
//	NOTES
//
//	ARGUMENTS
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::PrepareTable::popPrepareID(int iPrepareID_)
{
	//削除する
	ModVector<int>::Iterator i = m_veciPrepareID.begin();
	for (; i != m_veciPrepareID.end(); ++i)
	{
		if ((*i) == iPrepareID_)
		{
			//ひとつしかない
			m_veciPrepareID.erase(i);
			break;
		}
	}
}

//
//	FUNCTION public
//	Server::InstanceManager::PrepareTable::eraseAllPrepareID
//		-- 最適化結果をOptimizerから削除する
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
InstanceManager::PrepareTable::eraseAllPrepareID()
{
	ModVector<int>::Iterator i = m_veciPrepareID.begin();
	for (; i != m_veciPrepareID.end(); ++i)
	{
		Opt::Optimizer::erasePrepareStatement(*i);
	}

	m_veciPrepareID.clear();
}

//
//	FUNCTION public
//	Server::InstanceManager::create -- static factory
//
//	NOTES
//	static factory
//
//	ARGUMENTS
//	Server::Manager& cServerManager_
//		サーバマネージャ
//	const ModUnicodeString& cstrHostName_
//		クライアントのホスト名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//	static
InstanceManagerPointer
InstanceManager::create(Manager& cServerManager_,
								 const ModUnicodeString& cstrHostName_,
						int iProtocolVersion_)
{

	InstanceManagerPointer pInstMgr = new InstanceManager(
		cServerManager_,cstrHostName_, iProtocolVersion_);

	return pInstMgr;
}


//
//	FUNCTION private
//	Server::InstanceManager::InstanceManager -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Server::Manager& cServerManager_
//		サーバマネージャ
//	const ModUnicodeString& cstrHostName_
//		クライアントのホスト名
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InstanceManager::InstanceManager(Manager& cServerManager_,
								 const ModUnicodeString& cstrHostName_,
								 int iProtocolVersion_)
//								 ID iID_)
	: m_cServerManager(cServerManager_), m_cstrHostName(cstrHostName_),
	  m_iProtocolVersion(iProtocolVersion_),
	  m_bWaitWorker(false)
{
	Os::AutoCriticalSection cAuto(_cInstanceManagerLatch);

	// 新しいインスタンスIDを得る
	m_iID = ++_iNewInstanceID;
	// マップに登録する
	_cInstanceManagerMap.insert(m_iID, this);
	// 現在日時を得る
	m_cConnectedTime.setCurrent();
}

//
//	FUNCTION public
//	Server::InstanceManager::~InstanceManager -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InstanceManager::~InstanceManager()
{
}

//
//	FUNCTION public
//	Server::InstanceManager::terminate -- 終了処理を行う
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
//	なし
//
void
InstanceManager::terminate()
{
	{
		Os::AutoCriticalSection cAuto(_cInstanceManagerLatch);

		// マップから削除する
		Map::Iterator i = _cInstanceManagerMap.find(m_iID);
		; _TRMEISTER_ASSERT(i != _cInstanceManagerMap.end());
		_cInstanceManagerMap.erase(i);
	}
	
	// チェックポイント処理を実行不可にする
	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::Executor);

	{
		// 実行中のWorkerにキャンセルをリクエストする
		Os::AutoCriticalSection cAuto(m_cLatch);

		WorkerMap::Iterator i = m_mapWorker.begin();
		for (; i != m_mapWorker.end(); ++i)
		{
			(*i).second->stop();
		}
	}
	
	do
	{
		//終了していないトランザクションを終了する
		Os::AutoCriticalSection cAuto(m_cLatch);
		
		SessionMap::Iterator i = m_mapSession.begin();
		for (; i != m_mapSession.end(); ++i)

			// ここでセッションがロックされているかどうかチェックするが、
			// isLockedSessionを実行するとm_mapSessionが無限ループになるので、
			// 直接イテレータを参照し、チェックする

			if (!(*i).second->isLocked() && (*i).second->isAvailable()) {
				Transaction& trans = (*i).second->getTransaction();
				if (trans.isInProgress())

					// 終了するセッションで開始された
					// トランザクションが実行中であれば、ロールバックする

					trans.rollback();

				// 終了するセッションと連係している
				// トランザクションブランチをすべて待機する

				trans.xa_end();
			}
	}
	while (waitWorker() == false);
	
	{
		Os::AutoCriticalSection cAuto(m_cLatch);

		//終了していないトランザクションを終了する
		for (SessionMap::Iterator i = m_mapSession.begin();
			 i != m_mapSession.end(); ++i) {
			if ((*i).second->isAvailable()) {
				Transaction& trans = (*i).second->getTransaction();
				if (trans.isInProgress())
					trans.rollback();
				trans.xa_end();
			}
			delete (*i).second;
		}

		m_mapSession.clear();

		//コネクションプール内のものをすべて切断する
		ConnectionMap::Iterator k = m_mapConnection.begin();
		for (; k != m_mapConnection.end(); ++k)
		{
			(*k).second->close();
			delete (*k).second;
		}

		m_mapConnection.erase(m_mapConnection.begin(), m_mapConnection.end());

		//最適化結果を削除する
		PrepareMap::Iterator j = m_mapPrepare.begin();
		for (; j != m_mapPrepare.end(); ++j)
		{
			(*j).second->eraseAllPrepareID();
			delete (*j).second;
		}

		m_mapPrepare.clear();
	}
}

//
//	FUNCTION public
//	Server::InstanceManager::getSession -- セッションを得る
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	Server::Session*
//		セッション
//
//	EXCEPTIONS
//
Session*
InstanceManager::getSession(ID iSessionID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//マップを引く
	SessionMap::Iterator i = m_mapSession.find(iSessionID_);
	if (i == m_mapSession.end())
		_SYDNEY_THROW1(Exception::SessionNotExist, iSessionID_);

	return (*i).second;	
}

//
//	FUNCTION public
//	Server::InstanceManager::lockSession -- セッションをロックして得る
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	Server::Session*
//		セッション
//
//	EXCEPTIONS
//	Exception::SessionBusy
//		セッションがロックできない
//
Session*
InstanceManager::lockSession(ID iSessionID_, int iStatementType_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//マップを引く
	SessionMap::Iterator i = m_mapSession.find(iSessionID_);
	if (i == m_mapSession.end())
		_SYDNEY_THROW1(Exception::SessionNotExist, iSessionID_);

	if ((*i).second->tryLock(iStatementType_) == false)
		_SYDNEY_THROW0(Exception::SessionBusy);

	return (*i).second;
}

//
//	FUNCTION public
//	Server::InstanceManager::unlockSession -- セッションをアンロックする
//
//	NOTES
//
//	ARGUMENTS
//	Server::Session* pSession_
//		セッション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	
void
InstanceManager::unlockSession(Session* pSession_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//マップを引く
	SessionMap::Iterator i = m_mapSession.find(pSession_->getID());
	;_SYDNEY_ASSERT(i != m_mapSession.end());

	(*i).second->unlock();
}

//
//	FUNCTION public
//	Server::InstanceManager::pushSession -- セッションを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Session* pSession_
//		セッション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	
void
InstanceManager::pushSession(Session* pSession_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//マップに挿入する
	m_mapSession.insert(pSession_->getID(), pSession_);
}

//
//	FUNCTION public
//	Server::InstanceManager::popSession -- セッションを削除する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	Server::Session*
//		セッション
//
//	EXCEPTIONS
//
Session*
InstanceManager::popSession(ID iSessionID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//マップから削除する
	SessionMap::Iterator i = m_mapSession.find(iSessionID_);
	if (i == m_mapSession.end())
		_SYDNEY_THROW1(Exception::SessionNotExist, iSessionID_);

	if ((*i).second->tryLock(Statement::ObjectType::Object) == false)
		_SYDNEY_THROW0(Exception::SessionBusy);

	Session* pSession = (*i).second;

	m_mapSession.erase(i);

	return pSession;
}

//
//	FUNCTION public
//	Server::InstanceManager::isLockedSession -- セッションがロックされているか
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	bool
//		ロックされている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InstanceManager::isLockedSession(ID iSessionID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	bool result = false;

	SessionMap::Iterator i = m_mapSession.find(iSessionID_);
	if (i != m_mapSession.end())
	{
		if ((*i).second->isLocked() == true)
			result = true;
	}

	return result;
}

//
//	FUNCTION public
//	Server::InstanceManager::getDatabaseName
//		-- 指定されたセッションのデータベース名を得る
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iSessionID_
//		セッションID
//
//	RETURN
//	const ModUnicodeString&
//		データベース名
//
//	EXCEPTIONS
//
const ModUnicodeString&
InstanceManager::getDatabaseName(ID iSessionID_)
{
	return getSession(iSessionID_)->getDatabaseName();
}

// FUNCTION public
//	Server::InstanceManager::getUserName -- Get user name with a session ID
//
// NOTES
//
// ARGUMENTS
//	ID iSessionID_
//	
// RETURN
//	const ModUnicodeString&
//
// EXCEPTIONS

const ModUnicodeString&
InstanceManager::
getUserName(ID iSessionID_)
{
	return getSession(iSessionID_)->getUserName();
}

// FUNCTION public
//	Server::InstanceManager::isSuperUser -- Get session user is superuser or not
//
// NOTES
//
// ARGUMENTS
//	ID iSessionID_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
InstanceManager::
isSuperUser(ID iSessionID_)
{
	return getSession(iSessionID_)->isSuperUser();
}

//
//	FUNCTION public
//	Server::InstanceManager::pushConnection -- コネクションを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Communication::ServerConnection* pConnection_
//		コネクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	
void
InstanceManager::pushConnection(Communication::ServerConnection* pConnection_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	if (m_bWaitWorker == true)
		_SYDNEY_THROW0(Exception::ConnectionRanOut);

	//マップに挿入する
	m_mapConnection.insert(pConnection_->getSlaveID(), pConnection_);
}

//
//	FUNCTION public
//	Server::InstanceManager::popConnection -- コネクションを削除する
//
//	NOTES
//
//	ARGUMENTS
//	int iConnectionSlaveID_
//		コネクションID
//
//	RETURN
//	Communication::ServerConnection*
//		コネクション
//
//	EXCEPTIONS
//
Communication::ServerConnection*
InstanceManager::popConnection(int iConnectionSlaveID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	Communication::ServerConnection* pConnection = 0;

	//マップから削除する
	ConnectionMap::Iterator i = m_mapConnection.find(iConnectionSlaveID_);
	if (i != m_mapConnection.end())
	{
		//見つかったので削除する
		pConnection = (*i).second;
		m_mapConnection.erase(i);
	}

	return pConnection;
}


//
//	FUNCTION public
//	Server::InstanceManager::pushServerConnection -- サーバーコネクションを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Connection* pConnection_
//		コネクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	
void
InstanceManager::pushServerConnection(Server::Connection* pConnection_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	if (m_bWaitWorker == true)
		_SYDNEY_THROW0(Exception::ConnectionRanOut);

	//リストに挿入する
	m_listServerConnection.pushBack(pConnection_);
}

//
//	FUNCTION public
//	Server::InstanceManager::popServerConnection -- サーバーコネクションを削除する
//
//	NOTES
//
//	ARGUMENTS
//	int iConnectionSlaveID_
//		コネクションID
//
//	RETURN
//	Server::Connection*
//		コネクション
//  登録されていない場合は、0を返す
//
//	EXCEPTIONS
//
void
InstanceManager::removeServerConnection(Server::Connection* pConnection_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//マップから削除する
	ServerConnectionList::Iterator i = m_listServerConnection.find(pConnection_);
	if (i != m_listServerConnection.end())
	{
		//見つかったので削除する
		m_listServerConnection.erase(i);
	}
}



//
//	FUNCTION public
//	Server::InstanceManager::abortServerConnection -- サイバーコネクションに中断要求を発行する
//
//	NOTES
//
//	ARGUMENTS
//
//
//	RETURN
//	
//
//	EXCEPTIONS
//
void
InstanceManager::abortServerConnections()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	ServerConnectionList::Iterator i = m_listServerConnection.begin();
	for (; i != m_listServerConnection.end(); i++) {
		(*i)->abort();
	}
}

//
//	FUNCTION public
//	Server::InstanceManager::checkPrepareID -- 最適化結果IDがあるかチェックする
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InstanceManager::checkPrepareID(const ModUnicodeString& cstrDatabaseName_,
								int iPrepareID_)
{
	bool bResult = false;

	Os::AutoCriticalSection cAuto(m_cLatch);

	//PrepareMapを検索する
	PrepareMap::Iterator i = m_mapPrepare.find(cstrDatabaseName_);
	if (i != m_mapPrepare.end())
	{
		//追加する
		bResult = (*i).second->checkPrepareID(iPrepareID_);
	}

	return bResult;
}

//
//	FUNCTION public
//	Server::InstanceManager::pushPrepareID -- 最適化結果IDを追加する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::pushPrepareID(const ModUnicodeString& cstrDatabaseName_,
							   int iPrepareID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//PrepareMapを検索する
	PrepareMap::Iterator i = m_mapPrepare.find(cstrDatabaseName_);
	if (i == m_mapPrepare.end())
	{
		PrepareTable* pPrepareTable = new PrepareTable;
		pPrepareTable->pushPrepareID(iPrepareID_);
		m_mapPrepare.insert(cstrDatabaseName_, pPrepareTable);
	}
	else
	{
		//追加する
		(*i).second->pushPrepareID(iPrepareID_);
	}
}

//
//	FUNCTION public
//	Server::InstanceManager::popPrepareID -- 最適化結果IDを削除する
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrDatabaseName_
//		データベース名
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::popPrepareID(const ModUnicodeString& cstrDatabaseName_, int iPrepareID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);

	//PrepareMapを検索する
	PrepareMap::Iterator i = m_mapPrepare.find(cstrDatabaseName_);
	;_SYDNEY_ASSERT(i != m_mapPrepare.end());

	//削除する
	(*i).second->popPrepareID(iPrepareID_);
}

//
//	FUNCTION public
//	Server::InstanceManager::pushWorker -- Workerを追加する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Worker* pWorker_
//		追加するワーカ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::pushWorker(Worker* pWorker_)
{
	{
		Os::AutoCriticalSection cAuto(m_cLatch);
		if (m_bWaitWorker == true)
			_SYDNEY_THROW0(Exception::SessionNotAvailable);
		m_mapWorker.insert(pWorker_->getID(), pWorker_);
	}
	m_cServerManager.pushWorker(pWorker_);
}

//
//	FUNCTION public
//	Server::InstanceManager::cancelWorker -- Workerに中断を要求する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iWorkerID_
//		WorkerID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::cancelWorker(ID iWorkerID_)
{
	m_cServerManager.cancelWorker(iWorkerID_);
}

//
//	FUNCTION public
//	Server::InstanceManager::reportEndWorker -- WorkerをJoinするリクエストをWorkerManagerに送る
//
//	NOTES
//
//	ARGUMENTS
//	ID iWorkerID_
//		WorkerID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::reportEndWorker(ID iWorkerID_)
{
	m_cServerManager.reportEndWorker(iWorkerID_);
	{
		Os::AutoCriticalSection cAuto(m_cLatch);
		m_mapWorker.erase(iWorkerID_);
		if (m_bWaitWorker == true)
		{
			m_cWorkerEvent.set();
		}
	}
}


//	FUNCTION private
//	Server::InstanceManager::terminateWorkers
//		-- 各セッションを動作させているWorkerスレッドにstopを要求し、スレッドが止まる(Joinが完了する)まで待ち合わせる
//
//	NOTES
//
//	ARGUMENTS
//	セッションのリスト
//	
//
//	RETURN
//	なし
//
//
//	EXCEPTIONS
//
//

void
InstanceManager::terminateWorkers(const ModList<Server::ID>& listSessionID_)
{
	ModList<Server::Worker*> listWorker;
	stopWorkers(listSessionID_, listWorker);
	joinWorkers(listWorker);
}


//	FUNCTION private
//	Server::InstanceManager::stopWorkers --  セッションを動作させている各Workerスレッドにstopを要求する
//
//
//	NOTES
//
//	ARGUMENTS
//	セッションのリスト[IN]
//	Stopを要求したWorkerのリスト[OUT]
//
//	RETURN
//	
//
//
//	EXCEPTIONS
//
//

void InstanceManager::stopWorkers(const ModList<Server::ID>& listSessionID_, ModList<Server::Worker*>& listWorker_)
{
	ModList<Server::ID>::ConstIterator i = listSessionID_.begin();
	
	for(; i != listSessionID_.end(); ++i) {
		Server::Worker* pWorker = stopWorker(*i);
		if(pWorker != 0) {
			listWorker_.pushBack(pWorker);
		}
	}
}

//	FUNCTION private
//	Server::InstanceManager::stopWorker
//		-- セッションを動作させているWorkerスレッドにstopを要求する
//
//	NOTES
//
//	ARGUMENTS
//	セッションのリスト
//	
//
//	RETURN
//	なし
//
//
//	EXCEPTIONS
//
//
Server::Worker*
InstanceManager::stopWorker(const Server::ID iSessionID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	WorkerMap::Iterator i = m_mapWorker.begin();
	for (;i != m_mapWorker.end(); ++i) {
		if (iSessionID_ == (*i).second->getSessionID()) {
			if ((*i).second->trylockSync()) {
				return m_cServerManager.popWorker((*i).first);
			} else {
				// syncLockを取れない場合は、Sync実行中なのでスキップする
				return 0;
			}
		}
	}

	// セッションを動作中のWorkerが存在しない場合
	return 0;
}


//	FUNCTION private
//	Server::InstanceManager::joinWorkers
//		-- 各WorkerをJoin()して、管理対象から外す
//
//	NOTES
//
//	ARGUMENTS
//	Workerのリスト
//	
//
//	RETURN
//	なし
//
//
//	EXCEPTIONS
//
//
void
InstanceManager::joinWorkers(const ModList<Server::Worker*>& listWorker_)
{
	
	ModList<Server::Worker*>::ConstIterator i = listWorker_.begin();
	for(; i != listWorker_.end(); ++i) {
		(*i)->join();
		{
			Os::AutoCriticalSection cAuto(m_cLatch);
			m_mapWorker.erase((*i)->getID());
		}
		delete (*i);
	}
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Server::InstanceManager::pushWorkerQueue -- Workerをキューに登録する
//
//	NOTES
//
//	ARUGMENTS
//	Server::Worker* pWorker_
//		キューに登録するWorker
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::pushWorkerQueue(Worker* pWorker_)
{
	{
		Os::AutoCriticalSection cAuto(m_cLatch);
		m_mapWorker.insert(pWorker_->getID(), pWorker_);
	}
	m_cServerManager.pushWorkerQueue(pWorker_);
}
#endif

//
//	FUNCTION public
//	Server::InstanceManager::waitWorker -- すべてWorkerが終了するまで待つ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		Workerが存在しない場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
InstanceManager::waitWorker()
{
	Os::AutoTryCriticalSection cAuto(m_cLatch, false);
	cAuto.lock();

	m_bWaitWorker = true;
	
	if (m_mapWorker.getSize() != 0)
	{
		cAuto.unlock();
		m_cWorkerEvent.wait();
		return false;
	}
	return true;
}

//
//	FUNCTION public
//	Server::InstanceManager::getKey -- 共通鍵を取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const Communication::CryptKey::Pointer&
//		共通鍵
//
//	EXCEPTIONS
//
const Communication::CryptKey::Pointer&
InstanceManager::getKey()
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	return m_pCommonKey;
}

//
//	FUNCTION public
//	Server::InstanceManager::setKey -- 共通鍵を設定する
//
//	NOTES
//
//	ARGUMENTS
//	const Communication::CryptKey::Pointer& pKey_
//		設定する共通鍵
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InstanceManager::setKey(const Communication::CryptKey::Pointer& pKey_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	m_pCommonKey = pKey_;
}

//
//	FUNCTION public
//	Server::InstanceManager::lockMap -- Mapをロックする
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
//static
void
InstanceManager::lockMap()
{
	_cInstanceManagerLatch.lock();
}

//
//	FUNCTION public
//	Server::InstanceManager::unlockMap -- Mapのロックを解除する
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
//static
void
InstanceManager::unlockMap()
{
	_cInstanceManagerLatch.unlock();
}

//
//	FUNCTION public
//	Server::InstanceManager::lowerBound
//		-- InstanceManagerを取得する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iID_
//		InstanceManagerのID
//
//	RETURN
//	InstanceManager*
//	   	検索されたInstanceManagerへのポインター、存在しない場合は0を返す
//
//	EXCEPTIONS
//
//static
InstanceManagerPointer
InstanceManager::lowerBound(Server::ID iID_)
{
	//	【注意】
	//	_cInstanceManagerLatch がロックされていること

	InstanceManagerPointer r = 0;
	
	Map::Iterator i = ModLowerBound(_cInstanceManagerMap.begin(),
									_cInstanceManagerMap.end(),
									iID_,
									Map::Compare());

	if (i != _cInstanceManagerMap.end())
	{
		// ヒットした
		r = (*i).second;
	}

	return r;
}


//
//	FUNCTION public
//	Server::InstanceManager::terminateOtherSessionsOfAllInstanceManagers
//		-- 指定したセッションIDに該当するセッション以外のの全セッション、コネクションを終了する。
//
//	NOTES
//
//	ARGUMENTS
//	データベース名
//	セッションID
//	
//
//	RETURN
//	なし
//
//
//	EXCEPTIONS
//
//static
void
InstanceManager::terminateOtherSessionsOfAllInstanceManagers(const ModUnicodeString& cstrDatabaseName_, const ID iSessionID_)
{

	
	_AutoLockMap lockMap;
	InstanceManager::Map::Iterator i = _cInstanceManagerMap.begin();
	for (; i != _cInstanceManagerMap.end(); i++) {
		(*i).second->terminateOtherSessions(cstrDatabaseName_, iSessionID_);
	}
}


//	FUNCTION public
//	Server::InstanceManager::terminateOtherSessions
//		-- データベースが一致するセッションのうち、指定したセッションIDに該当するWorker以外のWorkerを終了する。
//
//	NOTES
//
//	ARGUMENTS
//	データベース名
//	セッションID
//	
//
//	RETURN
//	なし
//
//
//	EXCEPTIONS
//
//
void
InstanceManager::terminateOtherSessions(const ModUnicodeString& cstrDatabaseName_, const ID iSessionID_)
{
	ModList<Server::ID> listTerminateSession;
	createTerminateSessionList(cstrDatabaseName_, iSessionID_, listTerminateSession);
	terminateWorkers(listTerminateSession);
	rollbackSessions(listTerminateSession);
}



//	FUNCTION public
//	Server::InstanceManager::createTerminateSessionList
//		-- データベースが一致し、セッションIDが一致しないセッションIDのリストを作成する
//
//	NOTES
//
//	ARGUMENTS
//	データベース名[IN]
//	セッションID[IN]
//	セッションIDのリスト[OUT]
//
//	RETURN
//	なし
//
//
//	EXCEPTIONS
//
//
void
InstanceManager::createTerminateSessionList(const ModUnicodeString& cstrDatabaseName_,
											const ID iSessionID_, ModList<Server::ID>& listSessionID_)
{
	Os::AutoCriticalSection cAuto(m_cLatch);
	SessionMap::Iterator i = m_mapSession.begin();
	for (; i != m_mapSession.end(); ++i) {
		if((*i).first != iSessionID_ &&
		   (*i).second->getDatabaseName().compare(cstrDatabaseName_, ModFalse) == 0) {
			listSessionID_.pushBack((*i).first);
		}
	}
}

//	FUNCTION public
//	Server::InstanceManager::rollbackSessions
//		-- 各セッションをロールバックします
//
//	NOTES
//
//	ARGUMENTS
//	データベース名
//	セッションID
//	
//
//	RETURN
//	なし
//
//
//	EXCEPTIONS
//
//
void
InstanceManager::rollbackSessions(const ModList<Server::ID>& listSessionID_)
{

	Os::AutoCriticalSection cAuto(m_cLatch);
	ModList<Server::ID>::ConstIterator i = listSessionID_.begin();
	for(; i != listSessionID_.end(); ++i) {
		SessionMap::ConstIterator ite = m_mapSession.find(*i);
		if(ite == m_mapSession.end()) {
			continue;
		}
		Transaction& trans = (*ite).second->getTransaction();
		try	{
			;_SERVER_FAKE_ERROR(InstanceManager::terminateSession);

			if (trans.isInProgress())
			// 終了するセッションで開始された
			// トランザクションが実行中であれば、ロールバックする
				trans.rollback();

			// 終了するセッションと連係している
			// トランザクションブランチをすべて待機する
			trans.xa_end();
		}

#ifdef NO_CATCH_ALL
		catch (Exception::Object&)
#else
		catch (...)
#endif
		{
			SydErrorMessage << "rollback failed" << ModEndl;
			int iDatabaseID = trans.getDatabaseID();
			if(iDatabaseID != Schema::Object::ID::Invalid) {
				Checkpoint::Database::setAvailability(iDatabaseID, false);
			}

			_SYDNEY_RETHROW;
		}
	}
}



//
//	FUNCTION public
//	Server::InstanceManager::lowerBoundSession -- Sessionを検索する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iID_
//		lower_boundで検索するセッションID
//
//	RETURN
//	Server::Session*
//		ヒットしたSession、存在しない場合は0を返す
//
//	EXCEPTIONS
//
Session*
InstanceManager::lowerBoundSession(Server::ID iID_)
{
	//	【注意】
	//	_cInstanceManagerLatch と m_cLatch が、この順番でロックされていること

	Session* s = 0;

	SessionMap::Iterator i = ModLowerBound(m_mapSession.begin(),
										   m_mapSession.end(),
										   iID_,
										   SessionMap::Compare());
	if (i != m_mapSession.end())
	{
		// ヒットした
		s = (*i).second;
	}

	return s;
}

//
//	FUNCTION public
//	Server::InstanceManager::getCryptMode -- 暗号化モードを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	const ModUnicodeString&
//		暗号化モードを表す文字列
//
//	EXCEPTIONS
//
const ModUnicodeString*
InstanceManager::getCryptMode() const
{
	return 0;
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
