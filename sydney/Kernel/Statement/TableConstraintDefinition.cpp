// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TableConstraintDefinition.cpp -- TableConstraintDefinition
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/TableConstraintDefinition.h"
#include "Statement/Type.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Hint.h"
#include "Statement/Identifier.h"
#include "Statement/IndexDefinition.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#if 0
#include "Analysis/TableConstraintDefinition.h"
#endif


_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_ConstraintType,
		f_ColumnNameList,
		f_Clustered,
		f_Hint,
		f_ReferedTableName,
		f_ReferedColumnName,
		f__end_index
	};

	const char* const _pszTypeName[] =
	{
		0, // None
		"primary key",
		"unique",
		"foreign key"
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::TableConstraintDefinition -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		int iType_
//		ColumnNameList* pColumnNameList_
//		int iClustered_
//		Hint* pHint_
//		Identifier* pReferedTableName_
//		ColumnNameList* pReferedColumnName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableConstraintDefinition::TableConstraintDefinition(int iConstraintType_,
													 ColumnNameList* pColumnNameList_, 
													 int iClustered_,
													 Hint* pHint_,
													 Identifier* pReferedTableName_,
													 ColumnNameList* pReferedColumnName_)
	: Object(ObjectType::TableConstraintDefinition, f__end_index, Object::Reorganize)
{
	// ConstraintType を設定する
	setConstraintType(iConstraintType_);
	// ColumnNameList を設定する
	setColumnNameList(pColumnNameList_);
	// Clustered を設定する
	setClustered(iClustered_);
	// Hint を設定する
	setHint(pHint_);
	// ReferedTableNameを設定する
	setReferedTableName(pReferedTableName_);
	// ReferedColumnNameを設定する
	setReferedColumnName(pReferedColumnName_);
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::getConstraintType -- ConstraintType を得る
//
//	NOTES
//		ConstraintType を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
TableConstraintDefinition::getConstraintType() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_ConstraintType];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::setConstraintType -- ConstraintType を設定する
//
//	NOTES
//		ConstraintType を設定する
//
//	ARGUMENTS
//		int iConstraintType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableConstraintDefinition::setConstraintType(int iConstraintType_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_ConstraintType]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_ConstraintType] = pIntVal;
	}
	pIntVal->setValue(iConstraintType_);
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::getColumnNameList -- ColumnNameList を得る
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
TableConstraintDefinition::getColumnNameList() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_ColumnNameList];
	if ( pObj && ObjectType::ColumnNameList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::setColumnNameList -- ColumnNameList を設定する
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
TableConstraintDefinition::setColumnNameList(ColumnNameList* pColumnNameList_)
{
	m_vecpElements[f_ColumnNameList] = pColumnNameList_;
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::getClustered -- Clustered を得る
//
//	NOTES
//		Clustered を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
int
TableConstraintDefinition::getClustered() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_Clustered];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::setClustered -- Clustered を設定する
//
//	NOTES
//		Clustered を設定する
//
//	ARGUMENTS
//		int iClustered_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableConstraintDefinition::setClustered(int iClustered_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_Clustered]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_Clustered] = pIntVal;
	}
	pIntVal->setValue(iClustered_);
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::getHint -- Hint を得る
//
//	NOTES
//		Hint を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Hint*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Hint*
TableConstraintDefinition::getHint() const
{
	return _SYDNEY_DYNAMIC_CAST(Hint*, getElement(f_Hint, ObjectType::Hint));
}

//
//	FUNCTION public
//		Statement::TableConstraintDefinition::setHint -- Hint を設定する
//
//	NOTES
//		Hint を設定する
//
//	ARGUMENTS
//		Hint* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableConstraintDefinition::setHint(Hint* pHint_)
{
	setElement(f_Hint, pHint_);
}

// ReferedTableName を得る
Identifier*
TableConstraintDefinition::
getReferedTableName() const
{
	return _SYDNEY_DYNAMIC_CAST(Identifier*, getElement(f_ReferedTableName, ObjectType::Identifier));
}

// ReferedTableName を設定する
void
TableConstraintDefinition::
setReferedTableName(Identifier* pTableName_)
{
	if (pTableName_) {
		setElement(f_ReferedTableName, pTableName_);
	}
}

// ReferedColumnName を得る
ColumnNameList*
TableConstraintDefinition::
getReferedColumnName() const
{
	return _SYDNEY_DYNAMIC_CAST(ColumnNameList*, getElement(f_ReferedColumnName, ObjectType::ColumnNameList));
}

// ReferedColumnName を設定する
void
TableConstraintDefinition::
setReferedColumnName(ColumnNameList* pColumnName_)
{
	if (pColumnName_) {
		setElement(f_ReferedColumnName, pColumnName_);
	}
}

//
//	FUNCTION public
//	Statement::TableConstraintDefinition::copy -- 自身をコピーする
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
TableConstraintDefinition::copy() const
{
	return new TableConstraintDefinition(*this);
}

// FUNCTION public
//	Statement::TableConstraintDefinition::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
TableConstraintDefinition::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << _pszTypeName[getConstraintType()];
	if (getClustered() == IndexDefinition::Clustered) {
		stream << " clustered";
	}
	stream << '(' << getColumnNameList()->toSQLStatement(bForCascade_) << ')';
	if (getConstraintType() == ForeignKey) {
		stream << " references ";
		stream << (getReferedTableName() ? *getReferedTableName()->getIdentifier() : "(0)");
		if (getReferedColumnName()) {
			stream << '(' << getReferedColumnName()->toSQLStatement(bForCascade_) << ')';
		}
	}
	if (getHint()) {
		stream << getHint()->toSQLStatement(bForCascade_);
	}
	return stream.getString();
}

#if 0
namespace
{
	Analysis::TableConstraintDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
TableConstraintDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
