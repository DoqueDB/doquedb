// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Executor.cpp --
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
#include "FullText2/Executor.h"

#include "FullText2/OperatorNode.h"
#include "FullText2/SearchInformation.h"

#include "Common/Assert.h"

#include "Os/Limits.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

//
//	FUNCTION public
//	FullText2::Executor::Executor -- コンストラクタ
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
Executor::Executor()
	: m_pSearchInfo(0), m_bSearchInfoOwner(false),
	  m_pNode(0), m_bNodeOwner(false), m_bScore(false),
	  m_uiLower(1), m_uiUpper(Os::Limits<ModSize>::getMax()),
	  m_bNarrowing(false), m_uiCurrentID(0),
	  m_eKey(SortKey::DocID), m_eOrder(Order::Asc)
{
}

//
//	FUNCTION public
//	FullText2::Executor::~Executor -- デストラクタ
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
Executor::~Executor()
{
	if (m_bSearchInfoOwner) delete m_pSearchInfo;
	if (m_bNodeOwner) delete m_pNode;
}

//
//	FUNCTION public
//	FullText2::Executor::getResultSet -- 検索結果を取得する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::SimpleResultSet&
//		検索結果集合
//
//	EXCEPTIONS
//
SimpleResultSet&
Executor::getResultSet()
{
	// 検索する
	search(false);
	return m_cResultSet;
}

//
//	FUNCTION public
//	FullText2::Executor::getCount -- 検索結果件数を得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		検索結果件数
//
//	EXCEPTIONS
//
ModSize
Executor::getCount()
{
	return search(true);
}

//
//	FUNCTION private
//	FullText2::Executor::search -- 検索を実行する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	ModSize
//		検索結果件数
//
//	EXCEPTIONS
//
ModSize
Executor::search(bool bOnlyCount_)
{
	; _TRMEISTER_ASSERT(m_pNode);
	
	// リセットする
	m_pNode->reset();

	if (bOnlyCount_ == false)
	{
		// 検索結果の領域を1024個分予約しておく(16KB)
		m_cResultSet.clear();
		m_cResultSet.reserve(1024);
	}

	ModSize count = 0;
	DocumentID id = beginID();	// 検索開始位置
		
	while (id != UndefinedDocumentID)
	{
		id = m_pNode->lowerBound(*m_pSearchInfo, id, false);
		if (checkID(id) == false)
		{
			// 条件を満たしていないので次へ
			id = nextID(id);
			continue;
		}
		
		if (bOnlyCount_ == false)
		{
			// 必要ならスコアを求める
			DocumentScore score = 0;
			if (m_bScore)
			{
				; _TRMEISTER_ASSERT(m_pSearchInfo);
					
				score = m_pNode->getScore(*m_pSearchInfo);
			}

			// 結果を格納する
			m_cResultSet.pushBack(
				ModPair<DocumentID, DocumentScore>(id, score));
		}

		++count;
		
		// 次へ
		id = nextID(id);
	}

	// 検索直後は文書IDの昇順になっているので、
	// それ以外のソートが指定されている場合はソートする

	if (m_eKey != SortKey::DocID || m_eOrder != Order::Asc)
	{
		m_cResultSet.sort(m_eKey, m_eOrder);
	}

	return count;
}

//
//	FUNCTION private
//	FullText2::Executor::beginID
//		-- 先頭文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::DocumentID
//		先頭の文書ID
//
//	EXCEPTIONS
//
DocumentID
Executor::beginID()
{
	DocumentID id;
	
	if (m_bNarrowing)
	{
		id = m_uiCurrentID = *m_ite;
		++m_ite;
	}
	else
	{
		id = m_uiLower;
	}

	return id;
}

//
//	FUNCTION private
//	FullText2::Executor::nextID
//		-- 次の文書IDを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID currentID_
//		現在のID
//
//	RETURN
//	FullText2::DocumentID
//		次のID
//
//	EXCEPTIONS
//
DocumentID
Executor::nextID(DocumentID currentID_)
{
	DocumentID id = UndefinedDocumentID;
	
	if (m_bNarrowing)
	{
		while (m_ite != m_endIte)
		{
			m_uiCurrentID = *m_ite;
			if (m_uiCurrentID < currentID_)
			{
				++m_ite;
				continue;
			}
			
			id = m_uiCurrentID;
			++m_ite;
			break;
		}
	}
	else
	{
		if (currentID_ < m_uiUpper)
			id = currentID_ + 1;
	}

	return id;
}

//
//	FUNCTION private
//	FullText2::Executor::checkID -- 得られた文書IDが条件を満たしているか
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::DocumentID currentID_
//		文書ID
//
//	RETURN
//	bool
//		条件を満たしている場合はtrue、それ以外の場合はfalse
//
//	EXCEPTIONS
//
bool
Executor::checkID(DocumentID currentID_)
{
	bool r = false;
	
	if (m_bNarrowing)
	{
		if (m_uiCurrentID == currentID_)
			r = true;
	}
	else
	{
		if (currentID_ < m_uiUpper)
			r = true;
	}

	return r;
}

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
