// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Expand.h -- CONTAINS 述語のEXPAND句関連のクラス定義、関数宣言
// 
// Copyright (c) 2004, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_STATEMENT_EXPAND_H
#define __SYDNEY_STATEMENT_EXPAND_H

#include "Statement/Module.h"
#include "Statement/Object.h"

_SYDNEY_BEGIN
_SYDNEY_STATEMENT_BEGIN

class QueryExpression;
class ValueExpression;
class SortSpecificationList;
class LimitSpecification;

//	CLASS
//	Statement::Expand -- CONTAINS 述語を表すクラス
//
//	NOTES

class SYD_STATEMENT_FUNCTION Expand
	: public Object
{
public:
	//constructor
	Expand()
		: Object(ObjectType::Expand)
	{}
	// コンストラクタ
	Expand(QueryExpression* subquery,
		   SortSpecificationList* order,
		   LimitSpecification* limit);

	// 拡張につかう問い合わせを得る
	QueryExpression* getSubQuery() const;
	// 拡張に使う問い合わせを設定する
	void setSubQuery(QueryExpression* subquery);

	// 問い合わせ結果のソート順を得る
	SortSpecificationList* getSortSpecification() const;
	// 問い合わせ結果のソート順を設定する
	void setSortSpecification(SortSpecificationList* order);

	// LimitSpecification を得る
	LimitSpecification* getLimitSpecification() const;
	// LimitSpecification を設定する
	void setLimitSpecification(LimitSpecification* pLimitSpecification_);

	// 自分をコピーする
	Object* copy() const;

	// Analyzerを得る
#ifdef USE_OLDER_VERSION
	virtual const Analysis::Analyzer* getAnalyzer() const;
#endif
	virtual const Analysis::Interface::IAnalyzer* getAnalyzer2() const;

private:
};

_SYDNEY_STATEMENT_END
_SYDNEY_END

#endif // __SYDNEY_STATEMENT_EXPAND_H

//
// Copyright (c) 2004, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
