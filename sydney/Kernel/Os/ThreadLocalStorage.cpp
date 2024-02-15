// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ThreadLocalStorage.cpp -- スレッドローカル記憶域関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifdef OBSOLETE
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

#include "Os/Assert.h"
#include "Os/ThreadLocalStorage.h"
#include "Os/Unicode.h"

#include "Exception/SystemCall.h"

_TRMEISTER_USING
_TRMEISTER_OS_USING

namespace 
{

namespace _Literal
{
	#define	U(literal)	const UnicodeString	literal(#literal);

	// 関数名関係

#ifdef SYD_OS_WINDOWS
	U(TlsAlloc);
	U(TlsSetValue);
#endif
#ifdef SYD_OS_POSIX
	U(pthread_key_create);
	U(pthread_setspecific);
#endif

	#undef U
}

}

//	FUNCTION public
//	Os::ThreadLocalStorage::ThreadLocalStorage --
//		スレッドローカル記憶域を表すクラスのコンストラクター
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
ThreadLocalStorage::ThreadLocalStorage()
{
	// スレッドローカル記憶域を確保する
	//
	//【注意】	Windows 2000 では、同時に最大で 1088 個、
	//			Windows NT 4.0 では、同時に最大で 64 個しか確保できない

	_index = ::TlsAlloc();
	if (_index == 0xffffffff) {

		// システムコールのエラーを表す例外を投げる

		DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::TlsAlloc, osErrno);
	}
}
#endif
#ifdef SYD_OS_POSIX
ThreadLocalStorage::ThreadLocalStorage()
{
	// スレッド固有データキーを初期化する

	if (const int stat = ::pthread_key_create(&_key, 0))

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::pthread_key_create, stat);
}
#endif

//	FUNCTION public
//	Os::ThreadLocalStorage::setValue -- スレッドローカル記憶域に値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		void*			value
//			設定する値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

#ifdef SYD_OS_WINDOWS
void
ThreadLocalStorage::setValue(void* value)
{
	// スレッドローカル記憶域に値を設定する

	if (!::TlsSetValue(_index, value)) {

		// システムコールのエラーを表す例外を投げる

		DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::TlsSetValue, osErrno);
	}
}
#endif
#ifdef SYD_OS_POSIX
void
ThreadLocalStorage::setValue(void* value)
{
	// スレッド固有データキーに値を設定する

	if (const int stat = ::pthread_setspecific(_key, value))

		// システムコールのエラーを表す例外を投げる

		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::pthread_setspecific, stat);
}
#endif
#endif

//
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
