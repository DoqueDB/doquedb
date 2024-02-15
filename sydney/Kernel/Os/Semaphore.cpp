// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Semaphore.cpp -- セマフォ関連の関数定義
// 
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
#include <semaphore.h>
#endif
}

#ifdef SYD_OS_WINDOWS
#include "Os/Limits.h"
#endif
#include "Os/Semaphore.h"
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
	U(CreateSemaphore);
	U(ReleaseSemaphore);
	U(WaitForSingleObject);
#endif
#ifdef SYD_OS_POSIX
	U(sem_init);
	U(sem_post);
#endif

	#undef U
}

}

//	FUNCTION public
//	Os::Semaphore::Semaphore -- セマフォを表すクラスのコンストラクター
//
//	NOTES
//		セマフォのとり得る最大値は指定できない
//
//	ARGUMENTS
//		unsigned int		v
//			セマフォの初期値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Semaphore::Semaphore(unsigned int v)
{
#ifdef SYD_OS_WINDOWS

	// セマフォハンドルを生成する
	//
	//【注意】	POSIX と仕様をあわせるために
	//			セマフォのとり得る最大値には制限をつけない

	_handle = ::CreateSemaphore(0, v, Limits<int>::getMax(), 0);
	if (!_handle) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::CreateSemaphore, osErrno);
	}
#endif
#ifdef SYD_OS_POSIX

	// セマフォを初期化する

	if (const int stat = ::sem_init(&_semaphore, false, v))

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::sem_init, stat);
#endif
}

//	FUNCTION public
//	Os::Semaphore::lock -- 
//		セマフォの値が 0 より大きくなるまで待ち、大きくなったら 1 減らす
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
Semaphore::lock()
{
#ifdef SYD_OS_WINDOWS

	// セマフォの値が 0 より大きくなるまで待ち、大きくなったら 1 減らす

	if (::WaitForSingleObject(_handle, INFINITE) != WAIT_OBJECT_0) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::WaitForSingleObject, osErrno);
	}
#endif
#ifdef SYD_OS_POSIX

	// セマフォの値が 0 より大きくなるまで待ち、大きくなったら 1 減らす
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにはならないはず

	(void) ::sem_wait(&_semaphore);
#endif
}

//	FUNCTION public
//	Os::Semaphore::trylock --
//		セマフォの値が 0 より大きくか調べ、大きかったら 1 減らす
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			0 より大きかった
//		false
//			0 より大きくなかった
//
//	EXCEPTIONS

bool
Semaphore::trylock()
{
#ifdef SYD_OS_WINDOWS

	// セマフォの値が 0 より大きいか調べる

	switch (::WaitForSingleObject(_handle, 0)) {
	case WAIT_OBJECT_0:

		// 0 より大きかったので、1 減らした

		return true;

	case WAIT_TIMEOUT:

		// 0 より大きくなかった

		return false;
	}

	// システムコールのエラーを表す例外を投げる

	const DWORD osErrno = ::GetLastError();
	_TRMEISTER_THROW2(
		Exception::SystemCall, _Literal::WaitForSingleObject, osErrno);
#endif
#ifdef SYD_OS_POSIX

	// セマフォの値が 0 より大きくなるまで一瞬待ち、大きくなったら 1 減らす
	//
	//【注意】	引数はおかしくないはずなので、EINVAL のエラーにはならないはず

	return !::sem_trywait(&_semaphore);
#endif
}

//	FUNCTION public
//	Os::Semaphore::unlock -- セマフォの値を 1 増やす
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
Semaphore::unlock()
{
#ifdef SYD_OS_WINDOWS

	// セマフォの値を 1 増やす
	//
	//【注意】	POSIX と仕様をあわせるために常に 1 増やすことにする

	if (!::ReleaseSemaphore(_handle, 1, 0)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::ReleaseSemaphore, osErrno);
	}
#endif
#ifdef SYD_OS_POSIX

	// セマフォの値を 1 増やす

	if (const int stat = ::sem_post(&_semaphore))

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::sem_post, stat);
#endif
}

//
// Copyright (c) 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
