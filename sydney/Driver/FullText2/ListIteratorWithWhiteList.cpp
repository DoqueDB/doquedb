// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListIteratorWithWhiteList.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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
#include "FullText2/ListIteratorWithWhiteList.h"

#include "ModAlgorithm.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ListIteratorWithWhiteList::ListIteratorWithWhiteList
//		-- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ListIterator* pListIterator_
//		イテレータ
//	const Common::BitSet& cWhiteList_
//		集合
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListIteratorWithWhiteList::
ListIteratorWithWhiteList(ListIterator* pListIterator_,
						  const Common::BitSet& cWhiteList_)
	: ListIterator(),
	  m_pListIterator(pListIterator_),
	  m_cWhiteList(cWhiteList_),
	  m_uiCurrentID(0)
{
	// 先頭要素を割り当てる
	m_ite = m_cWhiteList.begin();
}

//
//	FUNCTION public
//	FullText2::ListIteratorWithWhiteList::~ListIteratorWithWhiteList
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
ListIteratorWithWhiteList::~ListIteratorWithWhiteList()
{
	// フリーリストを解放する
	clearFree();
	
	delete m_pListIterator;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithWhiteList::find -- 文書IDを検索する
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
ListIteratorWithWhiteList::find(SearchInformation& cSearchInfo_,
								DocumentID uiDocumentID_)
{
	//【注意】
	//	find では集合リストにあるかどうかチェックしない
	//	find はDUAL索引の単語境界の検索時にしか実行されないが、
	//	(その他の場合はエラーになる)
	//	単語境界の検索は、検索語がヒットした場合のみに実行される
	//	削除文書リストにあれば検索語がヒットするわけないので、
	//	ここでチェックする必要はない
	
	return m_pListIterator->find(cSearchInfo_, uiDocumentID_);
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithWhiteList::lowerBound
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
ListIteratorWithWhiteList::lowerBound(SearchInformation& cSearchInfo_,
									  DocumentID uiDocumentID_)
{
	//【注意】
	//	m_cWhiteList の最後の要素には、UndefinedDocumentIDがあるというのが前提
	
	if (uiDocumentID_ < m_uiCurrentID)
	{
		// 現在値よりも小さいのでリセット
		m_uiCurrentID = 0;
	}

	DocumentID id = uiDocumentID_;
	while (id != UndefinedDocumentID)
	{
		// まず転置リストイテレータを検索する
		id = m_pListIterator->lowerBound(cSearchInfo_, id);
		if (id == UndefinedDocumentID)
			// 見つからないので終了
			break;
		
		// 次に集合を検索する
		if (m_uiCurrentID == 0)
		{
			// 初めてなので、集合もlowerBoundする
			m_ite = m_cWhiteList.lowerBound(id);
		}
		else
		{
			// 一度 lowerBound しているので、シーケンシャルに探す
			while ((*m_ite) < id) ++m_ite;
		}

		if ((*m_ite) > id)
		{
			// 集合の方が先に進んでしまったので、
			// もう一度、転置リストイテレータを検索する
			
			id = *m_ite;
			m_uiCurrentID = id;	// 次からシーケンシャルに探させるため

			continue;
		}

		break;
	}

	m_uiCurrentID = id;
	
	return id;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithWhiteList::reset -- カーソルを先頭に戻す
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
ListIteratorWithWhiteList::reset()
{
	m_pListIterator->reset();
	m_ite = m_cWhiteList.begin();
	m_uiCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithWhiteList::next -- 次の値を得る
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
ListIteratorWithWhiteList::next(SearchInformation& cSearchInfo_)
{
	//【注意】
	//	m_cWhiteList の最後の要素には、UndefinedDocumentIDがあるというのが前提
	
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
		while (id != (*m_ite));

		m_uiCurrentID = id;
	}

	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::ListIteatorWithWhiteList::copy -- コピーする
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
ListIteratorWithWhiteList::copy() const
{
	return new ListIteratorWithWhiteList(m_pListIterator->copy(),
										 m_cWhiteList);
}

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
