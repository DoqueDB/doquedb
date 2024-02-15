// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Event.cpp -- イベント関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
#include <assert.h>
#endif
#ifdef SYD_OS_POSIX
#include <assert.h>
#include <pthread.h>
#include <sys/time.h>
#endif
}

#include "Os/Assert.h"
#include "Os/Event.h"
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
	U(CreateEvent);
	U(ResetEvent);
	U(SetEvent);
	U(WaitForSingleObject);
#endif
#ifdef SYD_OS_POSIX
	U(pthread_cond_init);
	U(pthread_cond_timedwait);
	U(pthread_cond_wait);
#endif

	#undef U
}

}

#ifdef SYD_OS_POSIX
//	FUNCTION public
//	Os::Event::Event -- イベントを表すクラスのコンストラクター
//
//	NOTES
//		
//	ARGUMENTS
//		Category::Value		category
//			生成するイベントの種類
//		bool				signaled
//			true
//				シグナル化されたイベントを生成する
//			false または指定されないとき
//				非シグナル化されたイベントを生成する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Event::Event(Category::Value category, bool signaled)
	: _category(category),
	  _waiters(0),
	  _signaled(false)
{
	// 条件変数を保護するためのミューテックスを初期化する
	//
	//【注意】	同じスレッドが多重でロックすることはないので、
	//			デフォルトの動作のミューテックスを初期化する
	//
	//【注意】	ミューテックス属性を与えないと、
	//			デフォルトで同プロセススレッド間ミューテックスになる
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_init(&_mutex, 0);

	// 条件変数を初期化する
	//
	//【注意】	引数はおかしくないので、
	//			EINVAL や EBUSY のエラーにならないはず

	if (const int stat = ::pthread_cond_init(&_condition, 0))

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::pthread_cond_init, stat);

	if (signaled)

		// シグナル化されたイベントを生成するので、
		// ここでシグナル化しておく

		set();
}
#endif

//	FUNCTION public
//	Os::Event::set -- イベントをシグナル化する
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
void
Event::set()
{
	// イベントを操作する準備をする

	prepare();

	// イベントをシグナル化する

	if (!::SetEvent(_handle)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::SetEvent, osErrno);
	}
}
#endif
#ifdef SYD_OS_POSIX
void
Event::set()
{
	// 条件変数を保護するミューテックスをロックする
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_lock(&_mutex);

	if (!_signaled)

		// 現時点でシグナル化されていない

		if (_waiters) {
			if (_category == Category::ManualReset)

				// 待っているスレッドがいるので、
				// 手動リセットイベントのときのみ
				// シグナル化されたことをおぼえておく

				_signaled = true;

			if (_category == Category::WakeUpOnlyOne)

				// 待っているスレッドのうち、ひとつだけ起こす

				(void) ::pthread_cond_signal(&_condition);
			else

				// 待っているスレッドをすべて起こす

				(void) ::pthread_cond_broadcast(&_condition);
		} else

			// 誰も待っていないので、シグナル化されたままとする

			_signaled = true;

	// 条件変数を保護するミューテックスのロックをはずす
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_unlock(&_mutex);
}
#endif

//	FUNCTION public
//	Os::Event::reset -- イベントを非シグナル化する
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
void
Event::reset()
{
	// イベントを操作する準備をする

	prepare();

	// イベントを非シグナル化する

	if (!::ResetEvent(_handle)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::ResetEvent, osErrno);
	}
}
#endif
#ifdef SYD_OS_POSIX
void
Event::reset()
{
	// 条件変数を保護するミューテックスをロックする
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_lock(&_mutex);

	// シグナル化されていることを忘れる

	_signaled = false;

	// 条件変数を保護するミューテックスのロックをはずす
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_unlock(&_mutex);
}
#endif

//	FUNCTION public
//	Os::Event::wait -- イベントがシグナル化するまで待つ
//
//	NOTES
//		
//	ARGUMENTS
//		unsigned int		msec
//			指定されたとき
//				最大待ち時間(単位ミリ秒)
//			指定されないとき
//				永久に待つ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

#ifdef SYD_OS_POSIX
void
Event::wait()
{
	// 条件変数を保護するミューテックスをロックする
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_lock(&_mutex);

	if (!_signaled) {

		// 現時点でシグナル化されていない

		// 待っているスレッド数を 1 増やす

		++_waiters;

		// シグナル化されるまで待つ
		//
		//【注意】	シグナル化を待っている間、
		//			条件変数を保護するための
		//			ミューテックスのロックは自動的にはずれ、
		//			待ちが終わった時点で、自動的にかかる

		if (const int stat = ::pthread_cond_wait(&_condition, &_mutex)) {

			// 待っているスレッド数を 1 減らす

			--_waiters;

			// 条件変数を保護するミューテックスのロックをはずす

			(void) ::pthread_mutex_unlock(&_mutex);

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::pthread_cond_wait, stat);
		}

		// 待ってからシグナル化された

		// 待っているスレッド数を 1 減らす

		--_waiters;

	} else if (_category != Category::ManualReset)

		// 手動リセットイベントでないので、非シグナル化する

		_signaled = false;

	// 条件変数を保護するミューテックスのロックをはずす
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_unlock(&_mutex);
}
#endif

