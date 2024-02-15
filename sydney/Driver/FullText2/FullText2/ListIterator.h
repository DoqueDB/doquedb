// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_LISTITERATOR_H
#define __SYDNEY_FULLTEXT2_LISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListManager.h"
#include "FullText2/LocationListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class SearchInformation;

//
//	CLASS
//	FullText2::ListIterator --
//
//	NOTES
//
//
class ListIterator : public LocationListManager
{
public:
	// コンストラクタ
	ListIterator();
	// デストラクタ
	virtual ~ListIterator();
	// コピーコンストラクタ
	ListIterator(const ListIterator& src_);

	// おおよその文書数を得る
	virtual ModSize getEstimateCount() = 0;
	// 索引単位の長さを得る
	virtual int getLength() = 0;

	// 文書IDを検索する
	virtual bool find(SearchInformation& cSearchInfo_,
					  DocumentID uiDocumentID_) = 0;
	
	// 文書IDをlower_bound検索する
	//
	//【注意】
	// 現在の文書IDより小さい文書IDが渡された場合にも正しく動作すること
	// 高速化のため、リセットを呼ぶタイミングは個々の実装に任されている
	// この動作は LeafNode とは異なる
	//
	virtual DocumentID lowerBound(SearchInformation& cSearchInfo_,
								  DocumentID uiDocumentID_) = 0;

	// カーソルを先頭に戻す
	virtual void reset() = 0;
	// 次の値を得る
	virtual DocumentID next(SearchInformation& cSearchInfo_) = 0;
	
	// 文書内頻度を得る
	virtual ModSize getTermFrequency() = 0;
	// 位置情報へのイテレータを得る
	virtual LocationListIterator::AutoPointer getLocationListIterator();

	// コピーする
	virtual ListIterator* copy() const = 0;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_LISTITERATOR_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
