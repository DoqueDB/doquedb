// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortListIterator.cpp --
// 
// Copyright (c) 2002, 2005, 2008, 2010, 2023 Ricoh Company, Ltd.
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

#include "Inverted/ShortListIterator.h"
#include "Inverted/InvertedList.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/LeafPage.h"
#include "Inverted/Types.h"
#include "Inverted/ShortListLocationListIterator.h"
#include "Inverted/FakeError.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortListIterator::ShortListIterator -- コンストラクタ
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
ShortListIterator::ShortListIterator(InvertedList& cInvertedList_)
	: ShortBaseListIterator(cInvertedList_),
	  m_iCurrentLocPosition(-1), m_uiCurrentLocOffset(0),
	  m_uiCurrentLocLength(0), m_uiCurrentFrequency(0), m_uiCurrentLocation(0),
	  m_pFree(0)
{
	// エリアを得る
	m_pHeadAddress = m_pArea->getHeadAddress();
}

//
//	FUNCTION public
//	Inverted::ShortListIterator::~ShortListIterator -- デストラクタ
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
	while (m_pFree)
	{
		ShortListLocationListIterator* p
			= _SYDNEY_DYNAMIC_CAST(ShortListLocationListIterator*, m_pFree);
		m_pFree = p->nextInstance;
		delete p;
	}
}

//
//	FUNCTION public
//	Inverted::ShortListIterator::getLocationListIterator -- 位置情報へのイテレータを得る
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
ShortListIterator::getLocationListIterator()
{
	// 位置情報との同期を取る
	synchronize();

	ShortListLocationListIterator* locations = 0;

	// LocationListiteratorを割り当てる
	if (m_pFree)
	{
		locations
			= _SYDNEY_DYNAMIC_CAST(ShortListLocationListIterator*, m_pFree);
		m_pFree = locations->nextInstance;
	}
	else
	{
		locations = new ShortListLocationListIterator(this);
	}

	// 初期化する
	locations->initialize(m_pHeadAddress,
						  m_uiCurrentFrequency,
						  m_uiCurrentLocDataOffset,
						  m_uiCurrentLocDataOffset + m_uiCurrentLocDataLength,
						  m_cInvertedList.getLocationCoder(),
						  m_uiCurrentLocation);

	return locations;
}

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::ShortListIterator::expunge -- DEBUG用
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
ShortListIterator::expunge()
{
	ShortBaseListIterator::expunge();
	; _INVERTED_FAKE_ERROR(ShortListIterator::expunge);
}

//
//	FUNCTION public
//	Inverted::ShortListIterator::undoExpunge -- DEBUG用
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//	const ModInvertedSmartLocationList& cLocationList_
//
//	RETURN
//
//	EXCEPTIONS
//
void
ShortListIterator::undoExpunge(ModUInt32 uiDocumentID_, const ModInvertedSmartLocationList& cLocationList_)
{
	ShortBaseListIterator::undoExpunge(uiDocumentID_, cLocationList_);
	; _INVERTED_FAKE_ERROR(ShortListIterator::undoExpunge);
}
#endif

//
//	FUNCTION public
//	Inverted::ShortListIterator::pushBack -- 不要な位置情報クラスをpushする
//
//	NOTES
//
//	ARGUMENTS
//	ShortListLocationListIterator* pFree_
//		不要な位置情報クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
ShortListIterator::pushBack(ShortListLocationListIterator* pFree_)
{
	pFree_->nextInstance = m_pFree;
	m_pFree = pFree_;
}

//
//	FUNCTION private
//	Inverted::ShortListIterator::setLocationInfomation -- 位置情報変数を設定する
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
ShortListIterator::setLocationInformation()
{
	// 位置情報の先頭オフセット
	ModSize uiOffset = m_uiCurrentLocOffset;

	// 頻度を読む
	m_uiCurrentFrequency
		= m_cInvertedList.readLocationFrequency(
			m_pHeadAddress, m_pArea->getLocationOffset(), uiOffset);

	// 頻度以外を読む
	if (m_uiCurrentFrequency == 1)
	{
		// 一件の位置が格納されている
		
		// 位置情報データの先頭位置
		m_uiCurrentLocDataOffset = uiOffset;

		// 位置を読み込む(頻度が1なので、ビット長は書かれていない)
		m_uiCurrentLocation
			= m_cInvertedList.readLocationData(0,
											   m_pHeadAddress,
											   m_pArea->getLocationOffset(),
											   uiOffset);
		// 位置情報データのビット長
		m_uiCurrentLocDataLength = uiOffset - m_uiCurrentLocDataOffset;
	}
	else if (m_uiCurrentFrequency > 1)
	{
		// 位置リストのビット長と、位置リストが格納されている

		// 個々の位置は読み込まない
		m_uiCurrentLocation = 0;
		
		// 位置情報データのビット長を読む
		m_uiCurrentLocDataLength
			= m_cInvertedList.readLocationBitLength(
				m_pHeadAddress,
				m_pArea->getLocationOffset(),
				uiOffset);
		// 位置情報データの先頭位置
		m_uiCurrentLocDataOffset = uiOffset;

		// 位置リストのビット長の分だけ、オフセットを進める
		uiOffset += m_uiCurrentLocDataLength;
	}
	else
	{
		// 削除によって0件になっていた場合
		m_uiCurrentLocation = 0;
		m_uiCurrentLocDataLength = 0;
		m_uiCurrentLocDataOffset = uiOffset;
	}

	// 位置情報全体のビット長を設定する
	m_uiCurrentLocLength = uiOffset - m_uiCurrentLocOffset;
}

//
//	FUNCTION private
//	Inverted::ShortListIterator::skipLocationInfomation -- 位置情報を読み飛ばす
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
ShortListIterator::skipLocationInformation()
{
	// 頻度を読み飛ばす
	ModSize uiFrequency = m_cInvertedList.readLocationFrequency(
		m_pHeadAddress, m_pArea->getLocationOffset(), m_uiCurrentLocOffset);

	// 頻度以外を読み飛ばす
	if (uiFrequency == 1)
	{
		// 一件の位置が格納されている
		
		// 位置を読み飛ばす
		m_cInvertedList.readLocationData(
			0, m_pHeadAddress,
			m_pArea->getLocationOffset(), m_uiCurrentLocOffset);
	}
	else
	{
		// 位置リストのビット長と、位置リストが格納されている

		// 位置リストのビット長を読み飛ばす
		ModSize length
			= m_cInvertedList.readLocationBitLength(
				m_pHeadAddress,
				m_pArea->getLocationOffset(),
				m_uiCurrentLocOffset);
		// 位置リストを読み飛ばす
		m_uiCurrentLocOffset += length;
	}
}

//
//	Copyright (c) 2002, 2005, 2008, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
