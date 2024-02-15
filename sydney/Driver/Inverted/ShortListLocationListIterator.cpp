// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortListLocationListIterator.cpp --
// 
// Copyright (c) 2002, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/ShortListLocationListIterator.h"
#include "Inverted/ShortListIterator.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::ShortListLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	ShortListIterator* pIterator_
//		親クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortListLocationListIterator::ShortListLocationListIterator(
	ShortListIterator* pIterator_)
	: m_pIterator(pIterator_), nextBitOffset(0)
{
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::~ShortListLocationListIterator
//		-- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortListLocationListIterator::~ShortListLocationListIterator()
{
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::initialize -- 初期化する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32* pHeadAddress_
//		先頭アドレス
//	ModSize uiFrequency_
//		文書内頻度
//	ModSize uiStartOffset_
//		開始ビットオフセット
//	ModSize uiEndOffset_
//		終端ビットオフセット
//	ModInvertedCoder* pCoder_
//		圧縮器
//	ModUint32 uiCurrentLocation_
//		頻度が1時の位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
ShortListLocationListIterator::initialize(
	ModUInt32* pHeadAddress_,
	ModSize uiFrequency_,
	ModSize uiStartOffset_,
	ModSize uiEndOffset_,
	ModInvertedCoder* pCoder_,
	ModUInt32 uiCurrentLocation_)
{
	start = pHeadAddress_;
	number = uiFrequency_;
	startBitOffset = uiStartOffset_;
	endBitOffset = uiEndOffset_;
	locationCoder = pCoder_;
	currentLocation = uiCurrentLocation_;

	reset();
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::next -- 次へ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortListLocationListIterator::next()
{
	bool dummy;
	next(dummy);
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::next -- 次へ
//
//	NOTES
//
//	ARGUMENTS
//	bool& isEnd_
//		終端かどうか
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortListLocationListIterator::next(bool& isEnd_)
{
	isEnd_ = true;
	if (currentBitOffset >= endBitOffset)
		return;

	if ((currentBitOffset = nextBitOffset) < endBitOffset)
	{
		locationCoder->get(locationGap, start, endBitOffset, nextBitOffset);
		currentLocation += locationGap;
		isEnd_ = false;
	}
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::reset -- 先頭へ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortListLocationListIterator::reset()
{
	currentBitOffset = 0;
	if (number != 1)
	{
		nextBitOffset = startBitOffset;
		currentLocation = 0;
		bool dummy;
		next(dummy);
	}
	else
	{
		// 頻度が1のとき
		nextBitOffset = endBitOffset;
	}
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::lowerBound -- 下限検索
//
//	NOTES
//
//	ARGUMENTS
//	const ModSize target_
//	   検索対象の値
//
//	RETURN
//	ModBoolean
//		ヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
ShortListLocationListIterator::lowerBound(const ModSize target_)
{
	while (currentBitOffset < endBitOffset) {
		if (currentLocation >= target_) {
			return ModTrue;
		}
		bool dummy;
		next(dummy);
	}
	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::find -- 位置情報を検索する
//
//	NOTES
//	Unary Coderの場合のみこの関数はModTrueを返す可能性がある
//
//	ARGUMENTS
//	ModSize uiLocation_
//		検索する位置情報
//
//	RETURN
//	ModBoolean
//		存在する場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
ShortListLocationListIterator::find(ModSize uiLocation_)
{
	if (locationCoder->find(uiLocation_, start, endBitOffset, startBitOffset)
		== ModTrue)
	{
		currentBitOffset = startBitOffset + uiLocation_ - 1;
		nextBitOffset = startBitOffset + uiLocation_;
		currentLocation = uiLocation_;
		return ModTrue;
	}
	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::ShortListLocationListIterator::release -- 開放する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
ShortListLocationListIterator::release()
{
	if (m_pIterator) m_pIterator->pushBack(this);
}

//
//	Copyright (c) 2002, 2005, 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
