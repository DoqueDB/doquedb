// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Node.h -- KdTree のノード
// 
// Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_NODE_H
#define __SYDNEY_KDTREE_NODE_H

#include "KdTree/Module.h"

#include "Common/BitSet.h"
#include "Common/LargeVector.h"

#include "ModVector.h"
#include "ModPair.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

class Allocator;
class Archiver;
class Entry;
class KdTreeIndex;

//	CLASS
//	KdTree::Node -- KdTree のノード
//
//	NOTES
//
class Node
{
	friend class KdTreeIndex;
	
public:
	// 探索タイプ
	struct TraceType
	{
		enum Value
		{
			Unknown,
			
			Normal,
			RicohVisualSearch,
			Serial
		};
	};
	
	// 検索状態
	class Status
	{
	public:
		// コンストラクタ
		Status(Node::TraceType::Value eTraceType_,
			   int iMaxCalcDistanceCount_, const Common::BitSet* pExpunge_)
			: m_eTraceType(eTraceType_),
			  m_iMaxCalcDistanceCount(iMaxCalcDistanceCount_),
			  m_iCalcDistanceCount(0), m_pExpunge(pExpunge_),
			  m_uiLimit(0) {}
		// デストラクタ
		~Status() {}

		// コピーコンストラクタ
		//【注意】	ビットセットはコピーしない
		Status(const Status& src_)
			: m_eTraceType(src_.m_eTraceType),
			  m_iMaxCalcDistanceCount(src_.m_iMaxCalcDistanceCount),
			  m_iCalcDistanceCount(0), m_pExpunge(src_.m_pExpunge),
			  m_uiLimit(src_.m_uiLimit) {}

		// 探索タイプを得る
		Node::TraceType::Value getTraceType() const
			{ return m_eTraceType; }

		// 続けていいか？
		bool isContinue()
			{ return (m_iCalcDistanceCount < m_iMaxCalcDistanceCount); }
		// 距離計算回数を増やす
		bool addCalcDistanceCount()
			{ return (m_iCalcDistanceCount++ < m_iMaxCalcDistanceCount); }

		// 削除されているか？
		bool isExpunge(const Entry* pEntry_);

		// 探索した空間内で近いものN個を保存する
		void setLimit(ModSize n_) { m_uiLimit = n_; }
		// 距離を求めたものを追加する
		// dsquare_の小さいものからm_iLimit個保持する
		void pushBack(const Entry* pEntry_, double dsquare_);

		// 探索タイプ
		Node::TraceType::Value m_eTraceType;
		// 最大計算回数
		int m_iMaxCalcDistanceCount;
		// 距離計算回数
		int m_iCalcDistanceCount;
		// 削除ビットマップ
		const Common::BitSet* m_pExpunge;

		// 近いものを取って置く数
		ModSize m_uiLimit;
		// 近いものを取って置く配列
		ModVector<ModPair<double, const Entry*> > m_vecEntry;
	};

	// コンストラクタ
	Node();
	// デストラクタ
	~Node();

	// ノードのエントリを得る
	const Entry* getValue() const { return m_pValue; }

	// 右ノードを得る
	const Node* getRight() const { return m_pRight; }
	// 左ノードを得る
	const Node* getLeft() const { return m_pLeft; }

	// 比較に使った次元を得る
	int getCompareDimension() const { return m_iDimension; }

	// KdTreeを作成する
	static
	void makeTree(Node*& pNode_,
				  Common::LargeVector<Entry*>::Iterator b_,
				  Common::LargeVector<Entry*>::Iterator e_,
				  Allocator& cAllocator_);

	// 最近傍検索
	const Entry* nnsearch(const Entry* condition_,
						  double& dsquare_,
						  Status& cStatus_) const;

	// ファイルに書き出す
	void dump(Archiver& archiver_);
	// ファイルから要素を読みだす
	bool load(Archiver& archiver_, Allocator& cAllocator_);

	// 挿入する
	void insert(Entry* pEntry_, Allocator& cAllocator_);

private:
	// KD-Treeの通常の最近傍検索 -- 再帰的に検索する
	const Entry* normalSearch(const Entry* condition_,
							  double& dsquare_,
							  Status& cStatus_) const;

	// 深さ優先探索 -- 再帰的に検索する
	const Entry* dfsearch(const Entry* condition_,
						  double& dsquare_,
						  Status& cStatus_,
						  Common::LargeVector<const Node*>& queue_) const;

	// 逐次検索で最近傍検索を行う -- 再帰的に検索する
	const Entry* serialSearch(const Entry* condition_,
							  double& dsquare_,
							  Status& cStatus_) const;

	// 一番分散の大きな次元を求める
	static
	int getMaxVarianceDimension(Common::LargeVector<Entry*>::Iterator& b_,
								Common::LargeVector<Entry*>::Iterator& e_);
	
	// 分岐に使った次元
	int m_iDimension;

	// ノードのエントリ
	Entry* m_pValue;
	
	// 右ノード
	Node* m_pRight;
	// 左ノード
	Node* m_pLeft;
};

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_NODE_H

//
//	Copyright (c) 2012, 2013, 2017, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
