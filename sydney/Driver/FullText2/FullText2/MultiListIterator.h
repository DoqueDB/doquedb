// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MultiListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_MULTILISTITERATOR_H
#define __SYDNEY_FULLTEXT2_MULTILISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/Types.h"
#include "FullText2/ListIterator.h"

#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::MultiListIterator --
//
//	NOTES
//	複数の要素に同じ文書が格納されることは想定していない
//
class MultiListIterator : public ListIterator
{
public:
	// コンストラクタ
	MultiListIterator();
	// デストラクタ
	virtual ~MultiListIterator();
	// コピーコンストラクタ
	MultiListIterator(const MultiListIterator& src_);

	// リザーブする
	virtual void reserve(ModSize size)
	{
		m_vecpIterator.reserve(size);
	}
	// 追加する
	virtual void pushBack(ListIterator* iterator);

	// おおよその文書数を得る
	ModSize getEstimateCount();
	// 索引単位の長さを得る
	int getLength();

	// 文書IDを検索する
	bool find(SearchInformation& cSearchInfo_, DocumentID uiDocumentID_);
	// 文書IDをlower_bound検索する
	DocumentID lowerBound(SearchInformation& cSearchInfo_,
						  DocumentID uiDocumentID_)
		{ return lowerBoundImpl(cSearchInfo_, uiDocumentID_); }

	// カーソルを先頭に戻す
	void reset() { resetImpl(); }
	// 次の値を得る
	DocumentID next(SearchInformation& cSearchInfo_);
	
	// 文書内頻度を得る
	ModSize getTermFrequency();
	// 位置情報へのイテレータを得る
	LocationListIterator::AutoPointer getLocationListIterator();

	// 対象文書IDの要素番号を得る - find時に利用する
	virtual int getElement(SearchInformation& cSearchInfo_,
						   DocumentID uiDocumentID_) = 0;
	
protected:
	// 最小の文書IDを設定する
	void set();
	// 初期化する
	void resetImpl();
	// 文書IDをlower_bound検索する
	DocumentID lowerBoundImpl(SearchInformation& cSearchInfo_,
							  DocumentID uiDocumentID_);
	
	// 現在の文書ID
	DocumentID m_uiCurrentID;
	// その他の中で最小の文書ID
	DocumentID m_uiOtherMinimumID;
	// 現在の文書IDを得ている配列要素番号
	int m_iCurrentElement;
	
	// ListIteratorの配列
	ModVector<ModPair<DocumentID, ListIterator*> > m_vecpIterator;
	// 高速化のためのキャッシュ
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator m_b;
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator m_e;
	ModVector<ModPair<DocumentID, ListIterator*> >::Iterator m_i;

	// findした直後かどうか
	bool m_bFind;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MULTILISTITERATOR_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
