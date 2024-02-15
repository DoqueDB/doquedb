// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RWLock.cpp -- RW ロック関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Os";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

extern "C" 
{
#ifdef SYD_OS_WINDOWS
#include <windows.h>
#endif
#ifdef SYD_OS_POSIX
#include <pthread.h>
#endif
}

#include "Os/RWLock.h"

#include "Exception/BadArgument.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace
{

namespace _RWLock
{
	//	TYPEDEF
	//	$$$:_RWLock::LockPointer -- ロック関数へのポインタ型
	//
	//	NOTES

	typedef	void (RWLock::* _LockPointer)();

	//	TYPEDEF
	//	$$$::_RWLock::TryLockPointer -- ロック試行関数へのポインタ型
	//
	//	NOTES

	typedef bool (RWLock::* _TryLockPointer)();

	//	TYPEDEF
	//	$$$::_RWLock::UnlockPointer -- アンロック関数へのポインタ型
	//
	//	NOTES

	typedef	void (RWLock::* _UnlockPointer)();
}

}

//	FUNCTION public
//	Os::RWLock::lock --	読み取り書き込みロックする
//
//	NOTES
//
//	ARGUMENTS
//		Os::RWLock::Mode::Value	mode
//			かけるロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			かけるロックのモードがおかしい

void
RWLock::lock(Mode::Value mode)
{
	static const _RWLock::_LockPointer table[RWLock::Mode::ValueNum + 1] =
	{
		&RWLock::lockBadArgument,
		&RWLock::lockRead,
		&RWLock::lockWrite,
		&RWLock::lockBadArgument
	};

	// 指定されたモードでロックする

	(this->*table[mode])();
}

//	FUNCTION private
//	Os::RWLock::lockRead --	読み取りロックする
//
//	NOTES
//		同じスレッドがなん度でも読み取りロックを重ねがけできる
//		
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
RWLock::lockRead()
{
#ifdef SYD_POSIX_RWLOCK

	// 読み取り書き込みロックに読み取りロックをかける
	//
	//【注意】	マニュアルになにを返すか記載がない

	(void) ::pthread_rwlock_rdlock(&_rwlock);
#else
	// 誰かが書き込みロック中、またはロック、
	// アンロック処理中でないことを保証するためにラッチする

	_latch.lock();

	// 読み取りロック数を 1 増やす

	if (!_reader++ && _waiter)

		// 誰も読み取りロックしていなかったとき、
		// 書き込みロック待ちしようとしているスレッドがいれば、
		// 起きないようにする
		//
		//【注意】	あるスレッドが読み取りロック中に、
		//			別スレッドが lockWrite でイベント待ちする直前に、
		//			その読み取りロックをはずし、
		//			再び読み取りロックすると、ここにくる可能性がある

		_event.reset();

	_latch.unlock();
#endif
}

//	FUNCTION private
//	Os::RWLock::lockWrite -- 書き込みロックする
//
//	NOTES
//		同じスレッドがなん度でも書き込みロックを重ねがけできる
//
//		読み取りロックしているスレッドが書き込みロックしようとすると、
//		デッドロックが起きる(SYD_POSIX_RWLOCK の仕様は未確認)
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
RWLock::lockWrite()
{
#ifdef SYD_POSIX_RWLOCK

	// 読み取り書き込みロックに書き込みロックをかける
	//
	//【注意】	マニュアルになにを返すか記載がない

	(void) ::pthread_rwlock_wrlock(&_rwlock);
#else
	// 誰かが書き込みロック中、またはロック、
	// アンロック処理中でないことを保証するためにラッチする

	_latch.lock();
	
	// 誰も読み取りロック中でないことを確認する

	while (_reader) {

		// 誰も読み取りロック中でなくなるまで、待つ

		++_waiter;
		_latch.unlock();

		_event.wait();

		_latch.lock();
		--_waiter;
	}

	// 【注意】	ラッチをロックしたままになる
#endif
}

//	FUNCTION private
//	Os::RWLock::lockBadArgument -- ロック時のエラー処理用関数
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
RWLock::lockBadArgument()
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//	FUNCTION public
//	Os::RWLock::trylock --	読み取り書き込みロックを試みる
//
//	NOTES
//		ロックを試みた結果、ロック待ちするようであれば、ロックをあきらめる
//
//	ARGUMENTS
//		Os::RWLock::Mode::Value	mode
//			かけるロックのモード
//
//	RETURN
//		true
//			与えられたモードでロックできた
//		false
//			与えられたモードでロックできなかった
//
//	EXCEPTIONS
//		Exception::BadArgument
//			かけるロックのモードがおかしい

bool
RWLock::trylock(Mode::Value mode)
{
	static const _RWLock::_TryLockPointer table[RWLock::Mode::ValueNum + 1] =
	{
		&RWLock::trylockBadArgument,
		&RWLock::trylockRead,
		&RWLock::trylockWrite,
		&RWLock::trylockBadArgument
	};

	// 指定されたモードでロックを試みる

	return (this->*table[mode])();
}

