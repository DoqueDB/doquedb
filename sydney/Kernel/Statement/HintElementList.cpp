// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// HintElementList.cpp -- HintElementList
// 
// Copyright (c) 1999, 2000, 2003, 2005, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/HintElementList.h"
#include "Statement/Type.h"
#include "Statement/HintElement.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModUnicodeOstrStream.h"
#if 0
#include "Analysis/HintElementList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::HintElementList::HintElementList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		HintElement* pHintElement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
HintElementList::HintElementList(HintElement* pHintElement_)
	: ObjectList(ObjectType::HintElementList)
{
	// HintElement を加える
	append(pHintElement_);
}

//
//	FUNCTION public
//		Statement::HintElementList::getHintElementAt -- HintElement を得る
//
//	NOTES
//		HintElement を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		HintElement*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
HintElement*
HintElementList::getHintElementAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(HintElement*, getAt(iAt_));
}

// FUNCTION public
//	Statement::HintElementList::toSQLStatement -- SQL文で値を得る
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
HintElementList::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ModUnicodeOstrStream stream;
	stream << '(';
	int n = getCount();
	for (int i = 0; i < n; ++i) {
		if (i > 0) stream << ',';
		stream << getAt(i)->toSQLStatement(bForCascade_);
	}
	stream << ')';
	return stream.getString();
}

#if 0
namespace
{
	Analysis::HintElementList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
HintElementList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2000, 2003, 2005, 2006, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
