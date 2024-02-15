// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListIteratorWithMax.cpp --
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
#include "FullText2/ListIteratorWithMax.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ListIteratorWithMax::ListIteratorWithMax -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListIterator* pListIterator_
//		イテレータ
//	FullText2::DocumentID uiMaxDocumentID_
//		最大文書ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListIteratorWithMax::ListIteratorWithMax(ListIterator* pListIterator_,
										 DocumentID uiMaxDocumentID_)
	: ListIterator(),
	  m_pListIterator(pListIterator_), m_uiMaxDocumentID(uiMaxDocumentID_),
	  m_uiCurrentDocumentID(0)
{
}

//
//	FUNCTION public
//	FullText2::ListIteratorWithMax::~ListIteratorWithMax -- デストラクタ
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
ListIteratorWithMax::~ListIteratorWithMax()
{
	// フリーリストを解放する
	clearFree();
	
	delete m_pListIterator;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithMax::find -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	bool
//		指定された文書IDが存在した場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
ListIteratorWithMax::find(SearchInformation& cSearchInfo_,
						  DocumentID uiDocumentID_)
{
	if (uiDocumentID_ > m_uiMaxDocumentID)
		return false;
	
	m_uiCurrentDocumentID = m_pListIterator->find(cSearchInfo_, uiDocumentID_);
	return m_uiCurrentDocumentID;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithMax::lowerBound -- 文書IDをlower_bound検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID uiDocumentID_
//		文書ID
//
//	RETURN
//	FullText2::DocumentID
//		ヒットした文書ID
//
//	EXCEPTIONS
//
DocumentID
ListIteratorWithMax::lowerBound(SearchInformation& cSearchInfo_,
								DocumentID uiDocumentID_)
{
	if (uiDocumentID_ > m_uiMaxDocumentID)
		return UndefinedDocumentID;
	
	m_uiCurrentDocumentID = m_pListIterator->lowerBound(cSearchInfo_,
														uiDocumentID_);
	if (m_uiCurrentDocumentID > m_uiMaxDocumentID)
	{
		m_uiCurrentDocumentID = UndefinedDocumentID;
	}

	return m_uiCurrentDocumentID;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithMax::reset -- カーソルを先頭に戻す
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
ListIteratorWithMax::reset()
{
	m_uiCurrentDocumentID = 0;
	m_pListIterator->reset();
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithMax::next -- 次の値を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	FullText2::DocumentID
//		次の文書ID
//
//	EXCEPTIONS
//
DocumentID
ListIteratorWithMax::next(SearchInformation& cSearchInfo_)
{
	if (m_uiCurrentDocumentID > m_uiMaxDocumentID)
		return UndefinedDocumentID;
	
	m_uiCurrentDocumentID = m_pListIterator->next(cSearchInfo_);
	if (m_uiCurrentDocumentID > m_uiMaxDocumentID)
		m_uiCurrentDocumentID = UndefinedDocumentID;

	return m_uiCurrentDocumentID;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithMax::copy -- コピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ListIterator*
//		コピー
//
//	EXCEPTIONS
//
ListIterator*
ListIteratorWithMax::copy() const
{
	return new ListIteratorWithMax(m_pListIterator->copy(), m_uiMaxDocumentID);
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
