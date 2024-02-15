// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SmartLocationListIterator.h -- 位置情報リストを操作するイテレータ
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

#ifndef __SYDNEY_FULLTEXT2_SMARTLOCATIONLISTITERATOR_H
#define __SYDNEY_FULLTEXT2_SMARTLOCATIONLISTITERATOR_H

#include "FullText2/Module.h"
#include "FullText2/LocationListIterator.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::SmartLocationListIterator --
//
//	NOTES
//
class SmartLocationListIterator : public LocationListIterator
{
public:
	// コンストラクタ
	SmartLocationListIterator(const ModUInt32* pBuffer_,
			  ModSize uiBitLength_,
			  ModSize uiCount_,
			  ModInvertedCoder* pCoder_,
			  ModSize uiLength_)
		: LocationListIterator(0),
		  m_pBuffer(pBuffer_), m_uiBitLength(uiBitLength_),
		  m_uiCount(uiCount_), m_pCoder(pCoder_), m_uiLength(uiLength_)
		{
			m_uiCurrent = 0;
			m_uiCurrentBitOffset = 0;
		}
		
	// デストラクタ
	virtual ~SmartLocationListIterator() {}

	// 位置情報を検索する
	bool find(ModSize location_, int& length_)
		{
			if (lowerBound(location_, length_) == location_)
				return true;
			return false;
		}
	ModSize lowerBound(ModSize location_, int& length_)
		{
			ModSize loc;
			while ((loc = next(length_)) >= location_);
			return loc;
		}
		
	// カーソルを先頭に戻す
	void reset()
		{
			m_uiCurrent = 0;
			m_uiCurrentBitOffset = 0;
		}
		
	// 次の値を得る
	ModSize next(int& length_)
		{
			ModSize location = UndefinedLocation;
			if (m_uiCurrentBitOffset < m_uiBitLength)
			{
				ModSize loc;
				// 書かれているのは差分だけ
				m_pCoder->get(loc, m_pBuffer, m_uiBitLength,
							  m_uiCurrentBitOffset);
				m_uiCurrent += loc;
				location = m_uiCurrent;
				length_ = m_uiLength;
			}
			return location;
		}

	// 解放する
	bool release() { return false; }
	// 文書内頻度を得る
	ModSize getTermFrequency() { return m_uiCount; }
	   
private:
	// バッファ
	const ModUInt32* m_pBuffer;
	// ビット長
	ModSize m_uiBitLength;
	// 頻度
	ModSize m_uiCount;
	// 圧縮器
	ModInvertedCoder* m_pCoder;
	// トークンの長さ
	ModSize m_uiLength;

	// 現在の位置データ
	ModSize m_uiCurrent;
	// 現在のオフセット
	ModSize m_uiCurrentBitOffset;
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_SMARTLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
