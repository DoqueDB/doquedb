// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortLeafNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_SHORTLEAFNODE_H
#define __SYDNEY_FULLTEXT2_SHORTLEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/LeafNode.h"
#include "FullText2/LocationListIterator.h"

#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ListIterator;
class ShortLeafLocationListIterator;

//
//	CLASS
//	FullText2::ShortLeafNode -- 検索語リストをあらわすクラス
//
//	NOTES
//
class ShortLeafNode : public LeafNode
{
public:
	// コンストラクタ
	ShortLeafNode(int iLength_);
	// デストラクタ
	virtual ~ShortLeafNode();
	// コピーコンストラクタ
	ShortLeafNode(const ShortLeafNode& src_);

	// トークンを追加する
	void pushBack(ListIterator* pToken_);

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

protected:
	typedef ModPair<DocumentID, ListIterator*>		NodePair;
	typedef ModVector<NodePair>						NodeVector;

	// 下限検索を行う
	DocumentID lowerBoundImpl(SearchInformation& cSearchInfo_,
							  DocumentID id_, bool isRough_);

	// 位置情報へのイテレータを得る
	ShortLeafLocationListIterator* getLocationListIteratorImpl();

	// 索引単位と位置の配列
	NodeVector	m_cVector;

	// 現在の文書ID
	DocumentID m_uiCurrentID;
	// 現在のTF
	ModSize m_uiTermFrequency;

	// 文字列長
	int m_iLength;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SHORTLEAFNODE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
