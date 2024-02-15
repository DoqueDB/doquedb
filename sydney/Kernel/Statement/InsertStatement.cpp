// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InsertStatement.cpp -- InsertStatement
// 
// Copyright (c) 1999, 2002, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/InsertStatement.h"
#include "Statement/Type.h"
#include "Statement/ColumnNameList.h"
#ifdef OBSOLETE
#include "Statement/CursorName.h"
#endif
#include "Statement/Identifier.h"
#include "Statement/QueryExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/InsertStatement_Bulk_NoColumnList.h"
#include "Analysis/InsertStatement_Bulk_ColumnList.h"
#include "Analysis/InsertStatement_Default.h"
#include "Analysis/InsertStatement_SubQuery_NoColumnList.h"
#include "Analysis/InsertStatement_SubQuery_ColumnList.h"
#endif
#include "Analysis/Operation/Insert.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
using namespace Statement;

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_TableName,
#ifdef OBSOLETE
		f_CursorName,
#endif
		f_QueryExpression,
		f_ColumnNameList,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::InsertStatement::InsertStatement -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pTableName_
//		QueryExpression* pQueryExpression_
//		ColumnNameList* pColumnNameList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
InsertStatement::InsertStatement(Identifier* pTableName_,
								 QueryExpression* pQueryExpression_,
								 ColumnNameList* pColumnNameList_)
		: Object(ObjectType::InsertStatement, f__end_index,
				 Object::Optimize, true)
{
	// If name begins from '#', object type become special type.
	if (pTableName_->getToken().getLength() > 0
		&& *(pTableName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryInsertStatement);
	}
	// TableName を設定する
	setTableName(pTableName_);
	// QueryExpression を設定する
	setQueryExpression(pQueryExpression_);
	// ColumnNameList を設定する
	setColumnNameList(pColumnNameList_);
}

#ifdef OBSOLETE
//
//	FUNCTION public
//		Statement::InsertStatement::InsertStatement -- コンストラクタ (3)
//
//	NOTES
//		コンストラクタ (3)
//
//	ARGUMENTS
//		Identifier* pTableName_
//		CursorName* pCursorName_
//		QueryExpression* pQueryExpression_
//		ColumnNameList* pColumnNameList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
InsertStatement::InsertStatement(Identifier* pTableName_, CursorName* pCursorName_,
								 QueryExpression* pQueryExpression_,
								 ColumnNameList* pColumnNameList_)
		: Object(ObjectType::InsertStatement, f__end_index,
				 Object::Optimize, true)
{
	// If name begins from '#', object type become special type.
	if (pTableName_->getToken().getLength() > 0
		&& *(pTableName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryInsertStatement);
	}
	// TableName を設定する
	setTableName(pTableName_);
	// CursorName を設定する
	setCursorName(pCursorName_);
	// QueryExpression を設定する
	setQueryExpression(pQueryExpression_);
	// ColumnNameList を設定する
	setColumnNameList(pColumnNameList_);
}
#endif

//
//	FUNCTION public
//		Statement::InsertStatement::getTableName -- TableName を得る
//
//	NOTES
//		TableName を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Identifier*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Identifier*
InsertStatement::getTableName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_TableName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::InsertStatement::setTableName -- TableName を設定する
//
//	NOTES
//		TableName を設定する
//
//	ARGUMENTS
//		Identifier* pTableName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
InsertStatement::setTableName(Identifier* pTableName_)
{
	m_vecpElements[f_TableName] = pTableName_;
}

//
//	FUNCTION public
//		Statement::InsertStatement::getTableNameString
//			-- TableName を ModUnicodeString で得る
//
//	NOTES
//		TableName を ModUnicodeString で得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeString*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const ModUnicodeString*
InsertStatement::getTableNameString() const
{
	Identifier* pIdentifier = getTableName();
	return pIdentifier ? pIdentifier->getIdentifier() : 0;
}

