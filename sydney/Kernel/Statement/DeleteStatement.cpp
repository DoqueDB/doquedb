// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DeleteStatement.cpp -- DeleteStatement
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2007, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/DeleteStatement.h"
#include "Statement/Type.h"
#ifdef OBSOLETE
#include "Statement/CursorName.h"
#endif
#include "Statement/Identifier.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#ifdef OBSOLETE
#include "Analysis/DeleteStatement_Positioned.h"
#endif
#include "Analysis/DeleteStatement_Searched.h"
#endif
#include "Analysis/Operation/Delete.h"

#include "ModOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_TableName,
		f_SearchCondition,
		f_CorrelationName,
#ifdef OBSOLETE
		f_CursorName,
#endif
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::DeleteStatement::DeleteStatement -- コンストラクタ (1)
//
//	NOTES
//
//	ARGUMENTS
//		Identifier* pTableName_
//		ValueExpression* pSearchCondition_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DeleteStatement::DeleteStatement(Identifier* pTableName_,
								 ValueExpression* pSearchCondition_)
	: Object(ObjectType::DeleteStatement, f__end_index, Object::Optimize, true)
{
	// If name begins from '#', object type become special type.
	if (pTableName_->getToken().getLength() > 0
		&& *(pTableName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryDeleteStatement);
	}
	// TableName を設定する
	setTableName(pTableName_);
	// SearchCondition を設定する
	setSearchCondition(pSearchCondition_);
}

//
//	FUNCTION public
//		Statement::DeleteStatement::DeleteStatement -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pTableName_
//		ValueExpression* pSearchCondition_
//		Identifier* pCorrelationName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DeleteStatement::DeleteStatement(Identifier* pTableName_,
								 ValueExpression* pSearchCondition_,
								 Identifier* pCorrelationName_)
	: Object(ObjectType::DeleteStatement, f__end_index, Object::Optimize, true)
{
	// If name begins from '#', object type become special type.
	if (pTableName_->getToken().getLength() > 0
		&& *(pTableName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryDeleteStatement);
	}
	// TableName を設定する
	setTableName(pTableName_);
	// SearchCondition を設定する
	setSearchCondition(pSearchCondition_);
	// CorrelationName を設定する
	setCorrelationName(pCorrelationName_);
}

//
//	FUNCTION public
//		Statement::DeleteStatement::getTableName -- TableName を得る
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
DeleteStatement::getTableName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_TableName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::DeleteStatement::setTableName -- TableName を設定する
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
DeleteStatement::setTableName(Identifier* pTableName_)
{
	m_vecpElements[f_TableName] = pTableName_;
}

//
//	FUNCTION public
//		Statement::DeleteStatement::getSearchCondition -- SearchCondition を得る
//
//	NOTES
//		SearchCondition を得る
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
DeleteStatement::getSearchCondition() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_SearchCondition];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::DeleteStatement::setSearchCondition
//				-- SearchCondition を設定する
//
//	NOTES
//		SearchCondition を設定する
//
//	ARGUMENTS
//		ValueExpression* pSearchCondition_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DeleteStatement::setSearchCondition(ValueExpression* pSearchCondition_)
{
	m_vecpElements[f_SearchCondition] = pSearchCondition_;
}

//
//	FUNCTION public
//		Statement::DeleteStatement::getCorrelationName -- CorrelationName を得る
//
//	NOTES
//		CorrelationName を得る
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
DeleteStatement::getCorrelationName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_CorrelationName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::DeleteStatement::setCorrelationName -- CorrelationName を設定する
//
//	NOTES
//		CorrelationName を設定する
//
//	ARGUMENTS
//		Identifier* pCorrelationName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DeleteStatement::setCorrelationName(Identifier* pCorrelationName_)
{
	m_vecpElements[f_CorrelationName] = pCorrelationName_;
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::DeleteStatement_Searched _analyzerSearched;
#ifdef OBSOLETE
	Analysis::DeleteStatement_Positioned _analyzerPositioned;
#endif
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DeleteStatement::
getAnalyzer() const
{
#ifdef OBSOLETE
	if (getCursorName())
		return &_analyzerPositioned;
#endif
	return &_analyzerSearched;
}
#endif

// FUNCTION public
//	Statement::DeleteStatement::getAnalyzer2 -- 
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
DeleteStatement::
getAnalyzer2() const
{
	return Analysis::Operation::Delete::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2005, 2006, 2007, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
