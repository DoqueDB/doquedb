// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Buffer.cpp -- ファイルバッファに関する処理を行うクラス関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2009, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Buffer.h"
#include "Checkpoint/Debug.h"

#include "Buffer/AutoPool.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{
}

//	FUNCTION private
//	Checkpoint::Buffer::flush --
//		直前のチェックポイント処理の終了時にダーティなバッファのうち、
//		現在もダーティなものをすべてフラッシュする
//
//	NOTES
//		チェックポイントスレッドから呼び出される
//
//	ARGUMENTS
//		bool				onlyMarked
//			true
//				直前のチェックポイント処理時にフラッシュされていなかった
//				バッファページのうち、今回もフラッシュされていないものを
//				フラッシュする
//			false
//				ダーティなバッファページをすべてフラッシュする
//
//	RETURN
//		true
//			ダーティなバッファページをすべてフラッシュした
//		false
//			フラッシュできなかったダーティなバッファページをマークした
//
//	EXCEPTIONS

// static
bool
Checkpoint::
Buffer::flush(bool onlyMarked)
{
	//【注意】	チェックポイントスレッドから呼び出されること

	Os::Memory::Size total = 0;

	unsigned int i = 0;
	do {
		switch (i) {
		case _TRMEISTER::Buffer::Pool::Category::Normal:

			// 通常データと論理ログデータを格納するバッファプールを処理する
			//
			//【注意】	読取専用のデータは永続化する必要はなく、
			//			一時データはバッファプールがあふれたときしか
			//			フラッシュしない

			// バッファプール記述子を生成する

			_TRMEISTER::Buffer::AutoPool pool(
				_TRMEISTER::Buffer::Pool::attach(
					static_cast<_TRMEISTER::Buffer::Pool::Category::Value>(i)));

			//【注意】	バッファ関連のすべてのデーモン処理は
			//			実行不可にされているので、
			//			裏でダーティリストの切り替えは発生しない

			// このバッファプールの直前のチェックポイント処理時に
			// マークしたバッファページのうち、
			// それからフラッシュされていないものがあれば、フラッシュする

			pool->flushDirtyPage(
				_TRMEISTER::Buffer::Pool::FlushablePageFilter::ForPool(
					onlyMarked, false), true);

			pool->syncFile();	// すべてのファイルをsyncする

			if (onlyMarked)

				// このバッファプールの
				// すべてのダーティなバッファページをマークする

				total += pool->markDirtyPage();
		}
	} while (++i < _TRMEISTER::Buffer::Pool::Category::Count);

	_SYDNEY_CHECKPOINT_EXECUTION_MESSAGE
		<< "Flushing buffer is finished" << ModEndl;

	return !total;
}

//
// Copyright (c) 2000, 2001, 2002, 2005, 2009, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
