// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TableExpression.cpp -- TableExpression
// 
// Copyright (c) 1999, 2002, 2006, 2008, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/TableExpression.h"
#include "Statement/Type.h"
#include "Statement/GroupByClause.h"
#include "Statement/HavingClause.h"
#include "Statement/TableReferenceList.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/TableExpression.h"
#endif

#include "Analysis/Query/TableExpression.h"

#include "ModOstrStream.h"

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_FromClause,
		f_WhereClause,
		f_GroupByClause,
		f_HavingClause,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::TableExpression::TableExpression -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		TableReferenceList* pFromClause_
//		ValueExpression* pWhereClause_
//		GroupByClause* pGroupByClause_
//		HavingClause* pHavingClause_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableExpression::TableExpression(TableReferenceList* pFromClause_, ValueExpression* pWhereClause_,
								 GroupByClause* pGroupByClause_, HavingClause* pHavingClause_)
	: Object(ObjectType::TableExpression, f__end_index)
{
	// FromClause を設定する
	setFromClause(pFromClause_);
	// WhereClause を設定する
	setWhereClause(pWhereClause_);
	// GroupByClause を設定する
	setGroupByClause(pGroupByClause_);
	// HavingClause を設定する
	setHavingClause(pHavingClause_);
}

//
//	FUNCTION public
//		Statement::TableExpression::getFromClause -- FromClause を得る
//
//	NOTES
//		FromClause を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		TableReferenceList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableReferenceList*
TableExpression::getFromClause() const
{
	TableReferenceList* pResult = 0;
	Object* pObj = m_vecpElements[f_FromClause];
	if ( pObj && ObjectType::TableReferenceList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(TableReferenceList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableExpression::setFromClause -- FromClause を設定する
//
//	NOTES
//		FromClause を設定する
//
//	ARGUMENTS
//		TableReferenceList* pFromClause_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableExpression::setFromClause(TableReferenceList* pFromClause_)
{
	m_vecpElements[f_FromClause] = pFromClause_;
}

//
//	FUNCTION public
//		Statement::TableExpression::getWhereClause -- WhereClause を得る
//
//	NOTES
//		WhereClause を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
TableExpression::getWhereClause() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_WhereClause];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableExpression::setWhereClause -- WhereClause を設定する
//
//	NOTES
//		WhereClause を設定する
//
//	ARGUMENTS
//		ValueExpression* pWhereClause_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableExpression::setWhereClause(ValueExpression* pWhereClause_)
{
	m_vecpElements[f_WhereClause] = pWhereClause_;
}

//
//	FUNCTION public
//		Statement::TableExpression::getGroupByClause -- GroupByClause を得る
//
//	NOTES
//		GroupByClause を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		GroupByClause*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
GroupByClause*
TableExpression::getGroupByClause() const
{
	GroupByClause* pResult = 0;
	Object* pObj = m_vecpElements[f_GroupByClause];
	if ( pObj && ObjectType::GroupByClause == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(GroupByClause*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableExpression::setGroupByClause -- GroupByClause を設定する
//
//	NOTES
//		GroupByClause を設定する
//
//	ARGUMENTS
//		GroupByClause* pGroupByClause_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableExpression::setGroupByClause(GroupByClause* pGroupByClause_)
{
	m_vecpElements[f_GroupByClause] = pGroupByClause_;
}

//
//	FUNCTION public
//		Statement::TableExpression::getHavingClause -- HavingClause を得る
//
//	NOTES
//		HavingClause を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		HavingClause*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
HavingClause*
TableExpression::getHavingClause() const
{
	HavingClause* pResult = 0;
	Object* pObj = m_vecpElements[f_HavingClause];
	if ( pObj && ObjectType::HavingClause == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(HavingClause*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableExpression::setHavingClause -- HavingClause を設定する
//
//	NOTES
//		HavingClause を設定する
//
//	ARGUMENTS
//		HavingClause* pHavingClause_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableExpression::setHavingClause(HavingClause* pHavingClause_)
{
	m_vecpElements[f_HavingClause] = pHavingClause_;
}

//
//	FUNCTION public
//	Statement::TableExpression::copy -- 自身をコピーする
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
TableExpression::copy() const
{
	return new TableExpression(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::TableExpression _analyzer;
}

// FUNCTION public
//	Statement::TableExpression::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
TableExpression::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::TableExpression::getAnalyzer2 -- 
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
TableExpression::
getAnalyzer2() const
{
	return Analysis::Query::TableExpression::create(this);
}

//
//	Copyright (c) 1999, 2002, 2006, 2008, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
