// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortNolocationNoTFList.cpp --
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

#include "Inverted/BatchNolocationNoTFList.h"
#include "Inverted/FakeError.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/MiddleNolocationNoTFList.h"
#include "Inverted/ShortNolocationNoTFList.h"
#include "Inverted/ShortNolocationNoTFListIterator.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFList::ShortNolocationNoTFList -- コンストラクタ(1)
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
ShortNolocationNoTFList::ShortNolocationNoTFList(InvertedUnit& cInvertedUnit_,
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
//	Inverted::ShortNolocationNoTFList::ShortNolocationNoTFList -- コンストラクタ(2)
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
ShortNolocationNoTFList::ShortNolocationNoTFList(InvertedUnit& cInvertedUnit_,
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
//	Inverted::ShortNolocationNoTFList::~ShortNolocationNoTFList -- デストラクタ
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
ShortNolocationNoTFList::~ShortNolocationNoTFList()
{
}

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFList::insert -- DEBUG用
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
ShortNolocationNoTFList::insert(ModUInt32 uiDocumentID_,
								const ModInvertedSmartLocationList& cLocationList_)
{
	// 位置情報リストのオフセットは常に0。エリアが存在しない時も呼ばれる。
	; _SYDNEY_ASSERT(m_bExist == false || (*m_ite)->getLocationOffset() == 0);
	bool result = ShortBaseList::insert(uiDocumentID_, cLocationList_);
	// 位置情報リストのオフセットは常に0。
	// insertに失敗した場合、エリアが存在しないこともある。
	; _SYDNEY_ASSERT(result == false || (*m_ite)->getLocationOffset() == 0);
	
	if (result == true)
	{
		; _INVERTED_FAKE_ERROR(ShortNolocationNoTFList::insert);
	}
	
	return result;
}

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFList::insert -- DEBUG用
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
ShortNolocationNoTFList::insert(InvertedList& cInvertedList_)
{
	bool result = ShortBaseList::insert(cInvertedList_);
	
	// 位置情報リストのオフセットは常に0。
	// insertに失敗した場合、エリアが存在しないこともある。
	; _SYDNEY_ASSERT(result == false || (*m_ite)->getLocationOffset() == 0);
	
	return result;
}

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFList::verify -- DEBUG用
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTINS
//
void
ShortNolocationNoTFList::verify(Admin::Verification::Treatment::Value uiTreatment_,
								Admin::Verification::Progress& cProgress_,
								const Os::Path& cRootPath_)
{
	// エリアが存在する時しか呼ばれない。
	// 転置リストは、setExist(false)を実行されないので、
	// ListManager::verify内のnextで得られたものは常に存在する。
	; _SYDNEY_ASSERT(m_bExist == true);
	
	// 位置情報リストのオフセットは常に0。
	; _SYDNEY_ASSERT((*m_ite)->getLocationOffset() == 0);

	ShortBaseList::verify(uiTreatment_, cProgress_, cRootPath_);
}
#endif

//
//	FUNCTION public
//	Inverted::ShortNolocationNoTFList::begin -- 転置のイテレータを得る
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
ShortNolocationNoTFList::begin() const
{
	return new ShortNolocationNoTFListIterator(const_cast<ShortNolocationNoTFList&>(*this));
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
ShortNolocationNoTFList::getCompressedBitLengthLocationList(
	const ModInvertedSmartLocationList& cLocationList_,
	ModSize& uiBitLength_)
{
	// 位置情報リストのオフセットは常に0。エリアが存在しない時も呼ばれる。
	; _SYDNEY_ASSERT(m_bExist == false || (*m_ite)->getLocationOffset() == 0);
	
	uiBitLength_ = 0;
	return 0;
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
ShortNolocationNoTFList::writeLocationList(const ModInvertedSmartLocationList& cLocationList_,
										   ModSize uiBitLength_,
										   ModUInt32* pHeadAddress_, ModSize& uiBitOffset_)
{
	; _SYDNEY_ASSERT(uiBitLength_ == 0);
	; _SYDNEY_ASSERT(uiBitOffset_ == 0);

	// エリアが存在する時しか呼ばれない。
	; _SYDNEY_ASSERT(m_bExist == true);
	// 位置情報リストのオフセットは常に0。
	; _SYDNEY_ASSERT((*m_ite)->getLocationOffset() == 0);
}

#ifdef DEBUG
//
//	FUNCTION private
//	Inverted::ShortNolocationNoTFList::insertOrExpandArea -- DEBUG用
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
ShortNolocationNoTFList::insertOrExpandArea(ModUInt32 uiDocumentID_,
											const ModInvertedSmartLocationList& cLocationList_,
											ModSize uiLocBitLength_)
{
	; _SYDNEY_ASSERT(uiLocBitLength_ == 0);
	
	// 位置情報リストのオフセットは常に0。エリアが存在しない時も呼ばれる。
	; _SYDNEY_ASSERT(m_bExist == false || (*m_ite)->getLocationOffset() == 0);
	bool result = ShortBaseList::insertOrExpandArea(uiDocumentID_, cLocationList_, uiLocBitLength_);
	// 位置情報リストのオフセットは常に0。
	// insertOrExpandAreaに失敗した場合、エリアが存在しないこともある。
	; _SYDNEY_ASSERT(result == false || (*m_ite)->getLocationOffset() == 0);
	
	return result;
}

//
//	FUNCTION private
//	Inverted::ShortNolocationNoTFList::expandArea -- DEBUG用
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
ShortNolocationNoTFList::expandArea(ModSize uiExpandUnit_)
{
	// エリアが存在する時しか呼ばれない。
	; _SYDNEY_ASSERT(m_bExist == true);
	
	// 位置情報リストのオフセットは常に0。
	; _SYDNEY_ASSERT((*m_ite)->getLocationOffset() == 0);
	bool result = ShortBaseList::expandArea(uiExpandUnit_);
	; _SYDNEY_ASSERT((*m_ite)->getLocationOffset() == 0);
	
	return result;
}
#endif

//
//	FUNCTION private
//	Inverted::ShortNolocationNoTFList::makeMiddleList -- 転置リストを作成する
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
ShortNolocationNoTFList::makeMiddleList()
{
	//
	// 該当するエリアが存在する時に使われる関数なので、引数にキーは不要。
	//
	
	return new MiddleNolocationNoTFList(getInvertedUnit(), m_pLeafPage, m_ite);
}

//
//	FUNCTION private
//	Inverted::ShortNolocationNoTFList::makeBatchList -- 転置リストを作成する
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
ShortNolocationNoTFList::makeBatchList(LeafPage::Area* pArea_)
{
	//
	// 挿入先のエリアが存在する時に使われるので、引数にキーは不要。
	//
	
	return new BatchNolocationNoTFList(getInvertedUnit(), pArea_);
}

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
