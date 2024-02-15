// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::SelectStatement -- SelectStatement
// 
// Copyright (c) 1999, 2002, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_SELECTSTATEMENT_H
#define __SYDNEY_STATEMENT_SELECTSTATEMENT_H

#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{
	class QuerySpecification;
	
//
//	CLASS
//		SelectStatement -- SelectStatement
//
//	NOTES
//		SelectStatement
//
class SYD_STATEMENT_FUNCTION SelectStatement : public Statement::Object
{
public:
	//constructor
	SelectStatement()
		: Object(ObjectType::SelectStatement)
	{}
	// コンストラクタ (2)
	explicit SelectStatement(QuerySpecification* pQuerySpecification_);

	// アクセサ
	// QuerySpecification を得る
	QuerySpecification* getQuerySpecification() const;
	// QuerySpecification を設定する
	void setQuerySpecification(QuerySpecification* pQuerySpecification_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	SelectStatement& operator=(const SelectStatement& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_SELECTSTATEMENT_H

//
// Copyright (c) 1999, 2002, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
