// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsPredicate.h -- CONTAINS 述語関連のクラス定義、関数宣言
// 
// Copyright (c) 2004, 2005, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_CONTAINSPREDICATE_H
#define __SYDNEY_STATEMENT_CONTAINSPREDICATE_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class ContainsOperand;
class ContainsOption;
class ValueExpression;

//	CLASS
//	Statement::ContainsPredicate -- CONTAINS 述語を表すクラス
//
//	NOTES

class SYD_STATEMENT_FUNCTION ContainsPredicate
	: public Object
{
public:
	//constructor
	ContainsPredicate()
		: Object(ObjectType::ContainsPredicate)
	{}
	// コンストラクタ
	ContainsPredicate(ValueExpression* s, ContainsOperand* pattern,
					  ContainsOption* option);

	// 演算対象の文字列を得る
	ValueExpression*
	getString() const;
	// 演算対象の文字列を設定する
	void
	setString(ValueExpression* s);

	// パターンを得る
	ContainsOperand*
	getPattern() const;
	// パターンを設定する
	void
	setPattern(ContainsOperand* pattern);

	// オプションを得る
	ContainsOption*
	getOption() const;
	// オプションを設定する
	void
	setOption(ContainsOption* option);

	// 自分をコピーする
	Object*
	copy() const;

#ifdef USE_OLDER_VERSION
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_CONTAINSPREDICATE_H

//
// Copyright (c) 2004, 2005, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
