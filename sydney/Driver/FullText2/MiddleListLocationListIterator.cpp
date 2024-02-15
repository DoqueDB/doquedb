// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListLocationListIterator.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/MiddleListLocationListIterator.h"
#include "FullText2/MiddleListIterator.h"
#include "FullText2/OverflowFile.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::MiddleListLocationListIterator::MiddleListLocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::LocationListManager* pListManager_
//		位置情報リストイテレータ管理クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleListLocationListIterator::
MiddleListLocationListIterator(LocationListManager* pListManager_)
	: LocationListIterator(pListManager_), m_cLocation(0)
{
}

//
//	FUNCTION public
//	FullText2::MiddleListLocationListIterator::~MiddleListLocationListIterator
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
//	FullText2::MiddleListLocationListIterator::initialize -- 初期化
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OverflowFile* pOverflowFile_
//		オーバーフローファイル
//	FullText2::OverflowPage::PagePointer& pStartLocPage_
//		開始LOCページ
//	FullText2::OverflowPage::LocBlock& cStartLocBlock_
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
	ModSize uiLength_,
	OverflowPage::PagePointer& pStartLocPage_,
	OverflowPage::LocBlock& cStartLocBlock_,
	ModSize uiFrequency_,
	ModSize uiStartBitOffset_,
	ModSize uiBitLength_,
	ModInvertedCoder* pCoder_,
	ModUInt32 currentLocation_)
{
	m_pOverflowFile = pOverflowFile_;
	m_uiLength = uiLength_;
	m_uiFrequency = uiFrequency_;
	m_pStartLocPage = pStartLocPage_;
	m_cStartLocBlock = cStartLocBlock_;
	m_uiStartOffset = uiStartBitOffset_;
	m_uiEndOffset = uiStartBitOffset_ + uiBitLength_;
	m_pCoder = pCoder_;
	m_uiCurrentLocation = currentLocation_;
	m_bEnd = false;

	resetImpl();
}

//
//	FUNCTION public
//	FullText2::MiddleListLocationListIterator::lowerBound -- 下限検索
//
//	NOTES
//	下限検索であるが、カーソルは単純に大きな方に移動していくのみである
//	そのため、現在値より小さい location_ を与えても正しい結果は得られない
//	しかし、転置の検索ではそのほうが都合がいいのでそうしている
//
//	ARGUMENTS
//	ModSize location_
//		検索対象の位置
//	int& length_
//		検索語の長さ
//
//	RETURN
//	ModSize
//		ヒットした場合は位置データ、存在しない場合はUndefinedLocation
//
//	EXCEPTIONS
//
ModSize
MiddleListLocationListIterator::lowerBound(ModSize location_, int& length_)
{
	length_ = static_cast<int>(m_uiLength);
	
	if (m_uiFrequency == 1)
	{
		m_bEnd = true;
		if (location_ <= m_uiCurrentLocation)
		{
			return m_uiCurrentLocation;
		}
		return UndefinedLocation;
	}

	ModSize location = (location_ - m_uiFirstLocationData);
	ModSize maxLoc = 0;
	ModSize loc = m_cLocation.lowerBoundImpl(location, maxLoc);
	while (loc == UndefinedLocation)
	{
		m_uiCurrentLocation = maxLoc + m_uiFirstLocationData;
		if (nextLocBlock() == false)
			return UndefinedLocation;

		location = (location_ - m_uiFirstLocationData);
		maxLoc = 0;
		loc = m_cLocation.lowerBoundImpl(location, maxLoc);
	}

	m_uiCurrentLocation = loc + m_uiFirstLocationData;
	
	return m_uiCurrentLocation;
}

//
//	FUNCTION public
//	FullText2::MiddleListLocationListIterator::find -- 検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize uiLocation_
//		検索する位置データ
//
//	RETURN
//	bool
//		存在する場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MiddleListLocationListIterator::find(ModSize uiLocation_, int& length)
{
	length = static_cast<int>(m_uiLength);
	
	if (m_uiFrequency == 1)
	{
		return (m_uiCurrentLocation == uiLocation_) ? true : false;
	}
	
	if (m_uiFirstLocationData >= uiLocation_)
	{
		// 現在の位置情報クラス内よりも前にあるので、resetする
		resetImpl();
	}
	
	while (m_uiCurrentBitLength < uiLocation_ && m_bEnd == false)
	{
		// 位置情報クラスを進める
		m_uiCurrentLocation = m_uiCurrentBitLength;
		nextLocBlock();
	}
	
	if (m_bEnd == false)
	{
		// 今の位置情報クラス内に存在する
		if (m_cLocation.find(uiLocation_ - m_uiFirstLocationData, length)
			== true)
		{
			m_uiCurrentLocation = uiLocation_;
			return true;
		}
	}

	return false;
}

//
//	FUNCTION private
//	FullText2::MiddleListLocationListIterator::nextImpl -- 次へ
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
ModSize
MiddleListLocationListIterator::nextImpl()
{
	if (m_bEnd == true)
		return UndefinedLocation;

	if (m_uiFrequency == 1)
	{
		m_bEnd = true;
		return m_uiCurrentLocation;
	}
	
	ModSize location = m_cLocation.nextImpl();
	while (location == UndefinedLocation)
	{
		// 次ページのロックブロックへ
		if (nextLocBlock() == false)
			return UndefinedLocation;

		// 先頭の値を得る
		location = m_cLocation.nextImpl();
	}

	m_uiCurrentLocation = m_uiFirstLocationData + location;

	return m_uiCurrentLocation;
}

//
//	FUNCTION private
//	FullText2::MiddleListLocationListIterator::resetImpl -- 先頭へ
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
MiddleListLocationListIterator::resetImpl()
{
	m_bEnd = false;
	if (m_uiFrequency == 1)
		return;

	m_uiCurrentLocation = 0;
	m_uiFirstLocationData = 0;
	m_uiCurrentBitLength
		= m_cStartLocBlock.getDataBitLength() - m_uiStartOffset;
	m_pCurrentLocPage = m_pStartLocPage;
	OverflowPage::LocBlock cLocBlock = m_cStartLocBlock;
	ModSize uiStartBitOffset = m_uiStartOffset;

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
						   m_uiLength,
						   0,
						   uiStartBitOffset,
						   uiStartBitOffset + m_uiCurrentBitLength,
						   m_pCoder);
}

//
//	FUNCTION private
//	FullText2::MiddleListLocationListIterator::nextLocBlock
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
	if (m_uiCurrentBitLength >= (m_uiEndOffset - m_uiStartOffset))
	{
		// 終わり
		m_bEnd = true;
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
						   m_uiLength,
						   0,
						   0,	// 最初以外のロックブロックは必ず0から始まる
						   uiBitLength,
						   m_pCoder);

	m_uiFirstLocationData = m_uiCurrentLocation;

	return true;
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
