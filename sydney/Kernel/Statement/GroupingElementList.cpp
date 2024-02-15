// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GroupingElementList.cpp -- GroupingElementList
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2010, 2023 Ricoh Company, Ltd.
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

#include "Statement/GroupingElementList.h"
#include "Statement/GroupingColumnReferenceList.h"
#include "Statement/Type.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#include "ModOstrStream.h"
#if 0
#include "Analysis/GroupingElementList.h"
#endif

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::GroupingElementList::GroupingElementList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		GroupingElement* pGroupingElement_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
GroupingElementList::GroupingElementList(Object* pElement_)
	: ObjectList(ObjectType::GroupingElementList)
{
	append(pElement_);
}

// FUNCTION public
//	Statement::GroupingElementList::addElement -- 要素を追加する
//
// NOTES
//
// ARGUMENTS
//	Object* pElement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
GroupingElementList::
addElement(Object* pElement_)
{
	if (getCount() == 1
		&& getAt(0)->getType() == ObjectType::GroupingColumnReferenceList
		&& pElement_->getType() == ObjectType::GroupingColumnReferenceList) {
		_SYDNEY_DYNAMIC_CAST(GroupingColumnReferenceList*, getAt(0))
			->merge(*_SYDNEY_DYNAMIC_CAST(GroupingColumnReferenceList*, pElement_));
		delete pElement_;
	} else {
		append(pElement_);
	}
}

#if 0
namespace
{
	Analysis::GroupingElementList _analyzer;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
GroupingElementList::
getAnalyzer() const
{
	return &_analyzer;
}
#endif

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
