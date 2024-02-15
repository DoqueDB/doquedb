// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// WithinOrderedLeafNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_WITHINORDEREDLEAFNODE_H
#define __SYDNEY_FULLTEXT2_WITHINORDEREDLEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/AndLeafNode.h"
#include "FullText2/LocationListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class WithinOrderedLeafLocationListIterator;

//
//	CLASS
//	FullText2::WithinOrderedLeafNode -- Withinをあたらすノード
//
//	NOTES
//  出現範囲の先頭のものの末尾位置から出現範囲の末尾のものの先頭位置までが、
//	uiLower_以上uiUpper_以下であるものを検索する
//
//	WithinOrderedLeafNode と WithinUnorderedLeafNode とはほとんど同じコードで
//	あるが、高速化の(仮想関数を減らし、コンパイラーが最適化しやすくする)ため、
//	別クラスとしている
//
class WithinOrderedLeafNode : public AndLeafNode
{
public:
	// コンストラクタ
	WithinOrderedLeafNode(ModSize uiLower_,
						  ModSize uiUpper_);
	// デストラクタ
	virtual ~WithinOrderedLeafNode();
	// コピーコンストラクタ
	WithinOrderedLeafNode(const WithinOrderedLeafNode& src_);

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
	WithinOrderedLeafLocationListIterator* getLocationListIteratorImpl();

	// 下限
	ModSize m_uiLower;
	// 上限
	ModSize m_uiUpper;
	
	// 現在のTF
	ModSize m_uiTermFrequency;
	// もっとも文書頻度の小さいノード
	NodeVector::Iterator m_iterator;
	// 初期化したかどうか
	bool m_bInitialized;

	// 位置情報へのイテレータ
	LocationListIterator::AutoPointer m_pLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_WITHINORDEREDLEAFNODE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
