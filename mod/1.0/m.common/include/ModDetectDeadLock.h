// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModDetectDeadLock.h -- デッドロック検出機能を提供するクラス定義
// 
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModDetectDeadLock_H__
#define __ModDetectDeadLock_H__

#include "ModCommonDLL.h"

#ifdef	MOD_DEBUG
#include "ModOsDriver.h"

class ModSyncBase;
class ModOsMutex;
class ModWaitingThread;
#endif

//	CLASS
//	ModDetectDeadLock -- デッドロック検出機能を提供するクラス
//
//	NOTES
//		ロックをしているスレッドをスレッドローカル変数で区別しているので、
//		異プロセススレッド間のデッドロックの検証はできない

//【注意】	private でないメソッドのうち、inline でないものは dllexport する
//			private なメソッドのうち、private でなく inline である
//			メソッドから呼び出されるものは dllexport する
//			private かつ static なメンバーのうち、private でなく inline である
//			メソッドから参照されるものは dllexport する

class ModDetectDeadLock
{
public:
	ModCommonDLL
	static void initialize();
	ModCommonDLL
	static void terminate();
#ifdef MOD_DEBUG
	ModCommonDLL
	static ModBoolean check(ModWaitingThread* waitingThread,
							ModSyncBase* object);

	ModCommonDLL
	static void lockDetectMutex();
	ModCommonDLL
	static void unlockDetectMutex();

	ModCommonDLL
	static ModWaitingThread* getWaitingThread();
	ModCommonDLL
	static void deleteWaitingThread(ModWaitingThread* waitingThread);

	ModCommonDLL
	static ModBoolean detectFlag;

private:
	static ModOsMutex* mutex;			// detect Mutex
	static ModOsDriver::ThreadSpecificKey* detectKey;
#endif
};

#endif	// __ModDetectDeadLock_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
