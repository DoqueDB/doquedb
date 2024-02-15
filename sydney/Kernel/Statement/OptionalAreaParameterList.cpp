// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OptionalAreaParameterList.cpp -- OptionalAreaParameterList
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
#include "Common/Assert.h"
#include "Statement/SQLParser.h"
#include "Statement/Type.h"
#include "Statement/IntegerValue.h"
#include "Statement/AlterAreaStatement.h"
#include "Statement/DropAreaStatement.h"
#include "Statement/OptionalAreaParameterList.h"

#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "ModOstrStream.h"

#include "Exception/NotSupported.h"
#if 0
#include "Analysis/OptionalAreaParameterList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::OptionalAreaParameterList::OptionalAreaParameterList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		AlterAreaStatement* pOptionalAreaParameter_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
OptionalAreaParameterList::OptionalAreaParameterList(AlterAreaStatement* pOptionalAreaParameter_)
	: ObjectList(ObjectType::OptionalAreaParameterList)
{
	// AlterAreaStatement を加える
	append(pOptionalAreaParameter_);
}

//
//	FUNCTION public
//		Statement::OptionalAreaParameterList::OptionalAreaParameterList -- コンストラクタ (3)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		DropAreaStatement* pOptionalAreaParameter_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
OptionalAreaParameterList::OptionalAreaParameterList(DropAreaStatement* pOptionalAreaParameter_)
	: ObjectList(ObjectType::OptionalAreaParameterList)
{
	// DropAreaStatement を加える
	append(pOptionalAreaParameter_);
}

#if 0
namespace
{
	Analysis::OptionalAreaParameterList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
OptionalAreaParameterList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
