// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SortEntry.h -- 
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

#ifndef __SYDNEY_KDTREE_SORTENTRY_H
#define __SYDNEY_KDTREE_SORTENTRY_H

#include "KdTree/Module.h"
#include "Utility/OpenMP.h"

#include "KdTree/Entry.h"

#include "Common/LargeVector.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::SortEntry -- 多次元ベクトルデータをソートする
//
//	NOTES
//
class SortEntry : public Utility::OpenMP
{
public:
	// コンストラクタ
	SortEntry(const Common::LargeVector<Entry*>::Iterator& b_,
			  const Common::LargeVector<Entry*>::Iterator& e_,
			  int iSortKey_);
	// デストラクタ
	virtual ~SortEntry();

	// 準備
	void prepare();
	// 並列実行
	void parallel();
	// 後処理
	void dispose();

private:
	// ソート対象
	Common::LargeVector<Entry*>::Iterator m_b;
	Common::LargeVector<Entry*>::Iterator m_e;
	
	// 比較クラス
	Entry::Less m_cLess;

	// スレッドごとの配列
	ModVector<Common::LargeVector<Entry*> > m_cEntryPerThread;
	// スレッドあたりの処理数
	int m_iCountPerThread;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_SORTENTRY_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
