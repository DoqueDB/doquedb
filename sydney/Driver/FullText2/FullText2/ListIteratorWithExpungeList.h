// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListIteratorWithExpungeList.h --
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

#ifndef __SYDNEY_FULLTEXT2_LISTITERATORWITHEXPUNGELIST_H
#define __SYDNEY_FULLTEXT2_LISTITERATORWITHEXPUNGELIST_H

#include "FullText2/Module.h"
#include "FullText2/ListIterator.h"

#include "Common/LargeVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ListIteratorWithExpungeList --
//
//	NOTES
//	削除文書リストにある文書IDをスキップしながら走査するListIterator
//
class ListIteratorWithExpungeList : public ListIterator
{
public:
	//コンストラクタ
	ListIteratorWithExpungeList(
		ListIterator* pListIterator_,
		const Common::LargeVector<DocumentID>& vecExpungeList_);
	//デストラクタ
	virtual ~ListIteratorWithExpungeList();

	// おおよその文書数を得る
	ModSize getEstimateCount()
		{ return m_pListIterator->getEstimateCount(); }
	// 索引単位の長さを得る
	int getLength()
		{ return m_pListIterator->getLength(); }

	// 文書IDを検索する
	bool find(SearchInformation& cSearchInfo_, DocumentID uiDocumentID_);
	// 文書IDをlower_bound検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID uiDocumentID_);

	// カーソルを先頭に戻す
	void reset();
	// 次の値を得る
	DocumentID next(SearchInformation& cSearchInfo_);
	
	// 文書内頻度を得る
	ModSize getTermFrequency()
		{ return m_pListIterator->getTermFrequency(); }
	// 位置情報へのイテレータを得る
	LocationListIterator::AutoPointer getLocationListIterator()
		{ return m_pListIterator->getLocationListIterator(); }

	// コピーする
	ListIterator* copy() const;

private:
	// ListIterator
	ListIterator* m_pListIterator;
	// 削除リスト
	const Common::LargeVector<DocumentID>& m_vecExpungeList;
	// 削除リストへのイテレータ
	Common::LargeVector<DocumentID>::ConstIterator m_ite;

	// 現在の文書ID
	DocumentID m_uiCurrentID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LISTITERATORWITHEXPUNGELIST_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
