// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoLatch.cpp -- オートラッチ関連の関数定義
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Schema/AutoLatch.h"
#include "Schema/File.h"
#include "Schema/SystemFile.h"

#include "Lock/Name.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::AutoLatch::AutoLatch --
//		オートラッチを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ラッチするトランザクションのトランザクション記述子
//		Lock::Name&			name
//			ラッチされるロック項目のロック名
//		bool				force
//			true
//				版を使わないトランザクションでもラッチする
//			false または指定されないとき
//				版を使わないトランザクションではラッチしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoLatch::AutoLatch(
	Trans::Transaction& trans, const Lock::Name& name, bool force)
{
	if (force || trans.isNoVersion())
		_latch = new Trans::AutoLatch(trans, name);
}

//	FUNCTION public
//	Schema::AutoLatch::AutoLatch --
//		オートラッチを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ラッチするトランザクションのトランザクション記述子
//		Schema::File&		file
//			ラッチされる論理ファイルを表すスキーマオブジェクト
//		bool				force
//			true
//				版を使わないトランザクションでもラッチする
//			false または指定されないとき
//				版を使わないトランザクションではラッチしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoLatch::AutoLatch(Trans::Transaction& trans, const File& file, bool force)
{
	if (force || trans.isNoVersion())
		_latch = new Trans::AutoLatch(
			trans, Lock::FileName(
				file.getDatabaseID(), file.getTableID(), file.getID()));
}

//	FUNCTION public
//	Schema::AutoLatch::AutoLatch --
//		オートラッチを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ラッチするトランザクションのトランザクション記述子
//		Schema::SystemTable::SystemFile&	file
//			ラッチされるシステム表を構成するファイル
//		bool				force
//			true
//				版を使わないトランザクションでもラッチする
//			false または指定されないとき
//				版を使わないトランザクションではラッチしない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

AutoLatch::AutoLatch(
	Trans::Transaction& trans, const SystemTable::SystemFile& file, bool force)
{
	if (force || trans.isNoVersion())
		_latch = new Trans::AutoLatch(
			trans, Lock::FileName(
				file.getDatabaseID(), file.getTableID(), file.getID()));
}

//	FUNCTION public
//	Schema::AutoLatch::unlatch -- オートラッチをアンラッチする
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

void
AutoLatch::unlatch()
{
	if (_latch.get())
		_latch->unlatch();
}

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
