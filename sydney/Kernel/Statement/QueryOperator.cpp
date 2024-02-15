// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryOperator.cpp -- QueryOperator
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2023 Ricoh Company, Ltd.
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

#include "Statement/QueryOperator.h"
#include "Statement/Type.h"
#include "Statement/ColumnNameList.h"
#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/QueryOperator.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_SetOperatorType,
		f_All,
		f_CorrespondingSpec,
		f__end_index
	};
}

//
//	FUNCTION 
//		Statement::QueryOperator::QueryOperator -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		int iSetOperatorType_
//		int iAll_
//		ColumnNameList* pCorrespondingSpec_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QueryOperator::QueryOperator(int iSetOperatorType_, int iAll_, ColumnNameList* pCorrespondingSpec_)
	: Object(ObjectType::QueryOperator, f__end_index)
{
	setSetOperatorType(iSetOperatorType_);
	setAll(iAll_);
	setCorrespondingSpec(pCorrespondingSpec_);
}

//
//	FUNCTION public
//		Statement::QueryOperator::getSetOperatorType -- SetOperatorType を得る
//
//	NOTES
//		SetOperatorType を得る
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
QueryOperator::getSetOperatorType() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_SetOperatorType];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::QueryOperator::setSetOperatorType -- SetOperatorType を設定する
//
//	NOTES
//		SetOperatorType を設定する
//
//	ARGUMENTS
//		int iSetOperatorType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryOperator::setSetOperatorType(int iSetOperatorType_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_SetOperatorType]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_SetOperatorType] = pIntVal;
	}
	pIntVal->setValue(iSetOperatorType_);
}

//
//	FUNCTION public
//		Statement::QueryOperator::getAll -- All を得る
//
//	NOTES
//		All を得る
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
QueryOperator::getAll() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_All];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::QueryOperator::setAll -- All を設定する
//
//	NOTES
//		All を設定する
//
//	ARGUMENTS
//		int iAll_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryOperator::setAll(int iAll_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_All]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_All] = pIntVal;
	}
	pIntVal->setValue(iAll_);
}

//
//	FUNCTION public
//		Statement::QueryOperator::getCorrespondingSpec -- CorrespondingSpec を得る
//
//	NOTES
//		CorrespondingSpec を得る
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
QueryOperator::getCorrespondingSpec() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_CorrespondingSpec];
	if  ( pObj && ObjectType::ColumnNameList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::QueryOperator::setCorrespondingSpec -- CorrespondingSpec を設定する
//
//	NOTES
//		CorrespondingSpec を設定する
//
//	ARGUMENTS
//		ColumnNameList* pCorrespondingSpec_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryOperator::setCorrespondingSpec(ColumnNameList* pCorrespondingSpec_)
{
	m_vecpElements[f_CorrespondingSpec] = pCorrespondingSpec_;
}

#if 0
namespace
{
	Analysis::QueryOperator _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
QueryOperator::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
