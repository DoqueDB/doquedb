// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortListIterator.cpp --
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
#include "FullText2/ShortListIterator.h"
#include "FullText2/FakeError.h"
#include "FullText2/InvertedList.h"
#include "FullText2/ShortListLocationListIterator.h"
#include "FullText2/Types.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ShortListIterator::ShortListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedList& cInvertedList_
//		転置リスト(Short or Batch)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortListIterator::ShortListIterator(InvertedList& cInvertedList_)
	: ShortBaseListIterator(cInvertedList_),
	  m_iCurrentLocPosition(-1), m_uiCurrentLocOffset(0),
	  m_uiCurrentLocLength(0), m_uiCurrentFrequency(0), m_uiCurrentLocation(0)
{
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::~ShortListIterator -- デストラクタ
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
ShortListIterator::~ShortListIterator()
{
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::reset -- 先頭へ戻る
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
ShortListIterator::reset()
{
	ShortBaseListIterator::reset();
	m_iCurrentLocPosition = -1;
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::getTermFrequency -- 位置情報内の頻度情報を得る
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
ShortListIterator::getTermFrequency()
{
	// 位置情報との同期を取る
	synchronize();
	return m_uiCurrentFrequency;
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::getLocationListIterator
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LocationListIterator*
//		位置情報イテレータへのポインタ
//		このメモリーは呼び出し側で解放する必要がある
//
//	EXCEPTIONS
//
LocationListIterator::AutoPointer
ShortListIterator::getLocationListIterator()
{
	// 位置情報との同期を取る
	synchronize();

	ShortListLocationListIterator* locations
		= _SYDNEY_DYNAMIC_CAST(ShortListLocationListIterator*, getFree());

	if (locations == 0)
		locations = new ShortListLocationListIterator(this);

	LocationListIterator::AutoPointer p(locations);
	locations->initialize(m_pHeadAddress,
						  getLength(),
						  m_uiCurrentFrequency,
						  m_uiCurrentLocDataOffset,
						  m_uiCurrentLocDataOffset + m_uiCurrentLocDataLength,
						  m_cInvertedList.getLocationCoder(),
						  m_uiCurrentLocation);

	return p;
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::expunge -- 現在位置のデータを削除する
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
ShortListIterator::expunge()
{
	// 位置情報との同期を取る
	synchronize();

	// 文書IDを削除する
	expungeDocumentID();
	// 位置情報を削除する
	expungeLocation();

	; _FULLTEXT2_FAKE_ERROR(ShortListIterator::expunge);
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::undoExpunge -- 削除の取り消し
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		挿入する文書ID
//	const FullText2::SmartLocationList& cLocationList_
//		挿入する位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortListIterator::undoExpunge(ModUInt32 uiDocumentID_,
							   const SmartLocationList& cLocationList_)
{
	// 位置情報との同期を取る
	synchronize(true);

	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	// 位置情報を挿入する

	// ビット長を得る
	ModSize bitLength;
	ModSize length = m_cInvertedList.getCompressedBitLengthLocationList(
		cLocationList_, bitLength);
	// 移動する
	ModSize uiDstOffset = m_uiCurrentLocOffset + length;
	ModSize uiSrcOffset = m_uiCurrentLocOffset;
	ModSize uiLength = m_pArea->getLocationOffset() - m_uiCurrentLocOffset;
	InvertedList::move(m_pHeadAddress, uiDstOffset,
						m_pHeadAddress, uiSrcOffset, uiLength);
	InvertedList::setOff(m_pHeadAddress, uiSrcOffset,
						 uiDstOffset - uiSrcOffset);
	// 書き込む
	m_cInvertedList.writeLocationList(cLocationList_, bitLength,
									  m_pHeadAddress, uiSrcOffset);
	// 位置情報の設定する
	setLocationInformation();

	// エリアを更新する
	m_pArea->setLocationOffset(m_pArea->getLocationOffset() + length);

	; _FULLTEXT2_FAKE_ERROR(ShortListIterator::undoExpunge);
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::getLocationOffset
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
ShortListIterator::getLocationOffset()
{
	synchronize();
	return m_uiCurrentLocOffset;
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::getLocationBitLength
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
ShortListIterator::getLocationBitLength()
{
	synchronize();
	return m_uiCurrentLocLength;
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::getLocationDataOffset
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
ShortListIterator::getLocationDataOffset()
{
	synchronize();
	return m_uiCurrentLocDataOffset;
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::getLocationDataBitLength
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
ShortListIterator::getLocationDataBitLength()
{
	synchronize();
	return m_uiCurrentLocDataLength;
}

//
//	FUNCTION public
//	FullText2::ShortListIterator::copy -- コピーを得る
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
ShortListIterator::copy() const
{
	return new ShortListIterator(*this);
}

//
//	FUNCTION private
//	FullText2::ShortListIterator::synchronize
//		-- 現在の文書IDの位置に位置情報を移動する
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
ShortListIterator::synchronize(bool bUndo_)
{
	if (m_iCurrentPosition != m_iCurrentLocPosition)
	{
		// 位置がずれているので、合わせる

		if (m_iCurrentLocPosition == -1 ||
			m_iCurrentPosition < m_iCurrentLocPosition)
		{
			// 文書IDより位置情報が進んでしまっているので、最初から探しなおす
			m_iCurrentLocPosition = 0;
			m_uiCurrentLocOffset = 0;
			m_uiCurrentLocLength = 0;
		}

		if (m_uiCurrentLocLength != 0)
		{
			// 現在位置がすでに読み込まれている
			m_uiCurrentLocOffset += m_uiCurrentLocLength;
			m_iCurrentLocPosition++;
			m_uiCurrentLocLength = 0;
		}

		// 直前まで読み飛ばす
		while (m_iCurrentLocPosition != m_iCurrentPosition)
		{
			//
			//	文書IDの位置まで位置情報を読み捨てる
			//

			// 頻度を読む
			ModSize frequency
				= m_cInvertedList.readLocationFrequency(
					m_pHeadAddress,
					m_pArea->getLocationOffset(),
					m_uiCurrentLocOffset);
			if (frequency == 1)
			{
				// 頻度が1なので、ビット長は書かれていない
				m_cInvertedList.readLocationData(
					0, m_pHeadAddress,
					m_pArea->getLocationOffset(), m_uiCurrentLocOffset);
			}
			else
			{
				// ビット長を読む
				ModSize length
					= m_cInvertedList.readLocationBitLength(
						m_pHeadAddress,
						m_pArea->getLocationOffset(),
						m_uiCurrentLocOffset);
				m_uiCurrentLocOffset += length;
			}

			m_iCurrentLocPosition++;
		}

		if (bUndo_ == false)
			
			//
			// 位置情報を設定する
			//

			setLocationInformation();
	}
}

//
//	FUNCTION private
//	FullText2::ShortListIterator::setLocationInfomation
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
ShortListIterator::setLocationInformation()
{
	ModSize uiOffset = m_uiCurrentLocOffset;
	// 頻度を読む
	m_uiCurrentFrequency
		= m_cInvertedList.readLocationFrequency(
			m_pHeadAddress, m_pArea->getLocationOffset(), uiOffset);
	if (m_uiCurrentFrequency == 1)
	{
		// 位置情報データの先頭位置
		m_uiCurrentLocDataOffset = uiOffset;
		// 頻度が1なので、ビット長は書かれていない
		m_uiCurrentLocation
			= m_cInvertedList.readLocationData(0,
											   m_pHeadAddress,
											   m_pArea->getLocationOffset(),
											   uiOffset);
		// 位置情報データのビット長
		m_uiCurrentLocDataLength = uiOffset - m_uiCurrentLocDataOffset;
	}
	else
	{
		m_uiCurrentLocation = 0;
		// ビット長を読む
		m_uiCurrentLocDataLength
			= m_cInvertedList.readLocationBitLength(
				m_pHeadAddress,
				m_pArea->getLocationOffset(),
				uiOffset);
		// 位置情報データの先頭位置
		m_uiCurrentLocDataOffset = uiOffset;
		uiOffset += m_uiCurrentLocDataLength;
	}
	
	// ビット長を設定する
	//
	// この変数が0ではないことで、現在の位置情報が読み込まれているかを
	// 確認できる
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	FullText2::ShortListIterator::expungeLocation -- 位置情報を削除する
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
ShortListIterator::expungeLocation()
{
	// 位置情報を削除する

	ModSize uiDstOffset = m_uiCurrentLocOffset;
	ModSize uiSrcOffset = m_uiCurrentLocOffset + m_uiCurrentLocLength;
	ModSize uiLength = m_pArea->getLocationOffset() - uiSrcOffset;
	InvertedList::move(m_pHeadAddress, uiDstOffset,
						m_pHeadAddress, uiSrcOffset, uiLength);
	InvertedList::setOff(m_pHeadAddress, uiDstOffset + uiLength,
						 uiSrcOffset - uiDstOffset);
	m_pArea->setLocationOffset(
		m_pArea->getLocationOffset() - m_uiCurrentLocLength);

	// 位置情報変数を設定する
	setLocationInformation();
}

//
//	Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
