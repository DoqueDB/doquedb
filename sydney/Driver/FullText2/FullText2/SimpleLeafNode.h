// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimpleLeafNode.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_SIMPLELEAFNODE_H
#define __SYDNEY_FULLTEXT2_SIMPLELEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/LeafNode.h"
#include "FullText2/LocationListIterator.h"
#include "FullText2/ListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::SimpleLeafNode -- １つの索引単位からなる検索語リスト
//
//	NOTES
//
class SimpleLeafNode : public LeafNode
{
public:
	// コンストラクタ
	SimpleLeafNode(ListIterator* pToken_);
	// デストラクタ
	virtual ~SimpleLeafNode();
	// コピーコンストラクタ
	SimpleLeafNode(const SimpleLeafNode& src_);

	// おおよその文書頻度を求める
	ModSize getEstimateCount(SearchInformation& cSearchInfo_)
		{ return m_pListIterator->getEstimateCount(); }
	// リセットする
	void reset() {}
	// 文書IDを検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID id_, bool isRough_);
	// 文書内頻度を得る
	ModSize getTermFrequency();
	// 位置情報へのイテレータを得る
	LocationListIterator::AutoPointer getLocationListIterator();

	// コピー
	LeafNode* copy() const;

private:
	// 索引単位
	ListIterator* m_pListIterator;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SIMPLELEAFNODE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
