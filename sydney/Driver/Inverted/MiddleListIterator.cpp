// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleListIterator.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2006, 2008, 2010, 2023 Ricoh Company, Ltd.
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

#include "Inverted/MiddleListIterator.h"
#include "Inverted/MiddleList.h"
#include "Inverted/OverflowFile.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/MiddleListLocationListIterator.h"
#include "Inverted/FakeError.h"

#include "Common/Assert.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::MiddleListIterator::MiddleListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::MiddleList& cMiddleList_
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
	  m_iCurrentLocPosition(-1), m_pFree(0)
{
	// 先頭文書IDを設定する
	setFirstDocumentID();
	// -> 関数内で純粋仮想関数setLocBlockByEmpty()を使うので基底クラスから移動
}

//
//	FUNCTION public
//	Inverted::MiddleListIterator::~MiddleListIterator -- デストラクタ
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
	while (m_pFree)
	{
		MiddleListLocationListIterator* p
			= _SYDNEY_DYNAMIC_CAST(MiddleListLocationListIterator*, m_pFree);
		m_pFree = p->nextInstance;
		delete p;
	}
}

//
//	FUNCTION public
//	Inverted::MiddleListIterator::getLocationListIterator -- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModInvertedLocationListIterator*
//		位置情報イテレータへのポインタ。このメモリーは呼び出し側で解放する必要がある
//
//	EXCEPTIONS
//
ModInvertedLocationListIterator*
MiddleListIterator::getLocationListIterator()
{
	// 位置情報との同期を取る
	synchronize();

	MiddleListLocationListIterator* locations = 0;

	// LocationListiteratorを割り当てる
	if (m_pFree)
	{
		locations
			= _SYDNEY_DYNAMIC_CAST(MiddleListLocationListIterator*, m_pFree);
		m_pFree = locations->nextInstance;
	}
	else
	{
		locations = new MiddleListLocationListIterator(this);
	}

	// 初期化する
	locations->initialize(m_pOverflowFile,
						  m_pLocPage,
						  m_cLocBlock,
						  m_uiCurrentFrequency,
						  m_uiCurrentLocDataOffset,
						  m_uiCurrentLocDataLength,
						  m_cInvertedList.getLocationCoder(),
						  m_uiCurrentLocation);

	return locations;
}

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::MiddleListIterator::expunge -- DEBUG用
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
MiddleListIterator::expunge()
{
	MiddleBaseListIterator::expunge();
	; _INVERTED_FAKE_ERROR(MiddleListIterator::expunge);
}

//
//	FUNCTION public
//	Inverted::MiddleListIterator::undoExpunge -- DEBUG用
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
MiddleListIterator::undoExpunge(ModUInt32 uiDocumentID_,
								const ModInvertedSmartLocationList& cLocationList_)
{
	MiddleBaseListIterator::undoExpunge(uiDocumentID_, cLocationList_);
	; _INVERTED_FAKE_ERROR(MiddleListIterator::undoExpunge);
}
#endif

//
//	FUNCTION public
//	Inverted::MiddleListIterator::isContinue --
//		位置情報がページを跨いでいるかどうか
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
	// ある索引語の位置情報が一つのLOCブロックに収まるとは限らない。
	// そのため、複数のLOCブロックに分けて、つまり複数のページに分けて
	// 格納されることもある。
	
	// 現在参照中の文書IDに対応する位置リストが、
	// 現在参照中のLOCブロックに全て収まっているかどうか確認する。
	
	synchronize();
	if (m_uiCurrentLocOffset + m_uiCurrentLocLength > m_uiBitLength)
		return true;
	return false;
}

//
//	FUNCTION private
//	Inverted::MiddleListIterator::doSynchronize --
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
MiddleListIterator::doSynchronize(bool bUndo_)
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

			m_uiCurrentLocOffset += m_uiCurrentLocLength;
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
//	Inverted::MiddleListIterator::setLocationInfomation -- 位置情報変数を設定する
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
	//	【注意】頻度とビット長(頻度が1の場合は位置情報)の間でページを跨いでいることはない
	//

	// 頻度を読む
	m_uiCurrentFrequency = m_cInvertedList.readLocationFrequency(m_pHeadAddress,
																 m_uiBitLength,
																 uiOffset);

	// 頻度以外を読む
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
	else if (m_uiCurrentFrequency > 1)
	{
		m_uiCurrentLocation = 0;
		// ビット長を読む
		m_uiCurrentLocDataLength = m_cInvertedList.readLocationBitLength(m_pHeadAddress,
																		 m_uiBitLength,
																		 uiOffset);
		// 位置情報データの先頭位置
		m_uiCurrentLocDataOffset = uiOffset;
		uiOffset += m_uiCurrentLocDataLength;
	}
	else
	{
		// 削除によって0件になっていた場合
		m_uiCurrentLocation = 0;
		m_uiCurrentLocDataLength = 0;
		m_uiCurrentLocDataOffset = uiOffset;
	}

	// ビット長を設定する
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	Inverted::MiddleListIterator::setLocationInfomation -- 位置情報変数を設定する
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
MiddleListIterator::skipLocationInformation()
{
	// 頻度を読む
	ModSize frequency = m_cInvertedList.readLocationFrequency(m_pHeadAddress,
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
		ModSize length = m_cInvertedList.readLocationBitLength(m_pHeadAddress,
															   m_uiBitLength,
															   m_uiCurrentLocOffset);
		m_uiCurrentLocOffset += length;
	}
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2006, 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
