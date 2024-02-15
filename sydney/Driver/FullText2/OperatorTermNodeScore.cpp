// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNodeScore.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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
#include "FullText2/OperatorTermNodeScore.h"
#include "FullText2/SearchInformation.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeScore::OperatorTermNodeScore -- コンストラクタ
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
OperatorTermNodeScore::OperatorTermNodeScore(const ModUnicodeString& cString_)
	: OperatorTermNode(cString_),
	  m_pCalculator(0), m_bInitCalculator(false),
	  m_pTermFrequency(0), m_pDocumentLength(0), m_dblIDF(0.0)
{
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeScore::~OperatorTermNodeScore -- デストラクタ
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
OperatorTermNodeScore::~OperatorTermNodeScore()
{
	delete m_pCalculator;
}

//
//	FUCNTION public
//	FullText2::OperatorTermNodeScore::OperatorTermNodeScore
//		-- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::OperatorTermNodeScore& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
OperatorTermNodeScore::OperatorTermNodeScore(const OperatorTermNodeScore& src_)
	: OperatorTermNode(src_)
{
	m_pCalculator = 0;
	if (src_.m_pCalculator)
		m_pCalculator = src_.m_pCalculator->copy();
	m_bInitCalculator = src_.m_bInitCalculator;
	
	m_vecScoreArg = src_.m_vecScoreArg;
	m_pTermFrequency = 0;
	m_pDocumentLength = 0;
	if (m_bInitCalculator)
		// 引数をメンバー変数に割り当てる
		assignArg();
	m_dblIDF = src_.m_dblIDF;
}

//
//	FUNCTION public
//	FullText2::OperatorTermNodeScore::setScoreCalculator
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
OperatorTermNodeScore::setScoreCalculator(ScoreCalculator* pCalculator_)
{
	if (m_pCalculator)
		delete m_pCalculator;
	m_pCalculator = pCalculator_;
}

//
//	FUNCTION protected
//	FullText2::OperatorTermNodeScore::initializeCalculator
//		-- スコア計算のための変数を初期化する
//
//	NOTES
//
//	ARUGMENTS
//	FullText2::SearchInformation cSearchInfo_
//		検索情報クラス
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorTermNodeScore::initializeCalculator(SearchInformation& cSearchInfo_)
{
	if (m_bInitCalculator == false)
	{
		// 初期化する

		// スコア計算に必要な引数を設定する
		m_pCalculator->initialize(m_vecScoreArg);

		// 検索中に値が変化しないものの数を設定する
		cSearchInfo_.initializeScoreArgument(m_vecScoreArg, this);

		// 検索中に値が変化しないものを事前に計算しておく
		m_pCalculator->prepare(m_vecScoreArg);

		// IDF項を計算し、保存する
		m_dblIDF = static_cast<DocumentScore>(
			m_pCalculator->secondStep(m_vecScoreArg));

		// 引数をメンバー変数に割り当てる
		assignArg();

		m_bInitCalculator = true;
	}
}

//
//	FUNCTION private
//	FullText2::OperatorTermNodeScore::assignArg -- スコア計算引数を割り当てる
//
//	NOTES
//
// 	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
OperatorTermNodeScore::assignArg()
{
	// 引数を割り当てる
	ModVector<ScoreCalculator::Argument>::Iterator i = m_vecScoreArg.begin();
	for (; i < m_vecScoreArg.end(); ++i)
	{
		if ((*i).m_eType == ScoreCalculator::Argument::TermFrequency)
			// 文書内頻度
			m_pTermFrequency = &(*i);
		else if ((*i).m_eType == ScoreCalculator::Argument::DocumentLength)
			// 文書長
			m_pDocumentLength = &(*i);
	}
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
