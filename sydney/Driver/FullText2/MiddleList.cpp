// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleList.cpp --
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

#include "FullText2/MiddleList.h"
#include "FullText2/MiddleListIterator.h"
#include "FullText2/OverflowFile.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/InvertedIterator.h"
#include "FullText2/FakeError.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//  FUNCTION public
//  FullText2::MiddleList::MiddleList -- コンストラクタ(1)
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::InvertedUnit& cInvertedUnit_
//	  転置ファイル
//  FullText2::LeafPage::PagePointer pLeafPage_
//	  リーフページ
//  FullText2::LeafPage::Iterator ite_
//	  該当する索引単位のエリアへのイテレータ
//
//  RETURN
//  なし
//
//  EXCEPTIONS
//  なし
//
MiddleList::MiddleList(InvertedUnit& cInvertedUnit_,
					   LeafPage::PagePointer pLeafPage_,
					   LeafPage::Iterator ite_)
	: MiddleBaseList(cInvertedUnit_, pLeafPage_, ite_)
{
}

//
//  FUNCTION public
//  FullText2::MiddleList::MiddleList -- コンストラクタ(2)
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
MiddleList::MiddleList(InvertedUnit& cInvertedUnit_,
					   LeafPage::Area* pTmpArea_)
	: MiddleBaseList(cInvertedUnit_, pTmpArea_)
{
}

//
//  FUNCTION public
//  FullText2::MiddleList::~MiddleList -- デストラクタ
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
MiddleList::~MiddleList()
{
}

#ifdef DEBUG
//
//  FUNCTION public
//  FullText2::MiddleList::insert -- DEBUG用
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
MiddleList::insert(ModUInt32 uiDocumentID_,
				   const SmartLocationList& cLocationList_)
{
	if (MiddleBaseList::insert(uiDocumentID_, cLocationList_) == false)
	{
		return false;
	}
	; _FULLTEXT2_FAKE_ERROR(MiddleList::insert);
	return true;
}
#endif

//
//  FUNCTION public
//  FullText2::MiddleList::getIterator -- 転置リストイテレータを得る
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
MiddleList::getIterator()
{
	return new MiddleListIterator(*this);
}

//
//  FUNCTION private
//  FullText2::MiddleList::insertLocation -- 位置情報を挿入する
//
//  NOTES
//
//  ARGUMENTS
//  const SmartLocationList& cLocationList_
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
MiddleList::insertLocation(const SmartLocationList& cLocationList_,
						   OverflowPage::PagePointer& pLocPage_,
						   OverflowPage::LocBlock& cLocBlock_)
{
	// 位置リストのビット長
	ModSize uiBitLength;
	// 位置情報全体のビット長
	ModSize locLength = getCompressedBitLengthLocationList(cLocationList_,
														   uiBitLength);
	ModSize uiDataUnitSize = cLocBlock_.getDataUnitSize();
	ModSize uiDataBitLength = cLocBlock_.getDataBitLength();
	ModSize uiNewDataUnitSize = calcUnitSize(locLength + uiDataBitLength);

	//
	// LOCページの確認
	//

	prepareForLocation(pLocPage_, uiDataUnitSize, uiNewDataUnitSize);
	pLocPage_->dirty();

	//
	// Locationの書き込み
	//

	if (pLocPage_->isLargeEnough(uiDataUnitSize, uiNewDataUnitSize) == false)
	{
		// まとめて挿入できない

		// 一つずつ挿入する
		LocationListIterator::AutoPointer i	= cLocationList_.getIterator();
		ModSize uiFrequency = cLocationList_.getCount();
		insertOneByOne(i.get(), pLocPage_, cLocBlock_,
					   uiFrequency, uiBitLength, locLength);
	}
	else
	{
		// そのまま入る

		// 必要な分だけ領域を確保する
		expandLocBlock(cLocBlock_, locLength, uiDataBitLength, uiDataUnitSize);

		// すべて書き込む
		writeLocationList(cLocationList_, uiBitLength,
						  cLocBlock_.getBuffer(), uiDataBitLength);

		// LOCブロックのヘッダを更新
		cLocBlock_.setDataBitLength(uiDataBitLength);
	}
}

