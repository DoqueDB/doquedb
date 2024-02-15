// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorOrNode.cpp --
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
#include "FullText2/OperatorOrNode.h"

#include "FullText2/SearchInformation.h"
#include "FullText2/ScoreCombiner.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::OperatorOrNode -- コンストラクタ
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
OperatorOrNode::OperatorOrNode()
	: LogicalOperatorNode(), m_uiCurrentID(0)
{
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::~OperatorOrNode -- デストラクタ
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
OperatorOrNode::~OperatorOrNode()
{
	ChildVector::Iterator i = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	for (; i < e; ++i)
		delete (*i).second;
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::OperatorOrNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorOrNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorOrNode::OperatorOrNode(const OperatorOrNode& src_)
	: LogicalOperatorNode(src_)
{
	m_uiCurrentID = src_.m_uiCurrentID;
	m_vecChildren.reserve(src_.m_vecChildren.getSize());
	ChildVector::ConstIterator i = src_.m_vecChildren.begin();
	ChildVector::ConstIterator e = src_.m_vecChildren.end();
	for (; i < e; ++i)
		m_vecChildren.pushBack(Child((*i).first, (*i).second->copy()));
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::pushBack -- 子ノードを追加する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::OperatorNode* child_
//		子ノード
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorOrNode::pushBack(OperatorNode* child_)
{
	m_vecChildren.pushBack(Child(0, child_));
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::reset -- リセットする
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
OperatorOrNode::reset()
{
	ChildVector::Iterator i = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	for (; i < e; ++i)
	{
		(*i).first = 0;
		(*i).second->reset();
	}
	m_uiCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::lowerBound -- 文書IDを検索する
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
OperatorOrNode::lowerBound(SearchInformation& cSearchInfo_,
						   DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値なら現在値をそのまま返す
		return m_uiCurrentID;
	
	ChildVector::Iterator b = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	ChildVector::Iterator i = b;

	m_uiCurrentID = UndefinedDocumentID;	// 最大値

	// 最小の文書IDを探す
	for (; i < e; ++i)
	{
		if ((*i).first < id_)
			(*i).first = (*i).second->lowerBound(cSearchInfo_, id_, isRough_);

		if ((*i).first < m_uiCurrentID)
			m_uiCurrentID = (*i).first;
	}

	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::getScore -- スコアを得る
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
OperatorOrNode::getScore(SearchInformation& cSearchInfo_)
{
	ChildVector::Iterator i = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	DocumentScore score = 0.0;

	if (m_uiCurrentID == UndefinedDocumentID)
		return score;

	for (; i < e; ++i)
	{
		if ((*i).first == m_uiCurrentID)
		{
			score = (*i).second->getScore(cSearchInfo_);	// 先頭のスコア
			++i; // 次へ
			break;
		}
	}
	for (; i < e; ++i)
	{
		if ((*i).first == m_uiCurrentID)
		{
			score = m_pCombiner->combine(score,
										 (*i).second->getScore(cSearchInfo_));
		}
	}

	return score;
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::copy -- コピーする
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
OperatorOrNode::copy() const
{
	return new OperatorOrNode(*this);
}

//
//	FUNCTION public
//	FullText2::OperatorOrNode::getEstimateCountLevel1
//		-- おおよその文書頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	ModSize
//		おおよその文書頻度
//
//	EXCEPTIONS
//
ModSize
OperatorOrNode::getEstimateCountLevel1(SearchInformation& cSearchInfo_)
{
	ModSize docCount = cSearchInfo_.getDocumentCount();
	ModSize count = 0;
	ChildVector::Iterator i = m_vecChildren.begin();
	ChildVector::Iterator e = m_vecChildren.end();
	for (; i < e; ++i)
	{
		ModSize n = (*i).second->getEstimateCountLevel1(cSearchInfo_);
		double r = static_cast<double>(n)/static_cast<double>(docCount);
		count += static_cast<ModSize>(
			static_cast<double>(docCount - count) * r);
	}
	
	return count;
}

//
//	Copyright (c) 2010, 2011, 2012, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
