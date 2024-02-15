// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationListIterator.cpp --
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
#include "SyDynamicCast.h"
#include "FullText2/MiddleNolocationListIterator.h"
#include "FullText2/MiddleNolocationList.h"
#include "FullText2/OverflowFile.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/FakeError.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::MiddleNolocationListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::MiddleNolocationList& cMiddleList_
//		ミドルリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleNolocationListIterator::MiddleNolocationListIterator(
	MiddleNolocationList& cMiddleList_)
	: MiddleBaseListIterator(cMiddleList_),
	  m_pHeadAddress(0),
	  m_iCurrentLocPosition(-1)
{
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::~MiddleNolocationListIterator
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
MiddleNolocationListIterator::~MiddleNolocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::getTermFrequency
//		-- 位置情報内の頻度情報を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		位置情報内の頻度情報
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationListIterator::getTermFrequency()
{
	// 位置情報との同期を取る
	synchronize();

	return m_uiCurrentFrequency;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::expunge -- 現在位置の情報を削除する
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
MiddleNolocationListIterator::expunge()
{
	synchronize();
	
	//
	//	文書IDを削除する
	//
	if (m_cIdBlock.getFirstDocumentID() == m_uiCurrentID)
	{
		// 先頭文書IDを削除する
		expungeFirstDocumentID();
	}
	else
	{
		// 先頭以外の文書IDを削除する
		expungeDocumentID();
	}

	// エリアの文書数を設定する
	m_pArea->setDocumentCount(m_pArea->getDocumentCount() - 1);

	//
	//	位置情報を削除する
	//
	expungeLocation();

	; _FULLTEXT2_FAKE_ERROR(MiddleNolocationListIterator::expunge);
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::undoExpunge -- 削除の取り消し
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		文書ID
//	const FullText2::SmartLoctionList& cLocationList_
//		位置情報リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleNolocationListIterator::
undoExpunge(ModUInt32 uiDocumentID_,
			const SmartLocationList& cLocationList_)
{
	synchronize(true);

	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	// 位置情報を挿入する
	undoExpungeLocation(cLocationList_);

	m_pArea->setDocumentCount(m_pArea->getDocumentCount() + 1);

	; _FULLTEXT2_FAKE_ERROR(MiddleNolocationListIterator::undoExpunge);
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::getHeadAddress
//		-- 現在の位置情報の先頭アドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModUInt32*
//		現在の位置情報のアドレス
//
//	EXCEPTIONS
//
ModUInt32*
MiddleNolocationListIterator::getHeadAddress()
{
	synchronize();
	return m_cLocBlock.getBuffer();
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::getLocationOffset
//		-- 現在の位置情報のオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報のオフセット
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationListIterator::getLocationOffset()
{
	synchronize();
	return m_uiCurrentLocOffset;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::getLocationBitLength
//		-- 現在の位置情報のビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報のビット長
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationListIterator::getLocationBitLength()
{
	synchronize();
	return m_uiCurrentLocLength;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::getLocationDataOffset
//		-- 現在の位置情報データのオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報データのオフセット
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationListIterator::getLocationDataOffset()
{
	synchronize();
	return m_uiCurrentLocOffset + m_uiCurrentLocLength;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::getLocationDataBitLength
//		-- 現在の位置情報データのビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		現在の位置情報データのビット長
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationListIterator::getLocationDataBitLength()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::isContinue
//		-- 位置情報がページを跨いでいるかどうか
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		跨いでいる場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MiddleNolocationListIterator::isContinue()
{
	// 頻度しか書き込まないので、ページを跨ぐことはない
	return false;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::expungeIdBlock
//		-- 現在位置のIDブロックと対応するLOCブロックを削除する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	bool
//		IDページを削除したらtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
MiddleNolocationListIterator::expungeIdBlock()
{
	if (m_cIdBlock.isExpunge() == true)
	{
		// まずLOCブロックを削除する
		PhysicalFile::PageID uiLocPageID = m_cIdBlock.getLocBlockPageID();
		unsigned short usOffset = m_cIdBlock.getLocBlockOffset();
		while (uiLocPageID != PhysicalFile::ConstValue::UndefinedPageID)
		{
			m_pLocPage = m_pOverflowFile->attachPage(uiLocPageID);
			m_cLocBlock = m_pLocPage->getLocBlock(usOffset);
			usOffset = 0;

			if (m_cLocBlock.isContinue())
			{
				// 次があったら
				uiLocPageID = m_pLocPage->getNextPageID();
			}
			else
			{
				// なかったら
				uiLocPageID = PhysicalFile::ConstValue::UndefinedPageID;
			}

			if (m_pArea->getLastLocationPageID() == m_pLocPage->getID()
				&& m_cLocBlock.getOffset() == m_pArea->getLocationOffset())
			{
				// 最終IDブロックの先頭のLOCブロック -> 続きフラグを削除する
				m_cLocBlock.unsetContinueFlag();
			}
			else
			{
				// LOCブロックを開放する
				m_pLocPage->freeLocBlock();
			}

			// 最終LOCページは削除できない
			if (m_pLocPage->getType() == OverflowPage::Type::LOC
				&& m_pLocPage->getLocBlockCount() == 0
				&& m_pArea->getLastLocationPageID() != m_pLocPage->getID())
			{
				// このページは不要
				m_pOverflowFile->freePage(m_pLocPage);
			}
		}

		if (m_cIdBlock != m_pArea->getHeadAddress())
		{
			// IDブロックを削除する
			m_pIdPage->freeIDBlock(m_uiIDBlockPosition);

			ModSize idBlockCount = m_pIdPage->getIDBlockCount();

			// 最終LOCページは削除できない
			if ((m_pIdPage->getType() == OverflowPage::Type::ID &&
				  m_pIdPage->getIDBlockCount() == 0) ||
				 (m_pIdPage->getType() == OverflowPage::Type::IDLOC &&
				  m_pIdPage->getIDBlockCount() == 0 &&
				  m_pIdPage->getLocBlockCount() == 0 &&
				  m_pArea->getLastLocationPageID() != m_pIdPage->getID()))
			{
				// このページは不要
				m_pOverflowFile->freePage(m_pIdPage);
			}

			if (idBlockCount == 0)
			{
				// DIRブロックを削除する
				int count = m_pArea->getDirBlockCount();
				for (int i = m_iDirBlockPosition; i < count-1; ++i)
				{
					m_pDirBlock[i] = m_pDirBlock[i+1];
				}

				return true;
			}
		}
	}

	return false;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationListIterator::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListIterator*
//		コピーしたオブジェクト
//
//	EXCEPTIONS
//
ListIterator*
MiddleNolocationListIterator::copy() const
{
	return new MiddleNolocationListIterator(*this);
}

//
//	FUNCTION private
//	FullText2::MiddleNolocationListIterator::synchronize
//		-- 現在の文書IDの位置に位置情報を移動する
//
//	NOTES
//
//	ARGUMENTS
//	bool bUndo_
//		Undo中かどうか (default false)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleNolocationListIterator::synchronize(bool bUndo_)
{
	if (m_bSynchronized == false)
	{
		// 現在のIDブロックに対応したLOCブロックか？
		if (m_cLocBlock.isInvalid() == true)
		{
			if (m_cIdBlock.isInvalid() == true)
			{
				nextIdBlock();
			}

			// 対応していないので対応したものを設定する
			m_uiBitLength = 0;
			PhysicalFile::PageID uiNextPageID = m_cIdBlock.getLocBlockPageID();
			unsigned short usOffset = m_cIdBlock.getLocBlockOffset();
			while (m_uiBitLength == 0)
			{
				if (m_pLocPage == 0 || m_pLocPage->getID() != uiNextPageID)
				{
					// 現在のページと異なっているので新しいページをattachする
					m_pLocPage = m_pOverflowFile->attachPage(uiNextPageID);
				}
				m_cLocBlock = m_pLocPage->getLocBlock(usOffset);
				m_iCurrentLocPosition = 0;
				m_uiCurrentLocOffset = 0;
				m_uiCurrentLocLength = 0;
				m_pHeadAddress = m_cLocBlock.getBuffer();
				m_uiBitLength = m_cLocBlock.getDataBitLength();

				uiNextPageID = m_pLocPage->getNextPageID();
				usOffset = 0;

				if (bUndo_ == true && m_iCurrentPosition == 0)
					break;
			}
		}

		if (m_uiCurrentLocLength != 0)
		{
			// 現在位置の位置情報がすでに読み込まれている

			m_uiCurrentLocOffset += m_uiCurrentLocLength;
			m_uiCurrentLocLength = 0;

			m_iCurrentLocPosition++;

			while (m_uiCurrentLocOffset >= m_uiBitLength)
			{
				if (bUndo_ == true
					&& m_iCurrentPosition == m_iCurrentLocPosition
					&& m_uiCurrentLocOffset == m_uiBitLength)
					break;

				// 次のLOCブロックへ
				m_pLocPage
					= m_pOverflowFile->attachPage(m_pLocPage->getNextPageID());
				m_cLocBlock = m_pLocPage->getLocBlock();
				m_uiCurrentLocOffset -= m_uiBitLength;
				m_pHeadAddress = m_cLocBlock.getBuffer();
				m_uiBitLength = m_cLocBlock.getDataBitLength();
			}
		}

		while (m_iCurrentPosition != m_iCurrentLocPosition)
		{
			//
			//	文書IDの位置まで位置情報を読み捨てる
			//

			// 頻度を読む
			ModSize frequency
				= m_cInvertedList.readLocationFrequency(m_pHeadAddress,
														m_uiBitLength,
														m_uiCurrentLocOffset);

			m_iCurrentLocPosition++;

			while (m_uiCurrentLocOffset >= m_uiBitLength)
			{
				if (bUndo_ == true
					&& m_iCurrentPosition == m_iCurrentLocPosition
					&& m_uiCurrentLocOffset == m_uiBitLength)
					break;

				// 次のLOCブロックへ
				m_pLocPage
					= m_pOverflowFile->attachPage(m_pLocPage->getNextPageID());
				m_cLocBlock = m_pLocPage->getLocBlock();
				m_uiCurrentLocOffset -= m_uiBitLength;
				m_pHeadAddress = m_cLocBlock.getBuffer();
				m_uiBitLength = m_cLocBlock.getDataBitLength();
			}
		}

		if (bUndo_ == false)
		{
			// 位置情報を設定する
			setLocationInformation();
		}

		m_bSynchronized = true;
	}
}

//
//	FUNCTION private
//	FullText2::MiddleNolocationListIterator::setLocationInfomation
//		-- 位置情報変数を設定する
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
MiddleNolocationListIterator::setLocationInformation()
{
	ModSize uiOffset = m_uiCurrentLocOffset;

	//
	//【注意】
	//		頻度とビット長(頻度が1の場合は位置情報)の間で
	//		ページを跨いでいることはない
	//

	// 頻度を読む
	m_uiCurrentFrequency = m_cInvertedList.readLocationFrequency(m_pHeadAddress,
																 m_uiBitLength,
																 uiOffset);
	// ビット長を設定する
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	FullText2::MiddleNolocationListIterator::expungeLocation
//		-- 位置情報を削除する
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
MiddleNolocationListIterator::expungeLocation()
{
	ModSize bitLength = m_uiCurrentLocLength;
	while (bitLength)
	{
		ModSize currentBitLength = bitLength;
		if (currentBitLength + m_uiCurrentLocOffset > m_uiBitLength)
		{
			// 現在の位置情報はLOCブロックを跨いでいる
			currentBitLength = m_uiBitLength - m_uiCurrentLocOffset;
		}

		m_pLocPage->dirty();

		// 削除する
		ModSize uiDstOffset = m_uiCurrentLocOffset;
		ModSize uiSrcOffset = m_uiCurrentLocOffset + currentBitLength;
		ModSize uiLength = m_uiBitLength - uiSrcOffset;
		if (uiLength)
		{
			InvertedList::move(m_pHeadAddress, uiDstOffset,
								m_pHeadAddress, uiSrcOffset, uiLength);
		}
		InvertedList::setOff(m_pHeadAddress,
							 uiDstOffset + uiLength,
							 uiSrcOffset - uiDstOffset);
		m_uiBitLength -= currentBitLength;
		m_cLocBlock.setDataBitLength(m_uiBitLength);

		bitLength -= currentBitLength;

		if (bitLength)
		{
			// 次のLOCブロックへ
			m_pLocPage
				= m_pOverflowFile->attachPage(m_pLocPage->getNextPageID());
			m_cLocBlock = m_pLocPage->getLocBlock();
			m_uiCurrentLocOffset = 0;
			m_pHeadAddress = m_cLocBlock.getBuffer();
			m_uiBitLength = m_cLocBlock.getDataBitLength();
		}
	}

	if (m_bSynchronized == true)
	{
		while (m_uiBitLength == m_uiCurrentLocOffset)
		{
			// 次のLOCブロックへ
			m_pLocPage
				= m_pOverflowFile->attachPage(m_pLocPage->getNextPageID());
			m_cLocBlock = m_pLocPage->getLocBlock();
			m_uiCurrentLocOffset = 0;
			m_pHeadAddress = m_cLocBlock.getBuffer();
			m_uiBitLength = m_cLocBlock.getDataBitLength();
		}

		setLocationInformation();
	}
	else
	{
		// ロックブロックをクリアする
		m_cLocBlock = OverflowPage::LocBlock();
	}
}

//
//	FUNCTION private
//	FullText2::MiddleNolocationListIterator::undoExpungeLocation
//		-- 位置情報の削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::SmartLocationList& cLocationList_
//		位置情報リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
MiddleNolocationListIterator::
undoExpungeLocation(const SmartLocationList& cLocationList_)
{
	ModSize frequency = cLocationList_.getCount();
	ModSize length = m_cInvertedList.getCompressedBitLengthFrequency(frequency);
	
	// 今のLOCブロックに書き込めるビット長を得る
	ModSize maxBitLength
		= m_cLocBlock.getDataUnitSize() * sizeof(ModUInt32) * 8;

	if (length + m_uiBitLength > maxBitLength)
	{
		// このブロックには何も書けない -> 次のブロックへ
		m_pLocPage
			= m_pOverflowFile->attachPage(m_pLocPage->getNextPageID());
		m_cLocBlock = m_pLocPage->getLocBlock();
		m_uiCurrentLocOffset = 0;
		m_pHeadAddress = m_cLocBlock.getBuffer();
		m_uiBitLength = m_cLocBlock.getDataBitLength();
				
		maxBitLength
			= m_cLocBlock.getDataUnitSize() * sizeof(ModUInt32) * 8;
	}

	m_pLocPage->dirty();

	// 書き込む分移動する
	ModSize uiDstOffset = m_uiCurrentLocOffset + length;
	ModSize uiSrcOffset = m_uiCurrentLocOffset;
	ModSize uiLength = m_uiBitLength - m_uiCurrentLocOffset;
	InvertedList::move(m_pHeadAddress, uiDstOffset,
					   m_pHeadAddress, uiSrcOffset, uiLength);
	InvertedList::setOff(m_pHeadAddress, uiSrcOffset,
						 uiDstOffset - uiSrcOffset);

	// 頻度を書く
	m_cInvertedList.writeLocationFrequency(frequency,
										   m_pHeadAddress,
										   m_uiCurrentLocOffset);

	m_cLocBlock.setDataBitLength(m_uiBitLength + length);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
