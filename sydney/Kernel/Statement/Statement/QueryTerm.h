// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::QueryTerm -- QueryTerm
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

#ifndef __SYDNEY_STATEMENT_QUERYTERM_H
#define __SYDNEY_STATEMENT_QUERYTERM_H

#include "Statement/Object.h"

_SYDNEY_BEGIN

namespace Statement
{
	class QueryOperator;
	class QueryTerm;
	
//
//	CLASS
//		QueryTerm -- QueryTerm
//
//	NOTES
//		QueryTerm
//
class SYD_STATEMENT_FUNCTION QueryTerm : public Statement::Object
{
public:
	//constructor
	QueryTerm()
		: Object(ObjectType::QueryTerm)
	{}
	// コンストラクタ (2)
	explicit QueryTerm(Statement::Object* pQueryPrimary_);
	// デストラクタ
	virtual ~QueryTerm();

	// アクセサ
	// QueryPrimary を得る
	Statement::Object* getQueryPrimary() const;
	// QueryPrimary を設定する
	void setQueryPrimary(Statement::Object* pQueryPrimary_);

	//自身をコピーする
	Object* copy() const;

#if 0
	// Analyzerを得る
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif

protected:
private:
	// 代入オペレーターは使わない
	QueryTerm& operator=(const QueryTerm& cOther_);

	// メンバ変数
};

} // namescape Statement

_SYDNEY_END

#endif // __SYDNEY_STATEMENT_QUERYTERM_H

//
// Copyright (c) 1999, 2002, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
