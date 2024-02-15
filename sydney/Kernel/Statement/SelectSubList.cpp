// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SelectSubList.cpp -- SelectSubList
// 
// Copyright (c) 1999, 2002, 2006, 2008, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/SelectSubList.h"
#include "Statement/DerivedColumn.h"
#include "Statement/Identifier.h"
#include "Statement/Type.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/SelectSubList_DerivedColumn.h"
#include "Analysis/SelectSubList_Identifier.h"
#endif

#include "Analysis/Query/SelectSubList.h"

#include "ModOstrStream.h"

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_DerivedColumnOrIdentifier,
		f_ExpressionType,
		f__end_index
	};
}

using namespace Statement;

SelectSubList::
SelectSubList(Statement::DerivedColumn* pDerivedColumn_)
	: Object(ObjectType::SelectSubList, f__end_index)
{
	// DerivedColumnOrIdentifier を設定する
	setDerivedColumnOrIdentifier(pDerivedColumn_);
	// ExpressionTypeを設定する
	setExpressionType(pDerivedColumn_->getExpressionType());
}

SelectSubList::
SelectSubList(Statement::Identifier* pIdentifier_)
	: Object(ObjectType::SelectSubList, f__end_index)
{
	// DerivedColumnOrIdentifier を設定する
	setDerivedColumnOrIdentifier(pIdentifier_);
	// ExpressionTypeを設定する
	setExpressionType(ValueExpression::type_MultipleColumn);
}

//
//	FUNCTION public
//		Statement::SelectSubList::getDerivedColumnOrIdentifier -- DerivedColumnOrIdentifier を得る
//
//	NOTES
//		DerivedColumnOrIdentifier を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Statement::Object*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Object*
SelectSubList::getDerivedColumnOrIdentifier() const
{
	return m_vecpElements[f_DerivedColumnOrIdentifier];
}

//
//	FUNCTION public
//		Statement::SelectSubList::setDerivedColumnOrIdentifier -- DerivedColumnOrIdentifier を設定する
//
//	NOTES
//		DerivedColumnOrIdentifier を設定する
//
//	ARGUMENTS
//		Statement::Object* pDerivedColumnOrIdentifier_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SelectSubList::setDerivedColumnOrIdentifier(Object* pDerivedColumnOrIdentifier_)
{
	m_vecpElements[f_DerivedColumnOrIdentifier] = pDerivedColumnOrIdentifier_;
}

// FUNCTION public
//	Statement::SelectSubList::getExpressionType -- ExpressionType を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
SelectSubList::
getExpressionType() const
{
	return getIntegerElement(f_ExpressionType);
}

// FUNCTION public
//	Statement::SelectSubList::setExpressionType -- ExpressionType を設定する
//
// NOTES
//
// ARGUMENTS
//	int iType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
SelectSubList::
setExpressionType(int iType_)
{
	setIntegerElement(f_ExpressionType, iType_);
}

//
//	FUNCTION public
//	Statement::SelectSubList::copy -- 自身をコピーする
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
SelectSubList::copy() const
{
	return new SelectSubList(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::SelectSubList_DerivedColumn _analyzerDerivedColumn;
	Analysis::SelectSubList_Identifier _analyzerIdentifier;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SelectSubList::
getAnalyzer() const
{
	switch (m_vecpElements[f_DerivedColumnOrIdentifier]->getType()) {
	case Statement::ObjectType::DerivedColumn:
		{
			return &_analyzerDerivedColumn;
		}
	case Statement::ObjectType::Identifier:
		{
			return &_analyzerIdentifier;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}
#endif

// FUNCTION public
//	Statement::SelectSubList::getAnalyzer2 -- 
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
SelectSubList::
getAnalyzer2() const
{
	return Analysis::Query::SelectSubList::create(this);
}

//
//	Copyright (c) 1999, 2002, 2006, 2008, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
