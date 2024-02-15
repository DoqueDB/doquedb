// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LimitSpecification.cpp -- LimitSpecification
// 
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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

#include "Statement/LimitSpecification.h"
#include "Statement/ValueExpression.h"

//#include "Analysis/LimitSpecification.h"

#include "ModOstrStream.h"


_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Limit,
		f_Offset,
		f__end_index
	};
}

_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::LimitSpecification::LimitSpecification -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		ValueExpression* limit
//		ValueExpression* offset
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
LimitSpecification::LimitSpecification(ValueExpression* limit, ValueExpression* offset)
	: Object(ObjectType::LimitSpecification, f__end_index)
{
	// Limit を設定する
	setLimit(limit);
	if (offset)
		// Offset を設定する
		setOffset(offset);
}

//
//	FUNCTION public
//		Statement::LimitSpecification::getLimit -- Limit を得る
//
//	NOTES
//		Limit を得る
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
LimitSpecification::getLimit() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(f_Limit,
				   ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::LimitSpecification::setLimit -- Limit を設定する
//
//	NOTES
//		Limit を設定する
//
//	ARGUMENTS
//		ValueExpression* pLimit_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
LimitSpecification::setLimit(ValueExpression* pLimit_)
{
	setElement(f_Limit, pLimit_);
}

//
//	FUNCTION public
//		Statement::LimitSpecification::getOffset -- Offset を得る
//
//	NOTES
//		Offset を得る
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
ValueExpression*
LimitSpecification::getOffset() const
{
	return _SYDNEY_DYNAMIC_CAST(
		ValueExpression*,
		getElement(f_Offset,
				   ObjectType::ValueExpression));
}

//
//	FUNCTION public
//		Statement::LimitSpecification::setOffset -- Offset を設定する
//
//	NOTES
//		Offset を設定する
//
//	ARGUMENTS
//		int iOffset_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
LimitSpecification::setOffset(ValueExpression* offset)
{
	setElement(f_Offset, offset);
}

//
//	FUNCTION public
//	Statement::LimitSpecification::copy -- 自身をコピーする
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
LimitSpecification::copy() const
{
	return new LimitSpecification(*this);
}

#if 0
namespace
{
	Analysis::LimitSpecification _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
LimitSpecification::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
