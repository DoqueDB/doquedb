// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsOption.cpp -- CONTAINS 述語関連の関数定義
// 
// Copyright (c) 2004, 2005, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Statement";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/ContainsOption.h"
#include "Statement/ContainsOperand.h"
#include "Statement/Expand.h"
#include "Statement/QueryExpression.h"
#include "Statement/ValueExpression.h"

#include "Common/Message.h"

#include "Exception/SQLSyntaxError.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{

namespace _ContainsOption
{
	namespace _Member
	{
		enum Value
		{
			Calculator,
			Combiner,
			AverageLength,
			Df,
			ScoreFunction,
			Expand,
			Extractor,
			ClusteredLimit,
			ScoreCombiner,
			TfScale,	// 未使用だが互換性のため維持する
			ClusteredCombiner,
			RankFrom,
			ValueNum
		};
	}
}

}

//	FUNCTION public
//	Statement::ContainsOption::ContainsOption --
//		CONTAINS 述語のオプションを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

ContainsOption::ContainsOption()
	: Object(ObjectType::ContainsOption, _ContainsOption::_Member::ValueNum)
{}

//	FUNCTION public
//	Statement::ContainsOption::getCalculator -- 計算器を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた計算器を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsOption::getCalculator() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::Calculator,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsOption::setCalculator -- 計算器を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			設定する計算器を計算する式が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setCalculator(ValueExpression* s)
{
	if (getElement(_ContainsOption::_Member::Calculator,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: calculator" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: calculator"));
	}
	setElement(_ContainsOption::_Member::Calculator, s);
}

//	FUNCTION public
//	Statement::ContainsOption::getCombiner -- 合成器を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた計算器を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsOption::getCombiner() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::Combiner,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsOption::setCombiner -- 合成器を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			設定する計算器を計算する式が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setCombiner(ValueExpression* s)
{
	if (getElement(_ContainsOption::_Member::Combiner,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: combiner" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: combiner"));
	}
	setElement(_ContainsOption::_Member::Combiner, s);
}

//	FUNCTION public
//	Statement::ContainsOption::getAverageLength -- 平均文書長を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた平均文書長を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsOption::getAverageLength() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::AverageLength,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsOption::setAverageLength -- 平均文書長を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			設定する平均文書長を計算する式が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setAverageLength(ValueExpression* s)
{
	if (getElement(_ContainsOption::_Member::AverageLength,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: average length" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: average length"));
	}
	setElement(_ContainsOption::_Member::AverageLength, s);
}

//	FUNCTION public
//	Statement::ContainsOption::getDf -- 文書頻度を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた文書頻度を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsOption::getDf() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::Df,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsOption::setDf -- 文書頻度を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			設定する文書頻度を計算する式が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setDf(ValueExpression* s)
{
	if (getElement(_ContainsOption::_Member::Df,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: df" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: df"));
	}
	setElement(_ContainsOption::_Member::Df, s);
}

//	FUNCTION public
//	Statement::ContainsOption::getScoreFunction -- スコア修正関数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたスコア修正関数を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsOption::getScoreFunction() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::ScoreFunction,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsOption::setScoreFunction -- スコア修正関数を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			設定するスコア修正関数を計算する式が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setScoreFunction(ValueExpression* s)
{
	if (getElement(_ContainsOption::_Member::ScoreFunction,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: score function" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: score function"));
	}
	setElement(_ContainsOption::_Member::ScoreFunction, s);
}

//	FUNCTION public
//	Statement::ContainsOption::getExpand -- 拡張指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた拡張指定が
//		格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

Expand*
ContainsOption::getExpand() const
{
	return _SYDNEY_DYNAMIC_CAST(
		Expand*,
		getElement(_ContainsOption::_Member::Expand,
				   ObjectType::Expand));
}

//	FUNCTION public
//	Statement::ContainsOption::setExpand -- 拡張指定を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Expand*	expand
//			設定する拡張指定が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setExpand(Expand* expand)
{
	if (getElement(_ContainsOption::_Member::Expand,
				   ObjectType::Expand) != 0) {
		SydInfoMessage << "Duplicate contains option: expand" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: expand"));
	}
	setElement(_ContainsOption::_Member::Expand, expand);
}

