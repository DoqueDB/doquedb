// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ValueExpressionList.cpp -- ValueExpressionList
// 
// Copyright (c) 1999, 2002, 2005, 2006, 2007, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Statement/ValueExpressionList.h"
#include "Statement/Type.h"
#include "Statement/IntegerValue.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"

#if 0
#include "Analysis/ValueExpressionList.h"
#endif
#include "Analysis/Value/ValueExpressionList.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::ValueExpressionList::ValueExpressionList -- コンストラクタ (1)
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
ValueExpressionList::ValueExpressionList()
	: ObjectList(ObjectType::ValueExpressionList),
	  m_iExpressionType(ValueExpression::type_Unknown)
{
}

//
//	FUNCTION public
//		Statement::ValueExpressionList::ValueExpressionList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ValueExpression* pValueExpression_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpressionList::ValueExpressionList(ValueExpression* pValueExpression_)
	: ObjectList(ObjectType::ValueExpressionList),
	  m_iExpressionType(pValueExpression_->getExpressionType())
{
	// ValueExpression を加える
	append(pValueExpression_);
}

//
//	FUNCTION public
//		Statement::ValueExpressionList::ValueExpressionList -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		const ValueExpressionList& cList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//

ValueExpressionList::
ValueExpressionList(const ValueExpressionList& cOther_)
	: ObjectList(cOther_),
	  m_iExpressionType(cOther_.m_iExpressionType)
{}

//
//	FUNCTION public
//		Statement::ValueExpressionList::getExpressionType -- ExpressionType を得る
//
//	NOTES
//		ExpressionType を得る
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
ValueExpressionList::getExpressionType() const
{
	return m_iExpressionType;
}

//
//	FUNCTION private
//		Statement::ValueExpressionList::setExpressionType -- ExpressionType を設定する
//
//	NOTES
//		ExpressionType を設定する
//
//	ARGUMENTS
//		int iExpressionType_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpressionList::setExpressionType(int iExpressionType_)
{
	m_iExpressionType = iExpressionType_;
}

//
//	FUNCTION public
//		Statement::ValueExpressionList::getValueExpressionAt -- ValueExpression を得る
//
//	NOTES
//		ValueExpression を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
ValueExpressionList::getValueExpressionAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(ValueExpression*, getAt(iAt_));
}

//
//	FUNCTION public
//		Statement::ValueExpressionList::merge -- ValueExpressionList を加える
//
//	NOTES
//		ValueExpressionList を加える
//		注意：引数の cValueExpressionList_ の ValueExpression の所有権が剥奪されるので注意
//
//	ARGUMENTS
//		ValueExpressionList* pValueExpressionList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ValueExpressionList::merge(ValueExpressionList& cValueExpressionList_)
{
	ObjectList::merge(cValueExpressionList_);
	setExpressionType(ValueExpression::mergeExpressionType(getExpressionType(),
														   cValueExpressionList_.getExpressionType()));
}

// FUNCTION public
//	Statement::ValueExpressionList::insertValueExpression -- ValueExpressionを先頭に加える
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pValueExpression_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ValueExpressionList::
insertValueExpression(ValueExpression* pValueExpression_)
{
	insert(pValueExpression_);
	setExpressionType(ValueExpression::mergeExpressionType(getExpressionType(),
														   pValueExpression_->getExpressionType()));
}

// FUNCTION public
//	Statement::ValueExpressionList::appendValueExpression -- ValueExpressionを加える
//
// NOTES
//
// ARGUMENTS
//	ValueExpression* pValueExpression_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ValueExpressionList::
appendValueExpression(ValueExpression* pValueExpression_)
{
	append(pValueExpression_);
	setExpressionType(ValueExpression::mergeExpressionType(getExpressionType(),
														   pValueExpression_->getExpressionType()));
}

//
//	FUNCTION public
//	Statement::ValueExpressionList::copy -- 自身をコピーする
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
ValueExpressionList::copy() const
{
	return new ValueExpressionList(*this);
}

#if 0
namespace
{
	Analysis::ValueExpressionList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ValueExpressionList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::ValueExpressionList::getAnalyzer2 -- 
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
ValueExpressionList::
getAnalyzer2() const
{
	return Analysis::Value::ValueExpressionList::create(this);
}

// FUNCTION public
//	Statement::ValueExpressionList::serialize -- 
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
ValueExpressionList::
serialize(ModArchive& cArchive_)
{
	ObjectList::serialize(cArchive_);
	cArchive_(m_iExpressionType);
}

//
//	Copyright (c) 1999, 2002, 2005, 2006, 2007, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
