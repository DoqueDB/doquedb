// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DelayListIterator.cpp --
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
#include "FullText2/DelayListIterator.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::DelayListIterator::DelayListIterator -- コンストラクタ
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
DelayListIterator::DelayListIterator()
	: MultiListIterator()
{
}

//
//	FUNCTION public
//	FullText2::DelayListIterator::~DelayListIterator -- デストラクタ
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
DelayListIterator::~DelayListIterator()
{
}

//
//	FUNCTION public
//	FullText2::DelayListIterator::DelayListIterator -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::DelayListIterator& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
DelayListIterator::DelayListIterator(const DelayListIterator& src_)
	: MultiListIterator(src_)
{
	m_vecMaxDocumentID = src_.m_vecMaxDocumentID;
}

//
//	FUNCTION public
//	FullText2::DelayListIterator::reserve -- 配列の領域を確保する
//
//	NOTES
//
//	ARGUMENTS
//	ModSize size
//		確保する要素数
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayListIterator::reserve(ModSize size_)
{
	MultiListIterator::reserve(size_);
	m_vecMaxDocumentID.reserve(size_);
}

//
//	FUNCTION public
//	FullText2::DelayListIterator::pushBack -- 要素を追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListIterator* iterator
//		追加する要素
//	FullText2::DocumentID uiMaxDocumentID
//		追加するLiteIteratorの最大文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
DelayListIterator::pushBack(ListIterator* iterator, DocumentID uiMaxDocumentID_)
{
	MultiListIterator::pushBack(iterator);
	m_vecMaxDocumentID.pushBack(uiMaxDocumentID_);
}

//
//	FUNCTION public
//	FullText2::DelayListIterator::getElement
//		-- ある文書IDの文書が格納されている要素の要素番号を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID uiDocumentID_
//		探している文書の文書ID
//
// 	RETURN
//	int
//		要素番号。見つからない場合は -1 を返す
//
//	EXCEPTIONS
//
int
DelayListIterator::getElement(SearchInformation& cSearchInfo_,
							  DocumentID uiDocumentID_)
{
	ModVector<DocumentID>::Iterator b = m_vecMaxDocumentID.begin();
	ModVector<DocumentID>::Iterator e = m_vecMaxDocumentID.end();
	ModVector<DocumentID>::Iterator i
		= ModLowerBound(b, e, uiDocumentID_, ModLess<DocumentID>());
	if (i == e)
		return -1;
	return (i - b);
}

//
//	FUNCTION public
//	FullText2::DelayListIterator::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::ListIterator*
//		コピーしたリストイテレータ
//
//	EXCEPTIONS
//
ListIterator*
DelayListIterator::copy() const
{
	return new DelayListIterator(*this);
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
