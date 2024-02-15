// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Mutex.h -- ミューテックス関連のクラス定義、関数宣言
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

#ifndef __TRMEISTER_OS_MUTEX_H
#define	__TRMEISTER_OS_MUTEX_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef OBSOLETE
#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif
#ifdef SYD_OS_POSIX
#endif

#include "Os/Module.h"
#ifdef SYD_OS_WINDOWS
#endif
#ifdef SYD_OS_POSIX
#include "Os/CriticalSection.h"
#endif

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

#ifdef SYD_OS_WINDOWS
//	CLASS
//	Os::Mutex -- ミューテックスを表すクラス
//
//	NOTES
//		異プロセススレッド間のミューテックスである
//		同じスレッドがなん度でもロックを重ねがけできる

class Mutex
{
public:
	// コンストラクター
	SYD_OS_FUNCTION
	Mutex();
	// デストラクター
	~Mutex();

	// ロックする
	SYD_OS_FUNCTION
	void					lock();
	// ロックを試みる
	SYD_OS_FUNCTION
	bool					trylock();
	// ロックをはずす
	SYD_OS_FUNCTION
	void					unlock();
	SYD_OS_FUNCTION
	void					unlock(int n);

private:
	// ミューテックスハンドル
	HANDLE					_handle;
};
#endif
#ifdef SYD_OS_POSIX
//	CLASS
//	Os::Mutex -- ミューテックスを表すクラス
//
//	NOTES
//		現状では、異プロセススレッド間のミューテックスは必要ないので
//		Os::CriticalSection をそのまま使う

class Mutex
	: public	CriticalSection
{};
#endif

#ifdef SYD_OS_WINDOWS
//	FUNCTION public
//	Os::Mutex::~Mutex -- ミューテックスを表すクラスのデストラクター
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
//		なし

inline
Mutex::~Mutex()
{
	if (_handle)

		// ミューテックスハンドルを破棄する

		(void) ::CloseHandle(_handle), _handle = 0;
}
#endif

_TRMEISTER_OS_END
_TRMEISTER_END
#endif

#endif	// __TRMEISTER_OS_MUTEX_H

//
// Copyright (c) 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
