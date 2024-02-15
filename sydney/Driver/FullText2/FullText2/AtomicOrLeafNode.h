// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AtomicOrLeafNode.h --
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

#ifndef __SYDNEY_FULLTEXT2_ATOMICORLEAFNODE_H
#define __SYDNEY_FULLTEXT2_ATOMICORLEAFNODE_H

#include "FullText2/Module.h"
#include "FullText2/OrLeafNode.h"

#include "FullText2/LocationListIterator.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class AtomicOrLeafLocationListIterator;

//
//	CLASS
//	FullText2::AtomicOrLeafNode -- アトミックORのノード
//
//	NOTES
//
class AtomicOrLeafNode : public OrLeafNode
{
public:
	// コンストラクタ
	AtomicOrLeafNode();
	// デストラクタ
	virtual ~AtomicOrLeafNode();
	// コピーコンストラクタ
	AtomicOrLeafNode(const AtomicOrLeafNode& src_);

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
	AtomicOrLeafLocationListIterator* getLocationListIteratorImpl();
	
	// 現在のTF
	ModSize m_uiTermFrequency;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_ATOMICORLEAFNODE_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
