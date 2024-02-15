// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListLocationListIterator.h --
// 
// Copyright (c) 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_MIDDLELISTLOCATIONLISTITERATOR_H
#define __SYDNEY_INVERTED_MIDDLELISTLOCATIONLISTITERATOR_H

#include "Inverted/Module.h"
#include "Inverted/OverflowPage.h"
#include "Inverted/ShortListLocationListIterator.h"

#include "ModInvertedCompressedLocationListIterator.h"
#include "ModPair.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class OverflowFile;
class MiddleListIterator;

//
//	CLASS
//	Inverted::MiddleListLocationListIterator --
//
//	NOTES
//
//
class MiddleListLocationListIterator
	: public ModInvertedCompressedLocationListIterator
{
	friend class MiddleListIterator;
	
public:
	//コンストラクタ
	MiddleListLocationListIterator(MiddleListIterator* pIterator_);
	//デストラクタ
	virtual ~MiddleListLocationListIterator();

	// 初期化
	void initialize(OverflowFile* pOverflowFile_,
					OverflowPage::PagePointer& pStartLocPage_,
					OverflowPage::LocBlock& cStartLocBlock_,
					ModSize uiFrequency_,
					ModSize uiStartBitOffset_,
					ModSize uiBitLength_,
					ModInvertedCoder* pCoder_,
					ModUInt32 currentLocation_);
	
	// 次へ
	void next();
	// 先頭へ
	void reset();

	// 終端か
	ModBoolean isEnd() const { return m_bEnd; }
	// 現在位置の位置情報を得る
	ModSize getLocation() { return currentLocation; }

	// 下限検索
	ModBoolean lowerBound(const ModSize target_);
	// 検索する
	ModBoolean find(ModSize uiLocation_);

	// ビット長を得る
	ModSize getBitLength() { return endBitOffset - startBitOffset; }
	
	// 開放する
	void release();

private:
	// 次のロックブロックへ
	bool nextLocBlock();
	// 次へ
	void nextInternal();

	MiddleListIterator* m_pIterator;

	// OverflowFile
	OverflowFile* m_pOverflowFile;

	// 開始ページ
	OverflowPage::PagePointer m_pStartLocPage;
	// 開始ロックブロック
	OverflowPage::LocBlock m_cStartLocBlock;
	
	// 現在のページ
	OverflowPage::PagePointer m_pCurrentLocPage;
	// 現在のロックブロックのビット長
	ModSize m_uiCurrentBitLength;

	// 現在の位置情報の最小位置情報
	ModSize m_uiFirstLocationData;
	// 現在の位置情報
	ShortListLocationListIterator m_cLocation;
	// 現在の位置情報の位置
	ModSize m_uiCurrentPosition;

	// 終わりかどうか
	ModBoolean m_bEnd;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_MIDDLELISTLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2002, 2005, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
