// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LoadEntry.h -- 
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_LOADENTRY_H
#define __SYDNEY_KDTREE_LOADENTRY_H

#include "KdTree/Module.h"
#include "Utility/OpenMP.h"

#include "KdTree/Allocator.h"
#include "KdTree/DataFile.h"
#include "KdTree/FileID.h"

#include "Common/LargeVector.h"

#include "Os/CriticalSection.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::LoadEntry -- 多次元ベクトルデータをロードする
//
//	NOTES
//
class LoadEntry : public Utility::OpenMP
{
public:
	// コンストラクタ
	LoadEntry(DataFile& cDataFile_,
			  Allocator& cAllocator_);
	// デストラクタ
	virtual ~LoadEntry();

	// ロード結果を取得する
	Common::LargeVector<Entry*>& getEntry() { return m_vecpEntry; }

	// 準備
	void prepare();
	// 並列実行
	void parallel();
	// 後処理
	void dispose();

	// 並列実行時の実行数 (実行後に有効となる)
	int getParallelCount() const { return m_iParallelCount; }

private:
	// 次にロードするエントリが格納されているページIDを得る
	DataFile::PageID getNextPageID();

	// 引数のデータ格納ファイル
	DataFile& m_cDataFile;
	// 多次元ベクトルデータ格納ファイル
	ModVector<DataFile*> m_vecpDataFile;
	// アロケータ
	Allocator& m_cAllocator;

	// ロードしたエントリ
	Common::LargeVector<Entry*> m_vecpEntry;

	// 処理中のページID
	DataFile::PageID m_uiCurrentPageID;

	// 排他制御用のクリティカルセクション
	Os::CriticalSection m_cPageLatch;

	// 並列時の並列実行数
	int m_iParallelCount;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_LOADENTRY_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
