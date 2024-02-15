// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortBaseListIterator.cpp --
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
#include "FullText2/ShortBaseListIterator.h"
#include "FullText2/InvertedList.h"
#include "FullText2/Types.h"
#include "FullText2/FakeError.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ShortBaseListIterator::ShortBaseListIterator -- コンストラクタ
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
ShortBaseListIterator::ShortBaseListIterator(InvertedList& cInvertedList_)
	: m_cInvertedList(cInvertedList_),
	  m_iCurrentPosition(-1), m_uiPrevID(0), m_uiCurrentID(0),
	  m_uiCurrentOffset(0), m_uiNextOffset(0)
{
	// ページを得る(参照カウンターを保持するため)
	m_pLeafPage = cInvertedList_.getLeafPage();
	// エリアを得る
	m_pArea = cInvertedList_.getArea();
	m_pHeadAddress = m_pArea->getHeadAddress();
	m_pTailAddress = m_pArea->getTailAddress();
}

//
//	FUNCTION public
//	FullText2::ShortBaseListIterator::~ShortBaseListIterator -- デストラクタ
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
//	FullText2::ShortBaseListIterator::getLength -- 索引単位の長さを得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	int
//		索引単位の長さ
//
//	EXCEPTIONS
//
int
ShortBaseListIterator::getLength()
{
	return (m_cInvertedList.isWordIndex() ?
			1 : static_cast<int>(m_pArea->getKeyLength()));
}

//
//	FUNCTION public
//	FullText2::ShortBaseListIterator::find -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	DocumentID uiDocumentID_
//		検索する文書ID
//	bool bUndo_
//		削除のUndo処理中か
//
//	RETURN
//	bool
//		検索にヒットした場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ShortBaseListIterator::find(DocumentID uiDocumentID_, bool bUndo_)
{
	if (lowerBound(uiDocumentID_, bUndo_) == uiDocumentID_)
		return true;
	return false;
}

//
//	FUNCTION public
//	FullText2::ShortBaseListIterator::lowerBound -- 文書IDをlower_bound検索する
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
//	DocumentID
//		検索できた場合はヒットした文書ID。存在しない場合はUndefinedDocumentID
//
//	EXCEPTIONS
//
DocumentID
ShortBaseListIterator::lowerBound(DocumentID uiDocumentID_, bool bUndo_)
{
	if (bUndo_ == false && m_pArea->getLastDocumentID() < uiDocumentID_)
	{
		// 存在しない
		m_uiCurrentID = UndefinedDocumentID;
		return m_uiCurrentID;
	}
	
	if (m_uiCurrentID > uiDocumentID_)
	{
		// すでに大きくなっているので、リセット
		reset();
	}

	while (m_uiCurrentID < uiDocumentID_)
	{
		// 次へ
		nextImpl();
	}

	return m_uiCurrentID;
}

//
//	FUNCTION protected
//	FullText2::ShortBaseListIterator::nextImpl -- 次へ進める
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::DocumentID
//		文書ID。存在しない場合は UndefinedDocumentID を返す
//
//	EXCEPTIONS
//
DocumentID
ShortBaseListIterator::nextImpl()
{
	if (isEndImpl()) return m_uiCurrentID;

	if (m_iCurrentPosition == -1)
	{
		// はじめて呼ばれた
		m_uiCurrentOffset = m_uiNextOffset = 0;
		m_uiPrevID = 0;
		m_iCurrentPosition = 0;

		if (m_pArea->getDocumentCount() == 0)
		{
			m_uiCurrentID = UndefinedDocumentID;
		}
		else
		{
			m_uiCurrentID = m_pArea->getFirstDocumentID();
		}

		return m_uiCurrentID;
	}

	++m_iCurrentPosition;

	m_uiPrevID = m_uiCurrentID;
	m_uiCurrentOffset = m_uiNextOffset;
	m_uiCurrentID = 0;

	// 文書IDを1つ読む
	m_uiCurrentID = m_cInvertedList.readDocumentID(m_uiPrevID,
												   m_pTailAddress,
												   m_pArea->getDocumentOffset(),
												   m_uiNextOffset);
	return m_uiCurrentID;
}

