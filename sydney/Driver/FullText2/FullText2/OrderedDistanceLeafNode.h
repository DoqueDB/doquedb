// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OrderedDistanceLeafNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_ORDEREDDISTANCELEAFNODE_H
#define __SYDNEY_FULLTEXT2_ORDEREDDISTANCELEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/AndLeafNode.h"
#include "FullText2/LocationListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class OrderedDistanceLeafLocationListIterator;

//
//	CLASS
//	FullText2::OrderedDistanceLeafNode -- 長い検索語をあたらすノード
//
//	NOTES
//	pushBackされた順に重なりなく出現する文書を検索するためのノード
//	検索語の異表記正規化時に展開された形態素が含まれていた場合に利用される
//	旧バージョンのトークナイザーの場合には、複数の形態素を含んでいた場合に
//	利用される
//
class OrderedDistanceLeafNode : public AndLeafNode
{
public:
	// コンストラクタ
	OrderedDistanceLeafNode();
	// デストラクタ
	virtual ~OrderedDistanceLeafNode();
	// コピーコンストラクタ
	OrderedDistanceLeafNode(const OrderedDistanceLeafNode& src_);

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
	// 初期化する
	void initialize(SearchInformation& cSearchInfo_);
	
	// 下限検索を行う
	DocumentID lowerBoundImpl(SearchInformation& cSearchInfo_,
							  DocumentID id_, bool isRough_);
	// 位置情報へのイテレータを得る
	OrderedDistanceLeafLocationListIterator* getLocationListIteratorImpl();

	// 現在のTF
	ModSize m_uiTermFrequency;
	// もっとも文書頻度の小さいノード
	NodeVector::Iterator m_iterator;
	// もっとも文書頻度の小さいノードを設定したかどうか
	bool m_bInitialized;

	// おおよその文書頻度
	ModSize m_uiEstimateCount;

	// 位置情報へのイテレータ
	LocationListIterator::AutoPointer m_pLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_ORDEREDDISTANCELEAFNODE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
