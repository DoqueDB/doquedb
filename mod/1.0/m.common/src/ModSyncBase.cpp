// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModSyncBase.cpp -- 同期オブジェクト基底クラス関連のメソッド定義
// 
// Copyright (c) 1998, 1999, 2005, 2023 Ricoh Company, Ltd.
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


#include "ModSyncBase.h"
#include "ModWaitingThread.h"
#include "ModDetectDeadLock.h"

//	FUNCTION public
//	ModSyncBase::lock -- 同期オブジェクトをロックする
//
//	NOTES
//		同期オブジェクトをロックするとデッドロックになるか検査し、
//		デッドロックにならなければ、必要があればロックする
//
//		他のスレッドがロックしていれば、
//		そのスレッドがロックをすべてはずすまで、
//		ロック待ちする(つまり、呼び出しから返らない)
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				ロックする回数
//			指定されないとき
//				1 が指定されたものみなす
//
//	RETURN
//		現在のロック数
//
//	EXCEPTIONS
#ifdef MOD_DEBUG
//		ModOsErrorDeadLock
//			デッドロックになるので、ロックできない
//			(ModSyncBase::beginLock より)
#endif

int
ModSyncBase::lock(unsigned int times)
{
	ModThreadId self = ModOsDriver::Thread::self();
	if (_count && _locker == self)

		// 自分がロックしている
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		return _count += times;

	try {
#ifdef MOD_DEBUG
		if (ModDetectDeadLock::detectFlag) {

			// デットロック検査などのロックの準備

			ModWaitingThread*	lockerThread = this->beginLock(ModTrue);

			// 同期オブジェクトを実際にロックする
			// ここで、ロック待ちするかもしれない

			this->lockReally();

			// デッドロック検査情報の登録などのロックの後処理

			this->endLock(lockerThread);
		} else
#endif
			this->lockReally();

		if (_unlockKilled) {

			// ロックした同期オブジェクトを
			// 呼び出しスレッドに記録しておく

			ModThread*	thread = ModThisThread::getThread();
			if (thread)
				thread->addLockingMutex(this);
		}
	} catch (ModException& exception) {
			ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {

		// ロック待ちの間にスレッドが強制終了された

		// 以下のように再送すると、
		//
		// ModException& e = ModError::getException();
		// e.setError(ModModuleOs,
		//			  ModCommonErrorUnexpected, ModErrorLevelFatal);
		// cerr << e.setMessage() << ModEndl;
		// throw e;
		//
		// 以下のようにコアをはく
		//
		// signal fault in critical section
		// signal number: 11, signal code: 1,                  
		// fault address: 0x0, pc: 0xef784990, sp: 0xeef05a78
		// ABORT: core dump

		// 以下はいかにも無謀
		//
		// ModThisThread::exit(0);

		ModUnexpectedThrowOs();
	}
#endif

	// 今ロックしたスレッドの ID とロック数を記録する

	_locker = self;
	return _count = times;
}

//	FUNCTION public
//	ModSyncBase::trylock -- 同期オブジェクトのロックを試みる
//
//	NOTES
//		同期オブジェクトをロックするとデッドロックになるか検査し、
//		デッドロックにならなければ、必要があればロックを試みる
//
//		他のスレッドがロックしていれば、
//		ロック待ちせずに、すぐ呼び出しから返る
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				ロックする回数
//			指定されないとき
//				1 が指定されたものみなす
//
//	RETURN
//		0 より大きい値
//			現在のロック数
//		0
//			ロックできなかった
//
//	EXCEPTIONS

int
ModSyncBase::trylock(unsigned int times)
{
	ModThreadId self = ModOsDriver::Thread::self();
	if (_count && _locker == self)

		// 自分がロックしている
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		return _count += times;

	ModBoolean	isLocked;
	try {
#ifdef MOD_DEBUG
		if (ModDetectDeadLock::detectFlag) {

			// デットロック検査などのロックの準備
			//
			//【注意】	ロック待ちしないのでデッドロック検査は必要ない

			ModWaitingThread*	lockerThread = this->beginLock(ModFalse);

			// 同期オブジェクトを実際にロックを試みる
			// ここで、ロック待ちするかもしれない

			isLocked = this->trylockReally();

			// デッドロック検査情報の登録などのロックの後処理

			this->endLock((isLocked) ? lockerThread : 0);
		} else
#endif
			isLocked = this->trylockReally();

		if (!isLocked)
			
			// ロックできなかった

			return 0;

		else if (_unlockKilled) {

			// ロックした同期オブジェクトを
			// 呼び出しスレッドに記録しておく

			ModThread*	thread = ModThisThread::getThread();
			if (thread)
				thread->addLockingMutex(this);
		}
	} catch (ModException& exception) {
		ModRethrow(exception);
	}
#ifndef NO_CATCH_ALL
	catch (...) {
		ModUnexpectedThrowOs();
	}
#endif

	// 今ロックしたスレッドの ID とロック数を記録する

	_locker = self;
	return _count = times;
}

//	FUNCTION public
//	ModSyncBase::unlock -- 同期オブジェクトをアンロックする
//
//	NOTES
//		必要があれば、同期オブジェクトを実際にアンロック後、
//		デッドロック用の検査情報を更新する
//
//	ARGUMENTS
//		unsigned int		times
//			指定されたとき
//				アンロックする回数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorNotLocked
//			ロックしていないのにアンロックしようとしている

void
ModSyncBase::unlock(unsigned int times)
{
	if (_count < (int) times || _locker != ModOsDriver::Thread::self())

		// ロックしていないのにアンロックしようとしている
		// または、ロックしている以上にはずそうとしている
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		ModMessageThrowOsError(ModOsErrorNotLocked);

	// 自分がロックしている

	if (!(_count -= times)) {
		try {
			if (_unlockKilled) {

				// アンロックする同期オブジェクトの記録を
				// 呼び出しスレッドから抹消しておく

				ModThread*	thread = ModThisThread::getThread();
				if (thread)
					thread->deleteLockingMutex(this);
			}
#ifdef MOD_DEBUG
			if (ModDetectDeadLock::detectFlag) {

				// 同期オブジェクトを実際にアンロックする

				this->unlockReally();

				// デッドロック検査情報の抹消などのアンロックの後処理

				this->endUnlock();
			} else
#endif
				this->unlockReally();

		} catch (ModException& exception) {

			_count += times;
			ModRethrow(exception);

		}
#ifndef NO_CATCH_ALL
		catch (...) {

			_count += times;
			ModUnexpectedThrowOs();
		}
#endif
	}
}

//	FUNCTION public
//	ModSyncBase::unlockAll -- 同期オブジェクトのロックをすべてはずす
//
//	NOTES
//		同期オブジェクトを実際にアンロック後、
//		デッドロック用の検査情報を更新する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		ModOsErrorNotLocked
//			ロックしていないのにアンロックしようとしている

void
ModSyncBase::unlockAll()
{
	if (_count <= 0 || _locker != ModOsDriver::Thread::self())

		// ロックしていないのにアンロックしようとしている
		// または、ロック数がおかしい
		//
		//【注意】	ロックしているスレッドのスレッド ID は
		//			ロックしていなければ設定されていないので、
		//			まず、ロックされていることを調べる必要がある

		ModMessageThrowOsError(ModOsErrorNotLocked);

	// 自分がロックしている

	unsigned int	times = _count;

	try {
		// 全ロックをはずすので、ロック数を 0 にする

		_count = 0;
#ifdef MOD_DEBUG
		if (ModDetectDeadLock::detectFlag) {

			// 同期オブジェクトを実際にアンロックする

			this->unlockReally();

			// デッドロック検査情報の抹消などのアンロックの後処理

			this->endUnlock();
		} else
#endif
			this->unlockReally();

	} catch (ModException& exception) {

		_count = times;
		ModRethrow(exception);

	}
#ifndef NO_CATCH_ALL
	catch (...) {

		_count = times;
		ModUnexpectedThrowOs();
	}
#endif
}

#ifdef MOD_DEBUG
//	FUNCTION private
//	ModSyncBase::beginLock --
//		ロック前にデッドロック検査情報を生成し、デッドロック検査を行う
//
//	NOTES
//
//	ARGUMENTS
//		ModBoolean			doCheck
//			ModTrue
//				ロックをしたときにデッドロックになるか調べる
//			ModFalse
//				ロックをしたときにデッドロックになるか調べない
//
//	RETURN
//		ロックするスレッドのロック待ち状態を表す情報
//
//	EXCEPTIONS
//		ModOsErrorDeadLock
//			デッドロックになるので、ロックできない

ModWaitingThread*
ModSyncBase::beginLock(ModBoolean doCheck)
{
	// デッドロック検査情報の更新のための許可を得る

	ModDetectDeadLock::lockDetectMutex();

	// 呼び出しスレッドのロック待ち情報を得る

	ModWaitingThread*	lockerThread = ModDetectDeadLock::getWaitingThread();

	// ロック待ち対象としてこの同期オブジェクトを登録しておく

	lockerThread->waitingTarget = this;

	if (doCheck) {
retry:
		// この同期オブジェクトをロックすると、
		// デッドロックになるか検査する

		if (ModDetectDeadLock::check(lockerThread, this) == ModFalse) {

			// ロック待ち対象の登録を抹消する

			lockerThread->waitingTarget = 0;

			// デッドロック検査情報の更新を終了する

			ModDetectDeadLock::unlockDetectMutex();

			// ロックすると、デッドロックになる

			ModThrowOsError(ModOsErrorDeadLock);
		}

		if (_lockingCount) {
			while (_lockingCount) {

				// この同期オブジェクトを
				// まさにロックしようとしているスレッドがいるので、
				// ロックしようとしているスレッドがなくなるまで、
				// 1 ミリ秒ずつ待ち続ける

				ModDetectDeadLock::unlockDetectMutex();
				ModThisThread::sleep(1);
				ModDetectDeadLock::lockDetectMutex();
			}

			// もう一度デッドロック検査を行う

			goto retry;
		}
	}

	// この同期オブジェクトを
	// ロックしようとしているスレッド数を 1 増やす

	_lockingCount++;

	// デッドロック検査情報の更新を終了する

	ModDetectDeadLock::unlockDetectMutex();

	return lockerThread;
}

//	FUNCTION private
//	ModSyncBase::endLock --
//		ロック後にデッドロック検査情報をロック後の状態にあわせる
//
//	NOTES
//
//	ARGUMENTS
//		ModWaitingThread*	lockerThread
//			0 以外の値
//				ロックしたスレッドのロック待ち状態を表す情報
//			0
//				ロックしなかったことを表す
//
//	RETURN
//		なし
//
//	EXCEPTIONS	

void
ModSyncBase::endLock(ModWaitingThread* lockerThread)
{
	// デッドロック検査情報の更新のための許可を得る

	ModDetectDeadLock::lockDetectMutex();

	if (lockerThread) {

		// この同期オブジェクトをロック中のスレッド情報を記録する

		_lockerThread = lockerThread;

		// 今ロックしたスレッドのロック保持数を 1 増やし、
		// ロック待ち対象の登録を抹消する

		_lockerThread->count++;
		_lockerThread->waitingTarget = 0;
	}

	// ロックの試みが終わったので、この同期オブジェクトを
	// ロックしようとしているスレッド数を 1 減らす

	_lockingCount--;

	// デッドロック検査情報の更新を終了する

	ModDetectDeadLock::unlockDetectMutex();
}

//	FUNCTION private
//	ModSyncBase::endUnlock --
//		アンロック後にデッドロック検査情報をアンロック後の状態にあわせる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
ModSyncBase::endUnlock()
{
	// デッドロック検査をしているので、
	// デッドロック検査情報を更新する

	// デッドロック検査情報の更新のための許可を得る

	ModDetectDeadLock::lockDetectMutex();

	if (_lockerThread &&
		_lockerThread->threadId == ModOsDriver::Thread::self())

		// ロックをはずしたので、
		// この同期オブジェクトをロック中のスレッド情報を抹消する

		_lockerThread = 0;

	// 呼び出しスレッドのロック待ち情報を得る
		
	ModWaitingThread*	lockerThread = ModDetectDeadLock::getWaitingThread();
	if (!(--lockerThread->count))

		// 保持しているロックがなくなったので、
		// ロック待ち情報を抹消する

		ModDetectDeadLock::deleteWaitingThread(lockerThread);

	// デッドロック検査情報の更新を終了する

	ModDetectDeadLock::unlockDetectMutex();
}
#endif	// MOD_DEBUG

//
// Copyright (c) 1998, 1999, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
