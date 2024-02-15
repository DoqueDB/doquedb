// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationList.h --
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

#ifndef __SYDNEY_FULLTEXT2_MIDDLENOLOCATIONLIST_H
#define __SYDNEY_FULLTEXT2_MIDDLENOLOCATIONLIST_H

#include "FullText2/Module.h"
#include "FullText2/MiddleBaseList.h"

#include "FullText2/LeafPage.h"
#include "FullText2/OverflowPage.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class InvertedIterator;
class InvertedUnit;
class OverflowFile;

//
//	CLASS
//	FullText2::MiddleNolocationList --
//
//	NOTES
//
//
class MiddleNolocationList : public MiddleBaseList
{
public:
	// コンストラクタ(1)
	MiddleNolocationList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
						 LeafPage::PagePointer pLeafPage_,	// リーフページ
						 LeafPage::Iterator ite_);		// リーフページのエリア
	// コンストラクタ(2)
	MiddleNolocationList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
						 LeafPage::Area* pTmpArea_);		// エリア
	// デストラクタ
	virtual ~MiddleNolocationList();

	// 位置情報を格納していないか
	bool isNolocation() const { return true; }
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	bool isNoTF() const { return false; }
	
#ifdef DEBUG
	bool insert(ModUInt32 uiDocumentID_,
				const SmartLocationList& cLocationList_);
#endif

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* getIterator();

protected:
	// 位置情報(実際はTFのみ)を1つ書き出す
	void insertLocation(const SmartLocationList& cLocationList_,
						OverflowPage::PagePointer& pLocPage_,
						OverflowPage::LocBlock& cLocBlock_);
	void insertLocation(InvertedIterator& cIterator_,
						OverflowPage::PagePointer& pLocPage_,
						OverflowPage::LocBlock& cLocBlock_);

	// ミドルリストを得る
	MiddleBaseList* makeTmpMiddleList(InvertedUnit& cInvertedUnit_,
									  LeafPage::Area* pTmpArea_);

};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MIDDLENOLOCATIONLIST_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
