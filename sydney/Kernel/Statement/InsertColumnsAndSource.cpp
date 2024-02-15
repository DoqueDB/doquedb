// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// InsertColumnsAndSource.cpp -- InsertColumnsAndSource
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

#include "Statement/InsertColumnsAndSource.h"
#include "Statement/Type.h"
#include "Statement/ColumnNameList.h"
#include "Statement/QueryExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/InsertColumnsAndSource.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Query,
		f_ColumnList,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::InsertColumnsAndSource::InsertColumnsAndSource -- コンストラクタ (1)
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
InsertColumnsAndSource::InsertColumnsAndSource()
	: Object(ObjectType::InsertColumnsAndSource, f__end_index)
{
}

//
//	FUNCTION public
//		Statement::InsertColumnsAndSource::InsertColumnsAndSource -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		QueryExpression* pQuery_
//		ColumnNameList* pColumnList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
InsertColumnsAndSource::InsertColumnsAndSource(QueryExpression* pQuery_, ColumnNameList* pColumnList_)
	: Object(ObjectType::InsertColumnsAndSource, f__end_index)
{
	// Query を設定する
	setQuery(pQuery_);
	// ColumnList を設定する
	setColumnList(pColumnList_);
}

//
//	FUNCTION public
//		Statement::InsertColumnsAndSource::getQuery -- Query を得る
//
//	NOTES
//		Query を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		QueryExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QueryExpression*
InsertColumnsAndSource::getQuery() const
{
	QueryExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Query];
	if ( pObj && ObjectType::QueryExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(QueryExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::InsertColumnsAndSource::setQuery -- Query を設定する
//
//	NOTES
//		Query を設定する
//
//	ARGUMENTS
//		QueryExpression* pQuery_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
InsertColumnsAndSource::setQuery(QueryExpression* pQuery_)
{
	m_vecpElements[f_Query] = pQuery_;
}

//
//	FUNCTION public
//		Statement::InsertColumnsAndSource::getColumnList -- ColumnList を得る
//
//	NOTES
//		ColumnList を得る
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
InsertColumnsAndSource::getColumnList() const
{
	ColumnNameList* pResult = 0;
	Object* pObj = m_vecpElements[f_ColumnList];
	if ( pObj && ObjectType::ColumnNameList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnNameList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::InsertColumnsAndSource::setColumnList -- ColumnList を設定する
//
//	NOTES
//		ColumnList を設定する
//
//	ARGUMENTS
//		ColumnNameList* pColumnList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
InsertColumnsAndSource::setColumnList(ColumnNameList* pColumnList_)
{
	m_vecpElements[f_ColumnList] = pColumnList_;
}

#if 0
namespace
{
	Analysis::InsertColumnsAndSource _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
InsertColumnsAndSource::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
