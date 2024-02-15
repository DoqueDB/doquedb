// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ThreadManager.h --
// 
// Copyright (c) 2002, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_THREADMANAGER_H
#define __SYDNEY_SERVER_THREADMANAGER_H

#include "Server/Module.h"
#include "Server/Type.h"
#include "Server/Thread.h"
#include "Common/Thread.h"
#include "Os/CriticalSection.h"
#include "Os/Event.h"

#include "ModHashMap.h"
#include "ModVector.h"
#ifdef OBSOLETE
#include "ModList.h"
#endif

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

//
//	CLASS
//	Server::ThreadManager --
//
//	NOTES
//
//
class ThreadManager : public Common::Thread
{
public:
	//コンストラクタ
	ThreadManager();
	//デストラクタ
	virtual ~ThreadManager();

	//スレッドを挿入する
	void pushThread(Server::Thread* pThread_);

	//Joinするスレッドを登録する
	void pushJoinThread(ID iThreadID_);

	//スレッドに終了要求を行う
	bool stopThread(ID iThreadID_);

	//スレッドに終了要求し、Threadを管理対象外にする
	Server::Thread* popThread(ID iThreadID_);

	//すべてのスレッドの実行を終了する
	void stopAllThread();

#ifdef OBSOLETE
	//キューに登録する
	void pushQueue(Server::Thread* pThread_);
#endif

private:
	//HashMap
	typedef ModHashMap<ID, Server::Thread*, ModHasher<ID> > ThreadMap;

	//スレッドとして起動されるメソッド
	void runnable();

	//次にjoinするスレッドを得る
	Server::Thread* getNextJoinThread();

#ifdef OBSOLETE
	//次に実行するキュー内のスレッドを得る
	Server::Thread* getNextQueueThread();
#endif

	//マップ排他用のクリティカルセクション
	Os::CriticalSection m_cThreadLatch;

	//スレッド起動イベント
	Os::Event m_cEvent;

	//スレッドのマップ
	ThreadMap m_mapThread;

	//joinするスレッドの配列
	ModVector<ID> m_vecThreadID;

#ifdef OBSOLETE
	//スレッドキュー
	ModList<Server::Thread*> m_listThread;
#endif
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_THREADMANAGER_H

//
//	Copyright (c) 2002, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
