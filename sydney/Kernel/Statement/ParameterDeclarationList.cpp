// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ParameterDeclarationList.cpp -- ParameterDeclarationList
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#include "Statement/ParameterDeclarationList.h"
#include "Statement/ParameterDeclaration.h"

#include "Analysis/Procedure/ParameterDeclarationList.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION public
//		Statement::ParameterDeclarationList::ParameterDeclarationList -- コンストラクタ (1)
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
//	その他
//		下位の例外はそのまま再送
//
ParameterDeclarationList::ParameterDeclarationList()
	: ObjectList(ObjectType::ParameterDeclarationList)
{
}

//
//	FUNCTION public
//		Statement::ParameterDeclarationList::ParameterDeclarationList -- コンストラクタ (2)
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ParameterDeclaration* pParameterDeclaration_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ParameterDeclarationList::ParameterDeclarationList(ParameterDeclaration* pParameterDeclaration_)
	: ObjectList(ObjectType::ParameterDeclarationList)
{
	// ParameterDeclaration を加える
	append(pParameterDeclaration_);
}

//
//	FUNCTION public
//		Statement::ParameterDeclarationList::getParameterDeclarationAt -- ParameterDeclaration を得る
//
//	NOTES
//		ParameterDeclaration を得る
//
//	ARGUMENTS
//		int iAt_
//
//	RETURN
//		ParameterDeclaration*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ParameterDeclaration*
ParameterDeclarationList::getParameterDeclarationAt(int iAt_) const
{
	ParameterDeclaration* pResult = 0;
	Object* pObj = m_vecpElements[iAt_];
	if ( pObj && ObjectType::ParameterDeclaration == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ParameterDeclaration*, pObj);
	return pResult;
}

//
//	FUNCTION public
//	Statement::ParameterDeclarationList::copy -- 自身をコピーする
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
ParameterDeclarationList::copy() const
{
	return new ParameterDeclarationList(*this);
}

// FUNCTION public
//	Statement::ParameterDeclarationList::getAnalyzer2 -- 
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
ParameterDeclarationList::
getAnalyzer2() const
{
	return Analysis::Procedure::ParameterDeclarationList::create(this);
}

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
