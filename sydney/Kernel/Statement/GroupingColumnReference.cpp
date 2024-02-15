// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GroupingColumnReference.cpp -- GroupingColumnReference
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/GroupingColumnReference.h"
#include "Statement/Type.h"
#include "Statement/ItemReference.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#include "Analysis/Query/GroupingColumnReferenceList.h"
#include "Analysis/Query/GroupingColumnReference.h"

#if 0
#include "Analysis/GroupingColumnReference.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Reference,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::GroupingColumnReference::GroupingColumnReference -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ItemReference* pItemReference_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
GroupingColumnReference::GroupingColumnReference(ItemReference* pItemReference_)
	: Object(ObjectType::GroupingColumnReference, f__end_index)
{
	setItemReference(pItemReference_);
}

//
//	FUNCTION public
//		Statement::GroupingColumnReference::GroupingColumnReference -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ItemReference* pItemReference_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
GroupingColumnReference::GroupingColumnReference(ValueExpression* pElementReference_)
	: Object(ObjectType::GroupingColumnReference, f__end_index)
{
	setElementReference(pElementReference_);
}



//
//	FUNCTION public
//		Statement::GroupingColumnReference::toSQLStatement -- SQL文を得る
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
GroupingColumnReference::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	return getItemReference()->toSQLStatement(bForCascade_);
}



//	FUNCTION public
//		Statement::GroupingColumnReference::getItemReference -- ItemReference を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ItemReference*
//
//	EXCEPTIONS

ItemReference*
GroupingColumnReference::getItemReference() const
{
	ItemReference* pResult = 0;
	Object* pObj = m_vecpElements[f_Reference];
	if ( pObj && ObjectType::ItemReference == pObj->getType() ) 
		pResult = _SYDNEY_DYNAMIC_CAST(ItemReference* ,pObj);

	return pResult;
}



//	FUNCTION public
//		Statement::GroupingColumnReference::getReference -- ItemReference を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ItemReference*
//
//	EXCEPTIONS

Statement::Object*
GroupingColumnReference::getReference() const
{
 
	return m_vecpElements[f_Reference];;
}

//
//	FUNCTION public
//		Statement::GroupingColumnReference::setItemReference -- ItemReference を設定する
//
//	NOTES
//		ItemReference を設定する
//
//	ARGUMENTS
//		ItemReference* pItemReference_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
GroupingColumnReference::
setItemReference(ItemReference* pItemReference_)
{
	m_vecpElements[f_Reference] = pItemReference_;
}

//
//	FUNCTION public
//		Statement::GroupingColumnReference::setElementReference -- ElementReference を設定する
//
//	NOTES
//		ItemReference を設定する
//
//	ARGUMENTS
//		ItemReference* pItemReference_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
GroupingColumnReference::
setElementReference(ValueExpression* pElementReference_)
{
	m_vecpElements[f_Reference] = pElementReference_;
}

// FUNCTION public
//	Statement::ValueExpression::getAnalyzer2 -- 
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
GroupingColumnReference::
getAnalyzer2() const
{
	return Analysis::Query::GroupingColumnReference::create(this);
}

#if 0
namespace
{
	Analysis::GroupingColumnReference _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
GroupingColumnReference::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
