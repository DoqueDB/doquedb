// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TablePrimary.cpp -- TablePrimary
// 
// Copyright (c) 1999, 2002, 2003, 2004, 2005, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/TablePrimary.h"
#include "Statement/Type.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Identifier.h"
#include "Statement/QueryExpression.h"
#include "Statement/Utility.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/TablePrimary_BaseTable.h"
#include "Analysis/TablePrimary_DerivedTable.h"
#include "Analysis/TablePrimary_JoinedTable.h"
#endif

#include "Analysis/Query/TablePrimary.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Reference,
		f_CorrelationName,
		f_DerivedColumnList,
		f_Query,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::TablePrimary::TablePrimary -- コンストラクタ (1)
//
//	NOTES
//		コンストラクタ (1)
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
TablePrimary::TablePrimary()
	: Object(ObjectType::TablePrimary, f__end_index),
	  m_eType(Type::Unknown)
{
}

//
//	FUNCTION public
//		Statement::TablePrimary::TablePrimary -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pReference_
//		Identifier* pCorrelationName_
//		ColumnNameList* pDerivedColumnList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TablePrimary::TablePrimary(Identifier* pReference_, Identifier* pCorrelationName_,
						   ColumnNameList* pDerivedColumnList_)
	: Object(ObjectType::TablePrimary, f__end_index),
	  m_eType(Type::Table)
{
	// Reference を設定する
	setReference(pReference_);
	// CorrelationName を設定する
	setCorrelationName(pCorrelationName_);
	// DerivedColumnList を設定する
	setDerivedColumnList(pDerivedColumnList_);
}

//
//	FUNCTION 
//		Statement::TablePrimary::TablePrimary -- コンストラクタ (3)
//
//	NOTES
//		DerivedTablePrimaryを作る
//
//	ARGUMENTS
//		QueryExpression* pQuery_
//		Identifier* pCorrelationName_
//		ColumnNameList* pDerivedColumnList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
TablePrimary::
TablePrimary(QueryExpression* pQuery_,
			 Identifier* pCorrelationName_,
			 ColumnNameList* pDerivedColumnList_)
	: Object(ObjectType::TablePrimary, f__end_index),
	  m_eType(Type::DerivedTable)
{
	// Query を設定する
	setQuery(pQuery_);
	// CorrelationName を設定する
	setCorrelationName(pCorrelationName_);
	// DerivedColumnList を設定する
	setDerivedColumnList(pDerivedColumnList_);
}

//
//	FUNCTION 
//		Statement::TablePrimary::TablePrimary -- コンストラクタ (4)
//
//	NOTES
//		JoinedTablePrimaryを作る
//
//	ARGUMENTS
//		Object* pJoinedTable_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
TablePrimary::
TablePrimary(Type::Value eType_, Object* pQuery_)
	: Object(ObjectType::TablePrimary, f__end_index),
	  m_eType(eType_)
{
	// Query を設定する
	setQuery(pQuery_);
}

// FUNCTION public
//	Statement::TablePrimary::TablePrimary -- コンストラクタ (5)
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pValueExpression_
//	Identifier* pCorrelationName_
//	ColumnNameList* pDerivedColumnList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

TablePrimary::
TablePrimary(ValueExpression* pValueExpression_,
			 Identifier* pCorrelationName_,
			 ColumnNameList* pDerivedColumnList_)
	: Object(ObjectType::TablePrimary, f__end_index),
	  m_eType(Type::UnnestTable)
{
	// Collection data を設定する
	setCollection(pValueExpression_);
	// CorrelationName を設定する
	setCorrelationName(pCorrelationName_);
	// DerivedColumnList を設定する
	setDerivedColumnList(pDerivedColumnList_);
}

// コピーコンストラクタ
TablePrimary::
TablePrimary(const TablePrimary& cOther_)
	: Object(cOther_),
	  m_eType(cOther_.m_eType)
{}

//
//	FUNCTION public
//		Statement::TablePrimary::getReference -- Reference を得る
//
//	NOTES
//		Reference を得る
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
TablePrimary::getReference() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Reference];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TablePrimary::setReference -- Reference を設定する
//
//	NOTES
//		Reference を設定する
//
//	ARGUMENTS
//		Identifier* pReference_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TablePrimary::setReference(Identifier* pReference_)
{
	m_vecpElements[f_Reference] = pReference_;
}

//
//	FUNCTION public
//		Statement::TablePrimary::getReferenceString
//			-- Reference を ModString で得る
//
//	NOTES
//		Reference を ModUnicodeString で得る
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
TablePrimary::getReferenceString() const
{
	Identifier* pIdentifier = getReference();
	return pIdentifier ? pIdentifier->getIdentifier() : 0;
}

