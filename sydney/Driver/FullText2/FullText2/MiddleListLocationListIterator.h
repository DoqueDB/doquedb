// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListLocationListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_MIDDLELISTLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_MIDDLELISTLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"
#include "FullText2/OverflowPage.h"
#include "FullText2/ShortListLocationListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class OverflowFile;
class MiddleListIterator;

//
//	CLASS
//	FullText2::MiddleListLocationListIterator --
//
//	NOTES
//
//
class MiddleListLocationListIterator : public LocationListIterator
{
	friend class MiddleListIterator;
	
public:
	//コンストラクタ
	MiddleListLocationListIterator(LocationListManager* pListManager_);
	//デストラクタ
	virtual ~MiddleListLocationListIterator();

	// 初期化
	void initialize(OverflowFile* pOverflowFile_,
					ModSize uiLength_,
					OverflowPage::PagePointer& pStartLocPage_,
					OverflowPage::LocBlock& cStartLocBlock_,
					ModSize uiFrequency_,
					ModSize uiStartBitOffset_,
					ModSize uiBitLength_,
					ModInvertedCoder* pCoder_,
					ModUInt32 currentLocation_);

	// 位置情報を検索する
	bool find(ModSize location_, int& length_);
	ModSize lowerBound(ModSize location_, int& length_);
	// カーソルを先頭に戻す
	void reset() { return resetImpl(); }
	// 次の値を得る
	ModSize next(int& length_)
		{
			length_ = static_cast<int>(m_uiLength);
			return nextImpl();
		}

	// 文書内頻度を得る
	ModSize getTermFrequency() { return m_uiFrequency; }
	
private:
	// ビット長を得る
	ModSize getBitLength() { return m_uiEndOffset - m_uiStartOffset; }
	
	// 次へ
	ModSize nextImpl();
	// リセット
	void resetImpl();
	// 次のロックブロックへ
	bool nextLocBlock();

	// OverflowFile
	OverflowFile* m_pOverflowFile;
	// 索引単位長
	ModSize m_uiLength;

	// 開始ページ
	OverflowPage::PagePointer m_pStartLocPage;
	// 開始ロックブロック
	OverflowPage::LocBlock m_cStartLocBlock;

	// 文書内頻度
	ModSize m_uiFrequency;
	// 位置情報
	ModUInt32 m_uiCurrentLocation;
	// 開始ビット位置
	ModSize m_uiStartOffset;
	// 終端ビット位置
	ModSize m_uiEndOffset;
	
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

	// ロケーションコーダー
	ModInvertedCoder* m_pCoder;

	// 終わりかどうか
	bool m_bEnd;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_MIDDLELISTLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
