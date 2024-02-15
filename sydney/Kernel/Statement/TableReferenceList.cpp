// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TableReferenceList.cpp -- TableReferenceList
// 
// Copyright (c) 1999, 2002, 2003, 2006, 2008, 2023 Ricoh Company, Ltd.
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
#include "Statement/TableReferenceList.h"
#include "Statement/Type.h"

#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"
#include "ModOstrStream.h"

#include "Exception/NotSupported.h"

#include "Analysis/Query/TableReferenceList.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::TableReferenceList::TableReferenceList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		Object* pObject_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
TableReferenceList::TableReferenceList(Object* pObject_)
	: ObjectList(ObjectType::TableReferenceList)
{
	// TableReference を加える
	append(pObject_);
}

//
//	FUNCTION public
//	Statement::TableReferenceList::copy -- 自身をコピーする
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
TableReferenceList::copy() const
{
	return new TableReferenceList(*this);
}

// FUNCTION public
//	Statement::TableReferenceList::getAnalyzer2 -- Analyzerを得る
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
TableReferenceList::
getAnalyzer2() const
{
	return Analysis::Query::TableReferenceList::create(this);
}

//
//	Copyright (c) 1999, 2002, 2003, 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