#ifdef SYD_OS_WINDOWS
bool
Event::wait(unsigned int msec)
{
	// イベントを操作する準備をする

	prepare();

	// イベントがシグナル化されるまで待つ

	switch (::WaitForSingleObject(_handle, msec)) {
	case WAIT_OBJECT_0:

		// シグナル化された

		if (_category == Category::WakeUpAll)

			// 手動リセットイベントを使っているが、
			// 待っているスレッドをすべて起こし、
			// かつスレッドが起きたときにリセットされるイベントなので、
			// ここでリセットしておく
			//
			//【注意】	ここでリセットしても、
			//			シグナル化されたときに待っていた
			//			他のスレッドもちゃんと起きているはず

			reset();

		return true;

	case WAIT_TIMEOUT:

		// 指定された時間待ったが、シグナル状態にならなかった

		return false;
	}

	// システムコールのエラーを表す例外を投げる

	const DWORD osErrno = ::GetLastError();
	_TRMEISTER_THROW2(
		Exception::SystemCall, _Literal::WaitForSingleObject, osErrno);
}
#endif
#ifdef SYD_OS_POSIX
bool
Event::wait(unsigned int msec)
{
	// 条件変数を保護するミューテックスをロックする
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_lock(&_mutex);

	if (!_signaled) {

		// 現時点でシグナル化されていない

		// 待っているスレッド数を 1 増やす

		++_waiters;

		// 指定された時間だけ、シグナル化されるのを待つ

		struct timeval	tv;
		::gettimeofday(&tv, 0);

		struct timespec	abs;
		abs.tv_sec = tv.tv_sec + msec / 1000;
		msec = msec % 1000 + tv.tv_usec / 1000;
		abs.tv_sec += msec / 1000;
		abs.tv_nsec = (msec % 1000) * 1000000 + (tv.tv_usec % 1000) * 1000;

		if (const int stat =
			::pthread_cond_timedwait(&_condition, &_mutex, &abs)) {

			// 待っているスレッド数を 1 減らす

			--_waiters;
			
			// 条件変数を保護するミューテックスのロックをはずす

			(void) ::pthread_mutex_unlock(&_mutex);

			switch (stat) {
			case 0:
			case ETIMEDOUT:

				// 指定された時間待ったが、シグナル化されなかった
				//
				//【注意】	ロックを得るまでに処理されていない
				//			シグナル化の回数が 0 になったときも、
				//			同様とみなす

				return false;
			}

			// システムコールのエラーを表す例外を投げる

			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::pthread_cond_timedwait, stat);
		}

		// 待ってからシグナル化された

		// 待っているスレッド数を 1 減らす

		--_waiters;

	} else if (_category != Category::ManualReset)

		// 手動リセットイベントでないので、非シグナル化する

		_signaled = false;

	// 条件変数を保護するミューテックスのロックをはずす
	//
	//【注意】	引数はおかしくないはずなので、
	//			EINVAL や EFAULT のエラーにならないはず

	(void) ::pthread_mutex_unlock(&_mutex);

	return true;
}
#endif

#ifdef SYD_OS_WINDOWS
//	FUNCTION private
//	Os::Event::prepare -- イベントハンドルが生成されていなければ、生成する
//
//	NOTES
//
//	ARGUMENTS
//		HANDLE&				handle
//			ここにイベントハンドルが設定されていなければ、生成し、設定する
//		bool				manual
//			true
//				手動リセットイベントを生成する
//			false
//				自動リセットイベントを生成する
//		bool				signaled
//			true
//				シグナル化されたイベントハンドルを生成する
//			false
//				非シグナル化されたイベントハンドルを生成する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Event::prepare(HANDLE& handle, bool manual, bool signaled)
{
	if (!handle) {

		// イベントハンドルが生成されていないので、生成する

		HANDLE src = ::CreateEvent(0, manual, signaled, 0);
		if (!src) {

			// システムコールのエラーを表す例外を投げる

			const DWORD osErrno = ::GetLastError();
			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::CreateEvent, osErrno);
		}

		// 生成したイベントハンドルを設定する

		if (::InterlockedCompareExchangePointer(
				syd_reinterpret_cast<VOID* volatile *>(&handle), src, 0))

			// イベントハンドルを生成している間に、
			// 他のスレッドにより生成、設定されてしまったので、
			// 今生成したハンドルは破棄する

			(void) ::CloseHandle(src);
	}
}
#endif

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
