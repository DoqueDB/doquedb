// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KdTreeIndex.h -- KD-Tree索引
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

#ifndef __SYDNEY_KDTREE_KDTREEINDEX_H
#define __SYDNEY_KDTREE_KDTREEINDEX_H

#include "KdTree/Module.h"

#include "KdTree/Allocator.h"
#include "KdTree/Entry.h"
#include "KdTree/FileID.h"
#include "KdTree/Node.h"

#include "Trans/TimeStamp.h"
#include "Trans/Transaction.h"

#include "Common/VectorMap.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class DataFile;
class IndexFile;
class KdTreeIndexSet;
class MakeTreeRecursive;

//	CLASS
//	KdTree::KdTreeIndex --
//
//	NOTES
//
class KdTreeIndex
{
	friend class KdTreeIndexSet;
	friend class IndexFile;		// 整合性検査のため
	
public:
	// 要求を確認するための抽象クラス
	class Direction
	{
	public:
		virtual bool isAbort() const = 0;
	};
	
	// コンストラクタ
	KdTreeIndex(int iDimension_, const Trans::TimeStamp& cTimeStamp_);
	// デストラクタ
	virtual ~KdTreeIndex();

	// 最近傍を得る
	const Entry* nnsearch(const Entry* condition_,
						  double& dsquare_,
						  Node::Status& status_) const
		{
			return m_pNode->nnsearch(condition_, dsquare_, status_);
		}

	// 逐次検索で最近傍を得る
	const Entry* serialSearch(const Entry* condition_,
							  double& dsquare_,
							  Node::Status& status_) const
		{
			return m_pNode->serialSearch(condition_, dsquare_, status_);
		}

	// 挿入する
	void insert(const Entry* pEntry_);
	// 削除する
	bool expunge(ModUInt32 uiRowID_);

	// 空か
	bool isEmpty() const { return (m_pNode == 0); }
	// 索引を作る
	void create(DataFile& cDataFile_);	// 差分ファイル用
	void create(DataFile& cDataFile_, const Direction& cDirection_,
				bool isSmallIndex_ = false);

	// 次元数を得る
	int getDimension() const { return m_cAllocator.getDimension(); }
	// サイズを得る
	ModUInt64 getSize() { return m_cAllocator.getSize(); }

private:
	struct Status
	{
		enum Value
		{
			Copy,	// 古い版の単なるコピー
			Fix		// ちゃんとした版
		};
	};
	
	// ファイルからロードする
	void load(IndexFile& cIndexFile_);
	// ファイルにダンプする
	void dump(IndexFile& cIndexFile_);

	// 再帰的にKD-Treeを作成する
	void makeTree(Node*& pNode_,
				  Common::LargeVector<Entry*>::Iterator b_,
				  Common::LargeVector<Entry*>::Iterator e_,
				  MakeTreeRecursive& cMakeTree_,
				  int iParallelCount_,
				  const Direction& cDirection_);
	
	// KD-Tree
	Node* m_pNode;

	// アロケータ
	Allocator m_cAllocator;

	// 最終更新タイムスタンプ
	Trans::TimeStamp m_cTimeStamp;
	// 更新トランザクションのリスト
	ModVector<Trans::Transaction::ID> m_vecModifierList;

	// 挿入されているエントリ
	// 削除されたものは削除フラグを立てて、この配列からも削除される
	// ただし差分ファイル用索引のみ
	Common::VectorMap<ModUInt32, Entry*, ModLess<ModUInt32> > m_vecEntry;

	// リスト用の前後のリンク
	KdTreeIndex* m_pPrev;
	KdTreeIndex* m_pNext;
	
	// ステータス
	Status::Value m_eStatus;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_KDTREEINDEX_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
