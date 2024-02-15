// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListIterator.h --
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

#ifndef __SYDNEY_INVERTED_MIDDLELISTITERATOR_H
#define __SYDNEY_INVERTED_MIDDLELISTITERATOR_H

#include "Inverted/Module.h"
#include "Inverted/MiddleBaseListIterator.h"
#include "Inverted/LeafPage.h"
#include "Inverted/OverflowPage.h"

class ModInvertedLocationListIterator;
#ifdef DEBUG
class ModInvertedSmartLocationList;
#endif

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class MiddleList;
class OverflowFile;
class MiddleListLocationListIterator;

//
//	CLASS
//	Inverted::MiddleListIterator --
//
//	NOTES
//
//
class MiddleListIterator : public MiddleBaseListIterator
{
public:
	// コンストラクタ
	MiddleListIterator(MiddleList& cMiddleList_);
	// デストラクタ
	virtual ~MiddleListIterator();

	// 位置情報へのイテレータを得る
	ModInvertedLocationListIterator* getLocationListIterator();
	
#ifdef DEBUG
	void expunge();
	void undoExpunge(ModUInt32 uiDocumentID_,
					 const ModInvertedSmartLocationList& cLocationList_);
#endif

	// 次のブロックに続くか？
	bool isContinue();

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
	void setLocBlockByEmpty()
		{ m_cLocBlock = OverflowPage::LocBlock(); }
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
	ModSize getLocDataOffset() const { return m_uiCurrentLocDataOffset; }
	ModSize getLocDataLength() const { return m_uiCurrentLocDataLength; }

	// 開放された位置情報クラス
	ModInvertedLocationListIterator* getFreeLocationListIterator() const
		{ return m_pFree; }
	void setFreeLocationListIterator(ModInvertedLocationListIterator* ite_)
		{ m_pFree = ite_; }

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
	ModSize m_uiCurrentLocDataOffset;
	ModSize m_uiCurrentLocDataLength;
	// 文書頻度が1のときの位置情報
	ModUInt32 m_uiCurrentLocation;

	// 開放された位置情報クラス
	ModInvertedLocationListIterator* m_pFree;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MIDDLELISTITERATOR_H

//
//	Copyright (c) 2002, 2003, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
