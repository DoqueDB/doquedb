// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ThreadManager.cpp --
// 
// Copyright (c) 2002, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Server/ThreadManager.h"
#ifdef OBSOLETE
#include "Server/Parameter.h"
#endif
#include "Os/CriticalSection.h"
#include "Os/AutoCriticalSection.h"
#include "Os/Event.h"

#include "ModHashMap.h"

_SYDNEY_USING
_SYDNEY_SERVER_USING

#ifdef OBSOLETE
namespace
{
	//
	//	VARIABLE local
	//	_$$::_cParameterMaximumConcurrentExecutionCount -- 最大同時実行数
	//
	//	NOTES
	//
	ParameterInteger _cParameterMaximumConcurrentExecutionCount("Server_MaximumConcurrentExecutionCount", 20, 1, 100);
}
#endif

//
//	FUNCTION public
//	Server::ThreadManager::ThreadManager -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
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
ThreadManager::ThreadManager()
{
}

//
//	FUNCTION public
//	Server::ThreadManager::~ThreadManager -- デストラクタ
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
ThreadManager::~ThreadManager()
{
}

//
//	FUNCTION public
//	Server::ThreadManager::pushThread -- 新しいThreadを追加する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Thread* pThread_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ThreadManager::pushThread(Server::Thread* pThread_)
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	//マップに登録
	m_mapThread.insert(pThread_->getID(), pThread_);
}

//
//	FUNCTION public
//	Server::ThreadManager::pushJoinThread -- JoinするThreadを登録する
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iThreadID_
//		JoinするID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ThreadManager::pushJoinThread(ID iThreadID_)
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	//配列に登録
	m_vecThreadID.pushBack(iThreadID_);

	//Joinするためにイベントを発生させる
	m_cEvent.set();
}

//
//	FUNCTION public
//	Server::ThreadManager::stopThread -- 停止要求を行う
//
//	NOTES
//
//	ARGUMENTS
//	Server::ID iThreadID_
//
//	RETURN
//	停止要求できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ThreadManager::stopThread(ID iThreadID_)
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	//マップからThreadを得る
	ThreadMap::Iterator i = m_mapThread.find(iThreadID_);

	if (i != m_mapThread.end())
	{
		//中断要求する
		(*i).second->stop();

		return true;
	}

	return false;
}


//
//	FUNCTION public
//	Server::ThreadManager::popThread --
//				停止要求を行い、Threadを管理対象外にする
//				本関数の呼び出し元でThreadをJoin()する必要がある
//	NOTES
//
//	ARGUMENTS
//	Server::ID iThreadID_
//
//	RETURN
//	停止要求できた場合は停止したThreadのインスタンス、それ以外の場合は0
//
//	EXCEPTIONS
//
Server::Thread*
ThreadManager::popThread(ID iThreadID_)
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	//マップからThreadを得る
	ThreadMap::Iterator i = m_mapThread.find(iThreadID_);

	if (i != m_mapThread.end())
	{
		//中断要求する
		Server::Thread* pThread = (*i).second;
		pThread->stop();
		pThread->abort();
		m_mapThread.erase(iThreadID_);

		return pThread;
	}

	return 0;
}


//
//	FUNCTION public
//	Server::ThreadManager::stopAllThread -- すべてのスレッドを停止する
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
ThreadManager::stopAllThread()
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	//Threadのマップをなめて、すべてのThreadをJoin配列に加える
	ThreadMap::Iterator i = m_mapThread.begin();
	for (; i != m_mapThread.end(); ++i)
	{
		//中断要求する
		(*i).second->stop();

		//配列に加える
		m_vecThreadID.pushBack((*i).first);
	}

	//Joinするためにイベントを発生させる
	m_cEvent.set();
	
	//終わりなのでabort要求もしとく
	abort();
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Server::ThreadManager::pushQueue -- スレッドをキューに登録する
//
//	NOTES
//
//	ARGUMENTS
//	Server::Thread* pThread_
//		キューに登録するスレッド
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ThreadManager::pushQueue(Server::Thread* pThread_)
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	if (static_cast<int>(m_mapThread.getSize()) >=  _cParameterMaximumConcurrentExecutionCount.get())
	{
		// 同時実行数が超えているので、キューに登録する
		m_listThread.pushBack(pThread_);
	}
	else
	{
		// 同時実行数制限に達していないので、そのまま実行
		pThread_->setEvent();
	}
}
#endif

//
//	FUNCTION private
//	Server::ThreadManager::runnable -- スレッドとして起動されるメソッド
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
ThreadManager::runnable()
{
	while (1)
	{
		//イベントを待つ
		m_cEvent.wait();

		//JoinするThreadがなくなるまで繰り返す
		while (Server::Thread* pThread = getNextJoinThread())
		{
			//Joinする
			pThread->join();

			//インスタンス削除
			delete pThread;
#ifdef OBSOLETE
			//キュー内のスレッドを起動する
			Server::Thread* pQueueThread = getNextQueueThread();
			if (pQueueThread)
			{
				// 実行を待っているスレッドがあるので、実行する
				pQueueThread->setEvent();
			}
#endif
		}

		//abort要求がきているか？
		if (isAborted() == true)
		{
			Os::AutoCriticalSection cAuto(m_cThreadLatch);
			
			// 削除配列が空なら抜ける
			if (m_vecThreadID.getSize() == 0)
				return;
		}
	}
}

//
//	FUNCTION private
//	Server::ThreadManager::getNextJoinThread -- 次にJoinするThreadを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Server::Thread*
//		JoinするThread
//
//	EXCEPTIONS
//
Server::Thread*
ThreadManager::getNextJoinThread()
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	Server::Thread* pThread = 0;

	while (pThread == 0)
	{
		if (m_vecThreadID.getSize() == 0)
			break;

		//ThreadIDを取り出す
		ID iThreadID = m_vecThreadID.getFront();
		m_vecThreadID.popFront();	//先頭の要素を削除

		//マップからThreadを得る
		ThreadMap::Iterator i = m_mapThread.find(iThreadID);

		if (i != m_mapThread.end())
		{
			//変数へ代入
			pThread = (*i).second;

			//要素を削除
			m_mapThread.erase(i);
		}
	}

	return pThread;
}

#ifdef OBSOLETE
//
//	FUNCTION private
//	Server::ThreadManager::getNextQueueThread -- 次に実行するキュー内のスレッドを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Server::Thread*
//		キュー内のスレッド。存在しない場合は0
//
//	EXCEPTIONS
//
Server::Thread*
ThreadManager::getNextQueueThread()
{
	Os::AutoCriticalSection cAuto(m_cThreadLatch);

	Server::Thread* pThread = 0;

	ModList<Server::Thread*>::Iterator i = m_listThread.begin();
	if (i != m_listThread.end())
	{
		pThread = *i;
		m_listThread.popFront();
	}

	return pThread;
}
#endif

//
//	Copyright (c) 2002, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
