// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleNolocationList.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "FullText2/FakeError.h"
#include "FullText2/InvertedIterator.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/MiddleNolocationList.h"
#include "FullText2/MiddleNolocationListIterator.h"
#include "FullText2/OverflowFile.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::MiddleNolocationList::MiddleNolocationList -- コンストラクタ(1)
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//  FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//  FullText2::LeafPage::Iterator ite_
//		該当する索引単位のエリアへのイテレータ
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
}

//
//  FUNCTION public
//  FullText2::MiddleNolocationList::MiddleNolocationList -- コンストラクタ(2)
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//	  転置ファイル
//  FullText2::LeafPage::Area* pTmpArea_
//	  一時的なエリア
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleNolocationList::MiddleNolocationList(InvertedUnit& cInvertedUnit_,
										   LeafPage::Area* pTmpArea_)
	: MiddleBaseList(cInvertedUnit_, pTmpArea_)
{
}

//
//  FUNCTION public
//  FullText2::MiddleNolocationList::~MiddleNolocationList -- デストラクタ
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
//  FullText2::MiddleNolocationList::insert -- DEBUG用
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
							 const SmartLocationList& cLocationList_)
{
	if (MiddleBaseList::insert(uiDocumentID_, cLocationList_) == false)
	{
		return false;
	}
	; _FULLTEXT2_FAKE_ERROR(MiddleNolocationList::insert);
	return true;
}
#endif

//
//  FUNCTION public
//  FullText2::MiddleNolocationList::getIterator -- 転置リストイテレータを得る
//
//  NOTES
//
//  ARGUMENTS
//  なし
//
//  RETURN
//  FullText2::InvertedIterator*
//	  転置リストイテレータ
//
//  EXCEPTIONS
//
InvertedIterator*
MiddleNolocationList::getIterator()
{
	return new MiddleNolocationListIterator(*this);
}

//
//  FUNCTION protected
//  FullText2::MiddleNolocationList::insertLocation -- 位置情報を挿入する
//
//  NOTES
//
//  ARGUMENTS
//  const FullText2::SmartLocationList& cLocationList_
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
MiddleNolocationList::insertLocation(const SmartLocationList& cLocationList_,
									 OverflowPage::PagePointer& pLocPage_,
									 OverflowPage::LocBlock& cLocBlock_)
{
	ModSize uiFrequency = cLocationList_.getCount();
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
	writeLocationFrequency(uiFrequency,
						   cLocBlock_.getBuffer(), uiDataBitLength);

	//
	// LOCブロックのヘッダの更新
	//
	cLocBlock_.setDataBitLength(uiDataBitLength);
}

//
//  FUNCTION protected
//  FullText2::MiddleNolocationList::insertLocation -- 位置情報を挿入する
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedIterator& ite_
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
MiddleNolocationList::insertLocation(FullText2::InvertedIterator& ite_,
									 OverflowPage::PagePointer& pLocPage_,
									 OverflowPage::LocBlock& cLocBlock_)
{
	ModSize uiFrequency = ite_.getTermFrequency();
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
//	FUNCTION protected
//	FullText2::MiddleNolocationList::makeTmpMiddleList
//		-- 一時的なミドルリストを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::Area* pTmpArea_
//		一時エリア
//
//	RETURN
//	FullText2::MiddleBaseList*
//		ミドルリスト
//
//	EXCEPTIONS
//
MiddleBaseList*
MiddleNolocationList::makeTmpMiddleList(InvertedUnit& cInvertedUnit_,
										LeafPage::Area* pTmpArea_)
{
	return new MiddleNolocationList(cInvertedUnit_, pTmpArea_);
}

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
