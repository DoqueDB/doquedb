// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SimilarPredicate.cpp -- SIMILAR 述語関連の関数定義
// 
// Copyright (c) 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/SimilarPredicate.h"
#include "Statement/Type.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#ifdef USE_OLDER_VERSION
#include "Analysis/SimilarPredicate.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{

namespace _SimilarPredicate
{
	namespace _Member
	{
		enum Value
		{
			String,
			Pattern,
			Escape,
			Language,
			ValueNum
		};
	}
}

}

//	FUNCTION public
//	Statement::SimilarPredicate::SimilarPredicate --
//		SIMILAR 述語を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	s
//			演算対照の文字列を計算する式を表すクラスを
//			格納する領域の先頭アドレス
//		Statement::ValueExpression* pattern
//			パターンを計算する式を表すクラスを格納する領域の先頭アドレス
//		Statement::ValueExpression*	escape
//			0 以外の値
//				エスケープ文字指定を計算する式を表すクラスを
//				格納する領域の先頭アドレス
//			0
//				エスケープ文字指定を計算する式が指定されなかった
//		Statement::ValueExpression*	language
//			0 以外の値
//				言語指定を計算する式を表すクラスを格納する領域の先頭アドレス
//			0
//				言語指定を計算する式が指定されなかった
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

SimilarPredicate::SimilarPredicate(
	ValueExpression* s, ValueExpression* pattern,
	ValueExpression* escape, ValueExpression* language)
	: Object(ObjectType::SimilarPredicate, _SimilarPredicate::_Member::ValueNum)
{
	; _SYDNEY_ASSERT(s);
	; _SYDNEY_ASSERT(pattern);

	// 演算対照の文字列を設定する
	setString(s);
	// パターンを設定する
	setPattern(pattern);
	// エスケープ文字指定を設定する
	setEscape(escape);
	// 言語指定を設定する
	setLanguage(language);
};

//	FUNCTION public
//	Statement::SimilarPredicate::getString -- 演算対照の文字列を得る
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
SimilarPredicate::getString() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_SimilarPredicate::_Member::String,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::SimilarPredicate::setString -- 演算対照の文字列を設定する
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
SimilarPredicate::setString(ValueExpression* s)
{
	setElement(_SimilarPredicate::_Member::String, s);
}

//	FUNCTION public
//	Statement::SimilarPredicate::getPattern -- パターンを得る
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

ValueExpression*
SimilarPredicate::getPattern() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_SimilarPredicate::_Member::Pattern,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::SimilarPredicate::setPattern -- パターンを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	pattern
//			設定するパターンを計算する式が格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
SimilarPredicate::setPattern(ValueExpression* pattern)
{
	setElement(_SimilarPredicate::_Member::Pattern, pattern);
}

//	FUNCTION public
//	Statement::SimilarPredicate::getEscape -- エスケープ文字指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたエスケープ文字指定を計算する式が
//		格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
SimilarPredicate::getEscape() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_SimilarPredicate::_Member::Escape,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::SimilarPredicate::setEscape -- エスケープ文字指定を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	escape
//			設定するエスケープ文字指定を計算する式が
//			格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
SimilarPredicate::setEscape(ValueExpression* escape)
{
	setElement(_SimilarPredicate::_Member::Escape, escape);
}

//	FUNCTION public
//	Statement::SimilarPredicate::getLanguage -- 言語指定の文字列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた言語指定を計算する式が格納されている領域の先頭アドレス
//
//	EXCEPTIONS
//		なし

ValueExpression*
SimilarPredicate::getLanguage() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(_SimilarPredicate::_Member::Language,
				   ObjectType::ValueExpression));
}

//	FUNCTION public
//	Statement::SimilarPredicate::setLanguage -- 言語指定を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::ValueExpression*	language
//			設定する言語指定を計算する式が格納されている領域の先頭アドレス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
SimilarPredicate::setLanguage(ValueExpression* language)
{
	setElement(_SimilarPredicate::_Member::Language, language);
}

//	FUNCTION public
//	Statement::SimilarPredicate::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		自身をコピーして生成された SIMILAR 述語を表すクラスを
//		格納する領域の先頭アドレス
//
//	EXCEPTIONS

Object*
SimilarPredicate::copy() const
{
	return new SimilarPredicate(*this);
}

// FUNCTION public
//	Statement::SimilarPredicate::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
SimilarPredicate::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream cStream;
	; _SYDNEY_ASSERT(getString());
	; _SYDNEY_ASSERT(getPattern());

	cStream << getString()->toSQLStatement(bForCascade_)
			<< " similar to "
			<< getPattern()->toSQLStatement(bForCascade_);

	if (ValueExpression* pEscape = getEscape()) {
		cStream << " escape " << pEscape->toSQLStatement(bForCascade_);
	}
	if (ValueExpression* pLanguage = getLanguage()) {
		cStream << " language " << pLanguage->toSQLStatement(bForCascade_);
	}
	return cStream.getString();
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::SimilarPredicate _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SimilarPredicate::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2005, 2007, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
