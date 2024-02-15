// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationNoTFListIterator.cpp --
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
#include "Inverted/MiddleNolocationNoTFList.h"
#include "Inverted/MiddleNolocationNoTFListIterator.h"
#include "Inverted/OverflowFile.h"

#include "Common/Assert.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::MiddleNolocationNoTFListIterator
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::MiddleNolocationNoTFList& cMiddleNolocationNoTFList_
//		ミドルリスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
MiddleNolocationNoTFListIterator::MiddleNolocationNoTFListIterator(
	MiddleNolocationNoTFList& cMiddleNolocationNoTFList_)
	: MiddleBaseListIterator(cMiddleNolocationNoTFList_)
{
	// 先頭文書IDを設定する
	setFirstDocumentID();
	// -> 関数内で純粋仮想関数setLocBlockByEmpty()を使うので基底クラスから移動
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::~MiddleNolocationNoTFListIterator --
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
MiddleNolocationNoTFListIterator::~MiddleNolocationNoTFListIterator()
{
}

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::expunge -- DEBUG用
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
MiddleNolocationNoTFListIterator::expunge()
{
	MiddleBaseListIterator::expunge();
	; _INVERTED_FAKE_ERROR(MiddleNolocationNoTFListIterator::expunge);
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::undoExpunge -- DEBUG用
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
MiddleNolocationNoTFListIterator::undoExpunge(
	ModUInt32 uiDocumentID_,
	const ModInvertedSmartLocationList& cLocationList_)
{
	MiddleBaseListIterator::undoExpunge(uiDocumentID_, cLocationList_);
	; _INVERTED_FAKE_ERROR(MiddleNolocationNoTFListIterator::undoExpunge);
}
#endif

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::getHeadAddress --
//		現在の位置情報の先頭アドレスを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModUInt32*
MiddleNolocationNoTFListIterator::getHeadAddress()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::getLocationOffset --
//		現在の位置情報のオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationNoTFListIterator::getLocationOffset()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::getLocationBitLength --
//		現在の位置情報のビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationNoTFListIterator::getLocationBitLength()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::getLocationDataOffset --
//		現在の位置情報データのオフセットを得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationNoTFListIterator::getLocationDataOffset()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::getLocationDataBitLength --
//		現在の位置情報データのビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
ModSize
MiddleNolocationNoTFListIterator::getLocationDataBitLength()
{
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationNoTFListIterator::expungeIdBlock --
//		現在位置のIDブロックを削除する
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
	//
	// IDブロックを実際に削除する。
	// 削除フラグは、先頭文書IDを削除し格納文書IDが0件になると、立つ。
	//
	
	if (m_cIdBlock.isExpunge() == true)
	{
		//
		// もし削除フラグが立ってなければ何もしない
		//
		
		if (m_cIdBlock != m_pArea->getHeadAddress())
		{
			// IDブロックはオーバーフローページ内にある

			//
			// もしリーフページ内なら最終IDブロックなので、何もしない
			//
			
			// IDブロックを削除する
			m_pIdPage->freeIDBlock(m_uiIDBlockPosition);
			
			// IDページを削除する
			; _SYDNEY_ASSERT(m_pIdPage->getType() == OverflowPage::Type::ID);
			if (m_pIdPage->getIDBlockCount() == 0)
			{
				// このページは不要
				m_pOverflowFile->freePage(m_pIdPage);

				//
				// DIRブロックを削除する
				// 対象のDIRブロックより後ろのブロックを前にずらす。
				// 最後のDIRブロックの処理(AREAのサイズの縮小)は、
				// 呼び出し先で行う。
				//
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
//	FUNCTION private
//	Inverted::MiddleNolocationNoTFListIterator::nextLocBlock --
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS
//
bool
MiddleNolocationNoTFListIterator::nextLocBlock(ModSize uiOddLength_)
{
	// 位置情報がないので呼ばれない。
	_SYDNEY_THROW0(Exception::Unexpected);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
