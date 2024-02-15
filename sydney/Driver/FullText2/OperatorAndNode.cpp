// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorAndNode.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
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
#include "FullText2/OperatorAndNode.h"

#include "FullText2/SearchInformation.h"
#include "FullText2/ScoreCombiner.h"
#include "Os/Limits.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::OperatorAndNode -- コンストラクタ
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
OperatorAndNode::OperatorAndNode()
	: LogicalOperatorNode(), m_uiCurrentID(0)
{
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::~OperatorAndNode -- デストラクタ
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
OperatorAndNode::~OperatorAndNode()
{
	ChildVector::Iterator i = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	for (; i < e; ++i)
		delete (*i);
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::OperatorAndNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorAndNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorAndNode::OperatorAndNode(const OperatorAndNode& src_)
	: LogicalOperatorNode(src_)
{
	m_uiCurrentID = src_.m_uiCurrentID;
	m_vecChildren.reserve(src_.m_vecChildren.getSize());
	ChildVector::ConstIterator i = src_.m_vecChildren.begin();
	ChildVector::ConstIterator e = src_.m_vecChildren.end();
	for (; i < e; ++i)
		m_vecChildren.pushBack((*i)->copy());
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::reset -- リセットする
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
OperatorAndNode::reset()
{
	ChildVector::Iterator i = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	for (; i < e; ++i)
		(*i)->reset();
	m_uiCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::lowerBound -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID id_
//		文書ID
//	bool isRough_
//		ラフモード
//
//	RETURN
//	Inveted::DocumentID
//		ヒットした文書のID
//
//	EXCEPTIONS
//
DocumentID
OperatorAndNode::lowerBound(SearchInformation& cSearchInfo_,
							DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値なら現在値をそのまま返す
		return m_uiCurrentID;

	if (m_vecChildren.getSize() == 0)
	{
		// 子ノードがない
		
		m_uiCurrentID = UndefinedDocumentID;
		return m_uiCurrentID;
	}
	
	ChildVector::Iterator b = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	ChildVector::Iterator i = b;

	m_uiCurrentID = id_;
	
	while (i < e)
	{
		DocumentID tmp = (*i)->lowerBound(cSearchInfo_,
										  m_uiCurrentID, isRough_);
		if (tmp != m_uiCurrentID)
		{
			m_uiCurrentID = tmp;
			if (m_uiCurrentID == UndefinedDocumentID)
				// これ以上存在していない
				return UndefinedDocumentID;

			if (i != b)
			{
				// 文書IDが変わったので初めから

				i = b;
				continue;
			}
		}
		++i;
	}

	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::getScore -- スコアを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	FullText2::DocumentScore
//		スコア
//
//	EXCEPTIONS
//
DocumentScore
OperatorAndNode::getScore(SearchInformation& cSearchInfo_)
{
	ChildVector::Iterator i = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	
	DocumentScore score = (*i)->getScore(cSearchInfo_);	// 先頭のスコア
	++i; // 次へ
	for (; i < e; ++i)
		score = m_pCombiner->combine(score, (*i)->getScore(cSearchInfo_));

	return score;
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::copy -- コピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OperatorNode*
//		コピー
//
//	EXCEPTIONS
//
OperatorNode*
OperatorAndNode::copy() const
{
	return new OperatorAndNode(*this);
}

//
//	FUNCTION public
//	FullText2::OperatorAndNode::getEstimateCountLevel1
//		-- 見積もり検索結果件数を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	ModSize
//		見積もり検索結果件数
//
//	EXCEPTIONS
//
ModSize
OperatorAndNode::getEstimateCountLevel1(SearchInformation& cSearchInfo_)
{
	double docCount = static_cast<double>(cSearchInfo_.getDocumentCount());
	double ratio = 1.0;
	ModVector<OperatorNode*>::Iterator i = m_vecChildren.begin();
	ModVector<OperatorNode*>::Iterator e = m_vecChildren.end();
	for (; i < e; ++i)
	{
		ModSize n = (*i)->getEstimateCountLevel1(cSearchInfo_);
		double r = static_cast<double>(n)/docCount;
		ratio *= r;
	}
	ModSize count = static_cast<ModSize>(docCount * ratio);
	return count;
}

//
//	Copyright (c) 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
