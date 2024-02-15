// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ContainsOperandList.cpp -- ContainsOperandList
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

#include "Statement/ContainsOperandList.h"
#include "Statement/ContainsOperand.h"

//#include "Analysis/ContainsOperandList.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::ContainsOperandList::ContainsOperandList -- コンストラクタ
//
//	NOTES
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
ContainsOperandList::ContainsOperandList()
	: ObjectList(ObjectType::ContainsOperandList)
{
}

//
//	FUNCTION public
//		Statement::ContainsOperandList::getContainsOperandAt -- ContainsOperand を得る
//
//	NOTES
//		ContainsOperand を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		ContainsOperand*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ContainsOperand*
ContainsOperandList::getContainsOperandAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(ContainsOperand*, getAt(iAt_));
}

//
//	FUNCTION public
//	Statement::ContainsOperandList::copy -- 自身をコピーする
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
ContainsOperandList::copy() const
{
	return new ContainsOperandList(*this);
}

#if 0
namespace
{
	Analysis::ContainsOperandList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
ContainsOperandList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
