// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ShortList.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#include "FullText2/ShortList.h"
#include "FullText2/InvertedUnit.h"
#include "FullText2/ShortListIterator.h"
#include "FullText2/LeafPage.h"
#include "FullText2/MiddleList.h"
#include "FullText2/BatchList.h"
#include "FullText2/FakeError.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::ShortList::ShortList -- コンストラクタ(1)
//
//	NOTES
//	該当する索引単位のエリアがある場合
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
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
	: ShortBaseList(cInvertedUnit_, pLeafPage_, ite_)
{
}

//
//	FUNCTION public
//	FullText2::ShortList::ShortList -- コンストラクタ(2)
//
//	NOTES
//	該当する索引単位のエリアが存在しない場合
//
//	ARGUMENTS
//	FullText2::InvertedUnit& cInvertedUnit_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
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
	: ShortBaseList(cInvertedUnit_, pszKey_, pLeafPage_, ite_)
{
}

//
//	FUNCTION public
//	FullText2::ShortList::~ShortList -- デストラクタ
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

//
//	FUNCTION public
//	FullText2::ShortList::insert -- 転置リストの挿入(1文書単位)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		挿入する文書の文書ID
//	const SmartLocationList& cLocationList_
//		位置情報配列
//
//	RETURN
//	bool
//		挿入できた場合はtrue(ShortListの範囲内であった)、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ShortList::insert(ModUInt32 uiDocumentID_,
				  const SmartLocationList& cLocationList_)
{
	// 位置情報のビット長を求める
	ModSize bitLength;
	ModSize uiLocBitLength
		= getCompressedBitLengthLocationList(cLocationList_, bitLength);

	//
	//	1文書の挿入(小転置への挿入)では、挿入のたびにページ間の再構成は行わない
	//	そのページに入らなかった場合のみ、ページ分割(1->2 or 1->3)が発生する
	//
	if (insertOrExpandArea(uiDocumentID_, uiLocBitLength) == false)
		// ショートリストの範囲外
		return false;

	// 途中でエラーになるかもしれないのでここでdirtyにする
	m_pLeafPage->dirty();

	// 文書IDを書く
	if ((*m_ite)->getLastDocumentID() == 0)
	{
		// 最終文書IDがない -> 初めて挿入される
		(*m_ite)->setFirstDocumentID(uiDocumentID_);
	}
	else
	{
		// 文書IDビットオフセット
		ModSize offset = (*m_ite)->getDocumentOffset();
		// 圧縮して格納する
		writeDocumentID((*m_ite)->getLastDocumentID(), uiDocumentID_,
						(*m_ite)->getTailAddress(), offset);
		// 文書IDビットオフセットを設定する
		(*m_ite)->setDocumentOffset(offset);
	}

	// 位置情報を書く
	{
		// 位置情報ビットオフセット
		ModSize offset = (*m_ite)->getLocationOffset();
		// 圧縮して格納する
		writeLocationList(cLocationList_, bitLength,
						  (*m_ite)->getHeadAddress(), offset);
		// 位置情報ビットオフセットを設定する
		(*m_ite)->setLocationOffset(offset);
	}

	// 最終文書IDを設定する
	(*m_ite)->setLastDocumentID(uiDocumentID_);
	// 頻度情報を設定する
	(*m_ite)->incrementDocumentCount();

	; _FULLTEXT2_FAKE_ERROR(ShortList::insert);

	return true;
}

//
//	FUNCTION public
//	FullText2::ShortList::getIterator -- 転置のイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::InvertedIterator*
//		転置のイテレータを得る
//
//	EXCEPTIONS
//
InvertedIterator*
ShortList::getIterator()
{
	return new ShortListIterator(*this);
}

//
//	FUNCTION protected
//	FullText2::ShortList::makeMiddleList -- ミドルリストを作成する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	FullText2::MiddleBaseList*
//		ミドルリスト
//
//	EXCEPTIONS
//
MiddleBaseList*
ShortList::makeMiddleList(InvertedUnit& cInvertedUnit_,
						  LeafPage::PagePointer pLeafPage_,
						  LeafPage::Iterator ite_)
{
	return new MiddleList(cInvertedUnit_, pLeafPage_, ite_);
}

//
//	FUNCTION protected
//	FullText2::ShortList::makeBatchList -- バッチリストを作成する
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	FullText2::BatchBaseList*
//		バッチリスト
//
//	EXCEPTIONS
//
BatchBaseList*
ShortList::makeBatchList(InvertedUnit& cInvertedUnit_,
						 LeafPage::Area* pArea_)
{
	return new BatchList(cInvertedUnit_, pArea_);
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
