// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationList.h --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MIDDLENOLOCATIONLIST_H
#define __SYDNEY_INVERTED_MIDDLENOLOCATIONLIST_H

#include "Inverted/LeafPage.h"
#include "Inverted/MiddleBaseList.h"
#include "Inverted/Module.h"
#include "Inverted/OverflowPage.h"

class ModInvertedSmartLocationList;

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class InvertedIterator;
class InvertedUnit;
class OverflowFile;

//
//	CLASS
//	Inverted::MiddleNolocationList --
//
//	NOTES
//
//
class MiddleNolocationList : public MiddleBaseList
{
public:
	// コンストラクタ
	MiddleNolocationList(InvertedUnit& cInvertedUnit_,		// 転置ファイル
				LeafPage::PagePointer pLeafPage_,	// リーフページ
				LeafPage::Iterator ite_);			// リーフページのエリア
	// デストラクタ
	virtual ~MiddleNolocationList();

	// 位置情報を格納していないか
	bool isNolocation() const { return true; }
	// TFを格納していないか (TFを格納しない時は位置情報も格納しない)
	bool isNoTF() const { return false; }
	
#ifdef DEBUG
	bool insert(ModUInt32 uiDocumentID_,
				const ModInvertedSmartLocationList& cLocationList_);
#endif

	// イテレータを得る -- 得られたインスタンスは呼び出し元で開放する
	InvertedIterator* begin() const;

	// 位置情報(実際はTFのみ)の圧縮ビット長を得る
	ModSize getCompressedBitLengthLocationList(
		const ModInvertedSmartLocationList& cLocationList_,
		ModSize& uiBitLength_);
	// 位置情報(実際はTFのみ)を圧縮して書き出す
	void writeLocationList(
		const ModInvertedSmartLocationList& cLocationList_,
		ModSize uiBitLength_,
		ModUInt32* pHeadAddress_,
		ModSize& uiBitOffset_);

private:
	// 位置情報(実際はTFのみ)を1つ書き出す
	void insertLocation(const ModInvertedSmartLocationList& cLocationList_,
						OverflowPage::PagePointer& pLocPage_,
						OverflowPage::LocBlock& cLocBlock_);
	void insertLocation(InvertedIterator& cIterator_,
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

#endif //__SYDNEY_INVERTED_MIDDLENOLOCATIONLIST_H

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
