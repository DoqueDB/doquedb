// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListLocationListIterator.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/MiddleListLocationListIterator.h"
#include "Inverted/MiddleListIterator.h"
#include "Inverted/OverflowFile.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::MiddleListLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::MiddleListItertor* pIterator_
//		親クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleListLocationListIterator::MiddleListLocationListIterator(
	MiddleListIterator* pIterator_)
	: m_pIterator(pIterator_), m_cLocation(0)
{
}

//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::~MiddleListLocationListIterator
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
MiddleListLocationListIterator::~MiddleListLocationListIterator()
{
}

//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::initialize -- 初期化
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::OverflowFile* pOverflowFile_
//		オーバーフローファイル
//	Inverted::OverflowPage::PagePointer& pStartLocPage_
//		開始LOCページ
//	Inverted::OverflowPage::LocBlock& cStartLocBlock_
//		開始LOCブロック
//	ModSize uiFrequency_
//		文書内頻度
//	ModSize uiStartBitOffset_
//		LOCブロック内のオフセット
//	ModSize uiBitLength_
//		総データビット長
//	ModInvertedCoder* pCoder_
//		圧縮器
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleListLocationListIterator::initialize(
	OverflowFile* pOverflowFile_,
	OverflowPage::PagePointer& pStartLocPage_,
	OverflowPage::LocBlock& cStartLocBlock_,
	ModSize uiFrequency_,
	ModSize uiStartBitOffset_,
	ModSize uiBitLength_,
	ModInvertedCoder* pCoder_,
	ModUInt32 currentLocation_)
{
	m_pOverflowFile = pOverflowFile_;
	number = uiFrequency_;
	m_pStartLocPage = pStartLocPage_;
	m_cStartLocBlock = cStartLocBlock_;
	startBitOffset = uiStartBitOffset_;
	endBitOffset = uiStartBitOffset_ + uiBitLength_;
	locationCoder = pCoder_;
	currentLocation = currentLocation_;

	reset();
}


//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::reset -- 先頭へ
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
MiddleListLocationListIterator::reset()
{
	currentBitOffset = 0;
	if (number != 1) currentLocation = 0;
	m_uiFirstLocationData = 0;
	m_bEnd = ModFalse;

	if (number == 1) return;
	
	m_uiCurrentBitLength
		= m_cStartLocBlock.getDataBitLength() - startBitOffset;
	m_pCurrentLocPage = m_pStartLocPage;
	OverflowPage::LocBlock cLocBlock = m_cStartLocBlock;
	ModSize uiStartBitOffset = startBitOffset;

	if (getBitLength() == 0) return;

	if (m_uiCurrentBitLength > getBitLength())
		m_uiCurrentBitLength = getBitLength();

	if (m_uiCurrentBitLength == 0)
	{
		// 最初のロックブロックには位置情報は何も書かれていない
		// 次のロックブロックを参照する
		
		m_pCurrentLocPage
			= m_pOverflowFile->attachPage(m_pCurrentLocPage->getNextPageID());
		cLocBlock = m_pCurrentLocPage->getLocBlock();
		uiStartBitOffset = 0;

		ModSize uiLocBlockBitLength = cLocBlock.getDataBitLength();
		if (getBitLength() > uiLocBlockBitLength)
		{
			m_uiCurrentBitLength = uiLocBlockBitLength;
		}
		else
		{
			m_uiCurrentBitLength = getBitLength();
		}
	}

		// 位置情報クラスを初期化する
	m_cLocation.initialize(cLocBlock.getBuffer(),
						   0,
						   uiStartBitOffset,
						   uiStartBitOffset + m_uiCurrentBitLength,
						   locationCoder);

	// 先頭の位置情報を得る
	currentLocation = m_cLocation.getLocationImpl();
}

//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::next -- 次へ
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
MiddleListLocationListIterator::next()
{
	if (m_bEnd == ModTrue)
		return;

	nextInternal();
}

//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::lowerBound -- 下限検索
//
//	NOTES
//
//	ARGUMENTS
//	const ModSize target_
//		検索対象の値
//
//	RETURN
//	ModBoolean
//		ヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
MiddleListLocationListIterator::lowerBound(const ModSize target_)
{
	while (m_bEnd == ModFalse) {
		if (currentLocation >= target_) {
			return ModTrue;
		}
		nextInternal();
	}
	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::find -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLocation_
//		検索する位置データ
//
//	RETURN
//	ModBoolean
//		ヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
MiddleListLocationListIterator::find(ModSize uiLocation_)
{
	if (number == 1)
	{
		return (currentLocation == uiLocation_) ? ModTrue : ModFalse;
	}
	
	if (m_uiFirstLocationData >= uiLocation_)
	{
		// 現在の位置情報クラス内よりも前にあるので、resetする
		reset();
	}
	
	while (m_uiCurrentBitLength < uiLocation_ && m_bEnd == ModFalse)
	{
		// 位置情報クラスを進める
		currentLocation = m_uiCurrentBitLength;
		nextLocBlock();
	}
	
	if (m_bEnd == ModFalse)
	{
		// 今の位置情報クラス内に存在する
		if (m_cLocation.find(uiLocation_ - m_uiFirstLocationData) == ModTrue)
		{
			currentLocation
				= m_uiFirstLocationData + m_cLocation.getLocationImpl();
			return ModTrue;
		}
	}

	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::MiddleListLocationListIterator::release
//		-- 開放する
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
MiddleListLocationListIterator::release()
{
	if (m_pIterator) m_pIterator->pushBack(this);
}

//
//	FUNCTION private
//	Inverted::MiddleListLocationListIterator::nextLocBlock
//		-- 次のロックブロックへ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		次のロックブロックが存在している場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	なし
//
bool
MiddleListLocationListIterator::nextLocBlock()
{
	if (m_uiCurrentBitLength >= (endBitOffset - startBitOffset))
	{
		// 終わり
		m_bEnd = ModTrue;
		return false;
	}
	
	m_pCurrentLocPage
		= m_pOverflowFile->attachPage(m_pCurrentLocPage->getNextPageID());
	OverflowPage::LocBlock cLocBlock = m_pCurrentLocPage->getLocBlock();
		
	ModSize uiBitLength = getBitLength() - m_uiCurrentBitLength;
	ModSize uiLocBlockBitLength = cLocBlock.getDataBitLength();
	if (uiBitLength > uiLocBlockBitLength)
		uiBitLength = uiLocBlockBitLength;

	m_uiCurrentBitLength += uiBitLength;

	// 位置情報クラスを初期化する
	m_cLocation.initialize(cLocBlock.getBuffer(),
						   0,
						   0,	// 最初以外のロックブロックは必ず0から始まる
						   uiBitLength,
						   locationCoder);

	m_uiFirstLocationData = currentLocation;

	return true;
}

//
//	FUNCTION private
//	Inverted::MiddleListLocationListIterator::nextInternal -- 次へ
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
MiddleListLocationListIterator::nextInternal()
{
	if (number == 1)
	{
		m_bEnd = ModTrue;
		return;
	}

	bool isEnd;
	m_cLocation.next(isEnd);

	if (isEnd == true)
	{
		// 次ページのロックブロックへ
		if (nextLocBlock() == false)
			return;
	}

	currentLocation = m_uiFirstLocationData + m_cLocation.getLocationImpl();
}

//
//	Copyright (c) 2002, 2005, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
