// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationNoTFListIterator.cpp --
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
#include "FullText2/MiddleNolocationNoTFListIterator.h"
#include "FullText2/MiddleNolocationNoTFList.h"
#include "FullText2/OverflowFile.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/FakeError.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::MiddleNolocationNoTFListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::MiddleNolocationNoTFList& cMiddleList_
//		ミドルリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleNolocationNoTFListIterator::MiddleNolocationNoTFListIterator(
	MiddleNolocationNoTFList& cMiddleList_)
	: MiddleBaseListIterator(cMiddleList_)
{
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::
//			~MiddleNolocationNoTFListIterator
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
MiddleNolocationNoTFListIterator::~MiddleNolocationNoTFListIterator()
{
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::getTermFrequency
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
MiddleNolocationNoTFListIterator::getTermFrequency()
{
	// 常に1
	return 1;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::expunge
//		-- 現在位置の情報を削除する
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
MiddleNolocationNoTFListIterator::expunge()
{
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

	; _FULLTEXT2_FAKE_ERROR(MiddleNolocationNoTFListIterator::expunge);
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::undoExpunge -- 削除の取り消し
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
MiddleNolocationNoTFListIterator::
undoExpunge(ModUInt32 uiDocumentID_,
			const SmartLocationList& cLocationList_)
{
	// 文書IDを挿入する
	undoExpungeDocumentID(uiDocumentID_);

	m_pArea->setDocumentCount(m_pArea->getDocumentCount() + 1);

	; _FULLTEXT2_FAKE_ERROR(MiddleNolocationNoTFListIterator::undoExpunge);
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::getHeadAddress
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
MiddleNolocationNoTFListIterator::getHeadAddress()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::getLocationOffset
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
MiddleNolocationNoTFListIterator::getLocationOffset()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::getLocationBitLength
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
MiddleNolocationNoTFListIterator::getLocationBitLength()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::getLocationDataOffset
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
MiddleNolocationNoTFListIterator::getLocationDataOffset()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::getLocationDataBitLength
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
MiddleNolocationNoTFListIterator::getLocationDataBitLength()
{
	return 0;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::isContinue
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
MiddleNolocationNoTFListIterator::isContinue()
{
	// LOC情報はなにもないので、ページを跨ぐことはない
	return false;
}

//
//	FUNCTION public
//	FullText2::MiddleNolocationNoTFListIterator::expungeIdBlock
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
MiddleNolocationNoTFListIterator::expungeIdBlock()
{
	if (m_cIdBlock.isExpunge() == true)
	{
		if (m_cIdBlock != m_pArea->getHeadAddress())
		{
			// IDブロックを削除する
			m_pIdPage->freeIDBlock(m_uiIDBlockPosition);

			if (m_pIdPage->getIDBlockCount() == 0)
			{
				// このページは不要
				m_pOverflowFile->freePage(m_pIdPage);

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
//	FullText2::MiddleNolocationNoTFListIterator::copy -- コピーを得る
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
MiddleNolocationNoTFListIterator::copy() const
{
	return new MiddleNolocationNoTFListIterator(*this);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
