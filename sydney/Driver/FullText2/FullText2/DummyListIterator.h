// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DummyListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_DUMMYLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_DUMMYLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/ListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::DummyListIterator
//		-- 索引単位が存在しない場合の転置イテレータ
//		   索引単位が存在しない場合の特別な処理を省くため
//
//	NOTES
//
//
class DummyListIterator : public ListIterator
{
public:
	//コンストラクタ
	DummyListIterator();
	//デストラクタ
	virtual ~DummyListIterator();

	// おおよその文書数を得る
	ModSize getEstimateCount() { return 0; }
	// 索引単位の長さを得る
	int getLength() { return 1; }

	// 文書IDを検索する
	bool find(SearchInformation& cSearchInfo_,
			  DocumentID uiDocumentID_) { return false; }
	// 文書IDをlower_bound検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID uiDocumentID_)
		{ return UndefinedDocumentID; }

	// カーソルを先頭に戻す
	void reset() {}
	// 次の値を得る
	DocumentID next(SearchInformation& cSearchInfo_)
		{ return UndefinedDocumentID; }
	
	// 文書内頻度を得る
	ModSize getTermFrequency() { return 0; }
	
	// コピーする
	ListIterator* copy() const;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_DUMMYLISTITERATOR_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
