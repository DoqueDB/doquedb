// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModDetectDeadLock.cpp -- デッドロック検出機能を提供するクラス定義
// 
// Copyright (c) 1998, 2009, 2023 Ricoh Company, Ltd.
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


#include "ModDetectDeadLock.h"
#ifdef MOD_DEBUG
#include "ModSyncBase.h"
#include "ModOsMutex.h"
#include "ModWaitingThread.h"
#include "ModParameter.h"

ModBoolean ModDetectDeadLock::detectFlag = ModFalse;
ModOsMutex* ModDetectDeadLock::mutex = 0;
ModOsDriver::ThreadSpecificKey* ModDetectDeadLock::detectKey = 0;
#endif

void
ModDetectDeadLock::initialize()
{
#ifdef MOD_DEBUG
	ModParameter parameter(ModFalse);	// map を作らないモード
	// parameterの読み込み
	ModDetectDeadLock::detectFlag = parameter.getBoolean("detectDeadLock");
	// detect用mutexの初期化
	ModDetectDeadLock::mutex = new ModOsMutex;
	// TLSキーの初期化
	ModDetectDeadLock::detectKey = new ModOsDriver::ThreadSpecificKey;
#endif
}

void
ModDetectDeadLock::terminate()
{
#ifdef MOD_DEBUG
	// TLSキーの破棄
	delete ModDetectDeadLock::detectKey;
	ModDetectDeadLock::detectKey = 0;
	// detect用mutexの破棄
	delete ModDetectDeadLock::mutex;
	ModDetectDeadLock::mutex = 0;
#endif
}

#ifdef MOD_DEBUG

ModBoolean
ModDetectDeadLock::check(ModWaitingThread* waitingThread, ModSyncBase* object)
{
#ifdef FULL_DEBUG
	ModDebugMessage << "CHECK DEADLOCK: ";
	ModDebugMessage << ModHex << (unsigned int)waitingThread;
	ModDebugMessage << "(" << ModDec << waitingThread->threadId << ")";
	ModDebugMessage << " -> " << ModHex << (ModUInt64)object;
#endif

	while (object && object->_lockerThread) {
#ifdef FULL_DEBUG
		ModDebugMessage << " => ";
		ModDebugMessage << ModHex << (unsigned int) object->_lockerThread;
		ModDebugMessage << "("<< ModDec <<
			object->_lockerThread->threadId << ")";
#endif
		if (object->_lockerThread == waitingThread) {
			// ループ発見->デッドロック
#ifdef FULL_DEBUG
			ModDebugMessage << " ==> DEADLOCK!!" << ModEndl;
#endif
			return ModFalse;
		}
		object = object->_lockerThread->waitingTarget;
#ifdef FULL_DEBUG
		ModDebugMessage << " -> " << ModHex << (unsigned int)object;
#endif
	}
#ifdef FULL_DEBUG
	ModDebugMessage << " OK!!" << ModEndl;
#endif
	return ModTrue;
}

void
ModDetectDeadLock::lockDetectMutex()
{
	ModDetectDeadLock::mutex->lock();
}

void
ModDetectDeadLock::unlockDetectMutex()
{
	ModDetectDeadLock::mutex->unlock();
}

ModWaitingThread*
ModDetectDeadLock::getWaitingThread()
{
	ModWaitingThread* waitingThread = (ModWaitingThread*)
		ModDetectDeadLock::detectKey->getValue();

	if (waitingThread == 0) {
		// 登録されていない場合は新たにポインターを得て返す。
		waitingThread = new ModWaitingThread;
		ModDetectDeadLock::detectKey->setValue(waitingThread);
	}

	return waitingThread;
}

void
ModDetectDeadLock::deleteWaitingThread(ModWaitingThread* waitingThread)
{
	// TLS をクリアする
	ModDetectDeadLock::detectKey->setValue(0);
	// ポインターを解放する
	delete waitingThread;
}

#endif // MOD_DEBUG

//
// Copyright (c) 1998, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
