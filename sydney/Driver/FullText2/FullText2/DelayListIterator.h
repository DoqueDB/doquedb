// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DelayListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_DELAYLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_DELAYLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/MultiListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::DelayListIterator --
//
//	NOTES
//	検索時に利用する転置リストイテレータ
//
class DelayListIterator : public MultiListIterator
{
public:
	//コンストラクタ
	DelayListIterator();
	//デストラクタ
	virtual ~DelayListIterator();
	// コピーコンストラクタ
	DelayListIterator(const DelayListIterator& src_);

	// 配列の領域を確保する
	void reserve(ModSize size);
	// 追加する - 文書IDは単純増加が前提
	void pushBack(ListIterator* iterator, DocumentID uiMaxDocumentID_);

	// ある文書IDの文書が格納されている要素の要素番号を得る - find時に利用する
	int getElement(SearchInformation& cSearchInfo_, DocumentID uiDocumentID_);

	// コピーを得る
	ListIterator* copy() const;
	
private:
	// 最大文書ID
	ModVector<DocumentID> m_vecMaxDocumentID;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_DELAYLISTITERATOR_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
