// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsPredicate.cpp -- CONTAINS 述語関連の関数定義
// 
// Copyright (c) 2004, 2005, 2008, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ContainsPredicate.h"
#include "Statement/ContainsOperand.h"
#include "Statement/ContainsOption.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/ContainsPredicate.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{

namespace _ContainsPredicate
{
	namespace _Member
	{
		enum Value
		{
			String,
			Pattern,
			Option,
			ValueNum
		};
	}
}

}

//	FUNCTION public
//	Statement::ContainsPredicate::ContainsPredicate --
//		CONTAINS 述語を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			演算対照の文字列を計算する式を表すクラスを
//			格納する領域の先頭アドレス
//		Statement::ContainsOperand* pattern
//			パターンを計算する式を表すクラスを格納する領域の先頭アドレス
//		Statement::Expand*	expand
//			0 以外の値
//				拡張指定
//			0
//				拡張なし
//		Statement::ValueExpression*	extractor
//			0 以外の値
//				抽出パラメーター
//			0
//				抽出パラメーター指定なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

ContainsPredicate::ContainsPredicate(
	ValueExpression* s, ContainsOperand* pattern,
	ContainsOption* option)
	: Object(ObjectType::ContainsPredicate, _ContainsPredicate::_Member::ValueNum)
{
	; _SYDNEY_ASSERT(s);
	; _SYDNEY_ASSERT(pattern);

	// 演算対照の文字列を設定する
	setString(s);
	// パターンを設定する
	setPattern(pattern);
	// オプションを設定する
	setOption(option);
};

//	FUNCTION public
//	Statement::ContainsPredicate::getString -- 演算対照の文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた演算対照の文字列を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
ContainsPredicate::getString() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_ContainsPredicate::_Member::String,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::ContainsPredicate::setString -- 演算対照の文字列を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			設定する演算対照の文字列を計算する式が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsPredicate::setString(ValueExpression* s)
{
	setElement(_ContainsPredicate::_Member::String, s);
}

//	FUNCTION public
//	Statement::ContainsPredicate::getPattern -- パターンを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたパターンを計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ContainsOperand*
ContainsPredicate::getPattern() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ContainsOperand*,
		getElement(_ContainsPredicate::_Member::Pattern,
				   ObjectType::ContainsOperand));
}

//	FUNCTION public
//	Statement::ContainsPredicate::setPattern -- パターンを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ContainsOperand*	pattern
//			設定するパターンを計算する式が格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsPredicate::setPattern(ContainsOperand* pattern)
{
	setElement(_ContainsPredicate::_Member::Pattern, pattern);
}

//	FUNCTION public
//	Statement::ContainsPredicate::getOption -- オプションを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		CONTAINS述語のオプションが格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ContainsOption*
ContainsPredicate::getOption() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ContainsOption*,
		getElement(_ContainsPredicate::_Member::Option,
				   ObjectType::ContainsOption));
}

//	FUNCTION public
//	Statement::ContainsPredicate::setOption -- オプションを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ContainsOption*	s
//			CONTAINS述語のオプションが
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
ContainsPredicate::setOption(ContainsOption* option)
{
	setElement(_ContainsPredicate::_Member::Option, option);
}

//	FUNCTION public
//	Statement::ContainsPredicate::copy -- 自身をコピーする
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
ContainsPredicate::copy() const
{
	return new ContainsPredicate(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::ContainsPredicate _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ContainsPredicate::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2004, 2005, 2008, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