//	FUNCTION private
//	Os::RWLock::trylockRead -- 読み取りロックを試みる
//
//	NOTES
//		同じスレッドがなん度でも読み取りロックを重ねがけできる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			読み取りロックできた
//		false
//			読み取りロックできなかった
//
//	EXCEPTIONS
//		なし

bool
RWLock::trylockRead()
{
#ifdef SYD_POSIX_RWLOCK

	// 読み取り書き込みロックに読み取りロックをかけてみる

	return ::pthread_rwlock_tryrdlock(&_rwlock) != EBUSY;
#else
	// 誰かが書き込みロック中、またはロック、
	// アンロック処理中でないことを保証するためにラッチを試みる

	if (_latch.trylock()) {

		// 読み取りロック数を 1 増やす

		++_reader;

		_latch.unlock();

		return true;
	}

	// 読み取りロックできなかった

	return false;
#endif
}

//	FUNCTION private
//	Os::RWLock::trylockWrite -- 書き込みロックを試みる
//
//	NOTES
//		同じスレッドがなん度でも書き込みロックを重ねがけできる
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			書き込みロックできた
//		false
//			書き込みロックできなかった
//
//	EXCEPTIONS
//		なし

bool
RWLock::trylockWrite()
{
#ifdef SYD_POSIX_RWLOCK

	// 読み取り書き込みロックに書き込みロックをかけてみる

	return ::pthread_rwlock_trywrlock(&_rwlock) != EBUSY;
#else
	// 誰かが書き込みロック中、またはロック、
	// アンロック処理中でないことを保証するためにラッチを試みる

	if (_latch.trylock()) {

		// 誰も読み取りロック中でないことを確認する

		if (!_reader)

			// 書き込みロックできた
			//
			// 【注意】	ラッチをロックしたままになる

			return true;

		_latch.unlock();
	}

	// 書き込みロックできなかった

	return false;
#endif
}

//	FUNCTION private
//	Os::RWLock::trylockBadArgument -- ロックの試み時のエラー処理用関数
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		false
//
//	EXCEPTIONS

bool
RWLock::trylockBadArgument()
{
	_TRMEISTER_THROW0(Exception::BadArgument);
	return false;
}

//	FUNCTION public
//	Os::RWLock::unlock -- 読み取り書き込みロックをはずす
//
//	NOTES
//		ロックしていないスレッドがはずしたときの動作は、不定である
//
//	ARGUMENTS
//		Os::RWLock::Mode::Value	mode
//			はずすロックのモード
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			はずすロックのモードがおかしい

void
RWLock::unlock(Mode::Value mode)
{
#ifdef SYD_POSIX_RWLOCK

	// 読み取り書き込みロックのロックをはずす
	//
	//【注意】	はずすロックのモードを指定する必要がない
	//
	//【注意】	マニュアルになにを返すか記載がない

	(void) ::pthread_rwlock_unlock(&_rwlock);
#else
	static const _RWLock::_UnlockPointer table[RWLock::Mode::ValueNum + 1] =
	{
		&RWLock::unlockBadArgument,
		&RWLock::unlockRead,
		&RWLock::unlockWrite,
		&RWLock::unlockBadArgument
	};

	// 指定されたモードでアンロックする

	(this->*table[mode])();
#endif
}

//	FUNCTION private
//	Os::RWLock::unlockRead -- 読み取りロックをはずす
//
//	NOTES
//		ロックしていないスレッドがはずしたときの動作は、不定である
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
RWLock::unlockRead()
{
#ifdef SYD_POSIX_RWLOCK

	// 読み取りロックをはずす

	(void) ::pthread_rwlock_unlock(&_rwlock);
#else
	// 誰かが書き込みロック中、またはロック、
	// アンロック処理中でないことを保証するためにラッチする

	_latch.lock();

	// 読み取りロック数を 1 減らす

	if (!--_reader && _waiter)

		// 最後の読み取りロックをはずそうとしている

		try {
			// 書き込みロックしようとして、
			// 読み取りロック中でなくなるのを待っている
			// スレッドがいるので、起こす

			_event.set();

		} catch (...) {

			++_reader;
			_latch.unlock();

			_TRMEISTER_RETHROW;
		}

	_latch.unlock();
#endif
}

//	FUNCTION private
//	Os::RWLock::unlockWrite -- 書き込みロックをはずす
//
//	NOTES
//		ロックしていないスレッドがはずしたときの動作は、不定である
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
RWLock::unlockWrite()
{
#ifdef SYD_POSIX_RWLOCK

	// 書き込みロックをはずす

	(void) ::pthread_rwlock_unlock(&_rwlock);
#else
	// ロックしたままのラッチをアンロックする

	_latch.unlock();
#endif
}

//	FUNCTION private
//	Os::RWLock::unlockBadArgument -- アンロック時のエラー処理用関数
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
RWLock::unlockBadArgument()
{
	_TRMEISTER_THROW0(Exception::BadArgument);
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
