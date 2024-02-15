// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TableCorrelationSpec.cpp -- TableCorrelationSpec
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

#include "Statement/TableCorrelationSpec.h"
#include "Statement/Type.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Identifier.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#include "ModUnicodeString.h"
#if 0
#include "Analysis/TableCorrelationSpec.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Correlation,
		f_Derived,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::TableCorrelationSpec::TableCorrelationSpec -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Identifier* pCorrelation
//		ColumnNameList* pDerived_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableCorrelationSpec::TableCorrelationSpec(Identifier* pCorrelation_, ColumnNameList* pDerived_)
	: Object(ObjectType::TableCorrelationSpec, f__end_index)
{
	// Correlation を設定する
	setCorrelation(pCorrelation_);
	// Derived を設定する
	setDerived(pDerived_);
}

//
//	FUNCTION public
//		Statement::TableCorrelationSpec::getCorrelation -- Correlation を得る
//
//	NOTES
//		Correlation を得る
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
TableCorrelationSpec::getCorrelation() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Correlation];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableCorrelationSpec::setCorrelation -- Correlation を設定する
//
//	NOTES
//		Correlation を設定する
//
//	ARGUMENTS
//		Identifier* pCorrelation_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableCorrelationSpec::setCorrelation(Identifier* pCorrelation_)
{
	m_vecpElements[f_Correlation] = pCorrelation_;
}

#ifdef OBSOLETE
//	FUNCTION public
//		Statement::TableCorrelationSpec::getCorrelationString
//			-- Correlation を ModString で得る
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
TableCorrelationSpec::getCorrelationString() const
{
	Identifier* pIdentifier = getCorrelation();
	return pIdentifier ? pIdentifier->getIdentifierString() : 0;
}
#endif

//
//	FUNCTION public
//		Statement::TableCorrelationSpec::getDerived -- Derived を得る
//
//	NOTES
//		Derived を得る
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
TableCorrelationSpec::getDerived() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_Derived];
	if ( pObj && ObjectType::ColumnNameList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::TableCorrelationSpec::setDerived -- Derived を設定する
//
//	NOTES
//		Derived を設定する
//
//	ARGUMENTS
//		ColumnNameList* pDerived_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
TableCorrelationSpec::setDerived(ColumnNameList* pDerived_)
{
	m_vecpElements[f_Derived] = pDerived_;
}

#if 0
namespace
{
	Analysis::TableCorrelationSpec _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
TableCorrelationSpec::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
