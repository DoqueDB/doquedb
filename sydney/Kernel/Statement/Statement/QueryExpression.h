// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Statement::QueryExpression -- QueryExpression
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_QUERYEXPRESSION_H
#define __SYDNEY_STATEMENT_QUERYEXPRESSION_H

#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class QueryExpression;
class QueryOperator;
class SortSpecificationList;
class LimitSpecification;
class Hint;
	
//
//	CLASS
//		QueryExpression -- QueryExpression
//
//	NOTES
//		QueryExpression
//
class SYD_STATEMENT_FUNCTION QueryExpression : public Statement::Object
{
public:
	//constructor
	QueryExpression()
		: Object(ObjectType::QueryExpression)
	{}
	// コンストラクタ (1) set operatorなし
	explicit QueryExpression(Object* pQueryTerm_);
	// コンストラクタ (2) set operatorあり
	QueryExpression(QueryOperator* pOperator_, QueryExpression* pLeft_, Object* pRight_);
	// デストラクタ
	virtual ~QueryExpression();
	
	// アクセサ
	// QueryTerm を得る
	Object* getQueryTerm() const;
	// QueryTerm を設定する
	void setQueryTerm(Object* pQueryTerm_);

	// Operator を得る
	QueryOperator* getOperator() const;
	// Operator を設定する
	void setOperator(QueryOperator* pOperator_);

	// Left を得る
	QueryExpression* getLeft() const;
	// Left を設定する
	void setLeft(QueryExpression* pLeft_);

	// SortSpecification を得る
	SortSpecificationList* getSortSpecification() const;
	// SortSpecification を設定する
	void setSortSpecification(SortSpecificationList* pSortSpecification_);

	// LimitSpecification を得る
	LimitSpecification* getLimitSpecification() const;
	// LimitSpecification を設定する
	void setLimitSpecification(LimitSpecification* pLimitSpecification_);

	// Hint を得る
	Hint* getHint() const;
	// Hint を設定する
	void setHint(Hint* pHint_);

	//自身をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

protected:
private:
	// 代入オペレーターは使わない
	QueryExpression& operator=(const QueryExpression& cOther_);

	// メンバ変数
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_QUERYEXPRESSION_H

//
// Copyright (c) 1999, 2002, 2003, 2006, 2008, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