//
//  FUNCTION private
//  FullText2::MiddleList::insertLocation -- 位置情報を挿入する
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
MiddleList::insertLocation(FullText2::InvertedIterator& ite_,
						   OverflowPage::PagePointer& pLocPage_,
						   OverflowPage::LocBlock& cLocBlock_)
{
	// 位置情報全体のビット長
	ModSize locLength = ite_.getLocationBitLength();
	ModSize uiDataUnitSize = cLocBlock_.getDataUnitSize();
	ModSize uiDataBitLength = cLocBlock_.getDataBitLength();
	ModSize uiNewDataUnitSize = calcUnitSize(locLength + uiDataBitLength);
	
	//
	// LOCページの確認
	//

	prepareForLocation(pLocPage_, uiDataUnitSize, uiNewDataUnitSize);
	pLocPage_->dirty();

	//
	// Locationの書き込み
	//

	if (ite_.isContinue() == true
		|| pLocPage_->isLargeEnough(uiDataUnitSize, uiNewDataUnitSize) == false)
	{
		// まとめて挿入できない
		// InvertedIteratorなので、複数LOCブロックにまたがった
		// 転置リストの可能性もある

		// 1つずつ挿入する
		LocationListIterator::AutoPointer i = ite_.getLocationListIterator();
		ModSize uiFrequency = ite_.getTermFrequency();
		ModSize uiBitLength = ite_.getLocationDataBitLength();
		insertOneByOne(i.get(), pLocPage_, cLocBlock_,
					   uiFrequency, uiBitLength, locLength);
	}
	else
	{
		// そのまま入る

		// 必要な分だけ領域を確保する
		expandLocBlock(cLocBlock_, locLength, uiDataBitLength, uiDataUnitSize);

		// すべて書き込む -> そのままコピーすればいい
		move(cLocBlock_.getBuffer(), uiDataBitLength,
			 ite_.getHeadAddress(), ite_.getLocationOffset(), locLength);
		uiDataBitLength += locLength;

		// LOCブロックのヘッダを更新
		cLocBlock_.setDataBitLength(uiDataBitLength);
	}
}

//
//  FUNCTION private
//  FullText2::MiddleList::insertOneByOne
//		-- 頻度、ビット長、位置リストを順々に挿入する
//
//  NOTES
//
//  ARGUMENTS
// 	FullText2::LocationListIterator* ite_
//	  位置情報リスト
//  FullText2::OverflowPage::PagePointer& pLocPage_
//	  ロックページ
//  FullText2::OverflowPage::LocBlock& cLocBlock_
//	  ロックブロック
//	ModSize uiFrequency_
//	  TF
//  ModSize uiBitLength_
//	  位置リストのビット長(頻度が1の場合は位置情報)
//	ModSize uiLocLength_
//	  位置情報全体のビット長
//
//  RETURN
//  ModSize
//	  書いたビット長
//
//  EXCEPTIONS
//
ModSize
MiddleList::insertOneByOne(LocationListIterator* ite_,
						   OverflowPage::PagePointer& pLocPage_,
						   OverflowPage::LocBlock& cLocBlock_,
						   ModSize uiFrequency_,
						   ModSize uiBitLength_,
						   ModSize uiLocLength_)
{
	if (uiFrequency_ == 1)
	{
		// 頻度が1の場合は、uiBitLength_に位置情報を設定する
		int dummy;
		uiBitLength_ = (*ite_).next(dummy);

		// 頻度と位置を書く
		uiLocLength_ -= writeFrequencyAndBitLength(pLocPage_, cLocBlock_,
												   uiFrequency_, uiBitLength_);
	}
	else
	{
		// 頻度とビット長を書く
		uiLocLength_ -= writeFrequencyAndBitLength(pLocPage_, cLocBlock_,
												   uiFrequency_, uiBitLength_);
		// 位置リストを挿入する
		insertLocationData(ite_, uiLocLength_, pLocPage_, cLocBlock_);

		// (*ite_).next()は不要。insertLocationData内のwriteLocationDataで
		// 実行されている。
	}

	return uiLocLength_;
}

//
//  FUNCTION private
//  FullText2::MiddleList::writeFrequencyAndBitLength -- 頻度とビット長を書く
//
//  NOTES
//
//  ARGUMENTS
//  FullText2::OverflowPage::PagePointer& pLocPage_
//	  ロックページ
//  FullText2::OverflowPage::LocBlock& cLocBlock_
//	  ロックブロック
//  ModSize uiFrequency_
//	  頻度
//  ModSize uiBitLength_
//	  ビット長(頻度が1の場合は位置情報)
//
//  RETURN
//  ModSize
//	  書いたビット長
//
//  EXCEPTIONS
//
ModSize
MiddleList::writeFrequencyAndBitLength(OverflowPage::PagePointer& pLocPage_,
									   OverflowPage::LocBlock& cLocBlock_,
									   ModSize uiFrequency_,
									   ModSize uiBitLength_)
{
	//
	//  【注意】
	//	頻度とビット長(頻度が1の場合は位置情報)の間でページを跨いではいけない
	//  まとめて書けない場合は新しいブロックを得る
	//

	ModSize uiDataUnitSize = cLocBlock_.getDataUnitSize();
	ModSize uiDataBitLength = cLocBlock_.getDataBitLength();

	//
	// 書き込みに必要な領域のビット長を調べる
	//
	
	// 頻度情報
	ModSize uiTotalBitLength = getCompressedBitLengthFrequency(uiFrequency_);
	if (uiFrequency_ > 1)
	{
		// ビット長
		uiTotalBitLength += getCompressedBitLengthBitLength(uiBitLength_);
	}
	else
	{
		// 位置情報
		uiTotalBitLength += getCompressedBitLengthLocation(0, uiBitLength_);
	}

	//
	// 必要な分だけ領域を確保する
	//
	expandLocBlock(pLocPage_, cLocBlock_,
				   uiTotalBitLength, uiDataBitLength, uiDataUnitSize);

	//
	//  頻度を書く
	//
	writeLocationFrequency(uiFrequency_,
						   cLocBlock_.getBuffer(), uiDataBitLength);

	//
	//  ビット長(頻度が1の場合は位置情報)を書く
	//
	if (uiFrequency_ > 1)
	{
		writeLocationBitLength(uiBitLength_,
							   cLocBlock_.getBuffer(), uiDataBitLength);
	}
	else
	{
		writeLocationData(0, uiBitLength_,
						  cLocBlock_.getBuffer(), uiDataBitLength);
	}

	//
	// LOCブロックのヘッダを更新
	//
	cLocBlock_.setDataBitLength(uiDataBitLength);
	
	pLocPage_->dirty();

	return uiTotalBitLength;
}

