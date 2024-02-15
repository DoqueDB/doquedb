// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HavingClause.cpp -- HavingClause
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2010, 2023 Ricoh Company, Ltd.
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

#include "Statement/HavingClause.h"
#include "Statement/Type.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/HavingClause.h"
#endif
#include "Analysis/Query/HavingClause.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Condition,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::HavingClause::HavingClause -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ValueExpression* pCondition_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
HavingClause::HavingClause(ValueExpression* pCondition_)
	: Object(ObjectType::HavingClause, f__end_index)
{
	setCondition(pCondition_);
}

//	FUNCTION public
//	Statement::HavingClause::getCondition -- Condition を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS

ValueExpression*
HavingClause::getCondition() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Condition];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::HavingClause::setCondition -- Condition を設定する
//
//	NOTES
//		Condition を設定する
//
//	ARGUMENTS
//		ValueExpression* pCondition_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
HavingClause::setCondition(ValueExpression* pCondition_)
{
	m_vecpElements[f_Condition] = pCondition_;
}

#if 0
namespace
{
	Analysis::HavingClause _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
HavingClause::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::HavingClause::getAnalyzer2 -- 
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
HavingClause::
getAnalyzer2() const
{
	return Analysis::Query::HavingClause::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