//	FUNCTION public
//	Statement::ContainsOption::getExtractor -- 抽出パラメーター指定の文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた抽出パラメーター指定を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsOption::getExtractor() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::Extractor,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsOption::setExtractor -- 抽出パラメーター指定を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	extractor
//			設定する抽出パラメーター指定を計算する式が格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setExtractor(ValueExpression* extractor)
{
	if (getElement(_ContainsOption::_Member::Extractor,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: extractor" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: extractor"));
	}
	setElement(_ContainsOption::_Member::Extractor, extractor);
}

//	FUNCTION public
//	Statement::ContainsOption::getClusteredLimit -- クラスター閾値の数値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたクラスター閾値を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsOption::getClusteredLimit() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::ClusteredLimit,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsOption::setClusteredLimit -- クラスター閾値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	clusteredLimit
//			設定するクラスター閾値を計算する式が格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsOption::setClusteredLimit(ValueExpression* clusteredLimit)
{
	if (getElement(_ContainsOption::_Member::ClusteredLimit,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: clustered limit" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: clustered limit"));
	}
	setElement(_ContainsOption::_Member::ClusteredLimit, clusteredLimit);
}

// FUNCTION public
//	Statement::ContainsOption::getScoreCombiner -- スコア合成器を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
ContainsOption::
getScoreCombiner() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::ScoreCombiner,
				   ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::ContainsOption::setScoreCombiner -- スコア合成器を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* comb
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ContainsOption::
setScoreCombiner(ValueExpression* comb)
{
	if (getElement(_ContainsOption::_Member::ScoreCombiner,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: score combiner" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: score combiner"));
	}
	setElement(_ContainsOption::_Member::ScoreCombiner, comb);
}

// FUNCTION public
//	Statement::ContainsOption::getClusteredCombiner -- クラスター合成器を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
ContainsOption::
getClusteredCombiner() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsOption::_Member::ClusteredCombiner,
				   ObjectType::ValueExpression));
}

// FUNCTION public
//	Statement::ContainsOption::setClusteredCombiner -- クラスター合成器を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* comb
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ContainsOption::
setClusteredCombiner(ValueExpression* comb)
{
	if (getElement(_ContainsOption::_Member::ClusteredCombiner,
				   ObjectType::ValueExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: clustered combiner" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: clustered combiner"));
	}
	setElement(_ContainsOption::_Member::ClusteredCombiner, comb);
}

// FUNCTION public
//	Statement::ContainsOption::getRankFrom -- get Rank From specification
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	QueryExpression*
//
// EXCEPTIONS

QueryExpression*
ContainsOption::
getRankFrom() const
{
	return _SYDNEY_DYNAMIC_CAST(
		QueryExpression*,
		getElement(_ContainsOption::_Member::RankFrom,
				   ObjectType::QueryExpression));
}

// FUNCTION public
//	Statement::ContainsOption::setRankFrom -- set Rank From specification
//
// NOTES
//
// ARGUMENTS
//	QueryExpression* rankFrom
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ContainsOption::
setRankFrom(QueryExpression* rankFrom)
{
	if (getElement(_ContainsOption::_Member::RankFrom,
				   ObjectType::QueryExpression) != 0) {
		SydInfoMessage << "Duplicate contains option: rank from" << ModEndl;
		_SYDNEY_THROW1(Exception::SQLSyntaxError, _TRMEISTER_U_STRING("Duplicate contains option: rank from"));
	}
	setElement(_ContainsOption::_Member::RankFrom, rankFrom);
}

//	FUNCTION public
//	Statement::ContainsOption::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自身をコピーして生成された CONTAINS 述語を表すクラスを
//		格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
ContainsOption::copy() const
{
	return new ContainsOption(*this);
}

//
//	Copyright (c) 2004, 2005, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
