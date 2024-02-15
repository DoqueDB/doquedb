// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MakeTreeRecursive.h -- 
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

#ifndef __SYDNEY_KDTREE_MAKETREERECURSIVE_H
#define __SYDNEY_KDTREE_MAKETREERECURSIVE_H

#include "KdTree/Module.h"
#include "Utility/OpenMP.h"

#include "KdTree/Allocator.h"
#include "KdTree/Entry.h"
#include "KdTree/Node.h"

#include "Common/LargeVector.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	CLASS
//	KdTree::MakeTreeRecursive -- 再帰的にKD-Treeを作成する
//
//	NOTES
//
class MakeTreeRecursive : public Utility::OpenMP
{
public:
	// コンストラクタ
	MakeTreeRecursive(Allocator& cAllocator_);
	// デストラクタ
	virtual ~MakeTreeRecursive();

	// KD-Treeを作成するエントリを設定する
	void setEntry(Node** ppNode_,
				  Common::LargeVector<Entry*>::Iterator b_,
				  Common::LargeVector<Entry*>::Iterator e_)
		{
			m_beginIte.pushBack(b_);
			m_endIte.pushBack(e_);
			m_vecpNode.pushBack(ppNode_);
		}

	// ノード数を得る
	int getNodeSize()
		{ return static_cast<int>(m_vecpNode.getSize()); }

	// 並列実行
	void parallel();

private:
	// スレッドで処理するエントリを得る
	bool getEntry(Common::LargeVector<Entry*>::Iterator& b_,
				  Common::LargeVector<Entry*>::Iterator& e_,
				  Node**& n_);
	
	// アロケータ
	Allocator& m_cAllocator;
	
	// エントリ
	ModVector<Common::LargeVector<Entry*>::Iterator> m_beginIte;
	ModVector<Common::LargeVector<Entry*>::Iterator> m_endIte;

	// ノード
	ModVector<Node**> m_vecpNode;

	// 次割り当て要素番号
	int m_iNext;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_MAKETREERECURSIVE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
