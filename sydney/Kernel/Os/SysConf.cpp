// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SysConf.cpp -- システム設定関連の関数定義
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
#include <float.h>
#include <limits.h>
#endif
#ifdef SYD_OS_POSIX
#include <assert.h>
#include <float.h>
#include <limits.h>
#include <string.h>
#include <sys/utsname.h>
#include <unistd.h>
#endif
}

#include "Os/Assert.h"
#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Limits.h"
#include "Os/SysConf.h"
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
	U(GetComputerName);
#endif
#ifdef SYD_OS_POSIX
	U(uname);
#endif

	#undef U
}

namespace _SysConf
{
	// 以下の情報を保護するためのラッチ
	CriticalSection			_latch;

	namespace _HostName
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Host::Name			_value;
	}

	namespace _OpenMax
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		unsigned int		_value;
	}

	namespace _PageSize
	{
		// 設定値をはじめて取得しようとしているか
		bool				_first = true;
		// 取得した設定値
		Memory::Size		_value;
	}
}

}

//	FUNCTION
//	Os::SysConf::HostName::get --
//		呼び出したプロセスが実行されているホストの名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたホスト名
//
//	EXCEPTIONS

const Host::Name&
SysConf::HostName::get()
{
	if (_SysConf::_HostName::_first) {

		// 呼び出したプロセスが実行されている
		// ホストの名前が求められていなければ、求める
		//
		//【注意】	ラッチするまでに他のスレッドが
		//			求める可能性があるので、
		//			ラッチ後に必ずもう一度確認する必要がある
		//
		//【注意】	常にラッチすることをさけている

		AutoCriticalSection	latch(_SysConf::_latch);

		if (_SysConf::_HostName::_first) {
#ifdef SYD_OS_WINDOWS

			// 呼び出したプロセスが実行されているホストの名前を得る
			//
			//【注意】	NetBIOS での名前を求めている
			//
			//【注意】	::GetComputerName は得られたホスト名の
			//			終端文字を除いた文字数を第 2 引数に設定する

			TCHAR	buf[MAX_COMPUTERNAME_LENGTH + 1];
			DWORD	n = sizeof(buf) / sizeof(TCHAR);

			if (!::GetComputerName(buf, &n)) {

				// システムコールのエラーを表す例外を投げる

				DWORD osErrno = ::GetLastError();
				_TRMEISTER_THROW2(
					Exception::SystemCall, _Literal::GetComputerName, osErrno);
			}

			_SysConf::_HostName::_value =
				Host::Name(static_cast<const Ucs2*>(buf));
#endif
#ifdef SYD_OS_POSIX

			// 現在の OS を識別する名前情報を得る

			struct utsname	buf;
			if (::uname(&buf) == -1) {

				// システムコールのエラーを表す例外を投げる

				const int osErrno = errno;
				_TRMEISTER_THROW2(
					Exception::SystemCall, _Literal::uname, osErrno);
			}

			_SysConf::_HostName::_value =
				Host::Name(buf.nodename, ModOs::Process::getEncodingType());
#endif
			// 次はシステム情報を求めることなく、
			// 記憶している値をそのまま返すようにする

			_SysConf::_HostName::_first = false;
		}
	}
	return _SysConf::_HostName::_value;
}

//	FUNCTION
//	Os::SysConf::OpenMax::get --
//		呼び出しプロセスのオープンできるファイル数の最大値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたファイル数
//
//	EXCEPTIONS

unsigned int
SysConf::OpenMax::get()
{
	if (_SysConf::_OpenMax::_first) {

		// プロセスのオープンできる
		// ファイル数の最大値が求められていなければ、求める
		//
		//【注意】	ラッチするまでに他のスレッドが
		//			求める可能性があるので、
		//			ラッチ後に必ずもう一度確認する必要がある
		//
		//【注意】	常にラッチすることをさけている

		AutoCriticalSection	latch(_SysConf::_latch);

		if (_SysConf::_OpenMax::_first) {
#ifdef SYD_OS_WINDOWS

			// MSDN Library によれば、Windows のひとつのプロセスで
			// 生成可能なハンドル数は 65,536 である
			//
			// そのすべてをファイルハンドルとして使用できるとは
			// 思えないので、半分としておく

			_SysConf::_OpenMax::_value = 65536 / 2;
#endif
#ifdef SYD_OS_POSIX

			// システム設定のプロセスが
			// オープンできるファイル数の最大値を求める

			_SysConf::_OpenMax::_value = sysconf(_SC_OPEN_MAX);
#endif
			// 次はシステム情報を求めることなく、
			// 記憶している値をそのまま返すようにする

			_SysConf::_OpenMax::_first = false;
		}
	}
	return _SysConf::_OpenMax::_value;
}

//	FUNCTION
//	Os::SysConf::PageSize::get -- システムのメモリページサイズを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたページサイズ(B 単位)
//
//	EXCEPTIONS
//		なし

Memory::Size
SysConf::PageSize::get()
{
	if (_SysConf::_PageSize::_first) {

		// システムのメモリページサイズが求められていなければ、求める
		//
		//【注意】	ラッチするまでに他のスレッドが
		//			求める可能性があるので、
		//			ラッチ後に必ずもう一度確認する必要がある
		//
		//【注意】	常にラッチすることをさけている

		AutoCriticalSection	latch(_SysConf::_latch);

		if (_SysConf::_PageSize::_first) {
#ifdef SYD_OS_WINDOWS

			// システム情報を求め、その中のページサイズを得る

			SYSTEM_INFO	info;
			::GetSystemInfo(&info);

			_SysConf::_PageSize::_value = info.dwPageSize;
#endif
#ifdef SYD_OS_POSIX

			// システムのページサイズを求め、返す

			_SysConf::_PageSize::_value = ::getpagesize();
#endif
			// 次はシステム情報を求めることなく、
			// 記憶している値をそのまま返すようにする

			_SysConf::_PageSize::_first = false;
		}
	}
	return _SysConf::_PageSize::_value;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
