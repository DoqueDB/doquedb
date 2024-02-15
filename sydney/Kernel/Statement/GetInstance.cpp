// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// GetInstance.cpp -- クラスIDで指定されたクラスのインスタンスを確保する
// 
// Copyright (c) 2000, 2001, 2012, 2023 Ricoh Company, Ltd.
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
#include "Common/Externalizable.h"
#include "Statement/Externalizable.h"
#include "Statement/Object.h"
#include "Statement/Type.h"

// すべてのクラスのヘッダが必要
#include "Statement/AllType.h"

_SYDNEY_USING
_SYDNEY_STATEMENT_USING

//
//	FUNCTION global
//	Statement::getClassInstance -- クラスのインスタンスを確保する
//
//	NOTES
//	シリアル化可能クラスのクラスIDからそのクラスのインスタンスを確保する。
//
//	ARGUMENTS
//	int iClassID_
//		クラスID
//
//	RETURN
//	Common::Externalizable*
//		シリアル化可能クラスのインスタンス。
//		存在しないクラスIDの場合は0を返す。
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
Common::Externalizable*
Statement::getClassInstance(int iClassID_)
{
	Common::Externalizable* pObject = 0;

	switch (iClassID_ - Statement::Externalizable::ClassID::Base)
	{
#define NEED_INSTANCE
#define TypeDef(name)	case ObjectType::name: pObject = new name; break;
#include "Statement/TypeList.h"
#undef TypeDef
#undef NEED_INSTANCE
	default:
		{
			if (iClassID_ >= Statement::Externalizable::ClassID::ValueExpression) {
				pObject = ValueExpression::getInstance(iClassID_);
			} else if (iClassID_ >= Statement::Externalizable::ClassID::ContainsOperand) {
				pObject = ContainsOperand::getInstance(iClassID_);
			}
			break;
		}
	}

	return pObject;
}

//
//	Copyright (c) 2000, 2001, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
