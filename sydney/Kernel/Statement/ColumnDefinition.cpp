// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnDefinition.cpp -- ColumnDefinition
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ColumnDefinition.h"
#include "Statement/ColumnConstraintDefinition.h"
#include "Statement/ColumnConstraintDefinitionList.h"
#include "Statement/Hint.h"
#include "Statement/Identifier.h"
#include "Statement/Literal.h"
#include "Statement/Type.h"
#include "Statement/Utility.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/SQLData.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"
#include "ModUnicodeOstrStream.h"

#if 0
#include "Analysis/ColumnDefinition.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_DefaultValue,
		f_ConstValue,
		f_Hint,
		f_Constraints,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::ColumnDefinition -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pName_
//		Common::SQLData* pDataType_
//		ValueExpression* pDefaultValue_
//		Hint* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnDefinition::ColumnDefinition(
	Identifier* pName_, Common::SQLData* pDataType_,
	ValueExpression* pDefaultValue_, ColumnConstraintDefinitionList* pConstraints_,
	ValueExpression* pConstValue_, Hint* pHint_, bool bUseOnUpdate_)
	: Object(ObjectType::ColumnDefinition, f__end_index, Object::Reorganize),
	  m_cDataType(),
	  m_bUseOnUpdate(bUseOnUpdate_)
{
	// Name を設定する
	setName(pName_);
	// DataType を設定する
	setDataType(*pDataType_);
	// DefaultValue を設定する
	setDefaultValue(pDefaultValue_);
	// ConstantValue を設定する
	setConstValue(pConstValue_);
	// Hint を設定する
	setHint(pHint_);
	// Constraint を設定する
	setConstraints(pConstraints_);
}

// コピーコンストラクタ
ColumnDefinition::
ColumnDefinition(const ColumnDefinition& cOther_)
	: Object(cOther_),
	  m_cDataType(cOther_.m_cDataType),
	  m_bUseOnUpdate(cOther_.m_bUseOnUpdate)
{
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::getName -- Name を得る
//
//	NOTES
//		Name を得る
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
ColumnDefinition::getName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::setName -- Name を設定する
//
//	NOTES
//		Name を設定する
//
//	ARGUMENTS
//		Identifier* pName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ColumnDefinition::setName(Identifier* pName_)
{
	m_vecpElements[f_Name] = pName_;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::ColumnDefinition::getNameString -- Name を ModString で得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const ModUnicodeString*
//
//	EXCEPTIONS

const ModUnicodeString*
ColumnDefinition::getNameString() const
{
	Identifier* pIdentifier = getName();
	return pIdentifier ? pIdentifier->getIdentifierString() : 0;
}
#endif

//
//	FUNCTION public
//		Statement::ColumnDefinition::getDataType -- DataType を得る
//
//	NOTES
//		DataType を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		const DataType&
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
const Common::SQLData&
ColumnDefinition::getDataType() const
{
	return m_cDataType;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::setDataType -- DataType を設定する
//
//	NOTES
//		DataType を設定する
//
//	ARGUMENTS
//		const DataType& cDataType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ColumnDefinition::setDataType(const Common::SQLData& cDataType_)
{
	m_cDataType = cDataType_;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::getDefaultValue -- DefaultValue を得る
//
//	NOTES
//		DefaultValue を得る
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
ColumnDefinition::getDefaultValue() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_DefaultValue];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::setDefaultValue -- DefaultValue を設定する
//
//	NOTES
//		DefaultValue を設定する
//
//	ARGUMENTS
//		ValueExpression* pDefaultValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ColumnDefinition::setDefaultValue(ValueExpression* pDefaultValue_)
{
	m_vecpElements[f_DefaultValue] = pDefaultValue_;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::getConstValue -- ConstValue を得る
//
//	NOTES
//		ConstValue を得る
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
ColumnDefinition::getConstValue() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_ConstValue];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::setConstValue -- ConstValue を設定する
//
//	NOTES
//		ConstValue を設定する
//
//	ARGUMENTS
//		ValueExpression* pConstValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ColumnDefinition::setConstValue(ValueExpression* pConstValue_)
{
	m_vecpElements[f_ConstValue] = pConstValue_;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::getHint -- Hint を得る
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
ColumnDefinition::getHint() const
{
	Hint* pResult = 0;
	Object* pObj = m_vecpElements[f_Hint];
	if ( pObj && ObjectType::Hint == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Hint*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::setHint -- Hint を設定する
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
ColumnDefinition::setHint(Hint* pHint_)
{
	m_vecpElements[f_Hint] = pHint_;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::getConstraints -- Constraints を得る
//
//	NOTES
//		Constraints を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ColumnConstraintDefinitionList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnConstraintDefinitionList*
ColumnDefinition::getConstraints() const
{
	ColumnConstraintDefinitionList* pResult = 0;
	Object* pObj = m_vecpElements[f_Constraints];
	if ( pObj && ObjectType::ColumnConstraintDefinitionList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnConstraintDefinitionList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ColumnDefinition::setConstraints -- Constraints を設定する
//
//	NOTES
//		Constraints を設定する
//
//	ARGUMENTS
//		ColumnConstraintDefinitionList* pConstraints_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ColumnDefinition::setConstraints(ColumnConstraintDefinitionList* pConstraints_)
{
	m_vecpElements[f_Constraints] = pConstraints_;
}

// FUNCTION public
//	Statement::ColumnDefinition::isUseOnUpdate -- UseOnUpdateを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
ColumnDefinition::
isUseOnUpdate() const
{
	return m_bUseOnUpdate;
}

//
//	FUNCTION public
//	Statement::ColumnDefinition::copy -- 自身をコピーする
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
ColumnDefinition::copy() const
{
	return new ColumnDefinition(*this);
}

// FUNCTION public
//	Statement::ColumnDefinition::toSQLStatement -- SQL文で値を得る
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
ColumnDefinition::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << (getName() ? *getName()->getIdentifier() : "(0)");
	stream << ' ' << getDataType().toSQLStatement();
	ValueExpression* pDefault = getDefaultValue();
	if (pDefault) {
		stream << ' ';
		if (pDefault->getOperator() != ValueExpression::op_GeneratorDefinition) {
			stream << "default ";
		}
		stream << pDefault->toSQLStatement(bForCascade_);
	}
	if (isUseOnUpdate()) {
		stream << " using on update";
	}
	if (getConstraints()) {
		stream << " " << getConstraints()->toSQLStatement(bForCascade_);
	}
	if (getHint()) {
		stream << " " << getHint()->toSQLStatement(bForCascade_);
	}
	return stream.getString();
}

#if 0
namespace
{
	Analysis::ColumnDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ColumnDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::ColumnDefinition::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ColumnDefinition::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::SQLDataType(cArchive_, m_cDataType);
	cArchive_(m_bUseOnUpdate);
}

//
//	Copyright (c) 1999, 2002, 2003, 2004, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
