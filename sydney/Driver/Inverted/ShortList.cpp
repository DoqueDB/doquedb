// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortList.cpp --
// 
// Copyright (c) 2002, 2003, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#include "Inverted/ShortList.h"
#include "Inverted/InvertedUnit.h"
#include "Inverted/ShortListIterator.h"
#include "Inverted/MiddleList.h"
#include "Inverted/BatchList.h"
#include "Inverted/FakeError.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public
//	Inverted::ShortList::ShortList -- コンストラクタ(1)
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
ShortList::ShortList(InvertedUnit& cInvertedUnit_,
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
//	Inverted::ShortList::ShortList -- コンストラクタ(2)
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
ShortList::ShortList(InvertedUnit& cInvertedUnit_,
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
//	Inverted::ShortList::~ShortList -- デストラクタ
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
ShortList::~ShortList()
{
}

#ifdef DEBUG
//
//	FUNCTION public
//	Inverted::ShortList::insert -- DEBUG用
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
ShortList::insert(ModUInt32 uiDocumentID_,
			  const ModInvertedSmartLocationList& cLocationList_)
{
	bool result = ShortBaseList::insert(uiDocumentID_, cLocationList_);
	if (result == true)
	{
		; _INVERTED_FAKE_ERROR(ShortList::insert);
	}
	return result;
}
#endif

//
//	FUNCTION public
//	Inverted::ShortList::begin -- 転置のイテレータを得る
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
ShortList::begin() const
{
	return new ShortListIterator(const_cast<ShortList&>(*this));
}

//
//	FUNCTION private
//	Inverted::ShortList::makeMiddleList -- 転置リストを作成する
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
ShortList::makeMiddleList()
{
	//
	// 該当するエリアが存在する時に使われる関数なので、引数にキーは不要。
	//
	
	return new MiddleList(getInvertedUnit(), m_pLeafPage, m_ite);
}

//
//	FUNCTION private
//	Inverted::ShortList::makeBatchList -- 転置リストを作成する
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
ShortList::makeBatchList(LeafPage::Area* pArea_)
{
	//
	// 挿入先のエリアが存在する時に使われるので、引数にキーは不要。
	//
	
	return new BatchList(getInvertedUnit(), pArea_);
}

//
//	Copyright (c) 2002, 2003, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
