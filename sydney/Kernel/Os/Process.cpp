// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Process.cpp -- プロセス関連の関数定義
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
#include <assert.h>
#include <io.h>
#endif
#ifdef SYD_OS_POSIX
#include <assert.h>
#include <unistd.h>
#include <sys/stat.h>
#endif
}

#include "Os/Assert.h"
#ifdef SYD_OS_WINDOWS
#include "Os/Memory.h"
#endif
#include "Os/Process.h"
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
	U(GetCurrentDirectory);
#endif
#ifdef SYD_OS_POSIX
	U(getcwd);
#endif

	#undef U
}

#ifdef SYD_OS_SOLARIS
	const int _iGetcwdInitialSize = 256;
	const int _iGetcwdStep = 64;
#endif

}

//	FUNCTION public
//	Os::Process::getCurrentDirectory --
//		カレントワーキングディレクトリの絶対パス名を取得する
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた絶対パス名
//
//	EXCEPTIONS

Path
Process::getCurrentDirectory()
{
#ifdef SYD_OS_WINDOWS

	// まず、カレントワーキングディレクトリの
	// 絶対パス名を格納するために必要な文字数を求める
	//
	//【注意】	MSDN Library 2002/10 には、求められる値は絶対パスを
	//			格納するために必要なバイト数であると書かれているが、
	//			必要な文字数の誤りである
	//
	//【注意】	求められる文字数は終端文字を含む

	const DWORD n = ::GetCurrentDirectory(0, 0);
	if (!n) {

		// システムコールのエラーを表す例外を投げる

		DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::GetCurrentDirectory, osErrno);
	}

	void* buf = 0;

	try {
		// 求めたサイズの領域を確保する

		buf = Memory::allocate(sizeof(TCHAR) * n);
		; _TRMEISTER_ASSERT(buf);

		// カレントワーキングディレクトリの絶対パス名を得る

		if (!::GetCurrentDirectory(n, static_cast<TCHAR*>(buf))) {

			// システムコールのエラーを表す例外を投げる

			DWORD osErrno = ::GetLastError();
			_TRMEISTER_THROW2(
				Exception::SystemCall, _Literal::GetCurrentDirectory, osErrno);
		}

		Path path(static_cast<const TCHAR*>(buf));
		Memory::free(buf);

		return path;

	} catch (...) {

		Memory::free(buf);
		_TRMEISTER_RETHROW;
	}
#endif
#ifdef SYD_OS_POSIX
    //getcwd(0,0) will crash on solaris. 
    //It always fail if the second parameter is 0 according to its spec.
	//The pathname length couldn't be got. This length is not limited on Solaris.
#ifdef SYD_OS_SOLARIS	
	int size = _iGetcwdInitialSize;
	char* p = NULL;
retry:
	if ((p = ::getcwd(0,size )) == NULL) {
	   if(errno==ERANGE){
			   size += _iGetcwdStep;
			   goto retry;
		   }
		 }
	#else
		char* p = ::getcwd(0,0);
	#endif	
	
	if (!p) {

		// システムコールのエラーを表す例外を投げる

		const int osErrno = errno;
		_TRMEISTER_THROW2(Exception::SystemCall, _Literal::getcwd, osErrno);
	}

	try {
		Path path(p);
		::free(p);

		return path;

	} catch (...) {

		if (p)
			::free(p);
		_TRMEISTER_RETHROW;
	}
#endif
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
