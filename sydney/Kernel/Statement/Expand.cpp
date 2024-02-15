// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Expand.cpp -- Expand
// 
// Copyright (c) 2004, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/Expand.h"
#include "Statement/QueryExpression.h"
#include "Statement/ValueExpression.h"
#include "Statement/SortSpecificationList.h"
#include "Statement/LimitSpecification.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/Expand.h"
#endif
#include "Analysis/Query/Expand.h"

#include "ModOstrStream.h"

_SYDNEY_USING

namespace
{
namespace _Expand
{
	namespace _Member
	{
		enum Value
		{
			SubQuery,
			Order,
			Limit,
			ValueNum
		};
	}
}
}

_SYDNEY_STATEMENT_USING

// コンストラクタ
Expand::
Expand(QueryExpression* subquery,
	   SortSpecificationList* order,
	   LimitSpecification* limit)
	: Object(ObjectType::Expand, _Expand::_Member::ValueNum)
{
	setSubQuery(subquery);
	if (order)
		setSortSpecification(order);
	if (limit)
		setLimitSpecification(limit);
}

// 拡張につかう問い合わせを得る
QueryExpression*
Expand::
getSubQuery() const
{
	return _SYDNEY_DYNAMIC_CAST(
		QueryExpression*,
		getElement(_Expand::_Member::SubQuery,
				   ObjectType::QueryExpression));
}

// 拡張に使う問い合わせを設定する
void
Expand::
setSubQuery(QueryExpression* subquery)
{
	setElement(_Expand::_Member::SubQuery, subquery);
}

// 問い合わせ結果のソート順を得る
SortSpecificationList*
Expand::
getSortSpecification() const
{
	return _SYDNEY_DYNAMIC_CAST(
		SortSpecificationList*,
		getElement(_Expand::_Member::Order,
				   ObjectType::SortSpecificationList));
}

// 問い合わせ結果のソート順を設定する
void
Expand::
setSortSpecification(SortSpecificationList* order)
{
	setElement(_Expand::_Member::Order, order);
}

// LimitSpecificationを得る
LimitSpecification*
Expand::
getLimitSpecification() const
{
	return _SYDNEY_DYNAMIC_CAST(
		LimitSpecification*,
		getElement(_Expand::_Member::Limit,
				   ObjectType::LimitSpecification));
}

// LimitSpecificationを設定する
void
Expand::
setLimitSpecification(LimitSpecification* limit)
{
	setElement(_Expand::_Member::Limit, limit);
}

// 自分をコピーする
Object*
Expand::
copy() const
{
	return new Expand(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::Expand _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
Expand::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::Expand::getAnalyzer2 -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
Expand::
getAnalyzer2() const
{
	return Analysis::Query::Expand::create(this);
}

//
//	Copyright (c) 2004, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