//
//	FUNCTION protected
//	FullText2::ShortBaseListIterator::expungeDocumentID
//		-- 現在位置の文書IDを削除する
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
			m_uiCurrentID = 0;
		}
		else
		{
			ModSize uiOffset = m_uiNextOffset;
			ModUInt32 uiNextID
				= m_cInvertedList.readDocumentID(m_uiCurrentID,
												 m_pTailAddress,
												 m_pArea->getDocumentOffset(),
												 uiOffset);

			// 後ろを移動
			uiDstOffset = 0;
			uiSrcOffset = uiOffset;
			uiLength = m_pArea->getDocumentOffset() - uiOffset;

			// 先頭文書ID
			m_pArea->setFirstDocumentID(uiNextID);

			// 値を設定する
			m_uiCurrentID = uiNextID;
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
		m_uiCurrentID = UndefinedDocumentID;

		// 位置をリセットする
		reset();
	}
	else
	{
		// 途中の文書IDを削除する

		ModSize uiOffset = m_uiNextOffset;
		ModUInt32 uiNextID
			= m_cInvertedList.readDocumentID(m_uiCurrentID,
											 m_pTailAddress,
											 m_pArea->getDocumentOffset(),
											 uiOffset);

		// 差分を書き直す
		m_uiNextOffset = m_uiCurrentOffset;
		InvertedList::setOffBack(m_pTailAddress,
								 m_uiCurrentOffset,
								 uiOffset - m_uiCurrentOffset);
		m_cInvertedList.writeDocumentID(m_uiPrevID,
										uiNextID,
										m_pTailAddress,
										m_uiNextOffset);

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
		InvertedList::setOffBack(m_pTailAddress,
								 uiDstOffset + uiLength,
								 uiSrcOffset - uiDstOffset);
	}

	// 文書IDビット長
	m_pArea->setDocumentOffset(
		m_pArea->getDocumentOffset() - (uiSrcOffset - uiDstOffset));
	// エリアの文書数を設定する
	m_pArea->decrementDocumentCount();
}

//
//	FUNCTION protected
//	FullText2::ShortBaseListIterator::undoExpungeDocumentID
//		-- 文書IDの削除を取り消す
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
		m_uiCurrentID = uiDocumentID_;
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
			InvertedList::setOffBack(m_pTailAddress, uiSrcOffset,
									 uiDstOffset - uiSrcOffset);
		}

		// 書き込む
		m_cInvertedList.writeDocumentID(uiDocumentID_, m_uiCurrentID,
										m_pTailAddress, uiSrcOffset);

		// 先頭文書ID
		m_pArea->setFirstDocumentID(uiDocumentID_);
		// ビット長
		m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() + length);

		// 値を設定する
		m_uiCurrentID = uiDocumentID_;
	}
	else if (isEndImpl() == true)
	{
		// 最終文書IDの削除を取り消す

		// 書き込む
		m_uiNextOffset = m_uiCurrentOffset;
		m_cInvertedList.writeDocumentID(m_uiPrevID, uiDocumentID_,
										m_pTailAddress, m_uiNextOffset);

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
		ModSize length = m_cInvertedList.getCompressedBitLengthDocumentID(
			m_uiPrevID, uiDocumentID_);
		length += m_cInvertedList.getCompressedBitLengthDocumentID(
			uiDocumentID_, m_uiCurrentID);
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
		InvertedList::setOffBack(m_pTailAddress, m_uiCurrentOffset,
								 uiDstOffset - m_uiCurrentOffset);

		// 書き込む
		m_uiNextOffset = m_uiCurrentOffset;
		m_cInvertedList.writeDocumentID(m_uiPrevID, uiDocumentID_,
										m_pTailAddress, m_uiNextOffset);
		ModSize uiOffset = m_uiNextOffset;
		m_cInvertedList.writeDocumentID(uiDocumentID_, m_uiCurrentID,
										m_pTailAddress, uiOffset);

		// ビット長
		m_pArea->setDocumentOffset(m_pArea->getDocumentOffset() + length);

		// 値を設定する
		m_uiCurrentID = uiDocumentID_;
	}

	// エリアを更新する
	m_pArea->incrementDocumentCount();
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
