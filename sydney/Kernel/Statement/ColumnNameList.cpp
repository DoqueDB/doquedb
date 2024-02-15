// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ColumnNameList.cpp -- ColumnNameList
// 
// Copyright (c) 1999, 2000, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ColumnNameList.h"
#include "Statement/Type.h"
#include "Statement/ColumnName.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#if 0
#include "Analysis/ColumnNameList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::ColumnNameList::ColumnNameList -- コンストラクタ (1)
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
//	その他
//		下位の例外はそのまま再送
//
ColumnNameList::ColumnNameList()
	: ObjectList(ObjectType::ColumnNameList)
{
}

//
//	FUNCTION public
//		Statement::ColumnNameList::ColumnNameList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ColumnName* pColumnName_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnNameList::ColumnNameList(ColumnName* pColumnName_)
	: ObjectList(ObjectType::ColumnNameList)
{
	// ColumnName を加える
	append(pColumnName_);
}

//
//	FUNCTION public
//		Statement::ColumnNameList::getColumnNameAt -- ColumnName を得る
//
//	NOTES
//		ColumnName を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		ColumnName*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ColumnName*
ColumnNameList::getColumnNameAt(int iAt_) const
{
	ColumnName* pResult = 0;
	Object* pObj = m_vecpElements[iAt_];
	if ( pObj && ObjectType::ColumnName == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ColumnName*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::ColumnNameList::copy -- 自身をコピーする
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
ColumnNameList::copy() const
{
	return new ColumnNameList(*this);
}

// FUNCTION public
//	Statement::ColumnNameList::toSQLStatement -- SQL文で値を得る
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
ColumnNameList::
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
	Analysis::ColumnNameList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ColumnNameList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2000, 2003, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
