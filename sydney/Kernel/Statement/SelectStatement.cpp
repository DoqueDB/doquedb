// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SelectStatement.cpp -- SelectStatement
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

#include "Statement/SelectStatement.h"
#include "Statement/Type.h"
#include "Statement/QuerySpecification.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/SelectStatement.h"
#endif

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_QuerySpecification,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::SelectStatement::SelectStatement -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		QuerySpecification* pQuerySpecification_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectStatement::SelectStatement(QuerySpecification* pQuerySpecification_)
	: Object(ObjectType::SelectStatement, f__end_index, Object::Optimize)
{
	// QuerySpecification を設定する
	setQuerySpecification(pQuerySpecification_);
}

//
//	FUNCTION public
//		Statement::SelectStatement::getQuerySpecification -- QuerySpecification を得る
//
//	NOTES
//		QuerySpecification を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		QuerySpecification*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QuerySpecification*
SelectStatement::getQuerySpecification() const
{
	QuerySpecification* pResult = 0;
	Object* pObj = m_vecpElements[f_QuerySpecification];
	if ( pObj && ObjectType::QuerySpecification == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(QuerySpecification*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::SelectStatement::setQuerySpecification -- QuerySpecification を設定する
//
//	NOTES
//		QuerySpecification を設定する
//
//	ARGUMENTS
//		QuerySpecification* pQuerySpecification_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SelectStatement::setQuerySpecification(QuerySpecification* pQuerySpecification_)
{
	m_vecpElements[f_QuerySpecification] = pQuerySpecification_;
}

//
//	FUNCTION public
//	Statement::SelectStatement::copy -- 自身をコピーする
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
SelectStatement::copy() const
{
	return new SelectStatement(*this);
}

#if 0
namespace
{
	Analysis::SelectStatement _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SelectStatement::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
