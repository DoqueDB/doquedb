// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationNoTFList.h --
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

#ifndef __SYDNEY_FULLTEXT2_SHORTNOLOCATIONNOTFLIST_H
#define __SYDNEY_FULLTEXT2_SHORTNOLOCATIONNOTFLIST_H

#include "FullText2/Module.h"
#include "FullText2/ShortBaseList.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::ShortNolocationNoTFList --
//
//	NOTES
//
//
class ShortNolocationNoTFList : public ShortBaseList
{
public:
	// コンストラクタ(1) - 該当する索引単位のエリアがある場合
	ShortNolocationNoTFList(
		InvertedUnit& cInvertedUnit_,			// 転置ファイル
		LeafPage::PagePointer pLeafPage_,		// リーフページ
		LeafPage::Iterator ite_);				// リーフページのエリア
	// コンストラクタ(2) - 該当する索引単位のエリアがない場合
	ShortNolocationNoTFList(
		InvertedUnit& cInvertedUnit_,			// 転置ファイル
		const ModUnicodeChar* pKey_,			// 索引単位
		LeafPage::PagePointer pLeafPage_,		// リーフページ
		LeafPage::Iterator ite_);				// リーフページのエリア
												// (lower_bound検索結果)
	// デストラクタ
	virtual ~ShortNolocationNoTFList();

	// 位置情報を格納していないか
	bool isNolocation() const { return true; }
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	bool isNoTF() const { return false; }
	
	// 転置リストの挿入 - 1文書挿入用
	bool insert(ModUInt32 uiDocumentID_,
				const SmartLocationList& cLocationList_);

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* getIterator();

protected:
	// ミドルリストのインスタンスを得る
	MiddleBaseList*
	makeMiddleList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				   LeafPage::PagePointer pLeafPage_,	// リーフページ
				   LeafPage::Iterator ite_);			// リーフページのエリア
	// バッチリストのインスタンスを得る
	BatchBaseList*
	makeBatchList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				  LeafPage::Area* pArea_);			// リーフページのエリア
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SHORTNOLOCATIONNOTFLIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
