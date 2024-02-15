// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReturnStatement.cpp -- ReturnStatement
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Statement/ReturnStatement.h"
#include "Statement/ValueExpression.h"

#include "Analysis/Procedure/ReturnStatement.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace {
	// メンバのm_vecpElements内でのindex
	enum {
		f_Value,
		f__end_index
	};
}

//
//	FUNCTION public
//		Statement::ReturnStatement::ReturnStatement -- コンストラクタ
//
//	NOTES
//		コンストラクタ (2)
//
//	ARGUMENTS
//		ValueExpression* pValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ReturnStatement::ReturnStatement(ValueExpression* pValue_)
	: Object(ObjectType::ReturnStatement, f__end_index)
{
	// Value を設定する
	setValue(pValue_);
}

//
//	FUNCTION public
//		Statement::ReturnStatement::getValue -- Value を得る
//
//	NOTES
//		Value を得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ValueExpression*
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
ValueExpression*
ReturnStatement::getValue() const
{
	ValueExpression* pResult = 0;
	Object* pObj = m_vecpElements[f_Value];
	if ( pObj && ObjectType::ValueExpression == pObj->getType() )
		pResult = _SYDNEY_DYNAMIC_CAST(ValueExpression*, pObj);
	return pResult;
}

//
//	FUNCTION public
//		Statement::ReturnStatement::setValue -- Value を設定する
//
//	NOTES
//		Value を設定する
//
//	ARGUMENTS
//		ValueExpression* pValue_
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
ReturnStatement::setValue(ValueExpression* pValue_)
{
	m_vecpElements[f_Value] = pValue_;
}

// FUNCTION public
//	Statement::ReturnStatement::toSQLStatement -- SQL文で値を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

//virtual
ModUnicodeString
ReturnStatement::
toSQLStatement(bool bForCascade_ /* = false */) const
{
	ValueExpression* pValue = getValue();
	if (pValue == 0) {
		return ModUnicodeString();
	}
	return pValue->toSQLStatement(bForCascade_);
}

//
//	FUNCTION public
//	Statement::ReturnStatement::copy -- 自身をコピーする
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
ReturnStatement::copy() const
{
	return new ReturnStatement(*this);
}

// FUNCTION public
//	Statement::ReturnStatement::getAnalyzer2 -- 
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
ReturnStatement::
getAnalyzer2() const
{
	return Analysis::Procedure::ReturnStatement::create(this);
}

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
