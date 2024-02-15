// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationListIterator.h --
// 
// Copyright (c) 2002, 2003, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MIDDLENOLOCATIONLISTITERATOR_H
#define __SYDNEY_INVERTED_MIDDLENOLOCATIONLISTITERATOR_H

#include "Inverted/LeafPage.h"
#include "Inverted/MiddleBaseListIterator.h"
#include "Inverted/Module.h"
#include "Inverted/OverflowPage.h"

#ifdef DEBUG
class ModInvertedSmartLocationList;
#endif

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class MiddleNolocationList;
class OverflowFile;

//
//	CLASS
//	Inverted::MiddleNolocationListIterator --
//
//	NOTES
//
//
class MiddleNolocationListIterator : public MiddleBaseListIterator
{
public:
	// コンストラクタ
	MiddleNolocationListIterator(MiddleNolocationList& cMiddleNolocationList_);
	// デストラクタ
	virtual ~MiddleNolocationListIterator();

#ifdef DEBUG
	void expunge();
	void undoExpunge(ModUInt32 uiDocumentID_,
					 const ModInvertedSmartLocationList& cLocationList_);
#endif

private:
	// 位置情報の位置を文書IDの位置まで移動する (引数1個)
	void doSynchronize(bool bUndo_);
	
	// 位置情報変数を設定する
	void setLocationInformation();
	// 位置情報変数を読み飛ばす
	void skipLocationInformation();

	//
	// アクセッサ
	//
	
	// LOCページ
	OverflowPage::PagePointer getLocPage() const { return m_pLocPage; }
	void setLocPage(OverflowPage* pPage_) { m_pLocPage = pPage_; }

	// LOCブロック
	OverflowPage::LocBlock& getLocBlock() { return m_cLocBlock; }
	void setLocBlock(OverflowPage::LocBlock cLocBlock_)
		{ m_cLocBlock = cLocBlock_; }
	void setLocBlockByEmpty() { m_cLocBlock = OverflowPage::LocBlock(); }
	// LOCブロックのデータ部の先頭アドレス
	ModUInt32* getLocBlockDataHeadAddress() { return m_pHeadAddress; }
	void setLocBlockDataHeadAddress(ModUInt32* p_) { m_pHeadAddress = p_; }
	// LOCブロックのデータ部のビット長
	ModSize getLocBlockDataLength() const { return m_uiBitLength; }
	void setLocBlockDataLength(ModSize uiLength_) { m_uiBitLength = uiLength_; }
	
	// 位置情報
	ModSize getLocOffset() const { return m_uiCurrentLocOffset; }
	void setLocOffset(ModSize uiOffset_) { m_uiCurrentLocOffset = uiOffset_; }
	ModSize getLocLength() const { return m_uiCurrentLocLength; }

	//
	// メンバ変数
	//
	
	// LOCブロック
	OverflowPage::LocBlock m_cLocBlock;
	// Locページ
	OverflowPage::PagePointer m_pLocPage;

	// 現在のLOCブロックのデータ部の先頭ポインタ
	ModUInt32* m_pHeadAddress;
	// 現在のLOCブロックのデータ部のビット長
	ModSize m_uiBitLength;

	// 現在の位置情報の位置
	int m_iCurrentLocPosition;
	// 現在の位置情報
	ModSize m_uiCurrentLocOffset;
	ModSize m_uiCurrentLocLength;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MIDDLENOLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2002, 2003, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
