// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationList.cpp --
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

#include "Inverted/BatchNolocationList.h"
#include "Inverted/FakeError.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/MiddleNolocationList.h"
#include "Inverted/ShortNolocationList.h"
#include "Inverted/ShortNolocationListIterator.h"

#include "Common/Assert.h"

#include "ModInvertedSmartLocationList.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortNolocationList::ShortNolocationList -- コンストラクタ(1)
//
//	NOTES
//	該当する索引単位のエリアがある場合
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	Inverted::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	Inverted::LeafPage::Iterator ite_
//		該当する索引単位のエリアへのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
ShortNolocationList::ShortNolocationList(InvertedUnit& cInvertedUnit_,
					 LeafPage::PagePointer pLeafPage_,
					 LeafPage::Iterator ite_)
	: ShortBaseList(cInvertedUnit_,
				   pLeafPage_,
				   ite_),
	  m_bExist(true)
{
}

//
//	FUNCTION public
//	Inverted::ShortNolocationList::ShortNolocationList -- コンストラクタ(2)
//
//	NOTES
//	該当する索引単位のエリアが存在しない場合
//
//	ARGUMENTS
//	Inverted::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	Inverted::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	Inverted::LeafPage::Iterator ite_
//		lower_boundで検索したエリアへのイテレータ(挿入位置)
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ShortNolocationList::ShortNolocationList(InvertedUnit& cInvertedUnit_,
					 const ModUnicodeChar* pszKey_,
					 LeafPage::PagePointer pLeafPage_,
					 LeafPage::Iterator ite_)
	: ShortBaseList(cInvertedUnit_,
				   pszKey_,
				   pLeafPage_,
				   ite_),
	  m_bExist(false)
{
}

//
//	FUNCTION public
//	Inverted::ShortNolocationList::~ShortNolocationList -- デストラクタ
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
ShortNolocationList::~ShortNolocationList()
{
}

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::ShortNolocationList::insert -- DEBUG用
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
ShortNolocationList::insert(ModUInt32 uiDocumentID_,
							const ModInvertedSmartLocationList& cLocationList_)
{
	bool result = ShortBaseList::insert(uiDocumentID_, cLocationList_);
	if (result == true)
	{
		; _INVERTED_FAKE_ERROR(ShortNolocationList::insert);
	}
	return result;
}
#endif

//
//	FUNCTION public
//	Inverted::ShortNolocationList::begin -- 転置のイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Inverted::InvertedIterator*
//		転置のイテレータを得る
//
//	EXCEPTIONS
//
InvertedIterator*
ShortNolocationList::begin() const
{
	return new ShortNolocationListIterator(const_cast<ShortNolocationList&>(*this));
}

//
//	FUNCTION public
//	Inverted::ShortNolocationList::getCompressedBitLengthLocationList -- 位置情報リストの圧縮ビット長を得る
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
ShortNolocationList::getCompressedBitLengthLocationList(const ModInvertedSmartLocationList& cLocationList_,
														ModSize& uiBitLength_)
{
	uiBitLength_ = 0;
	// 頻度情報
	return getCompressedBitLengthFrequency(cLocationList_.getSize());
}

//
//	FUNCTION public
//	Inverted::ShortNolocationList::writeLocationList -- 位置情報リストを書き込む
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
ShortNolocationList::writeLocationList(const ModInvertedSmartLocationList& cLocationList_,
									   ModSize uiBitLength_,
									   ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
	; _SYDNEY_ASSERT(uiBitLength_ == 0);

	// 頻度
	writeLocationFrequency(cLocationList_.getSize(), pHeadAddress_, uiBitOffset_);
}

//
//	FUNCTION private
//	Inverted::ShortNolocationList::makeMiddleList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	Inverted::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
ShortNolocationList::makeMiddleList()
{
	//
	// 該当するエリアが存在する時に使われる関数なので、引数にキーは不要。
	//
	
	return new MiddleNolocationList(getInvertedUnit(), m_pLeafPage, m_ite);
}

//
//	FUNCTION private
//	Inverted::ShortNolocationList::makeBatchList -- 転置リストを作成する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	Inverted::InvertedList*
//		転置リスト
//
//	EXCEPTIONS
//
InvertedList*
ShortNolocationList::makeBatchList(LeafPage::Area* pArea_)
{
	//
	// 挿入先のエリアが存在する時に使われるので、引数にキーは不要。
	//
	
	return new BatchNolocationList(getInvertedUnit(), pArea_);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
