// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CriticalSection.cpp -- クリティカルセクション関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2018, 2023 Ricoh Company, Ltd.
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

//
// ★注意★
// 本クラスを修正した場合は、
// CriticalSection のアドレスが格納されるレジスタに変更がないか動作確認をして確かめること。
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
#include <assert.h>
#endif
#ifdef SYD_OS_POSIX
#include <assert.h>
#include <pthread.h>
#endif
}

#ifdef SYD_OS_LINUX
#include "Os/CriticalSectionManager.h"
#endif

#include "Os/Assert.h"
#include "Os/CriticalSection.h"
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
#include "Os/Event.h"
#endif
#endif
#include "Os/Unicode.h"

#include "Exception/SystemCall.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace 
{

namespace _Literal
{
	#define	U(literal)	const UnicodeString	literal(#literal)

	// 関数名関係

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
	U(SetEvent);
	U(WaitForSingleObject);
#endif
#endif
#ifdef SYD_OS_POSIX
	U(pthread_mutexattr_init);
	U(pthread_mutex_lock);
	U(pthread_mutex_trylock);
#endif

	#undef U
}

}

#ifdef SYD_OS_POSIX
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
//	FUNCTION public
//	Os::CriticalSection::CriticalSection --
//		クリティカルセクションを表すクラスのコンストラクター
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

CriticalSection::CriticalSection()
{
	// ミューテックス属性を初期化する
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにならないはず

	pthread_mutexattr_t		attr;
	const int stat = ::pthread_mutexattr_init(&attr);
	if (stat)

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::pthread_mutexattr_init, stat);

	// ミューテックス属性にロックを重ねがけできるような
	// ミューテックスの種別を設定する
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにならないはず

	(void) ::pthread_mutexattr_settype(
		&attr,
#ifdef SYD_OS_LINUX
		PTHREAD_MUTEX_RECURSIVE_NP
#else
		PTHREAD_MUTEX_RECURSIVE
#endif
		);

	// このミューテックス属性を使って、ミューテックスを初期化する
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_init(&_mutex, &attr);

	// 使用済のミューテックス属性を破棄する
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにならないはず

	(void) ::pthread_mutexattr_destroy(&attr);

#ifdef SYD_OS_LINUX		
	// マネージャに登録する
	
	CriticalSectionManager::add(this);
#endif		
	
}
#else
#endif
#endif

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0
//	FUNCTION public
//	Os::CriticalSection::~CriticalSection --
//		クリティカルセクションを表すクラスのデストラクター
//
//	NOTES
//		ロック待ちしているスレッドが存在する
//		クリティカルセクションをデストラクトすると、
//		ロック待ちしているスレッドの動作は、不定である
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

CriticalSection::~CriticalSection()
{
	// ロックされたままの
	// クリティカルセクションを破棄してはいけない
	//
	//【注意】	CRITICAL_SECTION のメンバを参照しているため、
	//			将来、動作しない可能性がある

#ifdef SYD_CHECK_CRITICALSECTION
	; _TRMEISTER_ASSERT(_criticalSection.LockCount == -1 &&
					 _criticalSection.RecursionCount == 0);
#endif

	// クリティカルセクションを破棄する

	::DeleteCriticalSection(&_criticalSection);
}
#endif
#endif

//	FUNCTION public
//	Os::CriticalSection::lock --
//		クリティカルセクションをロックする
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

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
void
CriticalSection::lock()
{
	const Thread::ID	self = Thread::self();

	if (::InterlockedIncrement(syd_reinterpret_cast<LPLONG>(&_count)) == 1) {

		// 誰もロックしていない

		_locker = self;
		_recursiveCount = 1;

	} else if (_locker == self)

		// 自分がすでにロックしていた

		++_recursiveCount;
	else {

		// 他のスレッドがロックしている

		Event::prepare(_event, false, false);

		// ロック待ちのためのイベントがシグナル化されるまで待つ

		if (::WaitForSingleObject(_event, INFINITE) == WAIT_FAILED) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::WaitForSingleObject, osErrno);
		}

		// ロックできた

		_locker = self;
		_recursiveCount = 1;
	}
}
#endif
#endif
#ifdef SYD_OS_POSIX
void
CriticalSection::lock()
{
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
	// 自分がすでにロックしているか調べる
	//
	//【注意】	ロックしているスレッドのスレッド ID は
	//			ロックしていなければ設定されていないので、
	//			まず、ロックされていることを調べる必要がある

	const Thread::ID	self = Thread::self();
	if (!(_count && Thread::equal(_locker, self))) {
#endif
		// 自分がロックしていないので、ミューテックスをロックする
		// 他のスレッドがすでにロックしていれば、
		// そのスレッドのロックがはずれるまで、待つ
		//
		//【注意】	引数はおかしくないはずなので、
		//			EINVAL や EFAULT のエラーにならないはず

		(void) ::pthread_mutex_lock(&_mutex);

#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
		// ロックしたスレッドの ID を記録する

		_locker = self;
	}

	// ロック数を 1 増やす
	
	++_count;
#endif
}
#endif

