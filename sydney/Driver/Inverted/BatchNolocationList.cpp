// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BatchNolocationList.cpp --
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
#include "Inverted/BatchNolocationList.h"
#include "Inverted/BatchNolocationListIterator.h"

#include "Common/Assert.h"

#include "ModInvertedSmartLocationList.h"

#include "ModList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::BatchNolocationList::BatchNolocationList -- コンストラクタ(1)
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
BatchNolocationList::BatchNolocationList(InvertedUnit& cInvertedUnit_,
										 BatchListMap& cBatchListMap_,
										 const ModUnicodeChar* pszKey_)
: BatchBaseList(cInvertedUnit_, cBatchListMap_, pszKey_),
  m_pMap(&cBatchListMap_), m_pArea(0)
{
	m_pArea = LeafPage::Area::allocateArea(getKey(), getAllocateUnitSize());

	// 大きさを求める
	ModSize size = m_pArea->getUnitSize() * sizeof(ModUInt32);
	size += sizeof(BatchNolocationList);			// BatchNolocationList自体の大きさ
	size += getKey().getBufferSize();	// ModUnicodeStringの大きさ
	size += sizeof(ModListNode<BatchNolocationList*>);		// ListNodeの大きさ

	m_pMap->addListSize(size);
}

//
//	FUNCTION public
//	Inverted::BatchNolocationList::BatchNolocationList -- コンストラクタ(2)
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
BatchNolocationList::BatchNolocationList(InvertedUnit& cInvertedUnit_, LeafPage::Area* pArea_)
	: BatchBaseList(cInvertedUnit_, pArea_),
	  m_pMap(0), m_pArea(pArea_)
{
}

//
//	FUNCTION public
//	Inverted::BatchNolocationList::~BatchNolocationList -- デストラクタ
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
BatchNolocationList::~BatchNolocationList()
{
}

//
//	FUNCTION public
//	Inverted::BatchNolocationList::begin -- イテレータを得る
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
BatchNolocationList::begin() const
{
	return new BatchNolocationListIterator(const_cast<BatchNolocationList&>(*this));
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
BatchNolocationList::getCompressedBitLengthLocationList(const ModInvertedSmartLocationList& cLocationList_,
														ModSize& uiBitLength_)
{
	uiBitLength_ = 0;
	// 頻度情報
	return getCompressedBitLengthFrequency(cLocationList_.getSize());
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
BatchNolocationList::writeLocationList(const ModInvertedSmartLocationList& cLocationList_,
									   ModSize uiBitLength_,
									   ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
	; _SYDNEY_ASSERT(uiBitLength_ == 0);

	// 頻度
	writeLocationFrequency(cLocationList_.getSize(), pHeadAddress_, uiBitOffset_);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
