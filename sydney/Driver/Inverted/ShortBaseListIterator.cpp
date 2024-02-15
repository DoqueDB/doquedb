// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBaseListIterator.cpp --
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

#include "Inverted/InvertedList.h"
#include "Inverted/ShortBaseListIterator.h"
#include "Inverted/Types.h"

#include "Exception/Unexpected.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::ShortBaseListIterator -- コンストラクタ
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
ShortBaseListIterator::ShortBaseListIterator(InvertedList& cInvertedList_)
	: m_cInvertedList(cInvertedList_),
	  m_iCurrentPosition(0), m_uiPrevID(0), m_uiCurrentOffset(0), m_uiNextOffset(0)
{
	// エリアを得る
	m_pArea = m_cInvertedList.getArea();
	m_pTailAddress = m_pArea->getTailAddress();

	// 先頭文書IDを得る
	if (m_pArea->getDocumentCount() == 0)
		m_uiCurrentID = UndefinedDocumentID;
	else
		m_uiCurrentID = m_pArea->getFirstDocumentID();
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::~ShortBaseListIterator -- デストラクタ
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
ShortBaseListIterator::~ShortBaseListIterator()
{
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::next -- 次へ進める
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::next()
{
	if (isEnd() == ModTrue) return;

	// Positionをインクリメントする
	++m_iCurrentPosition;

	// 今までの情報をコピーする
	// PrevDocumentID
	m_uiPrevID = m_uiCurrentID;
	// Offset
	m_uiCurrentOffset = m_uiNextOffset;

	// 新しい情報を設定する
	// DocumentID
	m_uiCurrentID = m_cInvertedList.readDocumentID(
		m_uiPrevID,
		m_pTailAddress,
		m_pArea->getDocumentOffset(),	// 最終文書IDのオフセット
		m_uiNextOffset);	// 次の文書IDオフセットも更新される
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::reset -- 先頭へ戻る
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
ShortBaseListIterator::reset()
{
	// 先頭文書IDを得る
	if (m_pArea->getDocumentCount() == 0)
		m_uiCurrentID = UndefinedDocumentID;
	else
		m_uiCurrentID = m_pArea->getFirstDocumentID();

	// その他を初期化
	m_iCurrentPosition = 0;
	m_uiPrevID = 0;
	m_uiCurrentOffset = 0;
	m_uiNextOffset = 0;
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::find -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedIterator::DocumentID uiDocumentID_
//		検索する文書ID
//	bool bUndo_
//		削除のUndo処理中か
//
//	RETURN
//	ModBoolean
//		検索にヒットした場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
ShortBaseListIterator::find(const DocumentID uiDocumentID_, bool bUndo_)
{
	if (lowerBound(uiDocumentID_, bUndo_) == ModTrue)
		if (m_uiCurrentID == uiDocumentID_)
			return ModTrue;
	return ModFalse;
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::lowerBound -- 文書IDをlower_bound検索する
//
//	NOTES
//
//	ARGUMENTS
//	ModInvertedListIterator::DocumentID uiDocumentID_
//		検索する文書ID
//	bool bUndo_
//		削除のUndo処理中か
//
//	RETURN
//	ModBoolean
//		検索できた場合はModTrue、それ以外の場合はModFalse
//
//	EXCEPTIONS
//
ModBoolean
ShortBaseListIterator::lowerBound(const DocumentID uiDocumentID_, bool bUndo_)
{
	if (bUndo_ == false && m_pArea->getLastDocumentID() < uiDocumentID_)
	{
		// 存在しない
		m_uiCurrentID = UndefinedDocumentID;
		return ModFalse;
	}
	
	if (m_uiCurrentID > uiDocumentID_)
	{
		// すでに大きくなっているので、リセット
		reset();
	}

	while (m_uiCurrentID < uiDocumentID_)
	{
		// 次へ
		next();
	}

	return (isEnd() == ModFalse) ? ModTrue : ModFalse;
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::getInDocumentFrequency -- 位置情報内の頻度情報を得る
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
ShortBaseListIterator::getInDocumentFrequency()
{
	// 位置情報との同期を取る
	synchronize();

	return getTermFrequency();
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::getLocationListIterator -- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModInvertedLocationListIterator*
ShortBaseListIterator::getLocationListIterator()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::expunge -- 現在位置のデータを削除する
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
ShortBaseListIterator::expunge()
{
	// 位置情報との同期を取る
	synchronize();

	// 文書IDを削除する
	expungeDocumentID();

	// 位置情報を削除する
	expungeLocation();
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::undoExpunge -- 削除の取り消し
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		挿入する文書ID
//	const ModInvertedSmartLocationList& cLocationList_
//		挿入する位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	// 位置情報との同期を取る
	synchronize();

	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	// 位置情報を挿入する
	undoExpungeLocation(cLocationList_);
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::getLocationOffset -- 現在の位置情報のオフセットを得る
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
ShortBaseListIterator::getLocationOffset()
{
	synchronize();
	return getLocOffset();
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::getLocationBitLength -- 現在の位置情報のビット長を得る
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
ShortBaseListIterator::getLocationBitLength()
{
	synchronize();
	return getLocLength();
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::getLocationDataOffset -- 現在の位置情報データのオフセットを得る
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
ShortBaseListIterator::getLocationDataOffset()
{
	synchronize();
	return getLocDataOffset();
}

//
//	FUNCTION public
//	Inverted::ShortBaseListIterator::getLocationDataBitLength -- 現在の位置情報データのビット長を得る
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
ShortBaseListIterator::getLocationDataBitLength()
{
	synchronize();
	return getLocDataLength();
}

//
//	FUNCTION protected
//	Inverted::ShortBaseListIterator::synchronize -- 現在の文書IDの位置に位置情報を移動する
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
ShortBaseListIterator::synchronize()
{
	if (m_iCurrentPosition != getLocPosition())
	{
		// 位置がずれているので、合わせる

		if (getLocPosition() == -1 ||
			m_iCurrentPosition < getLocPosition())
		{
			// 文書IDより位置情報が進んでしまっているので、最初から探しなおす
			resetLocation();
		}

		if (getLocLength() != 0)
		{
			// 現在位置がすでに読み込まれている

			// 読み込み済みの次の位置を設定する
			setLocOffset(getLocOffset() + getLocLength());
			incrementLocPosition();
			setLocLength(0);
		}

		// 直前まで読み飛ばす
		while (getLocPosition() != m_iCurrentPosition)
		{
			//
			//	文書IDの位置まで位置情報を読み捨てる
			//

			// 位置情報を読み捨てる
			skipLocationInformation();

			// 位置をインクリメントする
			incrementLocPosition();
		}

		//
		// 位置情報を設定する
		//

		setLocationInformation();
	}
}

//
//	FUNCTION protected
//	Inverted::ShortBaseListIterator::resetLocation -- 位置の先頭に戻る
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
ShortBaseListIterator::resetLocation()
{
	setLocPosition(0);
	setLocOffset(0);
	setLocLength(0);
}

//
//	FUNCTION protected
//	Inverted::ShortBaseListIterator::expungeDocumentID -- 現在位置の文書IDを削除する
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
ShortBaseListIterator::expungeDocumentID()
{
	ModSize uiDstOffset = 0;
	ModSize uiSrcOffset = 0;
	ModSize uiLength = 0;

	if (m_pArea->getFirstDocumentID() == m_uiCurrentID)
	{
		// 先頭文書IDを削除する

		if (m_pArea->getLastDocumentID() == m_uiCurrentID)
		{
			// 1つしかエントリがない
			m_pArea->setFirstDocumentID(0);	// 先頭文書ID
			m_pArea->setLastDocumentID(0);	// 最終文書ID

			// 値を設定する
			reset();
		}
		else
		{
			ModSize uiOffset = m_uiNextOffset;
			ModUInt32 uiNextID = m_cInvertedList.readDocumentID(
				m_uiCurrentID, m_pTailAddress, m_pArea->getDocumentOffset(), uiOffset);

			// 後ろを移動
			uiDstOffset = 0;
			uiSrcOffset = uiOffset;
			uiLength = m_pArea->getDocumentOffset() - uiOffset;

			// 先頭文書ID
			m_pArea->setFirstDocumentID(uiNextID);

			// 値を設定する
			reset();
		}
	}
	else if (m_pArea->getLastDocumentID() == m_uiCurrentID)
	{
		// 最終文書IDを削除する

		// 後ろを移動
		uiDstOffset = m_uiCurrentOffset;
		uiSrcOffset = m_uiNextOffset;
		uiLength = 0;
		
		// 最終文書ID
		m_pArea->setLastDocumentID(m_uiPrevID);

		// 値を設定する
		reset();
	}
	else
	{
		// 途中の文書IDを削除する

		// Offset, PrevDocumentIDは変わらない。
		// DocumentID, NextOffset が変わる。

		// NextDocumentIDを得る
		ModSize uiOffset = m_uiNextOffset;
		ModUInt32 uiNextID = m_cInvertedList.readDocumentID(
			m_uiCurrentID, m_pTailAddress, m_pArea->getDocumentOffset(), uiOffset);

		// 差分を書き直す
		// 今までのDocumentIDとNextDocumentIDの領域をリセット
		m_uiNextOffset = m_uiCurrentOffset;
		InvertedList::setOffBack(
			m_pTailAddress, m_uiCurrentOffset, uiOffset - m_uiCurrentOffset);
		// 今までのDocumentIDの領域にNextDocumentIDを書く
		m_cInvertedList.writeDocumentID(
			m_uiPrevID, uiNextID, m_pTailAddress,
			m_uiNextOffset);	// 次の文書IDのオフセットも更新される

		// 後ろを移動
		uiDstOffset = m_uiNextOffset;
		uiSrcOffset = uiOffset;
		uiLength = m_pArea->getDocumentOffset() - uiOffset;

		// 値を設定する
		m_uiCurrentID = uiNextID;
	}

	if (uiDstOffset != uiSrcOffset)
	{
		if (uiLength != 0)
		{
			// 文書データを移動する
			InvertedList::moveBack(m_pTailAddress, uiDstOffset,
								   m_pTailAddress, uiSrcOffset, uiLength);
		}
		// 削除した分のビットをクリアする
		InvertedList::setOffBack(m_pTailAddress, uiDstOffset + uiLength, uiSrcOffset - uiDstOffset);
	}

	// 文書IDビット長
	m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() - (uiSrcOffset - uiDstOffset));
	// エリアの文書数を設定する
	m_pArea->decrementDocumentCount();
}

//
//	FUNCTION protected
//	Inverted::ShortBaseListIterator::expungeLocation -- 位置情報を削除する
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
ShortBaseListIterator::expungeLocation()
{
	// 位置情報を削除する

	ModSize uiDstOffset = getLocOffset();
	ModSize uiSrcOffset = getLocOffset() + getLocLength();
	ModSize uiLength = m_pArea->getLocationOffset() - uiSrcOffset;
	InvertedList::move(getHeadAddress(), uiDstOffset,
					   getHeadAddress(), uiSrcOffset, uiLength);
	InvertedList::setOff(getHeadAddress(), uiDstOffset + uiLength,
						 uiSrcOffset - uiDstOffset);

	// エリアを更新する
	m_pArea->setLocationOffset(
		m_pArea->getLocationOffset() - getLocLength());

	// 位置情報変数を設定する
	setLocationInformation();
}

//
//	FUNCTION protected
//	Inverted::ShortBaseListIterator::undoExpungeDocumentID -- 文書IDの削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		削除を取り消す文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::undoExpungeDocumentID(ModUInt32 uiDocumentID_)
{
	if (m_pArea->getFirstDocumentID() == 0)
	{
		// 1つもエントリが存在しない
		m_pArea->setFirstDocumentID(uiDocumentID_);
		m_pArea->setLastDocumentID(uiDocumentID_);

		// 値を設定する
		reset();

		// 位置も設定する
		resetLocation();
	}
	else if (m_pArea->getFirstDocumentID() == m_uiCurrentID)
	{
		// 先頭文書IDの削除を取り消す

		// ビット長
		ModSize length = m_cInvertedList.getCompressedBitLengthDocumentID(
			uiDocumentID_, m_uiCurrentID);

		// 後ろを移動
		ModSize uiDstOffset = length;
		ModSize uiSrcOffset = 0;
		ModSize uiLength = m_pArea->getDocumentOffset() - uiSrcOffset;
		if (uiLength)
		{
			InvertedList::moveBack(m_pTailAddress, uiDstOffset,
								   m_pTailAddress, uiSrcOffset, uiLength);
			InvertedList::setOffBack(m_pTailAddress, uiSrcOffset, uiDstOffset - uiSrcOffset);
		}

		// 書き込む
		m_cInvertedList.writeDocumentID(uiDocumentID_, m_uiCurrentID, m_pTailAddress, uiSrcOffset);

		// 先頭文書ID
		m_pArea->setFirstDocumentID(uiDocumentID_);
		// ビット長
		m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() + length);

		// 値を設定する
		reset();
	}
	else if (isEnd() == ModTrue)
	{
		// 最終文書IDの削除を取り消す

		// 書き込む
		m_uiNextOffset = m_uiCurrentOffset;
		m_cInvertedList.writeDocumentID(m_uiPrevID, uiDocumentID_, m_pTailAddress,
										m_uiNextOffset); //次の文書IDのオフセットも更新される

		// 最終文書ID
		m_pArea->setLastDocumentID(uiDocumentID_);
		// ビット長
		m_pArea->setDocumentOffset(m_uiNextOffset);

		// 値を設定する
		m_uiCurrentID = uiDocumentID_;
	}
	else
	{
		// 途中の文書IDの削除を取り消す

		// ビット長を求める
		ModSize length = m_cInvertedList.getCompressedBitLengthDocumentID(m_uiPrevID, uiDocumentID_);
		length += m_cInvertedList.getCompressedBitLengthDocumentID(uiDocumentID_, m_uiCurrentID);
		length -= m_uiNextOffset - m_uiCurrentOffset;

		// 後ろを移動する
		ModSize uiDstOffset = m_uiNextOffset + length;
		ModSize uiSrcOffset = m_uiNextOffset;
		ModSize uiLength = m_pArea->getDocumentOffset() - uiSrcOffset;
		if (uiLength)
		{
			InvertedList::moveBack(m_pTailAddress, uiDstOffset,
								   m_pTailAddress, uiSrcOffset, uiLength);
		}
		InvertedList::setOffBack(m_pTailAddress, m_uiCurrentOffset, uiDstOffset - m_uiCurrentOffset);

		// 書き込む
		m_uiNextOffset = m_uiCurrentOffset;
		m_cInvertedList.writeDocumentID(m_uiPrevID, uiDocumentID_, m_pTailAddress,
										m_uiNextOffset); // 次の文書IDのオフセットも更新される
		ModSize uiOffset = m_uiNextOffset;
		m_cInvertedList.writeDocumentID(uiDocumentID_, m_uiCurrentID, m_pTailAddress, uiOffset);

		// ビット長
		m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() + length);

		// 値を設定する
		m_uiCurrentID = uiDocumentID_;
	}
	
	// エリアを更新する
	m_pArea->incrementDocumentCount();
}

//
//	FUNCTION protected
//	Inverted::ShortBaseListIterator::undoExpungeLocation -- 位置情報の削除を取り消す
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedSmartLocationList& cLocationList_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::undoExpungeLocation(const ModInvertedSmartLocationList& cLocationList_)
{
	// ビット長を得る
	ModSize bitLength;
	ModSize length = m_cInvertedList.getCompressedBitLengthLocationList(cLocationList_, bitLength);
	// 移動する
	ModSize uiDstOffset = getLocOffset() + length;
	ModSize uiSrcOffset = getLocOffset();
	ModSize uiLength = m_pArea->getLocationOffset() - getLocOffset();
	InvertedList::move(getHeadAddress(), uiDstOffset,
					   getHeadAddress(), uiSrcOffset, uiLength);
	InvertedList::setOff(getHeadAddress(), uiSrcOffset, uiDstOffset - uiSrcOffset);
	// 書き込む
	m_cInvertedList.writeLocationList(cLocationList_, bitLength, getHeadAddress(), uiSrcOffset);
	// 位置情報の設定する
	setLocationInformation();

	// エリアを更新する
	m_pArea->setLocationOffset(m_pArea->getLocationOffset() + length);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::setLocationInformation -- 位置情報変数を設定する
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
ShortBaseListIterator::setLocationInformation()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::skipLocationInformation --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::skipLocationInformation()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::getLocPosition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
int
ShortBaseListIterator::getLocPosition() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::setLocPosition --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiPosition_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::setLocPosition(int iPosition_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::incrementLocPosition --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::incrementLocPosition()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::getLocOffset --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
ShortBaseListIterator::getLocOffset() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::setLocOffset --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiOffset_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::setLocOffset(ModSize uiOffset_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::getLocLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
ShortBaseListIterator::getLocLength() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::setLocLength --
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiLength_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortBaseListIterator::setLocLength(ModSize uiLength_)
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::getLocDataOffset --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
ShortBaseListIterator::getLocDataOffset() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::getLocDataLength --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
ShortBaseListIterator::getLocDataLength() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION private
//	Inverted::ShortBaseListIterator::getTermFrequency --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	ModSize
//
//	EXCEPTIONS
//
ModSize
ShortBaseListIterator::getTermFrequency() const
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
