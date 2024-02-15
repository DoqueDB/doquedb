// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UpdateSetClause.cpp -- UpdateSetClause
// 
// Copyright (c) 1999, 2002, 2006, 2010, 2023 Ricoh Company, Ltd.
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

#include "Statement/UpdateSetClause.h"
#include "Statement/Type.h"
#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/UpdateSetClause.h"
#endif
#include "Analysis/Operation/UpdateSetClause.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_ColumnName,
		f_ColumnNameList,
		f_Source,
		f__end_index
	};
}

// FUNCTION public
//	Statement::UpdateSetClause::UpdateSetClause -- コンストラクタ (1)
//
// NOTES
//
// ARGUMENTS
//	ColumnName* pColumnName_
//	ValueExpression* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

UpdateSetClause::
UpdateSetClause(ColumnName* pColumnName_, ValueExpression* pValue_)
	: Object(ObjectType::UpdateSetClause, f__end_index)
{
	// Column を設定する
	setColumnName(pColumnName_);
	// Source を設定する
	setSource(pValue_);
}

//
//	FUNCTION public
//		Statement::UpdateSetClause::UpdateSetClause -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ColumnNameList* pColumnNameList_
//		ValueExpression* pRowValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
UpdateSetClause::UpdateSetClause(ColumnNameList* pColumnNameList_, ValueExpression* pRowValue_)
	: Object(ObjectType::UpdateSetClause, f__end_index)
{
	// ColumnNameList を設定する
	setColumnNameList(pColumnNameList_);
	// Source を設定する
	setSource(pRowValue_);
}

//
//	FUNCTION public
//		Statement::UpdateSetClause::getColumnName -- ColumnName を得る
//
//	NOTES
//		ColumnName を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ColumnName*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnName*
UpdateSetClause::getColumnName() const
{
	return _SYDNEY_DYNAMIC_CAST(ColumnName*, getElement(f_ColumnName, ObjectType::ColumnName));
}

//
//	FUNCTION public
//		Statement::UpdateSetClause::setColumnName -- ColumnName を設定する
//
//	NOTES
//		ColumnName を設定する
//
//	ARGUMENTS
//		ColumnName* pColumnName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
UpdateSetClause::setColumnName(ColumnName* pColumnName_)
{
	m_vecpElements[f_ColumnName] = pColumnName_;
}

//
//	FUNCTION public
//		Statement::UpdateSetClause::getColumnNameList -- ColumnNameList を得る
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
UpdateSetClause::getColumnNameList() const
{
	return _SYDNEY_DYNAMIC_CAST(ColumnNameList*, getElement(f_ColumnNameList, ObjectType::ColumnNameList));
}

//
//	FUNCTION public
//		Statement::UpdateSetClause::setColumnNameList -- ColumnNameList を設定する
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
UpdateSetClause::setColumnNameList(ColumnNameList* pColumnNameList_)
{
	m_vecpElements[f_ColumnNameList] = pColumnNameList_;
}

//
//	FUNCTION public
//		Statement::UpdateSetClause::getSource -- Source を得る
//
//	NOTES
//		Source を得る
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
UpdateSetClause::getSource() const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getElement(f_Source, ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::UpdateSetClause::setSource -- Source を設定する
//
//	NOTES
//		Source を設定する
//
//	ARGUMENTS
//		ValueExpression* pSource_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
UpdateSetClause::setSource(ValueExpression* pSource_)
{
	m_vecpElements[f_Source] = pSource_;
}

//
//	FUNCTION public
//	Statement::UpdateSetClause::copy -- 自身をコピーする
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
UpdateSetClause::copy() const
{
	return new UpdateSetClause(*this);
}

#if 0
namespace
{
	Analysis::UpdateSetClause _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
UpdateSetClause::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::UpdateSetClause::getAnalyzer2 -- 
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
UpdateSetClause::
getAnalyzer2() const
{
	return Analysis::Operation::UpdateSetClause::create(this);
}


//
//	Copyright (c) 1999, 2002, 2006, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
