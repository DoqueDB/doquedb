// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsOption.h -- CONTAINS 述語関連のクラス定義、関数宣言
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_CONTAINSOPTION_H
#define __SYDNEY_STATEMENT_CONTAINSOPTION_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Expand;
class QueryExpression;
class ValueExpression;

//	CLASS
//	Statement::ContainsOption -- CONTAINS 述語を表すクラス
//
//	NOTES

class SYD_STATEMENT_FUNCTION ContainsOption
	: public Object
{
public:
	// コンストラクタ
	ContainsOption();

	// 計算器を得る
	ValueExpression*
	getCalculator() const;
	// 計算器を設定する
	void
	setCalculator(ValueExpression* calc);

	// 合成器を得る
	ValueExpression*
	getCombiner() const;
	// 合成器を設定する
	void
	setCombiner(ValueExpression* comb);

	// 平均文書長を得る
	ValueExpression*
	getAverageLength() const;
	// 平均文書長を設定する
	void
	setAverageLength(ValueExpression* ave);

	// 文書頻度を得る
	ValueExpression*
	getDf() const;
	// 文書頻度を設定する
	void
	setDf(ValueExpression* df);

	// スコア修正関数を得る
	ValueExpression*
	getScoreFunction() const;
	// スコア修正関数を設定する
	void
	setScoreFunction(ValueExpression* scoreFunction);

	// 拡張指定を得る
	Expand*
	getExpand() const;
	// 拡張指定を設定する
	void
	setExpand(Expand* expand);

	// 抽出パラメーター指定を得る
	ValueExpression*
	getExtractor() const;
	// 抽出パラメーター指定を設定する
	void
	setExtractor(ValueExpression* extractor);

	// クラスター閾値指定を得る
	ValueExpression*
	getClusteredLimit() const;
	// クラスター閾値指定を設定する
	void
	setClusteredLimit(ValueExpression* climit);

	// スコア合成器を得る
	ValueExpression*
	getScoreCombiner() const;
	// スコア合成器を設定する
	void
	setScoreCombiner(ValueExpression* comb);

	// クラスター合成器を得る
	ValueExpression*
	getClusteredCombiner() const;
	// クラスター合成器を設定する
	void
	setClusteredCombiner(ValueExpression* comb);

	// get Rank From specification
	QueryExpression*
	getRankFrom() const;

	// set Rank From specification
	void
	setRankFrom(QueryExpression* rankFrom);

	// 自分をコピーする
	Object*
	copy() const;
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_CONTAINSOPTION_H

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
