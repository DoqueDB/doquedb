//-*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Timer.h -- タイマーのクラス定義、関数宣言
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

#ifndef __TRMEISTER_OS_TIMER_H
#define	__TRMEISTER_OS_TIMER_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
#endif
#ifdef SYD_OS_POSIX
// #include <time.h>
#endif

#include "Os/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::Timer -- タイマーを表すクラス
//
//	NOTES
//		タイマー(ストップウオッチ)クラスである。分解能は1ms。
//		start から end までの時間をミリ秒単位で保持する
//		reset を呼ばずに start を再び実行した場合、時間は加算される

class SYD_OS_FUNCTION Timer
{
public:
	// コンストラクター
	Timer();
	// デストラクター
	virtual ~Timer();

	// 計測を開始する
	void start();
	// 計測を終了し、経過時間を加算する
	void end();

	// リセットする
	void reset()
		{ m_total = 0; }

	// 経過時間を取り出す(ミリ秒)
	unsigned int get() const
		{ return m_total; }

private:
	// 開始時間
#ifdef SYD_OS_WINDOWS
	LARGE_INTEGER		m_start;
	LARGE_INTEGER		m_frequency;
#endif
#ifdef SYD_OS_POSIX
	struct timespec		m_start;
#endif

	// 経過時間
	unsigned int		m_total;
};

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_TIMER_H

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
