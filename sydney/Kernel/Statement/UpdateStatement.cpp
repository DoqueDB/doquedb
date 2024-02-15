// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UpdateStatement.cpp -- UpdateStatement
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/UpdateStatement.h"
#include "Statement/Type.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"
#include "Statement/UpdateSetClauseList.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#ifdef OBSOLETE
#include "Analysis/UpdateStatement_Positioned.h"
#endif
#include "Analysis/UpdateStatement_Searched.h"
#endif

#include "Analysis/Operation/Update.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_TargetTable,
		f_SetClauseList,
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
//		Statement::UpdateStatement::UpdateStatement -- コンストラクタ (1)
//
//	NOTES
//
//	ARGUMENTS
//		Identifier* pTargetTable_
//		UpdateSetClauseList* pSetClauseList_
//		ValueExpression* pSearchCondition_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
UpdateStatement::UpdateStatement(Identifier* pTargetTable_,
								 UpdateSetClauseList* pSetClauseList_,
								 ValueExpression* pSearchCondition_)
		: Object(ObjectType::UpdateStatement, f__end_index, Object::Optimize, true)
{
	// If name begins from '#', object type become special type.
	if (pTargetTable_->getToken().getLength() > 0
		&& *(pTargetTable_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryUpdateStatement);
	}
	// TargetTable を設定する
	setTargetTable(pTargetTable_);
	// SetClauseList を設定する
	setSetClauseList(pSetClauseList_);
	// SearchCondition を設定する
	setSearchCondition(pSearchCondition_);
}

//
//	FUNCTION public
//		Statement::UpdateStatement::UpdateStatement -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pTargetTable_
//		UpdateSetClauseList* pSetClauseList_
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
UpdateStatement::UpdateStatement(Identifier* pTargetTable_,
								 UpdateSetClauseList* pSetClauseList_,
								 ValueExpression* pSearchCondition_,
								 Identifier* pCorrelationName_)
		: Object(ObjectType::UpdateStatement, f__end_index, Object::Optimize, true)
{
	// If name begins from '#', object type become special type.
	if (pTargetTable_->getToken().getLength() > 0
		&& *(pTargetTable_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryUpdateStatement);
	}
	// TargetTable を設定する
	setTargetTable(pTargetTable_);
	// SetClauseList を設定する
	setSetClauseList(pSetClauseList_);
	// SearchCondition を設定する
	setSearchCondition(pSearchCondition_);
	// CorrelationName を設定する
	setCorrelationName(pCorrelationName_);
}

//
//	FUNCTION public
//		Statement::UpdateStatement::getTargetTable -- TargetTable を得る
//
//	NOTES
//		TargetTable を得る
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
UpdateStatement::getTargetTable() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_TargetTable];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::UpdateStatement::setTargetTable -- TargetTable を設定する
//
//	NOTES
//		TargetTable を設定する
//
//	ARGUMENTS
//		Identifier* pTargetTable_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
UpdateStatement::setTargetTable(Identifier* pTargetTable_)
{
	m_vecpElements[f_TargetTable] = pTargetTable_;
}

//
//	FUNCTION public
//		Statement::UpdateStatement::getSearchCondition -- SearchCondition を得る
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
UpdateStatement::getSearchCondition() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_SearchCondition];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::UpdateStatement::setSearchCondition -- SearchCondition を設定する
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
UpdateStatement::setSearchCondition(ValueExpression* pSearchCondition_)
{
	m_vecpElements[f_SearchCondition] = pSearchCondition_;
}

//
//	FUNCTION public
//		Statement::UpdateStatement::getSetClauseList -- SetClauseList を得る
//
//	NOTES
//		SetClauseList を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		UpdateSetClauseList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
UpdateSetClauseList*
UpdateStatement::getSetClauseList() const
{
	UpdateSetClauseList* pResult = 0;
	Object* pObj = m_vecpElements[f_SetClauseList];
	if ( pObj && ObjectType::UpdateSetClauseList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(UpdateSetClauseList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::UpdateStatement::setSetClauseList -- SetClauseList を設定する
//
//	NOTES
//		SetClauseList を設定する
//
//	ARGUMENTS
//		UpdateSetClauseList* pSetClauseList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
UpdateStatement::setSetClauseList(UpdateSetClauseList* pSetClauseList_)
{
	m_vecpElements[f_SetClauseList] = pSetClauseList_;
}

//
//	FUNCTION public
//		Statement::UpdateStatement::getCorrelationName -- CorrelationName を得る
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
UpdateStatement::getCorrelationName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_CorrelationName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::UpdateStatement::setCorrelationName -- CorrelationName を設定する
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
UpdateStatement::setCorrelationName(Identifier* pCorrelationName_)
{
	m_vecpElements[f_CorrelationName] = pCorrelationName_;
}

//
//	FUNCTION public
//	Statement::UpdateStatement::copy -- 自身をコピーする
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
UpdateStatement::copy() const
{
	return new UpdateStatement(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
#ifdef OBSOLETE
	Analysis::UpdateStatement_Positioned _analyzerPositioned;
#endif
	Analysis::UpdateStatement_Searched _analyzerSearched;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
UpdateStatement::
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
//	Statement::UpdateStatement::getAnalyzer2 -- 
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
UpdateStatement::
getAnalyzer2() const
{
	return Analysis::Operation::Update::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2005, 2006, 2007, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
