// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DerivedColumn.cpp -- DerivedColumn
// 
// Copyright (c) 1999, 2002, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/DerivedColumn.h"
#include "Statement/Type.h"
#include "Statement/ColumnName.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/DerivedColumn.h"
#endif
#include "Analysis/Value/DerivedColumn.h"

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_ValueExpression,
		f_ColumnName,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::DerivedColumn::DerivedColumn -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ValueExpression* pValuExpression_
//		ColumnName* pColumnName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
DerivedColumn::DerivedColumn(ValueExpression* pValueExpression_, ColumnName* pColumnName_)
	: Object(ObjectType::DerivedColumn, f__end_index)
{
	// ValueExpression を設定する
	setValueExpression(pValueExpression_);
	// ColumnName を設定する
	setColumnName(pColumnName_);
}

// FUNCTION public
//	Statement::DerivedColumn::getExpressionType -- ExpressionType を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
DerivedColumn::
getExpressionType() const
{
	if (getValueExpression()) {
		return getValueExpression()->getExpressionType();
	}
	return ValueExpression::type_Unknown;
}

//
//	FUNCTION public
//		Statement::DerivedColumn::toSQLStatement -- SQL文を得る
//
//	NOTES
//		SQL文の文字列を得る。ただし、完全にSQL文を再構成するわけではない。
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ModUnicodeString
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
// SQL文で値を得る
//virtual
ModUnicodeString
DerivedColumn::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	// 別名の部分は内容の同一性に関係しない
	return getValueExpression()->toSQLStatement(bForCascade_);
}

//
//	FUNCTION public
//		Statement::DerivedColumn::getValueExpression -- ValueExpression を得る
//
//	NOTES
//		ValueExpression を得る
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
DerivedColumn::getValueExpression() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_ValueExpression];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::DerivedColumn::setValueExpression -- ValueExpression を設定する
//
//	NOTES
//		ValueExpression を設定する
//
//	ARGUMENTS
//		ValueExpression* pValueExpression_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
DerivedColumn::setValueExpression(ValueExpression* pValueExpression_)
{
	m_vecpElements[f_ValueExpression] = pValueExpression_;
}

//
//	FUNCTION public
//		Statement::DerivedColumn::getColumnName -- ColumnName を得る
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
DerivedColumn::getColumnName() const
{
	ColumnName* pResult = 0;
	Object* pObj = m_vecpElements[f_ColumnName];
	if ( pObj && ObjectType::ColumnName == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnName*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::DerivedColumn::setColumnName -- ColumnName を設定する
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
DerivedColumn::setColumnName(ColumnName* pColumnName_)
{
	m_vecpElements[f_ColumnName] = pColumnName_;
}

//
//	FUNCTION public
//	Statement::DerivedColumn::copy -- 自身をコピーする
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
DerivedColumn::copy() const
{
	return new DerivedColumn(*this);
}

#if 0
namespace
{
	Analysis::DerivedColumn _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
DerivedColumn::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::DerivedColumn::getAnalyzer2 -- 
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
DerivedColumn::
getAnalyzer2() const
{
	return Analysis::Value::DerivedColumn::create(this);
}

//
//	Copyright (c) 1999, 2002, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
