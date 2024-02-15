// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AsyncStatus.h -- 非同期操作の実行状況を表すクラス定義、関数宣言
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

#ifndef __TRMEISTER_OS_ASYNCSTATUS_H
#define	__TRMEISTER_OS_ASYNCSTATUS_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
#endif
#ifdef SYD_OS_POSIX
#endif

#include "Os/Module.h"
#include "Os/File.h"
#include "Os/Memory.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	CLASS
//	Os::AsyncStatus -- 非同期操作の実行状況を表すクラス
//
//	NOTES

class AsyncStatus
{
	friend class File;
public:
	// コンストラクター
	SYD_OS_FUNCTION
	AsyncStatus(File& file, File::Offset offset);
	// デストラクター
	~AsyncStatus();

	// 非同期操作が終了するまで待つ
	SYD_OS_FUNCTION
	Memory::Size			wait();
#ifdef OBSOLETE
	// 非同期操作が終了しているか
	SYD_OS_FUNCTION
	bool					isCompleted();
	SYD_OS_FUNCTION
	bool					isCompleted(Memory::Size& n);
#endif
private:
#ifdef SYD_OS_WINDOWS
	// 非同期操作対象のハンドル
	HANDLE&					_handle;
	// オーバーラップ構造体
	OVERLAPPED				_overlapped;
#endif
};

//	FUNCTION public
//	Os::AsyncStatus::~AsyncStatus --
//		非同期操作の実行状況を表すクラスのデストラクター
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

inline
AsyncStatus::~AsyncStatus()
{
#ifdef SYD_OS_WINDOWS
	if (_overlapped.hEvent)

		// イベントハンドルを破棄する
		//
		//【注意】	エラーが起きても無視する

		::CloseHandle(_overlapped.hEvent), _overlapped.hEvent = 0;
#endif
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_ASYNCSTATUS_H

//
// Copyright (c) 2000, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
