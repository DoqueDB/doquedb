// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Message.cpp -- メッセージをプリントする
// 
// Copyright (c) 2000, 2001, 2014, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/Message.h"
#include "Common/UnicodeString.h"
#include "Os/Thread.h"
#ifdef SYD_OS_LINUX
#include <sys/syscall.h>
#include <sys/types.h>
#endif

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//
//	FUNCTION public static
//	Common::Message::getBaseName -- ファイル名からディレクトリ部分を除く
//
//	NOTES
//	ファイル名からディレクトリ部分を除く。
//
//	ARGUMENTS
//	const ModUnicodeChar* pszSrcName_
//		取除く対象の文字列
//
//	RETURN
//	const ModUnicodeChar*
//		ディレクトリ等を除いた部分の先頭のポインタ
//
//	EXCEPTIONS
//	なし
//
const ModUnicodeChar*
Message::getBaseName(const ModUnicodeChar* pszSrcName_)
{
	const ModUnicodeChar* p = pszSrcName_;
	if (p)
	{
		const ModUnicodeChar* cp = &UnicodeChar::usNull;
		for (; *p != 0; p++)
		{
			switch (*p)
			{
			case UnicodeChar::usSlash: 		// '/'
			case UnicodeChar::usBackSlash:	// '\\'
				cp = p;
				break;
			default:
				;
			}
		}
		if (cp)
		{
			p = cp+1;
		}
		else
		{
			p = pszSrcName_;
		}
	}
	return p;
}

//
//	FUNCTION public static
//	Common::Message::getBaseName -- ファイル名からディレクトリ部分を除く
//
//	NOTES
//	ファイル名からディレクトリ部分を除く。
//
//	ARGUMENTS
//	const char* pszSrcName_
//		取除く対象の文字列
//
//	RETURN
//	const char*
//		ディレクトリ等を除いた部分の先頭のポインタ
//
//	EXCEPTIONS
//	なし
//
const char*
Message::getBaseName(const char* pszSrcName_)
{
	const char* p = pszSrcName_;
	if (p)
	{
		const char* cp = 0;
		for (; *p != 0; p++)
		{
			switch (*p)
			{
			case '/':
			case '\\':
				cp = p;
				break;
			default:
				;
			}
		}
		if (cp)
		{
			p = cp+1;
		}
		else
		{
			p = pszSrcName_;
		}
	}
	return p;
}

//
//	FUNCTION public static
//	Common::Message::getThreadID -- 表示用のスレッドIDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	unsigned int
//		スレッドID
//
//	EXCEPTIONS
//
unsigned int
Message::getThreadID()
{
		// POSIXの場合、Os::Thread::self は pthread_t を返すが、
		// Linux はカーネルが管理している LWP と pthread_t の数値が違い
		// デバッグしづらい
		// よって、ログにはLWPを出力する
	
#ifdef SYD_OS_LINUX
		return ::syscall(SYS_gettid);
#else
		return Os::Thread::self();
#endif
}

//
//	Copyright (c) 2000, 2001, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