#ifdef OBSOLETE
//
//	FUNCTION public
//		Statement::InsertStatement::getCursorName -- CursorName を得る
//
//	NOTES
//		CursorName を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		CursorName*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
CursorName*
InsertStatement::getCursorName() const
{
	CursorName* pResult = 0;
	Object* pObj = m_vecpElements[f_CursorName];
	if ( pObj && ObjectType::CursorName == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(CursorName*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::InsertStatement::setCursorName -- CursorName を設定する
//
//	NOTES
//		CursorName を設定する
//
//	ARGUMENTS
//		CursorName* pCursorName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
InsertStatement::setCursorName(CursorName* pCursorName_)
{
	m_vecpElements[f_CursorName] = pCursorName_;
}
#endif

//
//	FUNCTION public
//		Statement::InsertStatement::getQueryExpression -- QueryExpression を得る
//
//	NOTES
//		QueryExpression を得る
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
InsertStatement::getQueryExpression() const
{
	QueryExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_QueryExpression];
	if ( pObj && ObjectType::QueryExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(QueryExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::InsertStatement::setQueryExpression -- QueryExpression を設定する
//
//	NOTES
//		QueryExpression を設定する
//
//	ARGUMENTS
//		QueryExpression* pQueryExpression_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
InsertStatement::setQueryExpression(QueryExpression* pQueryExpression_)
{
	m_vecpElements[f_QueryExpression] = pQueryExpression_;
	if (pQueryExpression_
		&& (pQueryExpression_->getQueryTerm()->getType() == ObjectType::BulkSpecification)) {
		// change object type
		setType(ObjectType::BatchInsertStatement);
	}
}

//
//	FUNCTION public
//		Statement::InsertStatement::getColumnNameList -- ColumnNameList を得る
//
//	NOTES
//		ColumnNameList を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ColumnNameList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnNameList*
InsertStatement::getColumnNameList() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_ColumnNameList];
	if ( pObj && ObjectType::ColumnNameList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::InsertStatement::setColumnNameList -- ColumnNameList を設定する
//
//	NOTES
//		ColumnNameList を設定する
//
//	ARGUMENTS
//		ColumnNameList* pColumnNameList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
InsertStatement::setColumnNameList(ColumnNameList* pColumnNameList_)
{
	m_vecpElements[f_ColumnNameList] = pColumnNameList_;
}

//
//	FUNCTION public
//	Statement::InsertStatement::copy -- 自身をコピーする
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
InsertStatement::copy() const
{
	return new InsertStatement(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::InsertStatement_Bulk_NoColumnList _analyzerBulkNoColumnList;
	Analysis::InsertStatement_Bulk_ColumnList _analyzerBulkColumnList;
	Analysis::InsertStatement_Default _analyzerDefault;
	Analysis::InsertStatement_SubQuery_NoColumnList _analyzerNoColumnList;
	Analysis::InsertStatement_SubQuery_ColumnList _analyzerColumnList;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
InsertStatement::
getAnalyzer() const
{
	if (m_vecpElements[f_QueryExpression]) {
		if (m_vecpElements[f_ColumnNameList]) {
			if (getQueryExpression()->getQueryTerm()->getType() == ObjectType::BulkSpecification) {
				return &_analyzerBulkColumnList;
			} else {
				return &_analyzerColumnList;
			}
		} else {
			if (getQueryExpression()->getQueryTerm()->getType() == ObjectType::BulkSpecification) {
				return &_analyzerBulkNoColumnList;
			} else {
				return &_analyzerNoColumnList;
			}
		}
	}
	return &_analyzerDefault;
}
#endif

// FUNCTION public
//	Statement::InsertStatement::getAnalyzer2 -- 
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
InsertStatement::
getAnalyzer2() const
{
	return Analysis::Operation::Insert::create(this);
}

//
//	Copyright (c) 1999, 2002, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
