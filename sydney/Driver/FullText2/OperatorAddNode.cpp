// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorAddNode.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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
#include "FullText2/OperatorAddNode.h"
#include "FullText2/ScoreCombiner.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::OperatorAddNode -- コンストラクタ
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
OperatorAddNode::OperatorAddNode()
	: OperatorNode(), m_pCombiner(0), m_pFirst(0), m_pSecond(0),
	  m_uiCurrentID(0), m_uiSecondCurrentID(0)
{
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::~OperatorAddNode -- デストラクタ
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
OperatorAddNode::~OperatorAddNode()
{
	delete m_pCombiner;
	delete m_pFirst;
	delete m_pSecond;
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::OperatorAddNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorAddNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorAddNode::OperatorAddNode(const OperatorAddNode& src_)
	: OperatorNode(src_), m_pCombiner(0), m_pFirst(0), m_pSecond(0)
{
	if (src_.m_pCombiner)
		m_pCombiner = src_.m_pCombiner->copy();
	
	m_uiCurrentID = src_.m_uiCurrentID;
	m_uiSecondCurrentID = src_.m_uiSecondCurrentID;
	
	if (src_.m_pFirst) m_pFirst = src_.m_pFirst->copy();
	if (src_.m_pSecond) m_pSecond = src_.m_pSecond->copy();
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::setScoreCombiner
//		-- スコア合成器を設定する
//
//	NOTES
//	与えられたスコア合成器のインスタンスはデストラクト時に解放される
//
//	ARGUMENTS
//	FullText2::ScoreCombiner*
//		スコア合成器
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorAddNode::setScoreCombiner(ScoreCombiner* pCombiner_)
{
	if (m_pCombiner) delete m_pCombiner;
	m_pCombiner = pCombiner_;
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::reset -- リセットする
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
OperatorAddNode::reset()
{
	if (m_pFirst) m_pFirst->reset();
	if (m_pSecond) m_pSecond->reset();
	m_uiCurrentID = 0;
	m_uiSecondCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::lowerBound -- 文書IDを検索する
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
OperatorAddNode::lowerBound(SearchInformation& cSearchInfo_,
							DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値なら現在値をそのまま返す
		return m_uiCurrentID;

	if (m_pFirst == 0)
	{
		// 子ノードがない
		
		m_uiCurrentID = UndefinedDocumentID;
		return m_uiCurrentID;
	}

	// 結果集合としては m_pFirst のみになる
	// m_pSecond はスコア計算のためにのみ必要

	m_uiCurrentID = m_pFirst->lowerBound(cSearchInfo_, id_, isRough_);
	
	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::getScore -- スコアを得る
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
OperatorAddNode::getScore(SearchInformation& cSearchInfo_)
{
	DocumentScore score = m_pFirst->getScore(cSearchInfo_);

	if (m_uiSecondCurrentID < m_uiCurrentID)
	{
		m_uiSecondCurrentID = m_pSecond->lowerBound(cSearchInfo_,
													m_uiCurrentID,
													false);
	}
	
	if (m_uiCurrentID == m_uiSecondCurrentID)
	{
		// ヒットしたので、m_pSecondのスコアも考慮する
		score = m_pCombiner->combine(score,
									 m_pSecond->getScore(cSearchInfo_));
	}

	return score;
}

//
//	FUNCTION public
//	FullText2::OperatorAddNode::copy -- コピーする
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
OperatorAddNode::copy() const
{
	return new OperatorAddNode(*this);
}

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

