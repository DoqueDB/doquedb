// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TableElementList.cpp -- TableElementList
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Common/Assert.h"
#include "Statement/TableElementList.h"
#include "Statement/Type.h"

#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"
#if 0
#include "Analysis/TableElementList.h"
#endif

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::TableElementList::TableElementList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Statement::Object* pTableElement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableElementList::TableElementList(Object* pTableElement_)
	: ObjectList(ObjectType::TableElementList)
{
	// TableElement を加える
	append(pTableElement_);
}

//
//	FUNCTION public
//	Statement::TableElementList::copy -- 自身をコピーする
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
TableElementList::copy() const
{
	return new TableElementList(*this);
}

// FUNCTION public
//	Statement::TableElementList::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
TableElementList::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	int n = getCount();
	for (int i = 0; i < n; ++i) {
		if (i > 0) stream << ',';
		stream << getAt(i)->toSQLStatement(bForCascade_);
	}
	return stream.getString();
}

#if 0
namespace
{
	Analysis::TableElementList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
TableElementList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
