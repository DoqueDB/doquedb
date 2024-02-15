// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Thread.h -- スレッド関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_THREAD_H
#define	__TRMEISTER_OS_THREAD_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif
#ifdef SYD_OS_POSIX
// #include <pthread.h>
#endif

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	NAMESPACE
//	Os::Thread -- スレッドを表す名前空間
//
//	NOTES

namespace Thread
{
	//	TYPEDEF
	//	Os::Thread::ID -- スレッド ID を表す型	
	//
	//	NOTES
#ifdef SYD_OS_WINDOWS
	typedef	DWORD			ID;
#endif
#ifdef SYD_OS_POSIX
	typedef	pthread_t		ID;
#endif

	//	CLASS
	//	Os::Thread::Priority -- スレッドの実行優先度を表すクラス
	//
	//	NOTES

	struct Priority
	{
		//	ENUM
		//	Os::Thread::Priority::Value -- スレッドの実行優先度を表す値の型
		//
		//	NOTES

		enum Value
		{	
			TimeCritical =	0,
			Highest,
			AboveNormal,
			Normal,
			BelowNormal,
			Lowest,
			Idle,
			ValueNum
		};
	};

	// 呼び出したスレッドのスレッド ID を得る
	inline
	ID						self();
	// 2 つのスレッド ID が等しいか
	inline
	bool					equal(ID l, ID r);

	// スレッドの実行優先度を設定する
	inline
	void					setPriority(Priority::Value v);
}

//	FUNCTION public
//	Os::Thread::self -- 自分自身のスレッド ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自分自身のスレッド ID
//
//	EXCEPTIONS
//		なし

inline
Thread::ID
Thread::self()
{
#ifdef SYD_OS_WINDOWS
	return ::GetCurrentThreadId();
#endif
#ifdef SYD_OS_POSIX
	return ::pthread_self();
#endif
}

//	FUNCTION public
//	Os::Thread::equal -- 2 つのスレッド ID が等しいか調べる
//
//	NOTES
//
//	ARGUMENTS
//		Thread::ID			l
//			r と比較するスレッド ID
//		Thread::ID			r
//			l と比較するスレッド ID
//
//	RETURN
//		true
//			等しい
//		false
//			等しくない
//
//	EXCEPTIONS
//		なし

inline
bool
Thread::equal(Thread::ID l, Thread::ID r)
{
#ifdef SYD_OS_WINDOWS
	return l == r;
#endif
#ifdef SYD_OS_POSIX
	return ::pthread_equal(l, r);
#endif
}

//	FUNCTION public
//	Os::Thread::setPriority -- 呼び出しスレッドの優先度を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Os::Thread::Priority::Value	priority
//			新しい優先度
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Thread::setPriority(Priority::Value priority)
{
#ifdef SYD_OS_WINDOWS
	const int	table[] =
	{
		THREAD_PRIORITY_TIME_CRITICAL,
		THREAD_PRIORITY_HIGHEST,
		THREAD_PRIORITY_ABOVE_NORMAL,
		THREAD_PRIORITY_NORMAL,
		THREAD_PRIORITY_BELOW_NORMAL,
		THREAD_PRIORITY_LOWEST,
		THREAD_PRIORITY_IDLE
	};

	// 呼び出しスレッドの相対優先度を設定する
	//
	//【注意】	エラーは無視する

	(void) ::SetThreadPriority(::GetCurrentThread(), table[priority]);
#endif
#ifdef SYD_OS_POSIX

	/* POSIX 版は未実装 */

#endif
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_THREAD_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
