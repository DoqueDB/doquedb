// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNodeTf.cpp --
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
#include "FullText2/OperatorTermNodeTf.h"
#include "FullText2/OperatorTermNodeSingle.h"
#include "FullText2/SearchInformation.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::OperatorTermNodeTf -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const ModUnicodeString& cstrString_
//		文字列表記
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNodeTf::OperatorTermNodeTf(const ModUnicodeString& cString_)
	: OperatorTermNodeScore(cString_),
	  m_uiCurrentID(0)
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::~OperatorTermNodeTf -- デストラクタ
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
OperatorTermNodeTf::~OperatorTermNodeTf()
{
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	for (; i != m_vecpTermNode.end(); ++i)
	{
		delete (*i).m_pTermNode;
	}
	m_vecpTermNode.clear();
}

//
//	FUCNTION public
//	FullText2::OperatorTermNodeTf::OperatorTermNodeTf -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorTermNodeTf& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNodeTf::OperatorTermNodeTf(const OperatorTermNodeTf& src_)
	: OperatorTermNodeScore(src_)
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
										 (*i).m_dblScale));
	}
	m_uiCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::pushBack -- TermNodeを追加する
//
//	NOTES
//
//	ARGUMENTS
//	int field_
//		フィールド番号
//	FullText2::OperatorTermNodeSingle* node_
//		追加するTermNode
//	double tfScale_
//		TF項のスケール
//	double dummy
//	   	不使用
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorTermNodeTf::pushBack(int field_,
							 OperatorTermNodeSingle* node_,
							 double tfScale_,
							 double dummy)
{
	m_vecpTermNode.pushBack(NodeData(field_, node_, tfScale_));
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::getEstimateCountLevel1
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
OperatorTermNodeTf::getEstimateCountLevel1(SearchInformation& cSearchInfo_)
{
	ModSize count = 0;
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	for (; i < e; ++i)
	{
		ModSize n = (*i).m_pTermNode->getEstimateCountLevel1Impl(cSearchInfo_);
		if (count < n)
			count = n;
	}
	
	return count;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::reset -- リセットする
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
OperatorTermNodeTf::reset()
{
	m_uiCurrentID = 0;
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	for (; i != e; ++i)
	{
		(*i).m_pTermNode->resetImpl();
		(*i).m_uiID = 0;
	}
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::lowerBound -- 文書IDを検索する
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
OperatorTermNodeTf::lowerBound(SearchInformation& cSearchInfo_,
							   DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値なら現在値をそのまま返す
		return m_uiCurrentID;

	ModVector<NodeData>::Iterator b = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	ModVector<NodeData>::Iterator i = b;

	m_uiCurrentID = UndefinedDocumentID;	// 最大値

	// 最小の文書IDを探す
	for (; i < e; ++i)
	{
		if ((*i).m_uiID < id_)
		{
			SearchInformation& cSearchInfo
				= cSearchInfo_.getElement((*i).m_iField);
			
			(*i).m_uiID = (*i).m_pTermNode->lowerBoundImpl(cSearchInfo,
														   id_, isRough_);
		}

		if ((*i).m_uiID < m_uiCurrentID)
			m_uiCurrentID = (*i).m_uiID;
	}

	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::getScore -- スコアを得る
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
OperatorTermNodeTf::getScore(SearchInformation& cSearchInfo_)
{
	// 初期化する
	initializeCalculator(cSearchInfo_);

	if (m_pTermFrequency)
	{
		// 文書内頻度が必要

		(*m_pTermFrequency).m_dblValue = 0;

		ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
		ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
		for (; i != e; ++i)
		{
			if ((*i).m_uiID == m_uiCurrentID)
			{
				// 重みをかける
				(*m_pTermFrequency).m_dblValue +=
					((*i).m_dblScale
					 * (*i).m_pTermNode->getTermFrequencyImpl());
			}
		}
	}

	if (m_pDocumentLength)
	{
		// 文書長が必要
		ModSize length = 0;
		cSearchInfo_.getDocumentLength(m_uiCurrentID, length);
		(*m_pDocumentLength).m_dblValue = static_cast<double>(length);
	}

	// firstStep を計算し、事前に求めてあった IDF 項と掛け合わせる
	
	return static_cast<DocumentScore>(m_pCalculator->firstStep(m_vecScoreArg))
		* m_dblIDF;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OperatorNode*
//		コピーしたOperatorTermNodeTf
//
//	EXCEPTIONS
//
OperatorNode*
OperatorTermNodeTf::copy() const
{
	return new OperatorTermNodeTf(*this);
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::getTermFrequencyImpl -- 文書内頻度を得る
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
OperatorTermNodeTf::getTermFrequencyImpl()
{
	ModSize tf = 0;
	ModVector<NodeData>::Iterator i = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();
	for (; i != e; ++i)
	{
		if ((*i).m_uiID == m_uiCurrentID)
			tf += (*i).m_pTermNode->getTermFrequencyImpl();
	}
	return tf;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeTf::getDocumentFrequency -- 文書頻度を得る
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
OperatorTermNodeTf::getDocumentFrequency(SearchInformation& cSearchInfo_,
										 DocumentID bid_,
										 DocumentID eid_,
										 Frequency& cValue_,
										 bool bGetTotalTermFrequency_)
{
	// リセットする
	reset();

	DocumentID id = bid_;	// 先頭文書ID
	ModSize df = 0;			// 文書頻度
	ModUInt64 tt = 0;		// 総文書内頻度
	
	ModVector<NodeData>::Iterator b = m_vecpTermNode.begin();
	ModVector<NodeData>::Iterator e = m_vecpTermNode.end();

	// ループの中で仮想関数を呼び出したくないので
	ModVector<SearchInformation*>::Iterator sb
		= cSearchInfo_.getAllElement().begin();

	while (id < eid_)
	{
		ModVector<NodeData>::Iterator i = b;
		m_uiCurrentID = UndefinedDocumentID;	// 最大値にしておく

		// 最少の文書IDを探す
		for (; i < e; ++i)
		{
			if ((*i).m_uiID < id)
			{
				SearchInformation& cSearchInfo = *(*(sb + (*i).m_iField));

				(*i).m_uiID
					= (*i).m_pTermNode->lowerBoundImpl(cSearchInfo,
													   id, false);
			}

			if ((*i).m_uiID < m_uiCurrentID)
				m_uiCurrentID = (*i).m_uiID;
		}

		id = m_uiCurrentID;

		if (id < eid_)
		{
			// 文書頻度を増やす
			++df;

			if (bGetTotalTermFrequency_)
			{
				// 文書内頻度を加える
				tt += getTermFrequencyImpl();
			}

			// 次へ
			++id;
		}
	}

	cValue_.m_uiDocumentFrequency = df;
	cValue_.m_ulTotalTermFrequency = tt;
}

//
//	Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
