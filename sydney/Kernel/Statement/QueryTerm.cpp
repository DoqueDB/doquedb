// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// QueryTerm.cpp -- QueryTerm
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

#include "Statement/QueryTerm.h"
#include "Statement/Type.h"
#include "Statement/QueryOperator.h"
#include "Statement/QueryTerm.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/QueryTerm.h"
#endif

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_QueryPrimary,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION 
//		Statement::QueryTerm::QueryTerm -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Statement::Object* pQueryPrimary_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
QueryTerm::QueryTerm(Object* pQueryPrimary_)
	: Object(ObjectType::QueryTerm, f__end_index)
{
	setQueryPrimary(pQueryPrimary_);
}

//
//	FUNCTION 
//		Statement::QueryTerm::~QueryTerm -- デストラクタ
//
//	NOTES
//		デストラクタ
//		nextがあればdeleteする。nextはappend中でnewされるため、
//		ここでdeleteしないとリークしてしまう。
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
QueryTerm::~QueryTerm()
{
}

//
//	FUNCTION public
//		Statement::QueryTerm::getQueryPrimary -- QueryPrimary を得る
//
//	NOTES
//		QueryPrimary を得る
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
QueryTerm::getQueryPrimary() const
{
	return m_vecpElements[f_QueryPrimary];
}

//
//	FUNCTION public
//		Statement::QueryTerm::setQueryPrimary -- QueryPrimary を設定する
//
//	NOTES
//		QueryPrimary を設定する
//
//	ARGUMENTS
//		Statement::Object* pQueryPrimary_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
QueryTerm::setQueryPrimary(Object* pQueryPrimary_)
{
	m_vecpElements[f_QueryPrimary] = pQueryPrimary_;
}

//
//	FUNCTION public
//	Statement::QueryTerm::copy -- 自身をコピーする
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
QueryTerm::copy() const
{
	return new QueryTerm(*this);
}

#if 0
namespace
{
	Analysis::QueryTerm _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
QueryTerm::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
