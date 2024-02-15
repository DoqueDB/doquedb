// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorAndNotNode.cpp --
// 
// Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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
#include "FullText2/OperatorAndNotNode.h"
#include "FullText2/SearchInformation.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::OperatorAndNotNode -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorAndNotNode::OperatorAndNotNode(OperatorNode* pLeft_,
									   OperatorNode* pRight_)
	: OperatorNode(),
	  m_uiCurrentID(0), m_uiRightID(0),
	  m_pLeft(pLeft_), m_pRight(pRight_)
{
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::~OperatorAndNotNode -- デストラクタ
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
OperatorAndNotNode::~OperatorAndNotNode()
{
	delete m_pLeft;
	delete m_pRight;
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::OperatorAndNotNode -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const OperatorAndNotNode& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorAndNotNode::OperatorAndNotNode(const OperatorAndNotNode& src_)
	: OperatorNode(src_), m_uiCurrentID(0), m_uiRightID(0)
{
	m_pLeft = src_.m_pLeft->copy();
	m_pRight = src_.m_pRight->copy();
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::reset -- リセットする
//
//	NOTES
//
//	ARGUMETNS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorAndNotNode::reset()
{
	m_uiCurrentID = 0;
	m_uiRightID = 0;
	
	m_pLeft->reset();
	m_pRight->reset();
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::lowerBound -- 文書IDを検索する
//
//	NOTES
//
//	ARGUMETNS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//	FullText2::DocumentID id_
//		検索する文書ID
//	bool isRought_
//		ラフモードかどうか
//
//	RETURN
//	FullText2::DocumentID
//		検索された文書ID
//
//	EXCEPTIONS
//
DocumentID
OperatorAndNotNode::lowerBound(SearchInformation& cSearchInfo_,
							   DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値なら現在値をそのまま返す
		return m_uiCurrentID;

	m_uiCurrentID = UndefinedDocumentID;
	DocumentID id = id_;

	for (;;)
	{
		// 左のノードから右のノードの結果を除去する

		// 左のノードを検索する
		id = m_pLeft->lowerBound(cSearchInfo_, id, isRough_);
		if (id == UndefinedDocumentID)
			// ヒットしなかった
			break;

		if (m_uiRightID <= id)
			// ヒットした文書IDで右のノードを検索する
			m_uiRightID = m_pRight->lowerBound(cSearchInfo_, id, isRough_);

		if (m_uiRightID != id)
			// 右に存在しなかったので、ヒット
			break;

		++id;
	}

	m_uiCurrentID = id;

	return id;
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::getScore -- スコアを得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//	  	検索情報クラス
//
//	RETURN
//	FullText2::DocumentScore
//		スコア
//
//	EXCEPTIONS
//
DocumentScore
OperatorAndNotNode::getScore(SearchInformation& cSearchInfo_)
{
	if (m_uiCurrentID == UndefinedDocumentID)
		return static_cast<DocumentScore>(0.0);
	
	// And-Not のスコアは左ノードのスコアそのもの
	return m_pLeft->getScore(cSearchInfo_);
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OperatorNode*
// 		コピー
//
//	EXCEPTIONS
//
OperatorNode*
OperatorAndNotNode::copy() const
{
	return new OperatorAndNotNode(*this);
}

//
//	FUNCTION public
//	FullText2::OperatorAndNotNode::getEstimateCountLevel1
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
OperatorAndNotNode::getEstimateCountLevel1(SearchInformation& cSearchInfo_)
{
	double count = cSearchInfo_.getDocumentCount();
	double rightCount = m_pRight->getEstimateCountLevel1(cSearchInfo_);
	double r = 1.0 - (rightCount / count);
	double leftCount = m_pLeft->getEstimateCountLevel1(cSearchInfo_);

	return static_cast<ModSize>(leftCount * r);
}

//
//	Copyright (c) 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
