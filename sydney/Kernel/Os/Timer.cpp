// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Timer.cpp -- タイマークラス
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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
#include <time.h>
#endif
}

#include "Os/Timer.h"
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
	U(QueryPerformanceCounter);
	U(QueryPerformanceFrequency);
#endif
#ifdef SYD_OS_POSIX
	U(clock_gettime);
#endif

	#undef U
}

}

//
//	FUNCTION public
//	Os::Timer::Timer -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Timer::Timer()
	: m_total(0)
{
#ifdef SYD_OS_WINDOWS
	if (::QueryPerformanceFrequency(&m_frequency) == 0)
	{
		DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
						  _Literal::QueryPerformanceFrequency, osErrno);
	}
#endif
}

//
//	FUNCTION public
//	Os::Timer::~Timer -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
Timer::~Timer()
{
}

//
//	FUNCTION public
//	Os::Timer::start -- 計測を開始する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Timer::start()
{
#ifdef SYD_OS_WINDOWS
	if (::QueryPerformanceCounter(&m_start) == 0)
	{
		DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
						  _Literal::QueryPerformanceCounter, osErrno);
	}
#endif
#ifdef SYD_OS_POSIX
	if (::clock_gettime(CLOCK_REALTIME, &m_start) != 0)
	{
		int osErrno = errno;
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::clock_gettime, osErrno);
	}
#endif
}

//
//	FUNCTION public
//	Os::Timer::end -- 計測を終了し、経過時間を加算する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Timer::end()
{
#ifdef SYD_OS_WINDOWS
	LARGE_INTEGER e;
	if (::QueryPerformanceCounter(&e) == 0)
	{
		DWORD osErrno = ::GetLastError();
		_TRMEISTER_THROW2(Exception::SystemCall,
						  _Literal::QueryPerformanceCounter, osErrno);
	}
	LONGLONG t
		= ((e.QuadPart - m_start.QuadPart) * 1000) / m_frequency.QuadPart;
	m_total += static_cast<DWORD>(t);
#endif
#ifdef SYD_OS_POSIX
	struct timespec ts;
	if (::clock_gettime(CLOCK_REALTIME, &ts) != 0)
	{
		int osErrno = errno;
		_TRMEISTER_THROW2(
			Exception::SystemCall, _Literal::clock_gettime, osErrno);
	}
	m_total += static_cast<unsigned int>(
		(ts.tv_sec - m_start.tv_sec) * 1000);			// 秒 -> ミリ秒
	m_total += static_cast<unsigned int>(
		(ts.tv_nsec - m_start.tv_nsec) / 1000 /1000);	// ナノ秒 -> ミリ秒
#endif
}

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
