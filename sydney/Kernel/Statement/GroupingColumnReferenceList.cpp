// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GroupingColumnReferenceList.cpp -- GroupingColumnReferenceList
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/GroupingColumnReferenceList.h"
#include "Statement/Type.h"
#include "Statement/GroupingColumnReference.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/GroupingColumnReferenceList.h"
#endif
#include "Analysis/Query/GroupingColumnReferenceList.h"

#include "ModOstrStream.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::GroupingColumnReferenceList::GroupingColumnReferenceList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		GroupingColumnReference* pGroupingColumnReference_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
GroupingColumnReferenceList::GroupingColumnReferenceList(GroupingColumnReference* pGroupingColumnReference_)
	: ObjectList(ObjectType::GroupingColumnReferenceList)
{
	if (pGroupingColumnReference_)
		append(pGroupingColumnReference_);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::GroupingColumnReferenceList _analyzer;
}

// FUNCTION public
//	Statement::GroupingColumnReferenceList::getAnalyzer -- Analyzerを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Analyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Analyzer*
GroupingColumnReferenceList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::GroupingColumnReferenceList::getAnalyzer2 -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Analysis::Interface::IAnalyzer*
//
// EXCEPTIONS

//virtual
const Analysis::Interface::IAnalyzer*
GroupingColumnReferenceList::
getAnalyzer2() const
{
	return Analysis::Query::GroupingColumnReferenceList::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2010, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
