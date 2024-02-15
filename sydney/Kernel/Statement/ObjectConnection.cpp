// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectConnection.cpp --
// 
// Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
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
#include "Statement/ObjectConnection.h"

_SYDNEY_USING

using namespace Statement;

namespace {
}

//
//	FUNCTION public
//	Statement::ObjectConnection::ObjectConnection
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
ObjectConnection::ObjectConnection(ObjectType::Type eType_, unsigned int iNum_, Generate eGen_, bool bLogRec_)
	: Object(eType_, iNum_, eGen_, bLogRec_)
{
}

//
//	FUNCTION public
//	Statement::ObjectConnection::getScaler
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
ObjectConnection::getScaler(int iIndex_) const
{
	IntegerValue* pElement = _SYDNEY_DYNAMIC_CAST(IntegerValue*, getObject(iIndex_));
	; _SYDNEY_ASSERT(pElement);

	return  pElement->getValue();
}

//
//	FUNCTION public
//	Statement::ObjectConnection::setScaler
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
ObjectConnection::setScaler(int iIndex_, int iValue_)
{
	IntegerValue* pElement = _SYDNEY_DYNAMIC_CAST(IntegerValue*, getObject(iIndex_));

	if (pElement == 0)
	{
		pElement = new IntegerValue;
		setObject(iIndex_, pElement);
	}

	pElement->setValue(iValue_);
}

//
//	Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
