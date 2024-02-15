// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNodeAnd.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "FullText2/OperatorTermNodeAnd.h"
#include "FullText2/OperatorTermNodeSingle.h"
#include "FullText2/SearchInformation.h"
#include "FullText2/ScoreCalculator.h"
#include "FullText2/ScoreCombiner.h"
#include "Os/Limits.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::OperatorTermNodeAnd -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrString_
//		文字列表記
//	FullText2::ScoreCombiner* pCombiner_
//		スコア合成器
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNodeAnd::OperatorTermNodeAnd(const ModUnicodeString& cString_,
										 ScoreCombiner* pCombiner_)
	: OperatorTermNode(cString_),
	  m_uiCurrentID(0), m_pCombiner(pCombiner_)
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::~OperatorTermNodeAnd -- デストラクタ
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
OperatorTermNodeAnd::~OperatorTermNodeAnd()
{
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	for (; i != m_vecpTermNode.end(); ++i)
	{
		delete (*i).m_pTermNode;
	}
	m_vecpTermNode.clear();
	delete m_pCombiner;
}

//
//	FUCNTION public
//	FullText2::OperatorTermNodeAnd::OperatorTermNodeAnd
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorTermNodeAnd& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNodeAnd::OperatorTermNodeAnd(const OperatorTermNodeAnd& src_)
	: OperatorTermNode(src_)
{
	m_vecpTermNode.reserve(src_.m_vecpTermNode.getSize());
	ModVector<NodeData>::ConstIterator i = src_.m_vecpTermNode.begin();
	for (; i != src_.m_vecpTermNode.end(); ++i)
	{
		OperatorTermNodeSingle* term
			= _SYDNEY_DYNAMIC_CAST(OperatorTermNodeSingle*,
								   (*i).m_pTermNode->copy());
		m_vecpTermNode.pushBack(NodeData((*i).m_iField,
										 term,
										 (*i).m_dblScale,
										 (*i).m_dblGeta));
	}
	m_uiCurrentID = 0;
	m_pCombiner = src_.m_pCombiner->copy();
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::pushBack -- TermNodeを追加する
//
//	NOTES
//
//	ARGUMENTS
//	int field_
//		フィールド番号
//	FullText2::OperatorTermNodeSingle* node_
//		追加するTermNode
//	double scoreScale_
//		スコアのスケール値
//	double scoreGeta_
//		スコアのゲタ
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorTermNodeAnd::pushBack(int field_,
							  OperatorTermNodeSingle* node_,
							  double scoreScale_,
							  double scoreGeta_)
{
	m_vecpTermNode.pushBack(NodeData(field_,
									 node_,
									 scoreScale_,
									 scoreGeta_));
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::getEstimateCountLevel1
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
OperatorTermNodeAnd::getEstimateCountLevel1(SearchInformation& cSearchInfo_)
{
	ModSize count = Os::Limits<ModSize>::getMax();
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	for (; i < e; ++i)
	{
		ModSize n = (*i).m_pTermNode->getEstimateCountLevel1Impl(cSearchInfo_);
		if (n < count)
			count = n;
	}
	
	return count;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::reset -- リセットする
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
OperatorTermNodeAnd::reset()
{
	m_uiCurrentID = 0;
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	for (; i != m_vecpTermNode.end(); ++i)
	{
		(*i).m_pTermNode->resetImpl();
	}
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::lowerBound -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID id_
//		検索する文書ID
//	bool isRough_
//		ラフモードかどうか
//
//	RETURN
//	FullText2::DocumentID
//		見つかった文書ID
//
//	EXCEPTIONS
//
DocumentID
OperatorTermNodeAnd::lowerBound(SearchInformation& cSearchInfo_,
								DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値なら現在値をそのまま返す
		return m_uiCurrentID;

	ModVector<NodeData>::Iterator b = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	ModVector<NodeData>::Iterator i = b;

	if (b == e)
	{
		// 子ノードがない
		
		m_uiCurrentID = UndefinedDocumentID;
		return m_uiCurrentID;
	}

	m_uiCurrentID = id_;

	while (i < e)
	{
		SearchInformation& cSearchInfo
			= cSearchInfo_.getElement((*i).m_iField);
		
		DocumentID tmp
			= (*i).m_pTermNode->lowerBoundImpl(cSearchInfo, id_, isRough_);
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
//	FullText2::OperatorTermNodeAnd::getScore -- スコアを得る
//
//	NOTES
//
//	ARUGMENTS
//	FullText2::SearchInformation cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	FullText2::DocumentScore
//		文書スコア
//
//	EXCEPTIONS
//
DocumentScore
OperatorTermNodeAnd::getScore(SearchInformation& cSearchInfo_)
{
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	
	DocumentScore score
		= ((*i).m_pTermNode->getScoreImpl(
			   cSearchInfo_.getElement((*i).m_iField)) *
		   (*i).m_dblScale) + (*i).m_dblGeta;	// 先頭のスコア
	++i;

	for (; i < e; ++i)
	{
		score = m_pCombiner->combine(
			score,
			((*i).m_pTermNode->getScoreImpl(
				cSearchInfo_.getElement((*i).m_iField)) *
			 (*i).m_dblScale) + (*i).m_dblGeta);
	}
	
	return score;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OperatorNode*
//		コピーしたOperatorTermNodeAnd
//
//	EXCEPTIONS
//
OperatorNode*
OperatorTermNodeAnd::copy() const
{
	return new OperatorTermNodeAnd(*this);
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::getTermFrequency -- 文書内頻度を得る
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	ModSize
//		文書内頻度
//
//	EXCEPTIONS
//
ModSize
OperatorTermNodeAnd::getTermFrequency()
{
	ModSize tf = 0;
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	for (; i != m_vecpTermNode.end(); ++i)
		tf += (*i).m_pTermNode->getTermFrequencyImpl();
	return tf;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::setScoreCalculator
//		-- スコア計算器を設定する
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::ScoreCalculator* pCalculator_
//		スコア計算器
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorTermNodeAnd::setScoreCalculator(ScoreCalculator* pCalculator_)
{
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	if (i != e)
	{
		// 先頭要素にはそのまま
		(*i).m_pTermNode->setScoreCalculator(pCalculator_);
		++i;
	}
	for (; i < e; ++i)
	{
		// 先頭以外にはコピーを渡す
		(*i).m_pTermNode->setScoreCalculator(pCalculator_->copy());
	}
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeAnd::getDocumentFrequency -- 文書頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID bid_
//		開始文書ID
//	FullText2::DocumentID eid_
//		終端文書ID
//	FullText2::OperatorTermNode::Frequency& cValue_
//		頻度
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorTermNodeAnd::getDocumentFrequency(SearchInformation& cSearchInfo_,
										  DocumentID bid_,
										  DocumentID eid_,
										  Frequency& cValue_,
										  bool bGetTotalTermFrequency_)
{
	// リセットする
	reset();

	// 子の格納領域を得る
	ModVector<Frequency>& vecFrequency = cValue_.getChild(cSearchInfo_);

	DocumentID id = bid_;	// 先頭文書ID
	
	ModVector<NodeData>::Iterator b = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();

	if (b == e)
	{
		// 子ノードがない

		m_uiCurrentID = UndefinedDocumentID;
		return;
	}

	// 文書IDの配列
	ModVector<DocumentID> vecID(m_vecpTermNode.getSize(), bid_);
	ModVector<DocumentID>::Iterator d_b = vecID.begin();
	ModVector<DocumentID>::Iterator d_e = vecID.end();

	// ループの中で仮想関数を呼び出したくないので
	ModVector<SearchInformation*>::Iterator sb
		= cSearchInfo_.getAllElement().begin();
	ModVector<Frequency>::Iterator fb = vecFrequency.begin();

	while (id < eid_)
	{
		ModVector<NodeData>::Iterator i = b;
		ModVector<DocumentID>::Iterator d_i = d_b;
		m_uiCurrentID = UndefinedDocumentID;	// 最大値にしておく
		bool same = true;
		
		// 最少の文書IDを探す
		for (; i < e; ++i)
		{
			if (*d_i < id)
			{
				SearchInformation& cSearchInfo = *(*(sb + (*i).m_iField));
				Frequency& f = *(fb + (*i).m_iField);

				*d_i = (*i).m_pTermNode->lowerBoundImpl(cSearchInfo,
														id, false);

				if (*d_i < eid_)
				{
					// 文書頻度を増やす
					++f.m_uiDocumentFrequency;
				
					if (bGetTotalTermFrequency_)
					{
						f.m_ulTotalTermFrequency
							+= (*i).m_pTermNode->getTermFrequencyImpl();
					}
				}
			}

			if (d_i != d_b && m_uiCurrentID != *d_i)
				same = false;
			if (*d_i < m_uiCurrentID)
				m_uiCurrentID = *d_i;
		}

		id = m_uiCurrentID;

		if (id < eid_)
		{
			if (same)
			{
				// 文書頻度を増やす
				++cValue_.m_uiDocumentFrequency;

				// 総文書内頻度はスコア計算にしか利用しないので、
				// ここでは必要ない
			}

			// 次へ
			++id;
		}
	}
}

//
//	Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
