// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TableDefinition.cpp -- TableDefinition
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/TableDefinition.h"
#include "Statement/Type.h"
#include "Statement/Hint.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"
#include "Statement/TableElementList.h"
#include "Statement/DataValue.h"
#include "Statement/AreaOption.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "Common/UnicodeString.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#if 0
#include "Analysis/TableDefinition.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Name,
		f_Scope,
		f_ConstUpdate,
		f_Elements,
		f_InitialValue,
		f_Hint,
		f_AreaOpt,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::TableDefinition::TableDefinition -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pName_
//		int iConstUpdate_
//		TableElementList* pElements_
//		Hint* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableDefinition::TableDefinition(Identifier* pName_, int iScope_, int iConstUpdate_, 
	 							 TableElementList* pElements_, DataValue* pInitialValue_, 
								 Hint* pHint_, AreaOption* pAreaOpt_)
	: Object(ObjectType::TableDefinition, f__end_index, Object::Reorganize)
{
	// If name begins from '#', object type become special type.
	if (pName_->getToken().getLength() > 0
		&& *(pName_->getToken().getHead()) == Common::UnicodeChar::usSharp) {
		setType(ObjectType::TemporaryTableDefinition);
	}
	// Name を設定する
	setName(pName_);
	// Scope を設定する
	setScope(iScope_);
	// ConstUpdate を設定する
	setConstUpdate(iConstUpdate_);
	// Elements を設定する
	setElements(pElements_);
	// InitialValue を設定する
	setInitialValue(pInitialValue_);
	// Hint を設定する
	setHint(pHint_);
	// AreaOption を設定する
	setAreaOption(pAreaOpt_);
}

//
//	FUNCTION public
//		Statement::TableDefinition::getName -- Name を得る
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
TableDefinition::getName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Name];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableDefinition::setName -- Name を設定する
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
TableDefinition::setName(Identifier* pName_)
{
	m_vecpElements[f_Name] = pName_;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::TableDefinition::getNameString -- Name を ModString で得る
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
TableDefinition::getNameString() const
{
	Identifier* pIdentifier = getName();
	return pIdentifier ? pIdentifier->getIdentifierString() : 0;
}
#endif

//
//	FUNCTION public
//		Statement::TableDefinition::getScope -- Scope を得る
//
//	NOTES
//		Scope を得る
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
TableDefinition::getScope() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_Scope];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::TableDefinition::setScope -- Scope を設定する
//
//	NOTES
//		Scope を設定する
//
//	ARGUMENTS
//		int iScope_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableDefinition::setScope(int iScope_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_Scope]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_Scope] = pIntVal;
	}
	pIntVal->setValue(iScope_);
}

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::TableDefinition::getConstUpdate -- ConstUpdate を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		int
//
//	EXCEPTIONS

int
TableDefinition::getConstUpdate() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_ConstUpdate];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}
#endif

//
//	FUNCTION public
//		Statement::TableDefinition::setConstUpdate -- ConstUpdate を設定する
//
//	NOTES
//		ConstUpdate を設定する
//
//	ARGUMENTS
//		int iConstUpdate_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableDefinition::setConstUpdate(int iConstUpdate_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_ConstUpdate]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_ConstUpdate] = pIntVal;
	}
	pIntVal->setValue(iConstUpdate_);
}

//
//	FUNCTION public
//		Statement::TableDefinition::getElements -- Elements を得る
//
//	NOTES
//		Elements を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		TableElementList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableElementList*
TableDefinition::getElements() const
{
	TableElementList* pResult = 0;
	Object* pObj = m_vecpElements[f_Elements];
	if ( pObj && ObjectType::TableElementList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(TableElementList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableDefinition::setElements -- Elements を設定する
//
//	NOTES
//		Elements を設定する
//
//	ARGUMENTS
//		TableElementList* pElements_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableDefinition::setElements(TableElementList* pElements_)
{
	m_vecpElements[f_Elements] = pElements_;
}

#ifdef OBSOLETE
//	FUNCTION public
//	Statement::TableDefinition::getInitialValue -- InitialValue を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		DataValue*
//
//	EXCEPTIONS

DataValue*
TableDefinition::getInitialValue() const
{
	DataValue* pResult = 0;
	Object* pObj = m_vecpElements[f_InitialValue];
	if ( pObj && ObjectType::DataValue == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(DataValue*, pObj);
	return pResult;
}
#endif

//
//	FUNCTION public
//		Statement::TableDefinition::setInitialValue -- InitialValue を設定する
//
//	NOTES
//		InitialValue を設定する
//
//	ARGUMENTS
//		DataValue* pInitialValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableDefinition::setInitialValue(DataValue* pInitialValue_)
{
	m_vecpElements[f_InitialValue] = pInitialValue_;
}

//
//	FUNCTION public
//		Statement::TableDefinition::getHint -- Hint を得る
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
TableDefinition::getHint() const
{
	Hint* pResult = 0;
	Object* pObj = m_vecpElements[f_Hint];
	if ( pObj && ObjectType::Hint == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Hint*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableDefinition::setHint -- Hint を設定する
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
TableDefinition::setHint(Hint* pHint_)
{
	m_vecpElements[f_Hint] = pHint_;
}

//
//	FUNCTION public
//		Statement::TableDefinition::getAreaOption
//
//	NOTES
//		AreaOption を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	AreaOption* 
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
AreaOption* 
TableDefinition::getAreaOption() const
{
	AreaOption* pResult = 0;
	Object* pObj = m_vecpElements[f_AreaOpt];
	if ( pObj && ObjectType::AreaOption == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(AreaOption*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableDefinition::setAreaOption -- AreaOption を設定する
//
//	NOTES
//		AreaOption を設定する
//
//	ARGUMENTS
//		AreaOption* pHint_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void 
TableDefinition::setAreaOption(AreaOption* pAreaOpt_)
{
	m_vecpElements[f_AreaOpt] = pAreaOpt_;
}

//
//	FUNCTION public
//	Statement::TableDefinition::copy -- 自身をコピーする
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
TableDefinition::copy() const
{
	return new TableDefinition(*this);
}

// FUNCTION public
//	Statement::TableDefinition::toSQLStatement -- SQL文で値を得る
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
TableDefinition::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << "create table ";
	Identifier* pIdentifier = getName();
	stream << (pIdentifier ? pIdentifier->toSQLStatement(bForCascade_) : "(0)");
	stream << "(";
	stream << getElements()->toSQLStatement(bForCascade_);
	stream << ")";
	if (getHint()) {
		stream << " " << getHint()->toSQLStatement(bForCascade_);
	}
	if (getAreaOption()) {
		stream << " " << getAreaOption()->toSQLStatement(bForCascade_);
	}
	return stream.getString();
}

#if 0
namespace
{
	Analysis::TableDefinition _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
TableDefinition::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2007, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
