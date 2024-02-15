// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBaseList.h --
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

#ifndef __SYDNEY_FULLTEXT2_SHORTBASELIST_H
#define __SYDNEY_FULLTEXT2_SHORTBASELIST_H

#include "FullText2/Module.h"
#include "FullText2/InvertedList.h"
#include "FullText2/LeafFile.h"
#include "FullText2/LeafPage.h"
#include "FullText2/InvertedIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class BatchBaseList;
class InvertedUnit;
class MiddleBaseList;

//
//	CLASS
//	FullText2::ShortBaseList --
//
//	NOTES
//
//
class ShortBaseList : public InvertedList
{
public:
	// コンストラクタ(1) - 該当する索引単位のエリアがある場合
	ShortBaseList(InvertedUnit& cInvertedUnit_,			// 転置ファイル
				  LeafPage::PagePointer pLeafPage_,		// リーフページ
				  LeafPage::Iterator ite_);				// リーフページのエリア
	// コンストラクタ(2) - 該当する索引単位のエリアがない場合
	ShortBaseList(InvertedUnit& cInvertedUnit_,			// 転置ファイル
				  const ModUnicodeChar* pKey_,			// 索引単位
				  LeafPage::PagePointer pLeafPage_,		// リーフページ
				  LeafPage::Iterator ite_);				// リーフページのエリア
													//(lower_bound検索結果)
	// デストラクタ
	virtual ~ShortBaseList();

	// 転置リストの挿入 - マージ挿入用
	bool insert(InvertedList& cInvertedList_);
	
	// 整合性検査を行う
	void verify(Admin::Verification::Treatment::Value uiTreatment_,
				Admin::Verification::Progress& cProgress_,
				const Os::Path& cRootPath_);

	// ミドルリストへコンバートする
	InvertedList* convert();

protected:
	// エリア領域を挿入または拡張する
	bool insertOrExpandArea(ModUInt32 uiDocumentID_,
							ModSize uiLocBitLength_);
	// エリア領域を拡張する
	bool expandArea(ModSize uiExpandUnit_);
	// エリア領域を縮める
	void shotenArea(ModSize uiShotenUnit_);

	// リーフページにエリアが存在するか
	bool isExist() const { return m_bExist; }

	// ミドルリストのインスタンスを得る
	virtual MiddleBaseList*
	makeMiddleList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				   LeafPage::PagePointer pLeafPage_,	// リーフページ
				   LeafPage::Iterator ite_) = 0;		// リーフページのエリア
	// バッチリストのインスタンスを得る
	virtual BatchBaseList*
	makeBatchList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				  LeafPage::Area* pArea_) = 0;		// リーフページのエリア

	// 転置ファイルユニット
	InvertedUnit& m_cInvertedUnit;

	// 存在するかどうか
	bool m_bExist;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SHORTBASELIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