//
//	FUNCTION public
//		Statement::TablePrimary::getCorrelationName -- CorrelationName を得る
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
TablePrimary::getCorrelationName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_CorrelationName];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TablePrimary::setCorrelationName -- CorrelationName を設定する
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
TablePrimary::setCorrelationName(Identifier* pCorrelationName_)
{
	m_vecpElements[f_CorrelationName] = pCorrelationName_;
}

//
//	FUNCTION public
//		Statement::TablePrimary::getCorrelationNameString
//			-- CorrelationName を ModString で得る
//
//	NOTES
//		CorrelationName を ModUnicodeString で得る
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
TablePrimary::getCorrelationNameString() const
{
	Identifier* pIdentifier = getCorrelationName();
	return pIdentifier ? pIdentifier->getIdentifier() : 0;
}

//	FUNCTION public
//	Statement::TablePrimary::getDerivedColumnList -- DerivedColumnList を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ColumnNameList*
//
//	EXCEPTIONS

ColumnNameList*
TablePrimary::getDerivedColumnList() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_DerivedColumnList];
	if ( pObj && ObjectType::ColumnNameList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TablePrimary::setDerivedColumnList -- DerivedColumnList を設定する
//
//	NOTES
//		DerivedColumnList を設定する
//
//	ARGUMENTS
//		ColumnNameList* pDerivedColumnList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TablePrimary::setDerivedColumnList(ColumnNameList* pDerivedColumnList_)
{
	m_vecpElements[f_DerivedColumnList] = pDerivedColumnList_;
}

//	FUNCTION 
//	Statement::TablePrimary::getTablePrimaryType -- TablePrimaryの種別
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Statement::TablePrimary::Type::Value
//
//	EXCEPTIONS
//		なし

TablePrimary::Type::Value
TablePrimary::getTablePrimaryType() const
{
	return m_eType;
}

//	FUNCTION 
//	Statement::TablePrimary::getQuery -- Query を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Object*
//
//	EXCEPTIONS
//		なし

Object*
TablePrimary::getQuery() const
{
	return m_vecpElements[f_Query];
}

//
//	FUNCTION 
//		Statement::TablePrimary::setQuery -- Query を設定する
//
//	NOTES
//		Query を設定する
//
//	ARGUMENTS
//		Object* pQuery_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???
//
void
TablePrimary::setQuery(Object* pQuery_)
{
	m_vecpElements[f_Query] = pQuery_;
}

// FUNCTION public
//	Statement::TablePrimary::getCollection -- Collection を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ValueExpression*
//
// EXCEPTIONS

ValueExpression*
TablePrimary::
getCollection() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Query];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

// FUNCTION public
//	Statement::TablePrimary::setCollection -- Collection を設定する
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pCollection_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
TablePrimary::
setCollection(ValueExpression* pCollection_)
{
	m_vecpElements[f_Query] = pCollection_;
}

//
//	FUNCTION public
//	Statement::TablePrimary::copy -- 自身をコピーする
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
TablePrimary::copy() const
{
	TablePrimary* pResult = new TablePrimary(*this);
	pResult->m_eType = m_eType;
	return pResult;
}

#ifdef OBSOLETE // 現在のところStatement::Objectをマップに登録することはない
// ハッシュコードを計算する
//virtual
ModSize
TablePrimary::
getHashCode()
{
	ModSize value = Super::getHashCode();
	value <<= 4;
	value += m_eType;
	return value;
}

// 同じ型のオブジェクト同士でless比較する
//virtual
bool
TablePrimary::
compare(const Object& cObj_) const
{
	return Super::compare(cObj_)
		|| m_eType < _SYDNEY_DYNAMIC_CAST(const TablePrimary&, cObj_).m_eType;
}
#endif

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::TablePrimary_BaseTable _analyzerBase;
	Analysis::TablePrimary_DerivedTable _analyzerDerived;
	Analysis::TablePrimary_JoinedTable _analyzerJoined;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
TablePrimary::
getAnalyzer() const
{
	switch (m_eType) {
	case Type::Table:
		{
			return &_analyzerBase;
		}
	case Type::DerivedTable:
	case Type::Bulk:
		{
			return &_analyzerDerived;
		}
	case Type::JoinedTable:
		{
			return &_analyzerJoined;
		}
	case Type::UnnestTable:
		{
			// old optimizer can't process unnest
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	default:
		{
			; _SYDNEY_ASSERT(false);
		}
	}
	// never reached
	return 0;
}
#endif

// FUNCTION public
//	Statement::TablePrimary::getAnalyzer2 -- Analyzerを得る
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
TablePrimary::
getAnalyzer2() const
{
	return Analysis::Query::TablePrimary::create(this);
}

// FUNCTION public
//	Statement::TablePrimary::serialize -- 
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
TablePrimary::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	Utility::Serialize::EnumValue(cArchive_, m_eType);
}

//
//	Copyright (c) 1999, 2002, 2003, 2004, 2005, 2006, 2008, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
