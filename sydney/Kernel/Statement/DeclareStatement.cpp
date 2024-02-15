// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DatabaseDefinition.cpp --
// 
// Copyright (c) 2000, 2002, 2004, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char srcFile[] = __FILE__;
const char moduleName[] = "Statement";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Statement/Type.h"
#include "Statement/Literal.h"
#include "Statement/DeclareStatement.h"
#include "Statement/VariableName.h"
#include "Statement/Identifier.h"

#include "Analysis/Query/Declare.h"


_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_VariableName,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::DeclareStatement::DeclareStatement -- コンストラクタ(2)
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	Identifier* pId_
//		データベース名
//	VariableCreateOptionList* pOption_
//		エリアオプション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
DeclareStatement::DeclareStatement(VariableName* pValName_)
	: Object(ObjectType::DeclareStatement, f__end_index)
{
	m_vecpElements[f_VariableName] = pValName_;
}



//	FUNCTION public
//	Statement::DeclareStatement::getVariableName
//		-- データベース名取得
//
//	NOTES
//	データベース名取得
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Identifier*
//		データベース名
//
//	EXCEPTIONS
//	なし

const VariableName*
DeclareStatement::getVariableName() const
{
	VariableName* pResult = 0;
	Object* pObj = m_vecpElements[f_VariableName];
	if ( pObj && ObjectType::VariableName == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(VariableName*, pObj);	

	return pResult;
}





//
//	FUNCTION public
//	Statement::DeclareStatement::copy -- 自身をコピーする
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
DeclareStatement::copy() const
{
	return new DeclareStatement(*this);
}




// FUNCTION public
//	Statement::ValueExpression::getAnalyzer2 -- 
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
DeclareStatement::
getAnalyzer2() const
{
	return Analysis::Query::Declare::create(this);
}

//
//	Copyright (c) 2000, 2002, 2004, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
