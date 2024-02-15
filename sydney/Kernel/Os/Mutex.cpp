// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Mutex.cpp -- ミューテックス関連の関数定義
// 
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_OS_WINDOWS
extern "C"
{
#include <windows.h>
}

#include "Os/Mutex.h"
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

	U(CreateMutex);
	U(ReleaseMutex);
	U(WaitForSingleObject);

	#undef U
}

}

//	FUNCTION public
//	Os::Mutex::Mutex -- ミューテックスを表すクラスのコンストラクター
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

Mutex::Mutex()
	: _handle(0)
{
	// ミューテックスハンドルを生成する

	_handle = ::CreateMutex(0, false, 0);
	if (!_handle) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::CreateMutex, osErrno);
	}
}

//	FUNCTION public
//	Os::Mutex::lock -- ミューテックスをロックする
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
Mutex::lock()
{
	// ミューテックスがシグナル化されるまで待つ

	if (::WaitForSingleObject(_handle, INFINITE) == WAIT_FAILED) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::WaitForSingleObject, osErrno);
	}
}

//	FUNCTION public
//	Os::Mutex::trylock -- ミューテックスのロックを試みる
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

bool
Mutex::trylock()
{
	// ミューテックスがシグナル化されているか調べる
	// シグナル化されていなければ、待たない

	switch (::WaitForSingleObject(_handle, 0)) {
	case WAIT_OBJECT_0:

		// シグナル化された

		return true;

	case WAIT_TIMEOUT:

		// シグナル化されていない

		return false;
	}

	// システムコールのエラーを表す例外を投げる

	const DWORD osErrno = ::GetLastError();
	_TRMEISTER_THROW2(
		Exception::SystemCall, _Literal::WaitForSingleObject, osErrno);
}

//	FUNCTION public
//	Os::Mutex::unlock -- ミューテックスのロックをはずす
//
//	NOTES
//		ロックしていないスレッドがロックをはずしても、
//		呼び出し側はエラーにならない
//		または、ロックした回数より多く、
//		ロックをはずしてもエラーにならない
//		ただし、ロック待ちしているスレッドの動作は、不定である
//
//	ARGUMENTS
//		int			n
//			指定されたとき
//				ロックをはずす回数
//			指定されないとき
//				1 が指定されたものとみなす
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Mutex::unlock()
{
	// ミューテックスをシグナル化する

	if (!::ReleaseMutex(_handle)) {

		// システムコールのエラーを表す例外を投げる

		const DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::ReleaseMutex, osErrno);
	}
}

void
Mutex::unlock(int n)
{
	// 指定された回数ぶん、アンロックする

	while (n-- > 0)
		unlock();
}
#endif
#endif

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
