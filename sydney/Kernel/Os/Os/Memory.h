// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Memory.h -- メモリ関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_MEMORY_H
#define	__TRMEISTER_OS_MEMORY_H

#if !defined(SYD_OS_WINDOWS) && !defined(SYD_OS_POSIX)
#error require #include "SyDefault.h"
#endif

#ifdef SYD_OS_WINDOWS
// #include <windows.h>
// #include <stdlib.h>
// #include <string.h>
#endif
#ifdef SYD_OS_POSIX
// #include <stdlib.h>
// #include <string.h>
// #include <sys/mman.h>
#endif

#include "Os/Module.h"

#include "ModTypes.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	NAMESPACE
//	Os::Memory -- メモリを表す名前空間
//
//	NOTES

namespace Memory
{
	//	TYPEDEF
	//	Os::Memory::Offset -- メモリ領域内のオフセット値を表す型
	//
	//	NOTES

	typedef	ModOffset		Offset;

	//	TYPEDEF
	//	Os::Memory::Size -- メモリ領域のサイズ値を表す型
	//
	//	NOTES

	typedef	ModSize			Size;

	//	TYPEDEF
	//	Os::Memory::LongSize -- メモリ領域のサイズ値を表す型
	//
	//	NOTES

	typedef	ModUInt64		LongSize;

	// メモリー内容のコピー
	SYD_OS_FUNCTION
	void*
	copy(void* dst, const void* src, Size size);
	// メモリー内容の移動
	SYD_OS_FUNCTION
	void*
	move(void* dst, const void* src, Size size);

	// メモリー内容を 0 埋めする
	SYD_OS_FUNCTION
	void*
	reset(void* p, Size size);
	// メモリー内容の特定値による初期化
	SYD_OS_FUNCTION
	void*
	set(void* p, unsigned char c, Size size);

	// メモリー内容の比較
	SYD_OS_FUNCTION
	int
	compare(const void* l, const void* r, Size size);

	// ヒープ領域からメモリーを確保する
	SYD_OS_FUNCTION
	void*
	allocate(Size size);
	// ヒープ領域上のメモリーを解放する
	SYD_OS_FUNCTION
	void
	free(void*& p);

	// 仮想アドレス空間上の領域に物理ストレージを割り当てる
	SYD_OS_FUNCTION
	void*
	map(Size size, bool reset);
	// 仮想アドレス空間上の領域の物理ストレージの割り当てを解除する
	SYD_OS_FUNCTION
	void
	unmap(void*& p, Size size);
}

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_MEMORY_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
