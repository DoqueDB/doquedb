// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// MiddleList.cpp --
// 
// Copyright (c) 2002, 2003, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
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

#include "Inverted/MiddleList.h"
#include "Inverted/MiddleListIterator.h"
#include "Inverted/OverflowFile.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/InvertedIterator.h"
#include "Inverted/FakeError.h"

#include "ModInvertedLocationListIterator.h"
#include "ModInvertedSmartLocationList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//  FUNCTION public
//  Inverted::MiddleList::MiddleList -- コンストラクタ
//
//  NOTES
//  ShortListと違って、LeafPageに該当するエリアがない場合はMiddleListを
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
MiddleList::MiddleList(InvertedUnit& cInvertedUnit_, LeafPage::PagePointer pLeafPage_, LeafPage::Iterator ite_)
	: MiddleBaseList(cInvertedUnit_, pLeafPage_, ite_)
{
	m_pOverflowFile = cInvertedUnit_.getOverflowFile();
}

//
//  FUNCTION public
//  Inverted::MiddleList::~MiddleList -- デストラクタ
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
//  Inverted::MiddleList::insert -- DEBUG用
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
				   const ModInvertedSmartLocationList& cLocationList_)
{
	if (MiddleBaseList::insert(uiDocumentID_, cLocationList_) == false)
	{
		return false;
	}
	; _INVERTED_FAKE_ERROR(MiddleList::insert);
	return true;
}
#endif

//
//  FUNCTION public
//  Inverted::MiddleList::begin -- 転置リストイテレータを得る
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
MiddleList::begin() const
{
	return new MiddleListIterator(const_cast<MiddleList&>(*this));
}

//
//  FUNCTION private
//  Inverted::MiddleList::insertLocation -- 位置情報を挿入する
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
MiddleList::insertLocation(const ModInvertedSmartLocationList& cLocationList_,
						   OverflowPage::PagePointer& pLocPage_,
						   OverflowPage::LocBlock& cLocBlock_)
{
	// 位置リストのビット長
	ModSize uiBitLength;
	// 位置情報全体のビット長
	ModSize locLength = getCompressedBitLengthLocationList(cLocationList_, uiBitLength);
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
		ModAutoPointer<ModInvertedLocationListIterator> i = cLocationList_.begin();
		ModSize uiFrequency = cLocationList_.getSize();
		insertOneByOne(i, pLocPage_, cLocBlock_,
					   uiFrequency, uiBitLength, locLength);
	}
	else
	{
		// そのまま入る

		// 必要な分だけ領域を確保する
		expandLocBlock(cLocBlock_, locLength, uiDataBitLength, uiDataUnitSize);

		// すべて書き込む
		writeLocationList(cLocationList_, uiBitLength, cLocBlock_.getBuffer(), uiDataBitLength);

		// LOCブロックのヘッダを更新
		cLocBlock_.setDataBitLength(uiDataBitLength);
	}
}

//
//  FUNCTION private
//  Inverted::MiddleList::insertLocation -- 位置情報を挿入する
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
MiddleList::insertLocation(Inverted::InvertedIterator& ite_,
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
		// InvertedIteratorなので、複数LOCブロックにまたがった転置リストの可能性もある

		// 1つずつ挿入する
		ModAutoPointer<ModInvertedLocationListIterator> i = ite_.getLocationListIterator();
		ModSize uiFrequency = ite_.getInDocumentFrequency();
		ModSize uiBitLength = ite_.getLocationDataBitLength();
		insertOneByOne(i, pLocPage_, cLocBlock_,
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
//  Inverted::MiddleList::insertOneByOne -- 頻度、ビット長、位置リストを順々に挿入する
//
//  NOTES
//
//  ARGUMENTS
//  ModInvertedLocationListIterator* ite_
//	  位置情報リスト
//  Inverted::OverflowPage::PagePointer& pLocPage_
//	  ロックページ
//  Inverted::OverflowPage::LocBlock& cLocBlock_
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
MiddleList::insertOneByOne(ModInvertedLocationListIterator* ite_,
						   OverflowPage::PagePointer& pLocPage_,
						   OverflowPage::LocBlock& cLocBlock_,
						   ModSize uiFrequency_,
						   ModSize uiBitLength_,
						   ModSize uiLocLength_)
{
	if (uiFrequency_ == 1)
	{
		// 頻度が1の場合は、uiBitlengthに位置情報を設定する
		uiBitLength_ = (*ite_).getLocation();

		// 頻度と位置を書く
		uiLocLength_ -= writeFrequencyAndBitLength(pLocPage_, cLocBlock_,
												   uiFrequency_, uiBitLength_);
		(*ite_).next();
	}
	else
	{
		// 頻度とビット長を書く
		uiLocLength_ -= writeFrequencyAndBitLength(pLocPage_, cLocBlock_,
												   uiFrequency_, uiBitLength_);
		// 位置リストを挿入する
		insertLocationData(ite_, uiLocLength_, pLocPage_, cLocBlock_);

		// (*ite_).next()は不要。insertLocationData内のwriteLocationDataで実行されている。
	}

	return uiLocLength_;
}

//
//  FUNCTION private
//  Inverted::MiddleList::writeFrequencyAndBitLength -- 頻度とビット長を書く
//
//  NOTES
//
//  ARGUMENTS
//  Inverted::OverflowPage::PagePointer& pLocPage_
//	  ロックページ
//  Inverted::OverflowPage::LocBlock& cLocBlock_
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
	//  【注意】頻度とビット長(頻度が1の場合は位置情報)の間でページを跨いではいけない
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
	writeLocationFrequency(uiFrequency_, cLocBlock_.getBuffer(), uiDataBitLength);

	//
	//  ビット長(頻度が1の場合は位置情報)を書く
	//
	if (uiFrequency_ > 1)
	{
		writeLocationBitLength(uiBitLength_, cLocBlock_.getBuffer(), uiDataBitLength);
	}
	else
	{
		writeLocationData(0, uiBitLength_, cLocBlock_.getBuffer(), uiDataBitLength);
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
//  Inverted::MiddleList::insertLocationData -- 位置情報を挿入する
//
//  NOTES
//  頻度とビット長を書いた後に、この関数を呼び出す
//
//  ARGUMENTS
//  ModInvertedLocationListIterator* ite_
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
MiddleList::insertLocationData(ModInvertedLocationListIterator* ite_,
								   ModSize uiLocLength_,
								   OverflowPage::PagePointer& pLocPage_,
								   OverflowPage::LocBlock& cLocBlock_)
{
	ModSize uiLocLength = uiLocLength_;
	ModUInt32 uiLastLocation = 0;
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
		
		if (pLocPage_->isLargeEnough(uiDataUnitSize, uiNewDataUnitSize) == false)
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
			uiLastLocation = writeLocationData(uiLastLocation, ite_, cLocBlock_.getBuffer(),
											   uiDataBitLength, maxBitLength);
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
			expandLocBlock(cLocBlock_, uiLocLength, uiDataBitLength, uiDataUnitSize);

			// 残りをすべて書き込む
			writeLocationData(uiLastLocation, ite_, cLocBlock_.getBuffer(), uiDataBitLength);

			// LOCブロックのヘッダを更新
			cLocBlock_.setDataBitLength(uiDataBitLength);

			break;
		}
	}
}

//
//  Copyright (c) 2002, 2003, 2005, 2006, 2007, 2008, 2023 Ricoh Company, Ltd.
//  All rights reserved.
//
