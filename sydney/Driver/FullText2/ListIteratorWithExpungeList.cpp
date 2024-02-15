// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListIteratorWithExpungeList.cpp --
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
#include "FullText2/ListIteratorWithExpungeList.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ListIteratorWithExpungeList::ListIteratorWithExpungeList
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListIterator* pListIterator_
//		イテレータ
//	const Common::LargeVector<DocumentID>& vecExpungeList_
//		削除文書リスト
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListIteratorWithExpungeList::ListIteratorWithExpungeList(
	ListIterator* pListIterator_,
	const Common::LargeVector<DocumentID>& vecExpungeList_)
	: ListIterator(),
	  m_pListIterator(pListIterator_),
	  m_vecExpungeList(vecExpungeList_),
	  m_uiCurrentID(0)
{
	//【注意】
	//	削除文書リストは文書IDの昇順にソートされ、
	//	最後の要素は必ず UndefinedDocumentID である必要がある
	
	m_ite = m_vecExpungeList.begin();
}

//
//	FUNCTION public
//	FullText2::ListIteratorWithExpungeList::~ListIteratorWithExpungeList
//		-- デストラクタ
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
ListIteratorWithExpungeList::~ListIteratorWithExpungeList()
{
	// フリーリストを解放する
	clearFree();
	
	delete m_pListIterator;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithExpungeList::find -- 文書IDを検索する
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
ListIteratorWithExpungeList::find(SearchInformation& cSearchInfo_,
								  DocumentID uiDocumentID_)
{
	//【注意】
	//	find では削除文書リストにあるかどうかチェックしない
	//	find はDUAL索引の単語境界の検索時にしか実行されないが、
	//	(その他の場合はエラーになる)
	//	単語境界の検索は、検索語がヒットした場合のみに実行される
	//	削除文書リストにあれば検索語がヒットするわけないので、
	//	ここでチェックする必要はない
	
	return m_pListIterator->find(cSearchInfo_, uiDocumentID_);
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithExpungeList::lowerBound
//		-- 文書IDをlower_bound検索する
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
ListIteratorWithExpungeList::lowerBound(SearchInformation& cSearchInfo_,
										DocumentID uiDocumentID_)
{
	if (uiDocumentID_ < m_uiCurrentID)
	{
		// 現在値よりも小さいのでリセット
		m_uiCurrentID = 0;
	}
	
	DocumentID id = m_pListIterator->lowerBound(cSearchInfo_, uiDocumentID_);
	if (id != UndefinedDocumentID)
	{
		if (m_uiCurrentID == 0)
		{
			// 削除リストも検索する
			m_ite = ModLowerBound(m_vecExpungeList.begin(),
								  m_vecExpungeList.end(),
								  id, ModLess<DocumentID>());
		}
		else
		{
			// 一度 lowerBound しているので、シーケンシャルに探す
			while ((*m_ite) < id) ++m_ite;
		}
		
		if ((*m_ite) == id)
		{
			// 削除リストにあるものと同じなので、次に進める
			return next(cSearchInfo_);
		}
	}

	m_uiCurrentID = id;
	
	return id;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithExpungeList::reset -- カーソルを先頭に戻す
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
ListIteratorWithExpungeList::reset()
{
	m_pListIterator->reset();
	m_ite = m_vecExpungeList.begin();
	m_uiCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithExpungeList::next -- 次の値を得る
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
ListIteratorWithExpungeList::next(SearchInformation& cSearchInfo_)
{
	if (m_uiCurrentID != UndefinedDocumentID)
	{
		DocumentID id;
		do
		{
			id = m_pListIterator->next(cSearchInfo_);
			if (id == UndefinedDocumentID)
				break;
		
			while ((*m_ite) < id) ++m_ite;
		}
		while (id == (*m_ite));

		m_uiCurrentID = id;
	}

	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithExpungeList::copy -- コピーする
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
ListIteratorWithExpungeList::copy() const
{
	return new ListIteratorWithExpungeList(m_pListIterator->copy(),
										   m_vecExpungeList);
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
