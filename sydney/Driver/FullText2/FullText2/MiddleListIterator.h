// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_MIDDLELISTITERATOR_H
#define __SYDNEY_FULLTEXT2_MIDDLELISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/MiddleBaseListIterator.h"
#include "FullText2/LeafPage.h"
#include "FullText2/OverflowPage.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class MiddleList;
class OverflowFile;
class MiddleListLocationListIterator;

//
//	CLASS
//	FullText2::MiddleListIterator --
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

	// 位置情報内の文書頻度を得る
	ModSize getTermFrequency();
	// 位置情報へのイテレータを得る
	LocationListIterator::AutoPointer getLocationListIterator();

	// 現在位置のデータを削除する
	void expunge();

	// 現在位置にデータを挿入する
	void undoExpunge(ModUInt32 uiDocumentID_,
					 const SmartLocationList& cLocationList_);

	// 位置情報の先頭アドレスを得る
	ModUInt32* getHeadAddress();
	// 現在の位置情報のオフセットを得る
	ModSize getLocationOffset();
	// 現在の位置情報のビット長を得る
	ModSize getLocationBitLength();
	// 現在の位置情報データのオフセットを得る
	ModSize getLocationDataOffset();
	// 現在の位置情報データのビット長を得る
	ModSize getLocationDataBitLength();

	// 次のブロックに続くか？
	bool isContinue();

	// IDブロックを削除する
	bool expungeIdBlock();

	// コピーする
	ListIterator* copy() const;

private:
	// 位置情報の位置を文書IDの位置まで移動する
	void synchronize(bool bUndo_ = false);
	// 位置情報変数を設定する
	void setLocationInformation();

	// 位置情報を削除する
	void expungeLocation();
	// 位置情報の削除を取り消す
	void undoExpungeLocation(const SmartLocationList& cLocationList_);

	// LOCページ
	OverflowPage::PagePointer m_pLocPage;

	// 現在の位置情報へのポインタ
	ModUInt32* m_pHeadAddress;
	// 現在の位置情報のビット長
	ModSize m_uiBitLength;

	// 現在の位置情報の位置
	int m_iCurrentLocPosition;
	// 現在の位置情報
	ModSize m_uiCurrentLocOffset;
	ModSize m_uiCurrentLocLength;
	ModSize m_uiCurrentLocDataOffset;
	ModSize m_uiCurrentLocDataLength;
	// 現在の文書頻度
	ModSize m_uiCurrentFrequency;
	// 文書頻度が1のときの位置情報
	ModUInt32 m_uiCurrentLocation;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MIDDLELISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
