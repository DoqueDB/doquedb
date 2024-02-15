// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortListLocationListIterator.h --
// 
// Copyright (c) 2002, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_SHORTLISTLOCATIONLISTITERATOR_H
#define __SYDNEY_INVERTED_SHORTLISTLOCATIONLISTITERATOR_H

#include "Inverted/Module.h"
#include "ModInvertedCompressedLocationListIterator.h"
#include "ModInvertedCoder.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

class ShortListIterator;

//
//	CLASS
//	Inverted::ShortListLocationListIterator --
//
//	NOTES
//	ほとんどModInvertedCompressedLocationListIteratorの実装まま使用できるが、
//	実装方法が気に入らないので、すべて上書きすることにした
//
class ShortListLocationListIterator
	: public ModInvertedCompressedLocationListIterator
{
	friend class ShortListIterator;
	
public:
	//コンストラクタ
	ShortListLocationListIterator(ShortListIterator* pIterator_);
	//デストラクタ
	virtual ~ShortListLocationListIterator();

	// 初期化
	void initialize(ModUInt32* pHeadAddress_,
					ModSize uiFrequency_,
					ModSize uiStartOffset_,
					ModSize uiEndOffset_,
					ModInvertedCoder* pCode_,
					ModUInt32 uiCurrentLocation_ = 0);

	// 次へ
	void next();
	void next(bool& isEnd_);
	// 先頭へ
	void reset();

	// 位置情報を得る
	ModSize getLocation() { return getLocationImpl(); }
	ModSize getLocationImpl() { return currentLocation; }
	// 終端か
	ModBoolean isEnd() const { return (currentBitOffset >= endBitOffset) ? ModTrue : ModFalse; }

	// 下限検索
	ModBoolean lowerBound(const ModSize target_);
	// 位置情報を検索する(UNARYのみで使用可)
	ModBoolean find(ModSize uiLocation_);

	// ビット長を得る
	ModSize getBitLength() { return endBitOffset - startBitOffset; }

	// 開放する
	void release();

private:
	ShortListIterator* m_pIterator;
	
	// 次の読み込み位置
	ModSize nextBitOffset;
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SHORTLISTLOCATIONLISTITERATOR_H

//
//	Copyright (c) 2002, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
