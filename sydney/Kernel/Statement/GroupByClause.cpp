// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GroupByClause.cpp -- GroupByClause
// 
// Copyright (c) 1999, 2001, 2003, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
#include "Statement/GroupByClause.h"
#include "Statement/Type.h"
#include "Statement/GroupingElementList.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "ModOstrStream.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/GroupByClause.h"
#endif
#include "Analysis/Query/GroupByClause.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_GroupingElementList,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::GroupByClause::GroupByClause -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		GtoupingElementList* pGroupingElementList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
GroupByClause::GroupByClause(GroupingElementList* pGroupingElementList_)
	: Object(ObjectType::GroupByClause, f__end_index),
	  m_bHavingBy(false)
{
	setGroupingElementList(pGroupingElementList_);
}

//	FUNCTION public
//		Statement::GroupByClause::getGroupingElementList -- GroupingElementList を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		GroupingElementList*
//
//	EXCEPTIONS

GroupingElementList*
GroupByClause::getGroupingElementList() const
{
	GroupingElementList* pResult = 0;
	Object* pObj = m_vecpElements[f_GroupingElementList];
	if ( pObj && ObjectType::GroupingElementList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(GroupingElementList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::GroupByClause::setGroupingElementList -- GroupingElementList を設定する
//
//	NOTES
//		GroupingElementList を設定する
//
//	ARGUMENTS
//		GroupingElementList* pGroupingElementList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
GroupByClause::setGroupingElementList(GroupingElementList* pGroupingElementList_)
{
	m_vecpElements[f_GroupingElementList] = pGroupingElementList_;
}


#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::GroupByClause _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
GroupByClause::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

// FUNCTION public
//	Statement::GroupByClausue::getAnalyzer2 -- 
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
GroupByClause::
getAnalyzer2() const
{
	return Analysis::Query::GroupByClause::create(this);
}

// FUNCTION public
//	Statement::GroupByClause::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
GroupByClause::
serialize(ModArchive& cArchive_)
{
	Object::serialize(cArchive_);
	cArchive_(m_bHavingBy);
}

//
//	Copyright (c) 1999, 2001, 2003, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
