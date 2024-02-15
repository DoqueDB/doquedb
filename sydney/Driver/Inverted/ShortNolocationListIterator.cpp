// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationListIterator.cpp --
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

#include "Inverted/FakeError.h"
#include "Inverted/InvertedList.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/LeafPage.h"
#include "Inverted/ShortNolocationListIterator.h"
#include "Inverted/Types.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortNolocationListIterator::ShortNolocationListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedList& cInvertedList_
//		転置リスト(Short or Batch)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortNolocationListIterator::ShortNolocationListIterator(InvertedList& cInvertedList_)
	: ShortBaseListIterator(cInvertedList_),
	  m_iCurrentLocPosition(-1), m_uiCurrentLocOffset(0),
	  m_uiCurrentLocLength(0), m_uiCurrentFrequency(0)
{
	// エリアを得る
	m_pHeadAddress = m_pArea->getHeadAddress();
}

//
//	FUNCTION public
//	Inverted::ShortNolocationListIterator::~ShortNolocationListIterator -- デストラクタ
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

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::ShortNolocationListIterator::expunge -- DEBUG用
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
ShortNolocationListIterator::expunge()
{
	ShortBaseListIterator::expunge();
	; _INVERTED_FAKE_ERROR(ShortNolocationListIterator::expunge);
}

//
//	FUNCTION public
//	Inverted::ShortNolocationListIterator::undoExpunge -- DEBUG用
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
ShortNolocationListIterator::undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	ShortBaseListIterator::undoExpunge(uiDocumentID_, cLocationList_);
	; _INVERTED_FAKE_ERROR(ShortNolocationListIterator::undoExpunge);
}
#endif

//
//	FUNCTION private
//	Inverted::ShortNolocationListIterator::setLocationInfomation -- 位置情報変数を設定する
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
ShortNolocationListIterator::setLocationInformation()
{
	// 位置情報の先頭オフセット
	ModSize uiOffset = m_uiCurrentLocOffset;

	// 頻度を読む
	m_uiCurrentFrequency
		= m_cInvertedList.readLocationFrequency(
			m_pHeadAddress, m_pArea->getLocationOffset(), uiOffset);

	// 位置情報全体のビット長を設定する
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	Inverted::ShortNolocationListIterator::skipLocationInfomation -- 位置情報を読み飛ばす
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
ShortNolocationListIterator::skipLocationInformation()
{
	// 頻度を読み飛ばす
	m_cInvertedList.readLocationFrequency(
		m_pHeadAddress, m_pArea->getLocationOffset(), m_uiCurrentLocOffset);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
