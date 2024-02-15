// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationNoTFList.h --
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

#ifndef __SYDNEY_FULLTEXT2_MIDDLENOLOCATIONNOTFLIST_H
#define __SYDNEY_FULLTEXT2_MIDDLENOLOCATIONNOTFLIST_H

#include "FullText2/Module.h"
#include "FullText2/MiddleBaseList.h"

#include "FullText2/LeafPage.h"
#include "FullText2/OverflowPage.h"

#include "Admin/Verification.h"

_SYDNEY_BEGIN

namespace Os
{
	class Path;
}

_SYDNEY_FULLTEXT2_BEGIN

class InvertedIterator;
class InvertedUnit;
class OverflowFile;

//
//	CLASS
//	FullText2::MiddleNolocationNoTFList --
//
//	NOTES
//
//
class MiddleNolocationNoTFList : public MiddleBaseList
{
public:
	// コンストラクタ(1)
	MiddleNolocationNoTFList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
							 LeafPage::PagePointer pLeafPage_,	// リーフページ
							 LeafPage::Iterator ite_);	// リーフページのエリア
	// コンストラクタ(2)
	MiddleNolocationNoTFList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
							 LeafPage::Area* pTmpArea_);		// エリア
	// デストラクタ
	virtual ~MiddleNolocationNoTFList();

	// 位置情報を格納していないか
	bool isNolocation() const { return true; }
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	bool isNoTF() const { return true; }
	
	// 転置リストの挿入 - 1文書挿入用
	bool insert(ModUInt32 uiDocumentID_,
				const SmartLocationList& cLocationList_);
	// 転置リストの挿入 - マージ挿入用
	bool insert(InvertedList& cInvertedList_);

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* getIterator();

protected:
	// 文書IDを1つ書き出す
	bool insertDocumentID(ModUInt32 uiDocumentID_,
						  OverflowPage::IDBlock& cIdBlock_);
	
	// LOCブロックのデータ領域を拡張する
	void expandLocBlock(OverflowPage::LocBlock& cLocBlock_,
						ModSize uiInsertedLocBitLength_,
						ModSize uiDataBitLength_,
						ModSize uiDataUnitSize_);
	void expandLocBlock(OverflowPage::PagePointer& pLocPage_,
						OverflowPage::LocBlock& cLocBlock_,
						ModSize uiInsertedLocBitLength_,
						ModSize& uiDataBitLength_,
						ModSize& uiDataUnitSize_);
	
	// IDブロックを確保する
	bool allocateIDBlock(OverflowPage::IDBlock& cLastBlock_);
	
	// 最終IDブロックをIDページにコピーする
	bool copyIDBlock(OverflowPage::PagePointer& pIdPage_,
					 OverflowPage::IDBlock& cLastBlock_);

	// 最終IDブロック関係を初期化する
	void initializeLastBlock(OverflowPage::IDBlock& cLastBlock_,
							 ModUInt32 uiDocumentID_);

	// 自身を初期化する
	void initialize();
	
	// LOC関係の準備をする
	void prepareForLocation(OverflowPage::PagePointer& pLocPage_,
							ModSize uiUnitSize_,
							ModSize uiNewUnitSize_);
	
	// LOCブロックの整合性検査を行う (何もしない)
	void verifyLocBlock(
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_,
		const Os::Path& cRootPath_,
		const OverflowPage::IDBlock& cIdBlock_,
		OverflowPage::PagePointer& pLocPage_) {}

	// 最終LOCブロックの次のLOCページの整合性検査を行う (何もしない)
	void verifyNextLocPage(
		Admin::Verification::Treatment::Value uiTreatment_,
		Admin::Verification::Progress& cProgress_,
		const Os::Path& cRootPath_,
		OverflowPage::PagePointer& pLocPage_) {}

	// ミドルリストを得る
	MiddleBaseList* makeTmpMiddleList(InvertedUnit& cInvertedUnit_,
									  LeafPage::Area* pTmpArea_);
	
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MIDDLENOLOCATIONNOTFLIST_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
