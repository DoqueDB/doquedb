// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectSelection.cpp --
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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
#include "Statement/ObjectSelection.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

namespace
{
	// メンバのm_vecpElements内でのindex
	enum
	{
		f_ObjectType,
		f_Object,
		f__end_index
	};
}

//
//	FUNCTION public
//	Statement::ObjectConnection::ObjectSelection
//		-- コンストラクタ
//
//	NOTES
//	コンストラクタ
//
//	ARGUMENTS
//	なし
//
//	RETURN
//
//	EXCEPTIONS
//	なし
//
ObjectSelection::ObjectSelection(ObjectType::Type eType_)
	: Object(eType_, f__end_index)
{
}

//	FUNCTION public
//	Statement::ObjectSelection::getObjectType -- ObjectType を取得する
//
//	NOTES
//	ObjectType を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		ObjectType
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出

int
ObjectSelection::getObjectType() const
{
	const IntegerValue* pElement = _SYDNEY_DYNAMIC_CAST(
		const IntegerValue*, m_vecpElements[f_ObjectType]);
	; _SYDNEY_ASSERT(pElement);

	return pElement->getValue();
}

//
//	FUNCTION public
//	Statement::ObjectSelection::setObjectType -- ObjectType を設定する
//
//	NOTES
//	ObjectType を設定する
//
//	ARGUMENTS
//	int iActType_
//		ObjectType
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
ObjectSelection::setObjectType(int iType_)
{
	IntegerValue* pElement = _SYDNEY_DYNAMIC_CAST(IntegerValue*, m_vecpElements[f_ObjectType]);
	if (pElement == 0)
	{
		pElement = new IntegerValue;
		m_vecpElements[f_ObjectType] = pElement;
	}
	pElement->setValue(iType_);
}

//
//	FUNCTION public
//	Statement::ObjectSelection::getScaler
//		-- スカラ値を得る
//
//	NOTES
//	スカラ値を得る
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	int
//		スカラ値
//
//	EXCEPTIONS
//	なし
//
int
ObjectSelection::getScaler() const
{
	IntegerValue* pElement = _SYDNEY_DYNAMIC_CAST(IntegerValue*, getObject());
	; _SYDNEY_ASSERT(pElement);

	return  pElement->getValue();
}

//
//	FUNCTION public
//	Statement::ObjectSelection::setScaler
//		-- スカラ値を設定する
//
//	NOTES
//	スカラ値を設定する
//
//	ARGUMENTS
//	int
//		スカラ値
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
void
ObjectSelection::setScaler(int iValue_)
{
	IntegerValue* pElement = _SYDNEY_DYNAMIC_CAST(IntegerValue*, getObject());
	if (pElement == 0)
	{
		pElement = new IntegerValue;
		setObject(pElement);
	}
	pElement->setValue(iValue_);
}

//
//	FUNCTION public
//	Statement::ObjectSelection::getObject -- Object を取得する
//
//	NOTES
//	Object を取得する
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	Object*
//		Object
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
Object*
ObjectSelection::getObject() const
{
	return m_vecpElements[f_Object];
}


//
//	FUNCTION public
//	Statement::ObjectSelection::setObject -- Object を設定する
//
//	NOTES
//	Object を設定する
//
//	ARGUMENTS
//	Object* pcObject_
//		Object
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま送出
//
void
ObjectSelection::setObject(Statement::Object* pcObject_)
{
	_SYDNEY_ASSERT(m_vecpElements[f_Object] == 0);
	m_vecpElements[f_Object] = pcObject_;
}

//
//	Copyright (c) 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
