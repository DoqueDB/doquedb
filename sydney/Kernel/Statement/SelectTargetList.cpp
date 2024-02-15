// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SelectTargetList.cpp -- SelectTargetList
// 
// Copyright (c) 1999, 2002, 2004, 2008, 2011, 2023 Ricoh Company, Ltd.
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

#include "Statement/SelectTargetList.h"
#include "Statement/Type.h"
#include "Statement/VariableName.h"
#include "Statement/AllType.h"
#include "Statement/DeclareStatement.h"

#include "Common/Assert.h"
#include "Common/OutputArchive.h"
#include "Common/InputArchive.h"



#include "ModOstrStream.h"

_SYDNEY_USING



using namespace Statement;

//
//	FUNCTION public
//		Statement::SelectTargetList::SelectTargetList -- コンストラクタ (1)
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
SelectTargetList::SelectTargetList()
	: ObjectList(ObjectType::SelectTargetList)
{
}

//
//	FUNCTION public
//		Statement::SelectTargetList::SelectTargetList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		VariableName
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
//
SelectTargetList::SelectTargetList(VariableName* pVariableName_)
	: ObjectList(ObjectType::SelectTargetList)
{
	append(pVariableName_);
}


//
//	FUNCTION public
//		Statement::SelectTargetList::getVariableNameAt -- VariableName を得る
//
//	NOTES
//		VariableName を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		VariableName*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
VariableName*
SelectTargetList::getVariableNameAt(int iAt_) const
{
	VariableName* pResult;
	Object* pObj = m_vecpElements[iAt_];
	if ( pObj && ObjectType::VariableName == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(VariableName*, pObj);

	return pResult;
}


//
//	FUNCTION public
//	Statement::SelectTargetList::copy -- 自身をコピーする
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
SelectTargetList::copy() const
{
	return new SelectTargetList(*this);
}




// FUNCTION public
//	Statement::SelectTargetList::getAnalyzer2 -- 
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
SelectTargetList::
getAnalyzer2() const
{
	return 0;
	// Analysis::Query::SelectTargetList::create(this);
}

//
//	Copyright (c) 1999, 2002, 2004, 2008, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
