// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SelectSubListList.cpp -- SelectSubListList
// 
// Copyright (c) 1999, 2000, 2003, 2006, 2010, 2012, 2023 Ricoh Company, Ltd.
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

#include "Statement/SelectSubListList.h"
#include "Statement/SelectSubList.h"
#include "Statement/Type.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/SelectSubListList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::SelectSubListList::SelectSubListList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		SelectSubList* pSelectSubList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectSubListList::SelectSubListList(SelectSubList* pSelectSubList_)
	: ObjectList(ObjectType::SelectSubListList),
	  m_iExpressionType(0)
{
	// SelectSubList を加える
	append(pSelectSubList_);
	// ExpressionTypeを設定する
	m_iExpressionType = pSelectSubList_->getExpressionType();
}

//
//	FUNCTION public
//		Statement::SelectSubListList::getSelectSubListAt -- SelectSubList を得る
//
//	NOTES
//		SelectSubList を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		SelectSubList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectSubList*
SelectSubListList::getSelectSubListAt(int iAt_) const
{
	SelectSubList* pResult = 0;
	Object* pObj = m_vecpElements[iAt_];
	if ( pObj && ObjectType::SelectSubList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(SelectSubList*, pObj);
	return pResult;
}

// FUNCTION public
//	Statement::SelectSubListList::getExpressionType -- ExpressionTypeを得る
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
SelectSubListList::
getExpressionType() const
{
	return m_iExpressionType;
}

// FUNCTION public
//	Statement::SelectSubListList::mergeExpressionType -- ExpressionTypeを合成する
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
SelectSubListList::
mergeExpressionType(int iType_)
{
	m_iExpressionType = ValueExpression::mergeExpressionType(m_iExpressionType,
															 iType_);
}

//
//	FUNCTION public
//	Statement::SelectSubListList::copy -- 自身をコピーする
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
SelectSubListList::copy() const
{
	return new SelectSubListList(*this);
}

#if 0
namespace
{
	Analysis::SelectSubListList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SelectSubListList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::SelectSubListList::serialize -- 
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
SelectSubListList::
serialize(ModArchive& cArchive_)
{
	ObjectList::serialize(cArchive_);
	cArchive_(m_iExpressionType);
}

//
//	Copyright (c) 1999, 2000, 2003, 2006, 2010, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
