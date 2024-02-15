// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationListIterator.cpp --
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
#include "FullText2/ShortNolocationListIterator.h"

#include "FullText2/FakeError.h"
#include "FullText2/InvertedList.h"
#include "FullText2/Types.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::ShortNolocationListIterator
//		-- コンストラクタ
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
ShortNolocationListIterator::ShortNolocationListIterator(
	InvertedList& cInvertedList_)
	: ShortBaseListIterator(cInvertedList_),
	  m_iCurrentLocPosition(-1), m_uiCurrentLocOffset(0),
	  m_uiCurrentLocLength(0), m_uiCurrentFrequency(0)
{
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::~ShortNolocationListIterator
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
ShortNolocationListIterator::~ShortNolocationListIterator()
{
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::reset -- 先頭へ戻る
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
ShortNolocationListIterator::reset()
{
	ShortBaseListIterator::reset();
	m_iCurrentLocPosition = -1;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::getTermFrequency
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
ShortNolocationListIterator::getTermFrequency()
{
	// 位置情報との同期を取る
	synchronize();
	return m_uiCurrentFrequency;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::expunge
//		-- 現在位置のデータを削除する
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
ShortNolocationListIterator::expunge()
{
	// 位置情報との同期を取る
	synchronize();

	// 文書IDを削除する
	expungeDocumentID();
	// 位置情報を削除する
	expungeLocation();

	; _FULLTEXT2_FAKE_ERROR(ShortNolocationListIterator::expunge);
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::undoExpunge -- 削除の取り消し
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
ShortNolocationListIterator::
undoExpunge(ModUInt32 uiDocumentID_,
			const SmartLocationList& cLocationList_)
{
	// 位置情報との同期を取る
	synchronize(true);

	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	// 位置情報を挿入する

	// ビット長を得る
	ModSize length = m_cInvertedList.getCompressedBitLengthFrequency(
		cLocationList_.getCount());
	// 移動する
	ModSize uiDstOffset = m_uiCurrentLocOffset + length;
	ModSize uiSrcOffset = m_uiCurrentLocOffset;
	ModSize uiLength = m_pArea->getLocationOffset() - m_uiCurrentLocOffset;
	InvertedList::move(m_pHeadAddress, uiDstOffset,
						m_pHeadAddress, uiSrcOffset, uiLength);
	InvertedList::setOff(m_pHeadAddress, uiSrcOffset,
						 uiDstOffset - uiSrcOffset);
	// 書き込む
	m_cInvertedList.writeLocationFrequency(cLocationList_.getCount(),
										   m_pHeadAddress, uiSrcOffset);
	// 位置情報の設定する
	setLocationInformation();

	// エリアを更新する
	m_pArea->setLocationOffset(m_pArea->getLocationOffset() + length);

	; _FULLTEXT2_FAKE_ERROR(ShortNolocationListIterator::undoExpunge);
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::getLocationOffset
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
ShortNolocationListIterator::getLocationOffset()
{
	synchronize();
	return m_uiCurrentLocOffset;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::getLocationBitLength
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
ShortNolocationListIterator::getLocationBitLength()
{
	synchronize();
	return m_uiCurrentLocLength;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::getLocationDataOffset
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
ShortNolocationListIterator::getLocationDataOffset()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::getLocationDataBitLength
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
ShortNolocationListIterator::getLocationDataBitLength()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::ShortNolocationListIterator::copy -- コピーを得る
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
ShortNolocationListIterator::copy() const
{
	return new ShortNolocationListIterator(*this);
}

//
//	FUNCTION private
//	FullText2::ShortNolocationListIterator::synchronize
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
ShortNolocationListIterator::synchronize(bool bUndo_)
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
//	FullText2::ShortNolocationListIterator::setLocationInfomation
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
ShortNolocationListIterator::setLocationInformation()
{
	ModSize uiOffset = m_uiCurrentLocOffset;
	
	// 頻度を読む
	m_uiCurrentFrequency
		= m_cInvertedList.readLocationFrequency(
			m_pHeadAddress, m_pArea->getLocationOffset(), uiOffset);
	
	// ビット長を設定する
	//
	// この変数が0ではないことで、現在の位置情報が読み込まれているかを
	// 確認できる
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	FullText2::ShortNolocationListIterator::expungeLocation
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
ShortNolocationListIterator::expungeLocation()
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
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
