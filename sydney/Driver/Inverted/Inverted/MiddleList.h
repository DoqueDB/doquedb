// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleList.h --
// 
// Copyright (c) 2002, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MIDDLELIST_H
#define __SYDNEY_INVERTED_MIDDLELIST_H

#include "Inverted/Module.h"
#include "Inverted/MiddleBaseList.h"
#include "Inverted/LeafPage.h"
#include "Inverted/OverflowPage.h"

class ModInvertedLocationListIterator;
class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedIterator;
class InvertedUnit;
class OverflowFile;

//
//	CLASS
//	Inverted::MiddleList --
//
//	NOTES
//
//
class MiddleList : public MiddleBaseList
{
public:
	// コンストラクタ
	MiddleList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				LeafPage::PagePointer pLeafPage_,	// リーフページ
				LeafPage::Iterator ite_);			// リーフページのエリア
	// デストラクタ
	virtual ~MiddleList();

	// 位置情報を格納していないか
	bool isNolocation() const { return false; }
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	bool isNoTF() const { return false; }
	
#ifdef DEBUG
	bool insert(ModUInt32 uiDocumentID_,
				const ModInvertedSmartLocationList& cLocationList_);
#endif

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* begin() const;

private:
	// 位置情報を1つ書き出す
	void insertLocation(const ModInvertedSmartLocationList& cLocationList_,
						OverflowPage::PagePointer& pLocPage_,
						OverflowPage::LocBlock& cLocBlock_);
	void insertLocation(InvertedIterator& cIterator_,
						OverflowPage::PagePointer& pLocPage_,
						OverflowPage::LocBlock& cLocBlock_);

	// 頻度、ビット長、位置情報リストを順々に書き出す
	ModSize insertOneByOne(ModInvertedLocationListIterator* ite_,
						   OverflowPage::PagePointer& pLocPage_,
						   OverflowPage::LocBlock& cLocBlock_,
						   ModSize uiFrequency_,
						   ModSize uiBitLength_,
						   ModSize uiLocLength_);
	
	// 頻度とビット長、または、頻度と位置を書く
	ModSize writeFrequencyAndBitLength(OverflowPage::PagePointer& pLocPage_,
									   OverflowPage::LocBlock& cLocBlock_,
									   ModSize uiFrequency_,
									   ModSize uiBitLength_);
	
	// 位置情報データを一つずつ書く
	void insertLocationData(ModInvertedLocationListIterator* ite_,
							ModSize uiLocLength_,
							OverflowPage::PagePointer& pLocPage_,
							OverflowPage::LocBlock& cLocBlock_);
	
	// オーバフローファイル
	OverflowFile* getOverflowFile() const { return m_pOverflowFile; }
	void setOverflowFile(OverflowFile* pOverflowFile_)
		{ m_pOverflowFile = pOverflowFile_; }
	
	// オーバフローファイル
	OverflowFile* m_pOverflowFile;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MIDDLELIST_H

//
//	Copyright (c) 2002, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
