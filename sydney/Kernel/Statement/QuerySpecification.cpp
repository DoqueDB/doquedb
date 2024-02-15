// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QuerySpecification.cpp -- QuerySpecification
// 
// Copyright (c) 1999, 2002, 2006, 2008, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/QuerySpecification.h"
#include "Statement/Type.h"
#include "Statement/BulkSpecification.h"
#include "Statement/IntegerValue.h"
#include "Statement/SelectList.h"
#include "Statement/TableExpression.h"
#include "Statement/SelectTargetList.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/QuerySpecification.h"
#endif

#include "Analysis/Query/QuerySpecification.h"

#include "ModOstrStream.h"

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Quantifier,
		f_SelectList,
		f_Table,
		f_Output,
		f_SelectTargetList,
		f__end_index
	};
}

_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::QuerySpecification::QuerySpecification -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		int iQuantifier_
//		SelectList* pSelectList_
//		TableExpression* pTable_
//		BulkSpecification* pOutput_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QuerySpecification::QuerySpecification(int iQuantifier_, SelectList* pSelectList_, TableExpression* pTable_,
									   BulkSpecification* pOutput_, SelectTargetList* pSelectTargetList_)
	: Object(ObjectType::QuerySpecification, f__end_index, Object::Optimize)
{
	// Quantifier を設定する
	setQuantifier(iQuantifier_);
	// SelectList を設定する
	setSelectList(pSelectList_);
	// Table を設定する
	setTable(pTable_);
	// BulkSpecificationを設定する
	setOutput(pOutput_);
	// SelectTargetList(select into)を設定する
	setSelectTargetList(pSelectTargetList_);
}

//
//	FUNCTION public
//		Statement::QuerySpecification::getQuantifier -- Quantifier を得る
//
//	NOTES
//		Quantifier を得る
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
QuerySpecification::getQuantifier() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_Quantifier];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::QuerySpecification::setQuantifier -- Quantifier を設定する
//
//	NOTES
//		Quantifier を設定する
//
//	ARGUMENTS
//		int iQuantifier_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QuerySpecification::setQuantifier(int iQuantifier_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_Quantifier]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_Quantifier] = pIntVal;
	}
	pIntVal->setValue(iQuantifier_);
}

//
//	FUNCTION public
//		Statement::QuerySpecification::getSelectList -- SelectList を得る
//
//	NOTES
//		SelectList を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		SelectList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectList*
QuerySpecification::getSelectList() const
{
	SelectList* pResult = 0;
	Object* pObj = m_vecpElements[f_SelectList];
	if ( pObj && ObjectType::SelectList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(SelectList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QuerySpecification::setSelectList -- SelectList を設定する
//
//	NOTES
//		SelectList を設定する
//
//	ARGUMENTS
//		SelectList* pSelectList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QuerySpecification::setSelectList(SelectList* pSelectList_)
{
	m_vecpElements[f_SelectList] = pSelectList_;
}






//
//	FUNCTION public
//		Statement::QuerySpecification::setSelectTargetList -- SelectTargetList を設定する
//
//	NOTES
//		SelectTargetList を設定する
//
//	ARGUMENTS
//		SelectTargetList* pSelectTargertList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QuerySpecification::setSelectTargetList(SelectTargetList* pSelectTargetList_)
{
	m_vecpElements[f_SelectTargetList] = pSelectTargetList_;
}



//
//	FUNCTION public
//		Statement::QuerySpecification::getSelectTaregetList -- SelectTargetList を得る
//
//	NOTES
//		SelectTargetList を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpressionList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectTargetList*
QuerySpecification::getSelectTargetList() const
{
	SelectTargetList* pResult = 0;
	Object* pObj = m_vecpElements[f_SelectTargetList];
	if ( pObj && ObjectType::SelectTargetList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(SelectTargetList*, pObj);
	return pResult;
}


//
//	FUNCTION public
//		Statement::QuerySpecification::getTable -- Table を得る
//
//	NOTES
//		Table を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		TableExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableExpression*
QuerySpecification::getTable() const
{
	TableExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Table];
	if ( pObj && ObjectType::TableExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(TableExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QuerySpecification::setTable -- Table を設定する
//
//	NOTES
//		Table を設定する
//
//	ARGUMENTS
//		TableExpression* pTable_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QuerySpecification::setTable(TableExpression* pTable_)
{
	m_vecpElements[f_Table] = pTable_;
}

//
//	FUNCTION public
//		Statement::QuerySpecification::getOutput -- Output を得る
//
//	NOTES
//		Output を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		BulkSpecification*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
BulkSpecification*
QuerySpecification::getOutput() const
{
	BulkSpecification* pResult = 0;
	Object* pObj = m_vecpElements[f_Output];
	if ( pObj && ObjectType::BulkSpecification == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(BulkSpecification*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QuerySpecification::setOutput -- Output を設定する
//
//	NOTES
//		Output を設定する
//
//	ARGUMENTS
//		BulkSpecification* pOutput_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QuerySpecification::setOutput(BulkSpecification* pOutput_)
{
	m_vecpElements[f_Output] = pOutput_;
}

//
//	FUNCTION public
//	Statement::QuerySpecification::copy -- 自身をコピーする
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
QuerySpecification::copy() const
{
	return new QuerySpecification(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::QuerySpecification _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
QuerySpecification::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::QuerySpecification::getAnalyzer2 -- Analyzerを得る
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
QuerySpecification::
getAnalyzer2() const
{
	return Analysis::Query::QuerySpecification::create(this);
}

//
//	Copyright (c) 1999, 2002, 2006, 2008, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
