// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchNolocationNoTFList.cpp --
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

#include "Inverted/BatchListMap.h"
#include "Inverted/BatchNolocationNoTFList.h"
#include "Inverted/BatchNolocationNoTFListIterator.h"

#include "Common/Assert.h"

#include "ModList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::BatchNolocationNoTFList::BatchNolocationNoTFList -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	Inverted::BatchListMap& cBatchListMap_
//		バッチリストマップ
//	const ModUnicodeChar* pKey_
//		索引単位
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchNolocationNoTFList::BatchNolocationNoTFList(InvertedUnit& cInvertedUnit_,
												 BatchListMap& cBatchListMap_,
												 const ModUnicodeChar* pszKey_)
	: BatchBaseList(cInvertedUnit_, cBatchListMap_, pszKey_),
	  m_pMap(&cBatchListMap_), m_pArea(0)
{
	m_pArea = LeafPage::Area::allocateArea(getKey(), getAllocateUnitSize());

	// 大きさを求める
	ModSize size = m_pArea->getUnitSize() * sizeof(ModUInt32);
	size += sizeof(BatchNolocationNoTFList);			// BatchNolocationNoTFList自体の大きさ
	size += getKey().getBufferSize();	// ModUnicodeStringの大きさ
	size += sizeof(ModListNode<BatchNolocationNoTFList*>);		// ListNodeの大きさ

	m_pMap->addListSize(size);
}

//
//	FUNCTION public
//	Inverted::BatchNolocationNoTFList::BatchNolocationNoTFList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	LeafPage::Area* pArea_
//		リーフページのエリア
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
BatchNolocationNoTFList::BatchNolocationNoTFList(InvertedUnit& cInvertedUnit_, LeafPage::Area* pArea_)
	: BatchBaseList(cInvertedUnit_, pArea_),
	  m_pMap(0), m_pArea(pArea_)
{
}

//
//	FUNCTION public
//	Inverted::BatchNolocationNoTFList::~BatchNolocationNoTFList -- デストラクタ
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
BatchNolocationNoTFList::~BatchNolocationNoTFList()
{
}

//
//	FUNCTION public
//	Inverted::BatchNolocationNoTFList::begin -- イテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverted::InvertedItertor*
//		イテレータ
//
//	EXCEPTIONS
//
InvertedIterator*
BatchNolocationNoTFList::begin() const
{
	return new BatchNolocationNoTFListIterator(const_cast<BatchNolocationNoTFList&>(*this));
}

//
//	FUNCTION public
//	Inverted::BatchNolocationList::getCompressedBitLengthLocationList -- 位置情報リストの圧縮ビット長を得る
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報リスト
//	ModSize& uiBitLength_
//		位置情報データのビット長
//
//	RETURN
//	ModSize
//		位置情報全体の圧縮ビット長
//
//	EXCEPTIONS
//
ModSize
BatchNolocationNoTFList::getCompressedBitLengthLocationList(
	const ModInvertedSmartLocationList& cLocationList_,
	ModSize& uiBitLength_)
{
	uiBitLength_ = 0;
	return 0;
}

//
//	FUNCTION public
//	Inverted::BatchNolocationList::writeLocationList -- 位置情報リストを書き込む
//
//	NOTES
//
//	ARGUMENTS
//	const ModInvertedSmartLocationList& cLocationList_
//		位置情報リスト
//	ModSize uiBitLength_
//		位置情報データのビット長
//	ModUInt32* pHeadAddress_
//		先頭のアドレス
//	ModSize& uiBitOffset_
//		書き込むビット位置。呼び出し後は次に書き込むビット位置
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
BatchNolocationNoTFList::writeLocationList(const ModInvertedSmartLocationList& cLocationList_,
										   ModSize uiBitLength_,
										   ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
	; _SYDNEY_ASSERT(uiBitLength_ == 0);
	; _SYDNEY_ASSERT(uiBitOffset_ == 0);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