//	FUNCTION public
//	Os::CriticalSection::trylock --
//		クリティカルセクションのロックを試みる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ロックできた
//		false
//			他のスレッドがロックしているのでロックできなかった
//
//	EXCEPTIONS

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
bool
CriticalSection::trylock()
{
	const Thread::ID	self = Thread::self();

	// ロック数が 0 ならば、1 に設定する

	if (!::InterlockedCompareExchange(
			syd_reinterpret_cast<LONG* volatile>(&_count), 1, 0)) {

		// ロック数を 1 に設定できた

		_locker = self;
		_recursiveCount = 1;

	} else if (_locker == self) {

		// 自分がすでにロックしていた

		(void) ::InterlockedIncrement(syd_reinterpret_cast<LPLONG>(&_count));
		++_recursiveCount;

	} else

		// 他のスレッドがロックしている

		return false;

	// ロックできた

	return true;
}
#endif
#endif
#ifdef SYD_OS_POSIX
bool
CriticalSection::trylock()
{
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
	// 自分がすでにロックしているか調べる
	//
	//【注意】	ロックしているスレッドのスレッド ID は
	//			ロックしていなければ設定されていないので、
	//			まず、ロックされていることを調べる必要がある

	Thread::ID	self = Thread::self();
	if (!(_count && Thread::equal(_locker, self))) {
#endif
		// 自分がロックしていないので、ミューテックスのロックを試みる
		// 他のスレッドがロックしていれば、あきらめる
		//
		//【注意】	引数はおかしくないはずなので、
		//			EINVAL や EFAULT のエラーにならないはず

		if (::pthread_mutex_trylock(&_mutex) == EBUSY)

			// 他のスレッドがロックしていたので、あきらめる

			return false;

#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
		// ロックしたスレッドの ID を記録する

		_locker = self;
	}
	
	// ロック数を 1 増やす

	++_count;
#endif
	return true;
}
#endif

//	FUNCTION public
//	Os::CriticalSection::unlock --
//		クリティカルセクションのロックをはずす
//
//	NOTES
//		ロックしていないスレッドがロックをはずしても、
//		呼び出し側はエラーにならない
//		または、ロックした回数より多く、
//		ロックをはずしてもエラーにならない
//		ただし、ロック待ちしているスレッドの動作は、不定である
//
//	ARGUMENTS
//		int					n
//			指定されたとき
//				ロックをはずす回数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WIN98
void
CriticalSection::unlock()
{
	if (--_recursiveCount > 0)

		// まだ自分がロックしているので、ロック数を減らすだけ

		(void) ::InterlockedDecrement(syd_reinterpret_cast<LPLONG>(&_count));
	else {

		// 自分がしていたロックがこれですべてはずれる

		_locker = 0;

		// ロック数を減らす

		if (::InterlockedDecrement(syd_reinterpret_cast<LPLONG>(&_count)) > 0) {

			// ロック待ちしているスレッドがいる

			Event::prepare(_event, false, false);

			// ロック待ちのためのイベントをシグナル化して、
			// ロック待ちしているスレッドを起こす

			if (!::SetEvent(_event)) {

				// システムコールのエラーを表す例外を投げる

				const DWORD osErrno = ::GetLastError();
				_TRMEISTER_THROW2(
					Exception::SystemCall, _Literal::SetEvent, osErrno);
			}
		}
	}
}
#endif
#endif
#ifdef SYD_OS_POSIX
void
CriticalSection::unlock()
{
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
#else
	if (_count && Thread::equal(_locker, Thread::self()))
		if (!--_count)
#endif
			// 最後のミューテックスのロックをはずす
			//
			//【注意】	引数はおかしくないはずなので、
			//			EINVAL や EFAULT のエラーにならないはず
			//
			//【注意】	EPERM のエラーは無視する

			(void) ::pthread_mutex_unlock(&_mutex);
}
#endif

#ifdef OBSOLETE
void
CriticalSection::unlock(int n)
{
#ifdef SYD_OS_WINDOWS
#ifdef SYD_OS_WINNT4_0

	// 指定された回数ぶんロックされていない
	// クリティカルセクションをアンロックしてはいけない
	//
	//【注意】	CRITICAL_SECTION のメンバを参照しているため、
	//			将来、動作しない可能性がある

#ifdef SYD_CHECK_CRITICALSECTION
	; _TRMEISTER_ASSERT(_criticalSection.LockCount > (-1 + n) &&
					 _criticalSection.RecursionCount > (0 + n));
#endif

	// 指定された回数ぶん、クリティカルセクションから出る

	while (n-- > 0)
		::LeaveCriticalSection(&_criticalSection);
#endif
#ifdef SYD_OS_WIN98
	while (n-- > 0)
		unlock();
#endif
#endif
#ifdef SYD_OS_POSIX
#if defined(SYD_OS_LINUX) || defined(SYD_OS_SOL2_7)
	while (n-- > 0) {

		// 指定された回数ぶん、ミューテックスのロックをはずす
#else
	if (_count && Thread::equal(_locker, Thread::self())) {
		if (_count < n)
			n = _count;
		if (!(_count -= n))
#endif
			// 最後のミューテックスのロックをはずす
			//
			//【注意】	引数はおかしくないはずなので、
			//			EINVAL や EFAULT のエラーにならないはず
			//
			//【注意】	EPERM のエラーは無視する

			(void) ::pthread_mutex_unlock(&_mutex);
	}
#endif
}
#endif

#ifdef SYD_OS_POSIX
CriticalSection::~CriticalSection()
{
#ifdef SYD_OS_LINUX
	// マネージャから取り除く
	
	CriticalSectionManager::remove(this);
#endif
	
	
	// ミューテックスを破棄する
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_destroy(&_mutex);
	
}
#endif

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2018, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
