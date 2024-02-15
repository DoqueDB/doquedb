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
#include "Statement/IntegerValue.h"
#include "Statement/Literal.h"
#include "Statement/VariableName.h"

#include "Statement/Identifier.h"




_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Identifier,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::VariableName::VariableName -- コンストラクタ(2)
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
VariableName::VariableName(Identifier* pId_)
	: Object(ObjectType::VariableName, f__end_index)
{
	setIdentifier(pId_);
}


//	FUNCTION public
//	Statement::VariableName::getVariableName
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

const ModUnicodeString*
VariableName::getName() const
{
	Identifier* pResult = 0;
	Object* pObj = m_vecpElements[f_Identifier];
	if ( pObj && ObjectType::Identifier == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(Identifier*, pObj);	

	return pResult->getIdentifier();
}



//
//	FUNCTION public
//		Statement::VariableName::setIdentifier -- Identifier を設定する
//
//	NOTES
//		Identifier を設定する
//
//	ARGUMENTS
//		Identifier* pIdentifier_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
VariableName::setIdentifier(Identifier* pIdentifier_)
{
	m_vecpElements[f_Identifier] = pIdentifier_;
}




//
//	FUNCTION public
//	Statement::VariableName::copy -- 自身をコピーする
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
VariableName::copy() const
{
	return new VariableName(*this);
}

//
//	Copyright (c) 2000, 2002, 2004, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