//
//  FUNCTION private
//  FullText2::MiddleList::insertLocationData -- 位置情報を挿入する
//
//  NOTES
//  頻度とビット長を書いた後に、この関数を呼び出す
//
//  ARGUMENTS
//  FullText2::LocationListIterator* ite_
//	  位置情報イテレータ
//	ModSize uiLocLength_
//	  位置情報全体のビット長
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
MiddleList::insertLocationData(LocationListIterator* ite_,
							   ModSize uiLocLength_,
							   OverflowPage::PagePointer& pLocPage_,
							   OverflowPage::LocBlock& cLocBlock_)
{
	ModSize uiLocLength = uiLocLength_;
	ModUInt32 uiLastLocation = 0;
	ModUInt32 uiCurrentLocation = 0;
	ModSize uiDataUnitSize = cLocBlock_.getDataUnitSize();
	ModSize uiDataBitLength = cLocBlock_.getDataBitLength();
	ModSize uiNewDataUnitSize = 0;

	while (uiLocLength)
	{
		// uiLocLengthは更新されるので毎回計算する
		uiNewDataUnitSize = calcUnitSize(uiLocLength + uiDataBitLength);
		
		//
		// LOCページの確認
		//

		prepareForLocation(pLocPage_, uiDataUnitSize, uiNewDataUnitSize);
		pLocPage_->dirty();

		//
		// LocationDataの書き込み
		//
		
		if (pLocPage_->isLargeEnough(uiDataUnitSize, uiNewDataUnitSize)
			== false)
		{
			// 入りきらないので、入れられるだけ入れる

			// 最大まで拡張する
			cLocBlock_.expandUnitSize(pLocPage_->getFreeUnitSize());
			// LOCブロックのユニット数を更新
			uiDataUnitSize = cLocBlock_.getDataUnitSize();
			// 最大ビット長を計算
			ModSize maxBitLength = uiDataUnitSize * sizeof(ModUInt32) * 8;
			
			//
			// 書けるだけ書く
			// そして、書いた分だけuiLocLengthを減らす
			//
			uiLocLength += uiDataBitLength;
			uiLastLocation = writeLocationData(uiLastLocation,
											   uiCurrentLocation,
											   ite_,
											   cLocBlock_.getBuffer(),
											   uiDataBitLength,
											   maxBitLength);
			uiLocLength -= uiDataBitLength;
			
			// LOCブロックのヘッダを更新
			cLocBlock_.setDataBitLength(uiDataBitLength);
			cLocBlock_.setContinueFlag();

			// 新しいロックブロックを得る
			cLocBlock_ = allocateLocBlock(pLocPage_);
			uiDataUnitSize = cLocBlock_.getDataUnitSize();
			uiDataBitLength = cLocBlock_.getDataBitLength();
		}
		else
		{
			// そのまま入る

			// 必要な分だけ領域を確保する
			expandLocBlock(cLocBlock_, uiLocLength,
						   uiDataBitLength, uiDataUnitSize);

			// 残りをすべて書き込む
			writeLocationData(uiLastLocation, uiCurrentLocation, ite_,
							  cLocBlock_.getBuffer(), uiDataBitLength);

			// LOCブロックのヘッダを更新
			cLocBlock_.setDataBitLength(uiDataBitLength);

			break;
		}
	}
}

//
//	FUNCTION protected
//	FullText2::MiddleList::makeTmpMiddleList
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
MiddleList::makeTmpMiddleList(InvertedUnit& cInvertedUnit_,
							  LeafPage::Area* pTmpArea_)
{
	return new MiddleList(cInvertedUnit_, pTmpArea_);
}

//
//  Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
