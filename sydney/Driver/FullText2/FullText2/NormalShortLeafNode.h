// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalShortLeafNode.h --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_NORMALSHORTLEAFNODE_H
#define __SYDNEY_FULLTEXT2_NORMALSHORTLEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/LeafNode.h"
#include "FullText2/LocationListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class NormalShortLeafLocationListIterator;

//
//	CLASS
//	FullText2::NormalShortLeafNode -- 長い検索語をあたらすノード
//
//	NOTES
//	pushBackされた順に重なりなく出現する文書を検索するためのノード
//	検索語の異表記正規化時に展開された形態素が含まれていた場合に利用される
//	旧バージョンのトークナイザーの場合には、複数の形態素を含んでいた場合に
//	利用される
//
class NormalShortLeafNode : public LeafNode
{
public:
	// コンストラクタ
	NormalShortLeafNode();
	// デストラクタ
	virtual ~NormalShortLeafNode();
	// コピーコンストラクタ
	NormalShortLeafNode(const NormalShortLeafNode& src_);

	// おおよその文書頻度を求める
	ModSize getEstimateCount(SearchInformation& cSearchInfo_);
	// リセットする
	void reset();

	// NormalLeafNodeを設定する
	void setNormal(LeafNode* pNormal_)
		{ m_pNormal = pNormal_; }
	// ShortLeafNodeを設定する
	void setShort(ModSize pos_, LeafNode* pShort_)
		{ m_uiPos = pos_; m_pShort = pShort_; }
	
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
	// 下限検索を行う
	DocumentID lowerBoundImpl(SearchInformation& cSearchInfo_,
							  DocumentID id_, bool isRough_);
	// 位置情報へのイテレータを得る
	NormalShortLeafLocationListIterator* getLocationListIteratorImpl();

	// 現在の文書ID
	DocumentID m_uiCurrentID;
	// 現在のTF
	ModSize m_uiTermFrequency;
	// NormalLeafNode
	LeafNode* m_pNormal;
	// ShortLeafNode
	LeafNode* m_pShort;
	ModSize m_uiPos;
	
	// 位置情報へのイテレータ
	LocationListIterator::AutoPointer m_pLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_NORMALSHORTLEAFNODE_H

//
//	Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
