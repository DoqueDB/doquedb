// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InvertedList.cpp --
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
#include "FullText2/InvertedList.h"
#include "FullText2/InvertedUpdateFile.h"
#include "FullText2/InvertedIterator.h"

#include "Common/Assert.h"

#include "Exception/BadArgument.h"

#include "ModUnicodeString.h"
#include "ModAutoPointer.h"

#include "ModInvertedCoder.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::InvertedList::InvertedList -- コンストラクタ(1)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
///	ModSize uiKeyLength_
//		索引単位文字数
//	ModUInt32 uiListType_
//		リスト種別
//	FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
//		リーフページのエリアへのイテレータ
//	
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUpdateFile& cInvertedFile_,
						   const ModUnicodeChar* pszKey_,
						   ModSize uiKeyLength_,
						   ModUInt32 uiListType_,
						   LeafPage::PagePointer pLeafPage_,
						   LeafPage::Iterator ite_)
	: InvertedListCore(cInvertedFile_, pszKey_, uiKeyLength_),
	  m_uiListType(uiListType_),
	  m_pLeafPage(pLeafPage_),
	  m_ite(ite_)
{
}

//
//	FUNCTION public
//	FullText2::InvertedList::InvertedList -- コンストラクタ(2)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	ModSize uiListType_
//		リスト種別
//	FullText2::LeafPage::PagePointer pLeafPage_
//		リーフページ
//	FullText2::LeafPage::Iterator ite_
//		リーフページのエリアへのイテレータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUpdateFile& cInvertedFile_,
						   const ModUnicodeChar* pszKey_,
						   ModUInt32 uiListType_,
						   LeafPage::PagePointer pLeafPage_,
						   LeafPage::Iterator ite_)
	: InvertedListCore(cInvertedFile_, pszKey_),
	  m_uiListType(uiListType_),
	  m_pLeafPage(pLeafPage_),
	  m_ite(ite_)
{
}

//
//	FUNCTION public
//	FullText2::InvertedList::InvertedList -- コンストラクタ(3)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
///	ModSize uiKeyLength_
//		索引単位文字数
//	ModSize uiListType_
//		リスト種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUpdateFile& cInvertedFile_,
						   const ModUnicodeChar* pszKey_,
						   ModSize uiKeyLength_,
						   ModUInt32 uiListType_)
	: InvertedListCore(cInvertedFile_, pszKey_, uiKeyLength_),
	  m_uiListType(uiListType_)
{
}

//
//	FUNCTION public
//	FullText2::InvertedList::InvertedList -- コンストラクタ(4)
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::InvertedUpdateFile& cInvertedFile_
//		転置ファイル
//	const ModUnicodeChar* pszKey_
//		索引単位
//	ModSize uiListType_
//		リスト種別
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
InvertedList::InvertedList(InvertedUpdateFile& cInvertedFile_,
						   const ModUnicodeChar* pszKey_,
						   ModUInt32 uiListType_)
	: InvertedListCore(cInvertedFile_, pszKey_),
	  m_uiListType(uiListType_)
{
}

//
//	FUNCTION public
//	FullText2::InvertedList::~InvertedList -- デストラクタ
//
//	NOTES
//	デストラクタ。
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
InvertedList::~InvertedList()
{
}

//
//	FUNCTION public
//	FullText2::InvertedList::insert -- 転置リストの挿入(転置リスト単位)
//
//	NOTES
//	転置リスト単位での挿入をサポートする場合には、上書きする
//
//	ARGUMENTS
//	const FullText2::InvertedList& cInvertedList_
//		挿入する転置リスト
//
//	RETURN
//	bool
//		挿入できた場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//	常に Exception::BadArgument を投げる
//
bool
InvertedList::insert(InvertedList& cInvertedList_)
{
	// 転置リスト単位の挿入をサポートする場合、上書きする
	_SYDNEY_THROW0(Exception::BadArgument);
}

//
//	FUNCTION public
//	FullText2::InvertedList::expunge -- 転置リストからの削除(1文書)
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		削除する文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::expunge(ModUInt32 uiDocumentID_)
{
	ModAutoPointer<InvertedIterator> i = getIterator();

	// 削除する文書を検索する
	if ((*i).find(uiDocumentID_, false) == true)
	{
		// まずdirtyにする。削除の途中で例外が発生するかもしれないから
		if (m_pLeafPage) m_pLeafPage->dirty();

		// 削除する
		(*i).expunge();
	}
}

//
//	FUNCTION public
//	FullText2::InvertedList::expunge -- 転置リストからの削除(転置リスト単位)
//
//	NOTES
//	マージ時に転置リスト単位の削除を行うためのメソッド
//
//	ARGUMENTS
//	const FullText2::InvertedList& cInvertedList_
//		削除する転置リスト
//
//	RETURN
//	int
//		削除数
//
//	EXCEPTIONS
//
int
InvertedList::expunge(InvertedList& cInvertedList_)
{
	int n = 0;
	ModAutoPointer<InvertedIterator> src = cInvertedList_.getIterator();
	ModAutoPointer<InvertedIterator> dst = getIterator();
	DocumentID id;
	while ((id = (*src).next()) != UndefinedDocumentID)
	{
		int iUnitNumber = -1;
		
		// 大転置の文書IDに変換する

		if ((id = cInvertedList_.convertToBigDocumentID(id, iUnitNumber))
			== UndefinedDocumentID)
			// 見つからない
			continue;

		// 現在のユニットかどうか
		
		if (iUnitNumber != -1 && m_cInvertedFile.getUnitNumber() != iUnitNumber)
			continue;
		
		// この索引単位に挿入されていたら、削除する
		
		if ((*dst).find(id, false))
		{
			// まずdirtyにする。削除の途中で例外が発生するかもしれないから
			if (m_pLeafPage) m_pLeafPage->dirty();

			// 削除する
			(*dst).expunge();
			++n;
		}
	}

	return n;
}

//
//	FUNCTION public
//	FullText2::InvertedList::undoExpunge -- 削除の取り消しを行う(1文書単位)
//
//	NOTES
//	削除中にエラーが発生した場合に、途中の削除を取り消すために呼び出す
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		挿入する文書ID
//	const FullText2::SmartLocationList& cLocationList_
//		挿入する位置情報
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
InvertedList::undoExpunge(ModUInt32 uiDocumentID_,
						  const SmartLocationList& cLocationList_)
{
	ModAutoPointer<InvertedIterator> i = getIterator();

	// lower_boundで検索する
	(*i).lowerBound(uiDocumentID_, true);

	// 削除を取り消す
	(*i).undoExpunge(uiDocumentID_, cLocationList_);

	if (m_pLeafPage) m_pLeafPage->dirty();
}

//
//	FUNCTION public
//	FullText2::InvertedList::convertBigDocumentID
//		-- 削除用小転置の文書IDを大転置の文書IDに変換する
//
//	NOTES
//
//	ARGUMENTS
//	ModUInt32 uiDocumentID_
//		削除用小転置の文書ID
//	int& iUnitNumber_
//		ユニット番号
//
//	RETURN
//	ModUInt32
//		大転置の文書ID
//
//	EXCEPTIONS
//
ModUInt32
InvertedList::convertToBigDocumentID(ModUInt32 uiDocumentID_,
									 int& iUnitNumber_)
{
	return m_cInvertedFile.convertToBigDocumentID(uiDocumentID_, iUnitNumber_);
}

//
//	FUNCTION public
//	FullText2::InvertedList::vacuum
//		-- バキュームする
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
//
void
InvertedList::vacuum()
{
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
