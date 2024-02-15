// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OperatorTermNodeScore.h --
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

#ifndef __SYDNEY_FULLTEXT2_OPERATORTERMNODESCORE_H
#define __SYDNEY_FULLTEXT2_OPERATORTERMNODESCORE_H

#include "FullText2/Module.h"
#include "FullText2/OperatorTermNode.h"
#include "FullText2/ScoreCalculator.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::OperatorTermNodeScore -- 末端のノードをあらわすクラス
//
//	NOTES
//
class OperatorTermNodeScore : public OperatorTermNode
{
public:
	// コンストラクタ
	OperatorTermNodeScore(const ModUnicodeString& cString_);
	// デストラクタ
	virtual ~OperatorTermNodeScore();
	// コピーコンストラクタ
	OperatorTermNodeScore(const OperatorTermNodeScore& src_);

	// スコア計算器を設定する
	void setScoreCalculator(ScoreCalculator* pCalculator_);

protected:
	// スコア計算器の初期化を行う
	void initializeCalculator(SearchInformation& cSearchInfo_);
	
	// スコア計算器
	ScoreCalculator* m_pCalculator;

	// スコア計算器の初期化を行ったかどうか
	bool m_bInitCalculator;
	// スコア計算器に渡す引数
	ModVector<ScoreCalculator::Argument> m_vecScoreArg;
	// 文書内頻度
	ScoreCalculator::Argument* m_pTermFrequency;
	// 文書長
	ScoreCalculator::Argument* m_pDocumentLength;

	// IDF項
	DocumentScore m_dblIDF;

private:
	// スコア計算用の引数をメンバー変数に割り当てる
	void assignArg();
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_OPERATORTERMNODESCORE_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
