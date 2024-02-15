// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WithinUnorderedLeafNode.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_WITHINUNORDEREDLEAFNODE_H
#define __SYDNEY_FULLTEXT2_WITHINUNORDEREDLEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/ArrayLeafNode.h"
#include "FullText2/LocationListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class WithinUnorderedLeafLocationListIterator;

//
//	CLASS
//	FullText2::WithinUnorderedLeafNode -- Withinをあたらすノード
//
//	NOTES
//  出現範囲の先頭のものの末尾位置から出現範囲の末尾のものの先頭位置までが、
//	uiLower_以上uiUpper_以下であるものを検索する
//
//	WithinOrderedLeafNode と WithinUnorderedLeafNode とはほとんど同じコードで
//	あるが、高速化の(仮想関数を減らし、コンパイラーが最適化しやすくする)ため、
//	別クラスとしている
//
class WithinUnorderedLeafNode : public ArrayLeafNode
{
public:
	// コンストラクタ
	WithinUnorderedLeafNode(ModSize uiLower_,
							ModSize uiUpper);
	// デストラクタ
	virtual ~WithinUnorderedLeafNode();
	// コピーコンストラクタ
	WithinUnorderedLeafNode(const WithinUnorderedLeafNode& src_);

	// 領域をリザーブする
	void reserve(ModSize size_) { m_cVector.reserve(size_); }
	// ノードを追加する
	void pushBack(LeafNode* pNode_);
	
	// おおよその文書頻度を求める
	ModSize getEstimateCount(SearchInformation& cSearchInfo_);
	// リセットする
	void reset();

	// 文書IDを検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID id_, bool isRough_)
		{ return lowerBoundImpl(cSearchInfo_, id_, isRough_); }

	// 文書内頻度を得る
	ModSize getTermFrequency();
	// 位置情報へのイテレータを得る
	LocationListIterator::AutoPointer getLocationListIterator();
	// コピー
	LeafNode* copy() const;

private:
	typedef ModVector<ModPair<ModSize, LeafNode*> > NodeVector;
	
	// 初期化する
	void initialize(SearchInformation& cSearchInfo_);
	// 下限検索を行う
	DocumentID lowerBoundImpl(SearchInformation& cSearchInfo_,
							  DocumentID id_, bool isRough_);
	// 位置情報へのイテレータを得る
	WithinUnorderedLeafLocationListIterator* getLocationListIteratorImpl();

	// 検索ノード
	NodeVector	m_cVector;
	// 現在の文書ID
	DocumentID m_uiCurrentID;
	
	// 下限
	ModSize m_uiLower;
	// 上限
	ModSize m_uiUpper;

	// 現在のTF
	ModSize m_uiTermFrequency;
	
	// 初期化したかどうか
	bool m_bInitialized;
	// おおよその文書頻度
	ModSize m_uiEstimateCount;

	// 位置情報へのイテレータ
	LocationListIterator::AutoPointer m_pLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_WITHINUNORDEREDLEAFNODE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
