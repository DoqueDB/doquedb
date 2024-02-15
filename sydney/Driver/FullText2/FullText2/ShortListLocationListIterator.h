// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortListLocationListIterator.h --
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

#ifndef __SYDNEY_FULLTEXT2_SHORTLISTLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_SHORTLISTLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"

#include "ModInvertedCoder.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

class ShortListIterator;

//
//	CLASS
//	FullText2::ShortListLocationListIterator --
//
//	NOTES
//
class ShortListLocationListIterator : public LocationListIterator
{
	friend class ShortListIterator;
	
public:
	//コンストラクタ
	ShortListLocationListIterator(LocationListManager* pListManager_);
	//デストラクタ
	virtual ~ShortListLocationListIterator();

	// 初期化
	void initialize(ModUInt32* pHeadAddress_,
					ModSize uiLength_,
					ModSize uiFrequency_,
					ModSize uiStartOffset_,
					ModSize uiEndOffset_,
					ModInvertedCoder* pCode_,
					ModUInt32 uiCurrentLocation_ = 0);

	// 位置情報を検索する
	bool find(ModSize location_, int& length_);
	ModSize lowerBound(ModSize location_, int& length_)
		{
			ModSize dummy;
			length_ = static_cast<int>(m_uiLength);
			return lowerBoundImpl(location_, dummy);
		}
	ModSize lowerBoundImpl(ModSize location_, ModSize& maxLocation_);
	// カーソルを先頭に戻す
	void reset() { return resetImpl(); }
	void resetImpl();
	// 次の値を得る
	ModSize next(int& length_)
		{
			length_ = static_cast<int>(m_uiLength);
			return nextImpl();
		}
	ModSize nextImpl();

	// ビット長を得る
	ModSize getBitLength() { return m_uiEndOffset - m_uiStartOffset; }

	// 文書内頻度を得る
	ModSize getTermFrequency() { return m_uiFrequency; }

private:
	// 終了か？
	bool isEndImpl() const { return (m_uiNextOffset == m_uiEndOffset); }
	
	// 先頭位置
	ModUInt32* m_pHeadAddress;
	// 文書内頻度
	ModSize m_uiFrequency;
	// 開始ビット位置
	ModSize m_uiStartOffset;
	// 終端ビット位置
	ModSize m_uiEndOffset;
	// ロケーションコーダー
	ModInvertedCoder* m_pCoder;
	// 現在の位置情報
	ModUInt32 m_uiCurrentLocation;
	// 索引語長
	ModSize m_uiLength;
		
	// 次の読み込み位置
	ModSize m_uiNextOffset;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SHORTLISTLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
