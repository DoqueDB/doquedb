// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SelectList.cpp -- SelectList
// 
// Copyright (c) 1999, 2002, 2004, 2008, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/SelectList.h"
#include "Statement/Type.h"
#include "Statement/SelectSubListList.h"
#include "Statement/ValueExpression.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"

#include "Exception/NotSupported.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/SelectList_Asterisk.h"
#include "Analysis/SelectList_List.h"
#endif

#include "Analysis/Query/SelectList.h"

#include "ModOstrStream.h"

_SYDNEY_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_SelectSubListList,
		f__end_index
	};
}

using namespace Statement;

//
//	FUNCTION public
//		Statement::SelectList::SelectList -- コンストラクタ (1)
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
//		なし
//
SelectList::SelectList()
	: Object(ObjectType::SelectList, f__end_index)
{
}

//
//	FUNCTION public
//		Statement::SelectList::SelectList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		SelectSubListList* pSelectSubListList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectList::SelectList(SelectSubListList* pSelectSubListList_)
	: Object(ObjectType::SelectList, f__end_index)
{
	// SelectSubListList を設定する
	setSelectSubListList(pSelectSubListList_);
}

//
//	FUNCTION public
//		Statement::SelectList::getSelectSubListList -- SelectSubListList を得る
//
//	NOTES
//		SelectSubListList を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		SelectSubListList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectSubListList*
SelectList::getSelectSubListList() const
{
	SelectSubListList* pResult = 0;
	Object* pObj = m_vecpElements[f_SelectSubListList];
	if ( pObj && ObjectType::SelectSubListList == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(SelectSubListList*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::SelectList::setSelectSubListList -- SelectSubListList を設定する
//
//	NOTES
//		SelectSubListList を設定する
//
//	ARGUMENTS
//		SelectSubListList* pSelectSubListList_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
SelectList::setSelectSubListList(SelectSubListList* pSelectSubListList_)
{
	m_vecpElements[f_SelectSubListList] = pSelectSubListList_;
}

//
//	FUNCTION public
//		Statement::SelectList::asterisk -- select * の * を表わす
//
//	NOTES
//		select * の * を表わす
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		SelectList*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
SelectList*
SelectList::asterisk()
{
	return new SelectList();
}

//
//	FUNCTION public
//		Statement::SelectList::isAsterisk -- *か
//
//	NOTES
//		*か
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//
//	EXCEPTIONS
//		なし
//
bool
SelectList::isAsterisk() const
{
	return m_vecpElements[f_SelectSubListList] == 0;
}

//
//	FUNCTION public
//	Statement::SelectList::copy -- 自身をコピーする
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
SelectList::copy() const
{
	return new SelectList(*this);
}

#ifdef USE_OLDER_VERSION
namespace
{
	Analysis::SelectList_Asterisk _analyzer_asterisk;
	Analysis::SelectList_List _analyzer_sublist;
}

// Analyzerを得る
//virtual
const Analysis::Analyzer*
SelectList::
getAnalyzer() const
{
	if (isAsterisk())
		return &_analyzer_asterisk;
	else
		return &_analyzer_sublist;
}
#endif

// FUNCTION public
//	Statement::SelectList::getAnalyzer2 -- 
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
SelectList::
getAnalyzer2() const
{
	return Analysis::Query::SelectList::create(this);
}

//
//	Copyright (c) 1999, 2002, 2004, 2008, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
