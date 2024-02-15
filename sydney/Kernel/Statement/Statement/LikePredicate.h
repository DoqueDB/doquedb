// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LikePredicate.h -- LIKE 述語関連のクラス定義、関数宣言
// 
// Copyright (c) 2002, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_LIKEPREDICATE_H
#define __SYDNEY_STATEMENT_LIKEPREDICATE_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class Literal;
class ValueExpression;

//	CLASS
//	Statement::LikePredicate -- LIKE 述語を表すクラス
//
//	NOTES

class SYD_STATEMENT_FUNCTION LikePredicate
	: public Object
{
public:
	//constructor
	LikePredicate()
		: Object(ObjectType::LikePredicate)
	{}
	// コンストラクタ
	LikePredicate(ValueExpression* s, ValueExpression* pattern,
				  ValueExpression* escape, ValueExpression* language);

	// 演算対象の文字列を得る
	ValueExpression*
	getString() const;
	// 演算対象の文字列を設定する
	void
	setString(ValueExpression* s);

	// パターンを得る
	ValueExpression*
	getPattern() const;
	// パターンを設定する
	void
	setPattern(ValueExpression* pattern);

	// エスケープ文字指定を得る
	ValueExpression*
	getEscape() const;
	// エスケープ文字指定を設定する
	void
	setEscape(ValueExpression* escape);

	// 言語指定を得る
	ValueExpression*
	getLanguage() const;
	// 言語指定を設定する
	void
	setLanguage(ValueExpression* language);

	// 自分をコピーする
	Object*
	copy() const;

	// SQL文で値を得る
	virtual ModUnicodeString toSQLStatement(bool bForCascade_ = false) const;

#ifdef USE_OLDER_VERSION
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_LIKEPREDICATE_H

//
// Copyright (c) 2002, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
