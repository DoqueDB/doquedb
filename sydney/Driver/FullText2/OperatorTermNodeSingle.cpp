// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNodeSingle.cpp --
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
#include "FullText2/OperatorTermNodeSingle.h"
#include "FullText2/SearchInformation.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::OperatorTermNodeSingle -- コンストラクタ
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
OperatorTermNodeSingle::OperatorTermNodeSingle(const ModUnicodeString& cString_,
											   LeafNode* node_)
	: OperatorTermNodeScore(cString_),
	  m_pLeafNode(node_), m_uiCurrentID(0)
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::~OperatorTermNodeSingle -- デストラクタ
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
OperatorTermNodeSingle::~OperatorTermNodeSingle()
{
	delete m_pLeafNode;
}

//
//	FUCNTION public
//	FullText2::OperatorTermNodeSingle::OperatorTermNodeSingle
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorTermNodeSingle& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNodeSingle::
OperatorTermNodeSingle(const OperatorTermNodeSingle& src_)
	: OperatorTermNodeScore(src_)
{
	m_pLeafNode = src_.m_pLeafNode->copy();
	m_uiCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::getEstimateCountImpl
//		-- 見積もり検索件数を得る
//
//	NOTES
//
//	ARGUMENTS
//	FullText2::SearchInformation& cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	ModSize
//		見積もり検索件数
//
//	EXCEPTIONS
//
ModSize
OperatorTermNodeSingle::getEstimateCountLevel1Impl(
	SearchInformation& cSearchInfo_)
{
	// 下位の見積もりには削除された文書も含まれてしまっている
	// よって、ここで削除された文書の割合だけ少ない数値にする

	ModSize docCount = cSearchInfo_.getDocumentCount();
	ModSize expungeCount = cSearchInfo_.getExpungeDocumentCount();
	double count = m_pLeafNode->getEstimateCount(cSearchInfo_);
	double r = 1.0 - static_cast<double>(expungeCount)
		/ static_cast<double>(docCount + expungeCount);
	ModSize c =  static_cast<ModSize>(count * r);
	if (c > docCount)
		// 最大でも登録文書数は超えない
		c = docCount;
	return c;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::resetImpl -- リセットする
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
OperatorTermNodeSingle::resetImpl()
{
	m_pLeafNode->reset();
	m_uiCurrentID = 0;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::lowerBoundImpl -- 文書IDを検索する
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
OperatorTermNodeSingle::lowerBoundImpl(SearchInformation& cSearchInfo_,
									   DocumentID id_, bool isRough_)
{
	if (id_ <= m_uiCurrentID)
		// 小さい値なら現在値をそのまま返す
		return m_uiCurrentID;

	m_uiCurrentID = m_pLeafNode->lowerBound(cSearchInfo_, id_, isRough_);
	return m_uiCurrentID;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::getScoreImpl -- スコアを得る
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
OperatorTermNodeSingle::getScoreImpl(SearchInformation& cSearchInfo_)
{
	// 初期化する
	initializeCalculator(cSearchInfo_);

	if (m_pTermFrequency)
	{
		// 文書内頻度が必要

		(*m_pTermFrequency).m_dblValue
			= m_pLeafNode->getTermFrequency();
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
//	FullText2::OperatorTermNodeSingle::copy -- コピーを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::OperatorNode*
//		コピーしたOperatorTermNodeSingle
//
//	EXCEPTIONS
//
OperatorNode*
OperatorTermNodeSingle::copy() const
{
	return new OperatorTermNodeSingle(*this);
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::getTermFrequencyImpl -- 文書内頻度を得る
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
OperatorTermNodeSingle::getTermFrequencyImpl()
{
	return m_pLeafNode->getTermFrequency();
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeSingle::getDocumentFrequency -- 文書頻度を得る
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
OperatorTermNodeSingle::getDocumentFrequency(SearchInformation& cSearchInfo_,
											 DocumentID bid_,
											 DocumentID eid_,
											 Frequency& cValue_,
											 bool bGetTotalTermFrequency_)
{
	// リセットする
	resetImpl();

	DocumentID id = bid_;	// 先頭文書ID
	ModSize df = 0;			// 文書頻度
	ModUInt64 tt = 0;		// 総文書内頻度
	
	while ((id = lowerBoundImpl(cSearchInfo_, id, false))	// ラフではない
		   != UndefinedDocumentID)
	{
		if (id >= eid_)
			// 終わり
			break;

		// 検索結果が得られたので、文書頻度を増やす
		++df;

		if (bGetTotalTermFrequency_)
		{
			// 文書内頻度を加える
			tt += getTermFrequencyImpl();
		}

		// 次へ
		++id;
	}

	cValue_.m_uiDocumentFrequency = df;
	cValue_.m_ulTotalTermFrequency = tt;
}

//
//	Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
