// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// UpdateSetClauseList.cpp -- UpdateSetClauseList
// 
// Copyright (c) 1999, 2002, 2003, 2005, 2006, 2010, 2023 Ricoh Company, Ltd.
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

#include "Statement/UpdateSetClauseList.h"
#include "Statement/Type.h"
#include "Statement/UpdateSetClause.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/UpdateSetClauseList.h"
#endif
#include "Analysis/Operation/UpdateSetClauseList.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::UpdateSetClauseList::UpdateSetClauseList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		UpdateSetClause* pSetClause_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
UpdateSetClauseList::UpdateSetClauseList(UpdateSetClause* pSetClause_)
	: ObjectList(ObjectType::UpdateSetClauseList)
{
	// SetClause を加える
	append(pSetClause_);
}

//
//	FUNCTION public
//		Statement::UpdateSetClauseList::getSetClauseAt -- SetClause を得る
//
//	NOTES
//		SetClause を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		UpdateSetClause*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
UpdateSetClause*
UpdateSetClauseList::getSetClauseAt(int iAt_) const
{
	return _SYDNEY_DYNAMIC_CAST(UpdateSetClause*, getAt(iAt_));
}

//
//	FUNCTION public
//	Statement::UpdateSetClauseList::copy -- 自身をコピーする
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
UpdateSetClauseList::copy() const
{
	return new UpdateSetClauseList(*this);
}

#if 0
namespace
{
	Analysis::UpdateSetClauseList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
UpdateSetClauseList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::UpdateSetClauseList::getAnalyzer2 -- 
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
UpdateSetClauseList::
getAnalyzer2() const
{
	return Analysis::Operation::UpdateSetClauseList::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2005, 2006, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
