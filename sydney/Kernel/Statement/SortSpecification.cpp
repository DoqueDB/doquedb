// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SortSpecification.cpp -- SortSpecification
// 
// Copyright (c) 1999, 2002, 2006, 2023 Ricoh Company, Ltd.
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

#include "Statement/SortSpecification.h"
#include "Statement/Type.h"
#include "Statement/IntegerValue.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/SortSpecification.h"
#endif


_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_SortKey,
		f_OrderingSpecification,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::SortSpecification::SortSpecification -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ValueExpression* pSortKey_
//		int iOrderingSpecification_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SortSpecification::SortSpecification(ValueExpression* pSortKey_, int iOrderingSpecification_)
	: Object(ObjectType::SortSpecification, f__end_index)
{
	// SortKey を設定する
	setSortKey(pSortKey_);
	// OrderingSpecification を設定する
	setOrderingSpecification(iOrderingSpecification_);
}

//
//	FUNCTION public
//		Statement::SortSpecification::getSortKey -- SortKey を得る
//
//	NOTES
//		SortKey を得る
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
SortSpecification::getSortKey() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_SortKey];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::SortSpecification::setSortKey -- SortKey を設定する
//
//	NOTES
//		SortKey を設定する
//
//	ARGUMENTS
//		ValueExpression* pSortKey_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SortSpecification::setSortKey(ValueExpression* pSortKey_)
{
	m_vecpElements[f_SortKey] = pSortKey_;
}

//
//	FUNCTION public
//		Statement::SortSpecification::getOrderingSpecification -- OrderingSpecification を得る
//
//	NOTES
//		OrderingSpecification を得る
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
SortSpecification::getOrderingSpecification() const
{
	int iResult = 0;
	Object* pObj = m_vecpElements[f_OrderingSpecification];
	if ( pObj && ObjectType::IntegerValue == pObj->getType() )
		iResult = _SYDNEY_DYNAMIC_CAST(IntegerValue*, pObj)->getValue();
	return iResult;
}

//
//	FUNCTION public
//		Statement::SortSpecification::setOrderingSpecification -- OrderingSpecification を設定する
//
//	NOTES
//		OrderingSpecification を設定する
//
//	ARGUMENTS
//		int iOrderingSpecification_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SortSpecification::setOrderingSpecification(int iOrderingSpecification_)
{
	IntegerValue* pIntVal = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_OrderingSpecification]);

	if (pIntVal == 0)
	{
		pIntVal = new IntegerValue;
		m_vecpElements[f_OrderingSpecification] = pIntVal;
	}
	pIntVal->setValue(iOrderingSpecification_);
}

//
//	FUNCTION public
//	Statement::SortSpecification::copy -- 自身をコピーする
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
SortSpecification::copy() const
{
	return new SortSpecification(*this);
}

#if 0
namespace
{
	Analysis::SortSpecification _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SortSpecification::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
