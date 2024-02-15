// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryExpression.cpp -- QueryExpression
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2008, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/QueryExpression.h"
#include "Statement/Type.h"
#include "Statement/QueryExpression.h"
#include "Statement/QueryOperator.h"
#include "Statement/SortSpecificationList.h"
#include "Statement/LimitSpecification.h"
#include "Statement/Hint.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/QueryExpression_SetOperator.h"
#include "Analysis/QueryExpression_NoSetOperator.h"
#endif

#include "Analysis/Query/QueryExpression.h"

#include "ModOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_QueryTerm,
		f_Operator,
		f_Left,
		f_SortSpecification,
		f_LimitSpecification,
		f_Hint,
		f__end_index
	};
}


//
//	FUNCTION 
//		Statement::QueryExpression::QueryExpression -- コンストラクタ (1) set operatorなし
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object* pQueryTerm_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

QueryExpression::QueryExpression(Object* pQueryTerm_)
	: Object(ObjectType::QueryExpression, f__end_index)
{
	setQueryTerm(pQueryTerm_);
}

//
//	FUNCTION 
//		Statement::QueryExpression::QueryExpression -- コンストラクタ (2) set operatorあり
//
//	NOTES
//
//	ARGUMENTS
//		QueryOperator* pOperator_
//		QueryExpression* pLeft_
//		Object* pRight_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QueryExpression::QueryExpression(QueryOperator* pOperator_, QueryExpression* pLeft_, Object* pRight_)
	: Object(ObjectType::QueryExpression, f__end_index)
{
	setOperator(pOperator_);
	setLeft(pLeft_);
	setQueryTerm(pRight_);
}

//
//	FUNCTION 
//		Statement::QueryExpression::~QueryExpression -- デストラクタ
//
//	NOTES
//		デストラクタ
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
QueryExpression::~QueryExpression()
{
}

//
//	FUNCTION public
//		Statement::QueryExpression::getQueryTerm -- QueryTerm を得る
//
//	NOTES
//		QueryTerm を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
QueryExpression::getQueryTerm() const
{
	return m_vecpElements[f_QueryTerm];
}

//
//	FUNCTION public
//		Statement::QueryExpression::setQueryTerm -- QueryTerm を設定する
//
//	NOTES
//		QueryTerm を設定する
//
//	ARGUMENTS
//		QueryTerm* pQueryTerm_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryExpression::setQueryTerm(Object* pQueryTerm_)
{
	m_vecpElements[f_QueryTerm] = pQueryTerm_;
}

//	FUNCTION public
//	Statement::QueryExpression::getOperator -- Operator を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		QueryOperator*
//
//	EXCEPTIONS

QueryOperator*
QueryExpression::getOperator() const
{
	QueryOperator* pResult = 0;
	Object* pObj = m_vecpElements[f_Operator];
	if ( pObj && ObjectType::QueryOperator == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(QueryOperator*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QueryExpression::setOperator -- Operator を設定する
//
//	NOTES
//		Operator を設定する
//
//	ARGUMENTS
//		QueryOperator* pOperator_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryExpression::setOperator(QueryOperator* pOperator_)
{
	m_vecpElements[f_Operator] = pOperator_;
}

//
//	FUNCTION public
//		Statement::QueryExpression::getLeft -- Left を得る
//
//	NOTES
//		Left を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		QueryExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QueryExpression*
QueryExpression::getLeft() const
{
	QueryExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Left];
	if ( pObj && ObjectType::QueryExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(QueryExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QueryExpression::setLeft -- Left を設定する
//
//	NOTES
//		Left を設定する
//
//	ARGUMENTS
//		QueryExpression* pLeft_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryExpression::setLeft(QueryExpression* pLeft_)
{
	m_vecpElements[f_Left] = pLeft_;
}

//
//	FUNCTION public
//		Statement::QueryExpression::getSortSpecification -- SortSpecification を得る
//
//	NOTES
//		SortSpecification を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		SortSpecificationList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SortSpecificationList*
QueryExpression::getSortSpecification() const
{
	SortSpecificationList* pResult = 0;
	Object* pObj = m_vecpElements[f_SortSpecification];
	if ( pObj && ObjectType::SortSpecificationList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(SortSpecificationList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QueryExpression::setSortSpecification -- SortSpecification を設定する
//
//	NOTES
//		SortSpecification を設定する
//
//	ARGUMENTS
//		SortSpecificationList* pSortSpecification_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryExpression::setSortSpecification(SortSpecificationList* pSortSpecification_)
{
	m_vecpElements[f_SortSpecification] = pSortSpecification_;
}

//
//	FUNCTION public
//		Statement::QueryExpression::getLimitSpecification -- LimitSpecification を得る
//
//	NOTES
//		LimitSpecification を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		LimitSpecificationList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
LimitSpecification*
QueryExpression::getLimitSpecification() const
{
	LimitSpecification* pResult = 0;
	Object* pObj = m_vecpElements[f_LimitSpecification];
	if ( pObj && ObjectType::LimitSpecification == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(LimitSpecification*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QueryExpression::setLimitSpecification -- LimitSpecification を設定する
//
//	NOTES
//		LimitSpecification を設定する
//
//	ARGUMENTS
//		LimitSpecificationList* pLimitSpecification_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryExpression::setLimitSpecification(LimitSpecification* pLimitSpecification_)
{
	m_vecpElements[f_LimitSpecification] = pLimitSpecification_;
}

//
//	FUNCTION public
//		Statement::QueryExpression::getHint -- Hint を得る
//
//	NOTES
//		Hint を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		HintList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Hint*
QueryExpression::getHint() const
{
	Hint* pResult = 0;
	Object* pObj = m_vecpElements[f_Hint];
	if ( pObj && ObjectType::Hint == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Hint*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QueryExpression::setHint -- Hint を設定する
//
//	NOTES
//		Hint を設定する
//
//	ARGUMENTS
//		HintList* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryExpression::setHint(Hint* pHint_)
{
	m_vecpElements[f_Hint] = pHint_;
}

//
//	FUNCTION public
//	Statement::QueryExpression::copy -- 自身をコピーする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Statement::Object*
//
//	EXCEPTIONS
//	なし
//
Object*
QueryExpression::copy() const
{
	return new QueryExpression(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::QueryExpression_SetOperator _analyzerOperator;
	Analysis::QueryExpression_NoSetOperator _analyzerNoOperator;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
QueryExpression::
getAnalyzer() const
{
	// set operatorがあるか
	if (m_vecpElements[f_Operator]) {
		return &_analyzerOperator;
	}
	return &_analyzerNoOperator;
}
#endif

// FUNCTION public
//	Statement::QueryExpression::getAnalyzer2 -- 
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
QueryExpression::
getAnalyzer2() const
{
	return Analysis::Query::QueryExpression::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2008, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
