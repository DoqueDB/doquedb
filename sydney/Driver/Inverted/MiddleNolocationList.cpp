// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationList.cpp --
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
#include "Inverted/InvertedIterator.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/MiddleNolocationList.h"
#include "Inverted/MiddleNolocationListIterator.h"
#include "Inverted/OverflowFile.h"

#include "Common/Assert.h"

#include "ModInvertedSmartLocationList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//  FUNCTION public
//  Inverted::MiddleNolocationList::MiddleNolocationList -- コンストラクタ
//
//  NOTES
//  ShortListと違って、LeafPageに該当するエリアがない場合はMiddleNolocationListを
//  構築することはできない。
//
//  ARGUMENTS
//  Inverted::InvertedUnit& cInvertedUnit_
//	  転置ファイル
//  Inverted::LeafPage::PagePointer pLeafPage_
//	  リーフページ
//  Inverted::LeafPage::Iterator ite_
//	  該当する索引単位のエリアへのイテレータ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleNolocationList::MiddleNolocationList(InvertedUnit& cInvertedUnit_,
										   LeafPage::PagePointer pLeafPage_,
										   LeafPage::Iterator ite_)
: MiddleBaseList(cInvertedUnit_, pLeafPage_, ite_)
{
	m_pOverflowFile = cInvertedUnit_.getOverflowFile();
}

//
//  FUNCTION public
//  Inverted::MiddleNolocationList::~MiddleNolocationList -- デストラクタ
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleNolocationList::~MiddleNolocationList()
{
}

#ifdef DEBUG
//
//  FUNCTION public
//  Inverted::MiddleNolocationList::insert -- DEBUG用
//
//  NOTES
//
//  ARGUMENTS
//
//  RETURN
//
//  EXCEPTIONS
//
bool
MiddleNolocationList::insert(ModUInt32 uiDocumentID_,
							 const ModInvertedSmartLocationList& cLocationList_)
{
	if (MiddleBaseList::insert(uiDocumentID_, cLocationList_) == false)
	{
		return false;
	}
	; _INVERTED_FAKE_ERROR(MiddleNolocationList::insert);
	return true;
}
#endif

//
//  FUNCTION public
//  Inverted::MiddleNolocationList::begin -- 転置リストイテレータを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  Inverted::InvertedIterator*
//	  転置リストイテレータ
//
//  EXCEPTIONS
//
InvertedIterator*
MiddleNolocationList::begin() const
{
	return new MiddleNolocationListIterator(const_cast<MiddleNolocationList&>(*this));
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationList::getCompressedBitLengthLocationList -- 位置情報リストの圧縮ビット長を得る
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
MiddleNolocationList::getCompressedBitLengthLocationList(const ModInvertedSmartLocationList& cLocationList_,
														 ModSize& uiBitLength_)
{
	uiBitLength_ = 0;
	// 頻度情報
	return getCompressedBitLengthFrequency(cLocationList_.getSize());
}

//
//	FUNCTION public
//	Inverted::MiddleNolocationList::writeLocationList -- 位置情報リストを書き込む
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
MiddleNolocationList::writeLocationList(const ModInvertedSmartLocationList& cLocationList_,
										ModSize uiBitLength_,
										ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
	; _SYDNEY_ASSERT(uiBitLength_ == 0);

	// 頻度
	writeLocationFrequency(cLocationList_.getSize(), pHeadAddress_, uiBitOffset_);
}

//
//  FUNCTION private
//  Inverted::MiddleNolocationList::insertLocation -- 位置情報を挿入する
//
//  NOTES
//
//  ARGUMENTS
//  const ModInvertedSmartLocationList& cLocationList_
//	  位置情報リスト
//  OverflowPage::PagePointer& pLocPage_
//	  Locページ
//  OverflowPage::LocBlock& cLocBlock_
//	  LOCブロック
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
MiddleNolocationList::insertLocation(const ModInvertedSmartLocationList& cLocationList_,
									 OverflowPage::PagePointer& pLocPage_,
									 OverflowPage::LocBlock& cLocBlock_)
{
	ModSize uiFrequency = cLocationList_.getSize();
	ModSize locLength = getCompressedBitLengthFrequency(uiFrequency);
	ModSize uiDataUnitSize = cLocBlock_.getDataUnitSize();
	ModSize uiDataBitLength = cLocBlock_.getDataBitLength();

	// prepareForLocationは不要。
	// expandLocBlock内のexpandUnitSizeで実行される
	pLocPage_->dirty();

	//
	// MiddleListと異なり、TFしか書きこまないので、
	// 位置情報の各要素を順々に書くような処理は不要
	//
	
	//
	// 必要な分だけ領域を確保する。
	//
	expandLocBlock(pLocPage_, cLocBlock_,
				   locLength, uiDataBitLength, uiDataUnitSize);
	
	//
	// TFの書き込み
	//
	writeLocationFrequency(uiFrequency, cLocBlock_.getBuffer(), uiDataBitLength);

	//
	// LOCブロックのヘッダの更新
	//
	cLocBlock_.setDataBitLength(uiDataBitLength);
}

//
//  FUNCTION private
//  Inverted::MiddleNolocationList::insertLocation -- 位置情報を挿入する
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::InvertedIterator& ite_
//	  転置リストイテレータ
//  OverflowPage::PagePointer& pLocPage_
//	  Locページ
//  OverflowPage::LocBlock& cLocBlock_
//	  LOCブロック
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//
void
MiddleNolocationList::insertLocation(Inverted::InvertedIterator& ite_,
							   OverflowPage::PagePointer& pLocPage_,
							   OverflowPage::LocBlock& cLocBlock_)
{
	ModSize uiFrequency = ite_.getInDocumentFrequency();
	ModSize locLength = ite_.getLocationBitLength();
	ModSize uiDataUnitSize = cLocBlock_.getDataUnitSize();
	ModSize uiDataBitLength = cLocBlock_.getDataBitLength();
	ModSize uiNewDataUnitSize = calcUnitSize(locLength + uiDataBitLength);

	//
	// LOCページの確認
	//

	// prepareForLocationは不要。
	// expandLocBlock内のexpandUnitSizeで実行される
	pLocPage_->dirty();

	//
	// MiddleListと異なり、ite_ にはTFしか格納されていないので、
	// 複数のLOCブロックにまたがっていることはない。
	// また、TFしか書きこまないので、
	// 位置情報の各要素を順々に書くような処理も不要。
	// 

	//
	// 必要な分だけ領域を確保する。
	//
	expandLocBlock(pLocPage_, cLocBlock_,
				   locLength, uiDataBitLength, uiDataUnitSize);

	//
	// そのままコピーで、TFを書き込む
	//
	move(cLocBlock_.getBuffer(), uiDataBitLength,
		 ite_.getHeadAddress(), ite_.getLocationOffset(), locLength);
	uiDataBitLength += locLength;

	//
	// LOCブロックのヘッダを更新
	//
	cLocBlock_.setDataBitLength(uiDataBitLength);
}

//
//  Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
