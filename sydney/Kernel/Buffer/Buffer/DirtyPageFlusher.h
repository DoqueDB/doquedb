// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DirtyPageFlusher.h --
//		ダーティバッファページ書き込みスレッドに関するクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_BUFFER_DIRTYPAGEFLUSHER_H
#define	__SYDNEY_BUFFER_DIRTYPAGEFLUSHER_H

#include "Buffer/Module.h"
#include "Buffer/DaemonThread.h"

#include "ModTimeSpan.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

//	CLASS
//	Buffer::DirtyPageFlusher --
//		ダーティバッファページ書き込みスレッドを表すクラス
//
//	NOTES
//		ダーティバッファページ書き込み走査スレッドとは、
//		ダーティなバッファページを管理するためのリストを操作して、
//		バッファページのバッファメモリの内容をディスクへ書き込み、
//		リストを空にする常駐型スレッドである

class DirtyPageFlusher
	: public	DaemonThread
{
public:
	// コンストラクター
	DirtyPageFlusher(unsigned int timeout, bool enable);
	// デストラクター
	~DirtyPageFlusher();

private:
	// スレッドが繰り返し実行する関数
	void
	repeatable();
};

//	FUNCTION public
//	Buffer::DirtyPageFlusher::DirtyPageFlusher --
//		ダーティバッファページ書き込みスレッドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		unsigned int		timeout
//			ダーティバッファページの書き込みの実行間隔(単位ミリ秒)
//		bool				enable
//			true
//				ダーティバッファページの書き込みを可能にする
//			false
//				ダーティバッファページの書き込みを不可にする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
DirtyPageFlusher::DirtyPageFlusher(unsigned int timeout, bool enable)
	: DaemonThread(timeout, enable)
{}

//	FUNCTION public
//	Buffer::DirtyPageFlusher::~DirtyPageFlusher --
//		ダーティバッファページ書き込みスレッドを表すクラスのデストラクター
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
DirtyPageFlusher::~DirtyPageFlusher()
{}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_DIRTYPAGEFLUSHER_H

//
// Copyright (c) 2000, 2002, 2003, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
