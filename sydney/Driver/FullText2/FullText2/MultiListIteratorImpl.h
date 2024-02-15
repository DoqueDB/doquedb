// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListIteratorImpl.h --
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

#ifndef __SYDNEY_FULLTEXT2_MULTILISTITERATORIMPL_H
#define __SYDNEY_FULLTEXT2_MULTILISTITERATORIMPL_H

#include "FullText2/Module.h"
#include "FullText2/MultiListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedSection;

//
//	CLASS
//	FullText2::MultiListIteratorImpl --
//
//	NOTES
//	複数の要素に同じ文書が格納されることは想定していない
//
class MultiListIteratorImpl : public MultiListIterator
{
public:
	// コンストラクタ
	MultiListIteratorImpl();
	// デストラクタ
	virtual ~MultiListIteratorImpl();
	// コピーコンストラクタ
	MultiListIteratorImpl(const MultiListIteratorImpl& src_);

	// コピーを得る
	ListIterator* copy() const;
	
	// ある文書IDの文書が格納されている要素の要素番号を得る - find時に利用する
	int getElement(SearchInformation& cSearchInfo_, DocumentID uiDocumentID_);
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MULTILISTITERATOR_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
