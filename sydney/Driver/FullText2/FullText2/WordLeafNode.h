// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WordLeafNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_WORDLEAFNODE_H
#define __SYDNEY_FULLTEXT2_WORDLEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/UnaryLeafNode.h"
#include "FullText2/LocationListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ListIterator;
class WordLeafLocationListIterator;

//
//	CLASS
//	FullText2::WordLeafNode -- 単語単位検索用のノード
//
//	NOTES
//	このクラスはDUAL索引の exactword, simleword, wordhead, wordtail 時に
//	呼び出される。WORD索引では利用されない。
//
class WordLeafNode : public UnaryLeafNode
{
public:
	// コンストラクタ
	WordLeafNode(LeafNode* pLeafNode_,
				 ListIterator* pSeparator_);
	// デストラクタ
	virtual ~WordLeafNode();
	// コピーコンストラクタ
	WordLeafNode(const WordLeafNode& src_);

	// 確認する単語境界を設定する
	void pushWordPosition(ModSize pos_)
		{ m_cWordPosition.pushBack(pos_); }
	void setWordPosition(const ModVector<ModSize>& cWordPosition_)
		{ m_cWordPosition = cWordPosition_; m_bExact = true; }

	// おおよその文書頻度を求める
	ModSize getEstimateCount(SearchInformation& cSearchInfo_);
	
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
	WordLeafLocationListIterator* getLocationListIteratorImpl();
	
	// 単語境界
	ListIterator*		m_pSeparator;

	// 単語境界を確認する位置
	ModVector<ModSize>	m_cWordPosition;

	// 現在のTF
	ModSize m_uiTermFrequency;

	// 厳格一致か否か
	bool m_bExact;

	// 位置情報へのイテレータ
	LocationListIterator::AutoPointer m_pLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_WORDLEAFNODE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
