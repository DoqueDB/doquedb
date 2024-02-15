// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationListIterator.cpp --
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
#include "SyDynamicCast.h"

#include "Inverted/FakeError.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/MiddleNolocationList.h"
#include "Inverted/MiddleNolocationListIterator.h"
#include "Inverted/OverflowFile.h"

#include "Common/Assert.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::MiddleNolocationListIterator::MiddleNolocationListIterator --
//		コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::MiddleNolocationList& cMiddleNolocationList_
//		ミドルリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleNolocationListIterator::MiddleNolocationListIterator(
	MiddleNolocationList& cMiddleNolocationList_)
	: MiddleBaseListIterator(cMiddleNolocationList_),
	  m_pHeadAddress(0), m_iCurrentLocPosition(-1)
{
	// 先頭文書IDを設定する
	setFirstDocumentID();
	// -> 関数内で純粋仮想関数setLocBlockByEmpty()を使うので基底クラスから移動
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationListIterator::~MiddleNolocationListIterator --
//		デストラクタ
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

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::MiddleNolocationListIterator::expunge -- DEBUG用
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
MiddleNolocationListIterator::expunge()
{
	MiddleBaseListIterator::expunge();
	; _INVERTED_FAKE_ERROR(MiddleNolocationListIterator::expunge);
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationListIterator::undoExpunge -- DEBUG用
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
MiddleNolocationListIterator::undoExpunge(
	ModUInt32 uiDocumentID_,
	const ModInvertedSmartLocationList& cLocationList_)
{
	MiddleBaseListIterator::undoExpunge(uiDocumentID_, cLocationList_);
	; _INVERTED_FAKE_ERROR(MiddleNolocationListIterator::undoExpunge);
}
#endif

//
//	FUNCTION private
//	Inverted::MiddleNolocationListIterator::doSynchronize --
//		現在の文書IDの位置に位置情報を移動する (引数1個)
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
MiddleNolocationListIterator::doSynchronize(bool bUndo_)
{
	// MiddleListIteratorとMiddleNolocationListIteratorのsynchronizeは同一。
	// 基底クラスで実装すると、各派生クラスのメンバ変数へのアクセッサのコストが
	// 大きくなるので、各派生クラスで実装することにした。
	
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
				//
				// 次のLOCブロックを設定
				//
				
				; _SYDNEY_ASSERT(uiNextPageID
								 != PhysicalFile::ConstValue::UndefinedPageID);
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

				//
				// 次のLOCブロックの準備
				//

				uiNextPageID = m_pLocPage->getNextPageID();
				usOffset = 0;

				if (bUndo_ == true && m_iCurrentPosition == 0)
					break;
			}
		}

		if (m_uiCurrentLocLength != 0)
		{
			// 現在位置の位置情報がすでに読み込まれているので、
			// 現在位置を読み捨てる

			m_uiCurrentLocOffset = m_uiCurrentLocOffset + m_uiCurrentLocLength;
			m_uiCurrentLocLength = 0;

			; _SYDNEY_ASSERT(m_iCurrentPosition > m_iCurrentLocPosition);
			++m_iCurrentLocPosition;

			while (m_uiCurrentLocOffset >= m_uiBitLength)
			{
				if (bUndo_ == true
					&& m_iCurrentPosition == m_iCurrentLocPosition
					&& m_uiCurrentLocOffset == m_uiBitLength)
					break;

				// 次のLOCブロックへ
				nextLocBlock(m_uiCurrentLocOffset - m_uiBitLength);
			}
		}

		while (m_iCurrentPosition != m_iCurrentLocPosition)
		{
			//
			//	文書IDの位置まで位置情報を読み捨てる
			//

			// 位置情報を読み捨てる
			skipLocationInformation();

			// 位置をインクリメントする
			; _SYDNEY_ASSERT(m_iCurrentPosition > m_iCurrentLocPosition);
			++m_iCurrentLocPosition;

			while (m_uiCurrentLocOffset >= m_uiBitLength)
			{
				if (bUndo_ == true
					&& m_iCurrentPosition == m_iCurrentLocPosition
					&& m_uiCurrentLocOffset == m_uiBitLength)
					break;

				// 次のLOCブロックへ
				nextLocBlock(m_uiCurrentLocOffset - m_uiBitLength);
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
//	Inverted::MiddleNolocationListIterator::setLocationInfomation --
//		位置情報変数を設定する
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

	// 頻度を読む
	m_uiCurrentFrequency = m_cInvertedList.readLocationFrequency(m_pHeadAddress,
																 m_uiBitLength,
																 uiOffset);
	// ビット長を設定する
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	Inverted::MiddleNolocationListIterator::skipLocationInfomation --
//		位置情報変数を読み飛ばす
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
MiddleNolocationListIterator::skipLocationInformation()
{
	// 頻度を読む
	ModSize frequency =
		m_cInvertedList.readLocationFrequency(m_pHeadAddress,
											  m_uiBitLength,
											  m_uiCurrentLocOffset);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
