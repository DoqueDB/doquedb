// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListIterator.cpp --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/MiddleListIterator.h"
#include "FullText2/MiddleList.h"
#include "FullText2/OverflowFile.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/MiddleListLocationListIterator.h"
#include "FullText2/FakeError.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::MiddleListIterator::MiddleListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::MiddleList& cMiddleList_
//		ミドルリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleListIterator::MiddleListIterator(MiddleList& cMiddleList_)
	: MiddleBaseListIterator(cMiddleList_),
	  m_pHeadAddress(0),
	  m_iCurrentLocPosition(-1)
{
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::~MiddleListIterator -- デストラクタ
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
MiddleListIterator::~MiddleListIterator()
{
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::getTermFrequency
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
MiddleListIterator::getTermFrequency()
{
	// 位置情報との同期を取る
	synchronize();

	return m_uiCurrentFrequency;
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::getLocationListIterator
//		-- 現在の位置情報のイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedLocationListIterator*
//		位置情報のイテレータ
//
//	EXCEPTIONS
//
LocationListIterator::AutoPointer
MiddleListIterator::getLocationListIterator()
{
	synchronize();

	MiddleListLocationListIterator* locations
		= _SYDNEY_DYNAMIC_CAST(MiddleListLocationListIterator*, getFree());
	if (locations == 0)
		locations = new MiddleListLocationListIterator(this);

	LocationListIterator::AutoPointer p(locations);
	locations->initialize(m_pOverflowFile,
						  m_pArea->getKeyLength(),
						  m_pLocPage,
						  m_cLocBlock,
						  m_uiCurrentFrequency,
						  m_uiCurrentLocDataOffset,
						  m_uiCurrentLocDataLength,
						  m_cInvertedList.getLocationCoder(),
						  m_uiCurrentLocation);
	
	return p;
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::expunge -- 現在位置の情報を削除する
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
MiddleListIterator::expunge()
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

	; _FULLTEXT2_FAKE_ERROR(MiddleListIterator::expunge);
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::undoExpunge -- 削除の取り消し
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
MiddleListIterator::undoExpunge(ModUInt32 uiDocumentID_,
								const SmartLocationList& cLocationList_)
{
	synchronize(true);

	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	// 位置情報を挿入する
	undoExpungeLocation(cLocationList_);

	m_pArea->setDocumentCount(m_pArea->getDocumentCount() + 1);

	; _FULLTEXT2_FAKE_ERROR(MiddleListIterator::undoExpunge);
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::getHeadAddress
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
MiddleListIterator::getHeadAddress()
{
	synchronize();
	return m_cLocBlock.getBuffer();
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::getLocationOffset
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
MiddleListIterator::getLocationOffset()
{
	synchronize();
	return m_uiCurrentLocOffset;
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::getLocationBitLength
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
MiddleListIterator::getLocationBitLength()
{
	synchronize();
	return m_uiCurrentLocLength;
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::getLocationDataOffset
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
MiddleListIterator::getLocationDataOffset()
{
	synchronize();
	return m_uiCurrentLocDataOffset;
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::getLocationDataBitLength
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
MiddleListIterator::getLocationDataBitLength()
{
	synchronize();
	return m_uiCurrentLocDataLength;
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::isContinue
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
MiddleListIterator::isContinue()
{
	synchronize();
	if (m_uiCurrentLocOffset + m_uiCurrentLocLength > m_uiBitLength)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::MiddleListIterator::expungeIdBlock
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
MiddleListIterator::expungeIdBlock()
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
//	FullText2::MiddleListIterator::copy -- コピーを得る
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
MiddleListIterator::copy() const
{
	return new MiddleListIterator(*this);
}

//
//	FUNCTION private
//	FullText2::MiddleListIterator::synchronize
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
MiddleListIterator::synchronize(bool bUndo_)
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

			++m_iCurrentLocPosition;

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
			if (frequency == 1)
			{
				// 頻度が1なので、ビット長は書かれていない
				m_cInvertedList.readLocationData(0,
												 m_pHeadAddress,
												 m_uiBitLength,
												 m_uiCurrentLocOffset);
			}
			else
			{
				// ビット長を読む
				ModSize length =
					m_cInvertedList.readLocationBitLength(m_pHeadAddress,
														  m_uiBitLength,
														  m_uiCurrentLocOffset);
				m_uiCurrentLocOffset += length;
			}

			++m_iCurrentLocPosition;

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
//	FullText2::MiddleListIterator::setLocationInfomation
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
MiddleListIterator::setLocationInformation()
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
	if (m_uiCurrentFrequency == 1)
	{
		// 位置情報データの先頭位置
		m_uiCurrentLocDataOffset = uiOffset;
		// 頻度が1なので、ビット長は書かれていない
		m_uiCurrentLocation = m_cInvertedList.readLocationData(0,
															   m_pHeadAddress,
															   m_uiBitLength,
															   uiOffset);
		// 位置情報データのビット長
		m_uiCurrentLocDataLength = uiOffset - m_uiCurrentLocDataOffset;
	}
	else
	{
		m_uiCurrentLocation = 0;
		// ビット長を読む
		m_uiCurrentLocDataLength
			= m_cInvertedList.readLocationBitLength(m_pHeadAddress,
													m_uiBitLength,
													uiOffset);
		// 位置情報データの先頭位置
		m_uiCurrentLocDataOffset = uiOffset;
		uiOffset += m_uiCurrentLocDataLength;
	}
	// ビット長を設定する
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	FullText2::MiddleListIterator::expungeLocation -- 位置情報を削除する
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
MiddleListIterator::expungeLocation()
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
//	FullText2::MiddleListIterator::undoExpungeLocation
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
MiddleListIterator::undoExpungeLocation(const SmartLocationList& cLocationList_)
{
	ModSize bitLength;
	ModSize length
		= m_cInvertedList.getCompressedBitLengthLocationList(cLocationList_,
															 bitLength);
	LocationListIterator::AutoPointer i;
	LocationListIterator::AutoPointer j;
	ModUInt32 uiPrevLocation = 0;
	ModUInt32 uiCurrentLocation = 0;
	ModUInt32 loc = 0;
	bool initial = true;
	int dummy;

	while (length)
	{
		// 今のLOCブロックに書き込めるビット長を得る
		ModSize maxBitLength
			= m_cLocBlock.getDataUnitSize() * sizeof(ModUInt32) * 8;
		ModSize currentLength = length;
		
		while (currentLength + m_uiBitLength > maxBitLength)
		{
			// いっぺんに書き込めないので、書き込める最大値を求める
			currentLength = 0;

			if (initial == true)
			{
				// 頻度とビット長
				ModSize frequency = cLocationList_.getCount();
				currentLength +=
					m_cInvertedList.getCompressedBitLengthFrequency(frequency);
				if (frequency > 1)
					currentLength += m_cInvertedList
						.getCompressedBitLengthBitLength(bitLength);

				if (currentLength + m_uiBitLength > maxBitLength)
				{
					// このブロックには何も書けない -> 次のブロックへ
					m_pLocPage = m_pOverflowFile->attachPage(
						m_pLocPage->getNextPageID());
					m_cLocBlock = m_pLocPage->getLocBlock();
					m_uiCurrentLocOffset = 0;
					m_pHeadAddress = m_cLocBlock.getBuffer();
					m_uiBitLength = m_cLocBlock.getDataBitLength();
				
					maxBitLength
						= m_cLocBlock.getDataUnitSize() * sizeof(ModUInt32) * 8;
					currentLength = length;

					continue;
				}
			
				i = cLocationList_.getIterator();	// 書き込み用
				j = cLocationList_.getIterator();	// サイズ用
				loc = j->next(dummy);	// 先頭の位置情報
			}
			
			ModSize l = 0;
			ModUInt32 prev = uiPrevLocation;
			while (currentLength <= maxBitLength - m_uiBitLength)
			{
				// 長さがぴったりになっても、次に行く
				// 終端の場合はここには来ないので、それでいい
				
				l = m_cInvertedList.getCompressedBitLengthLocation(prev, loc);
				currentLength += l;
				prev = loc;
				
				if (currentLength <= maxBitLength - m_uiBitLength)
					// 超えていなかったら次へ
					loc = j->next(dummy);
			}
			
			currentLength -= l;	// 超えちゃったので引く
		}

		m_pLocPage->dirty();

		// 書き込む分移動する
		ModSize uiDstOffset = m_uiCurrentLocOffset + currentLength;
		ModSize uiSrcOffset = m_uiCurrentLocOffset;
		ModSize uiLength = m_uiBitLength - m_uiCurrentLocOffset;
		InvertedList::move(m_pHeadAddress, uiDstOffset,
							m_pHeadAddress, uiSrcOffset, uiLength);
		InvertedList::setOff(m_pHeadAddress, uiSrcOffset,
							 uiDstOffset - uiSrcOffset);

		// 書き込む
		if (length == currentLength)
		{
			// そのまま書き込める
			if (initial == true)
				// すべてのデータを一度に書き込む
				m_cInvertedList.writeLocationList(cLocationList_,
												  bitLength,
												  m_pHeadAddress,
												  m_uiCurrentLocOffset);
			else
				// 残りのデータを一度に書き込む
				m_cInvertedList.writeLocationData(uiPrevLocation,
												  uiCurrentLocation,
												  i.get(),
												  m_pHeadAddress,
												  m_uiCurrentLocOffset);
		}
		else
		{
			if (initial == true)
			{
				ModUInt32 saveCurrent = m_uiCurrentLocOffset;
				// 頻度と位置情報を書く
				ModSize frequency = cLocationList_.getCount();
				m_cInvertedList.writeLocationFrequency(frequency,
													   m_pHeadAddress,
													   m_uiCurrentLocOffset);
				if (frequency > 1)
					m_cInvertedList
						.writeLocationBitLength(bitLength,
												m_pHeadAddress,
												m_uiCurrentLocOffset);
				
				currentLength -= (m_uiCurrentLocOffset - saveCurrent);
				length -= (m_uiCurrentLocOffset - saveCurrent);
				m_uiBitLength += (m_uiCurrentLocOffset - saveCurrent);
				m_cLocBlock.setDataBitLength(m_uiBitLength);
				
				initial = false;
			}
			
			// 位置情報を書けるところまで書く
			uiPrevLocation = m_cInvertedList.writeLocationData(
				uiPrevLocation,
				uiCurrentLocation,
				i.get(),
				m_pHeadAddress,
				m_uiCurrentLocOffset,
				m_uiCurrentLocOffset + currentLength);
		}

		m_cLocBlock.setDataBitLength(m_uiBitLength + currentLength);
		length -= currentLength;

		if (length)
		{
			// 次のLOCブロックを得る
			m_pLocPage
				= m_pOverflowFile->attachPage(m_pLocPage->getNextPageID());
			m_cLocBlock = m_pLocPage->getLocBlock();
			m_uiCurrentLocOffset = 0;
			m_pHeadAddress = m_cLocBlock.getBuffer();
			m_uiBitLength = m_cLocBlock.getDataBitLength();
		}
	}
}

//
//	Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
