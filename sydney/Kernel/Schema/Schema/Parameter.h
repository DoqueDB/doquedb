// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parameter.h -- パラメーター関連のクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_PARAMETER_H
#define	__SYDNEY_SCHEMA_PARAMETER_H

#include "Schema/Module.h"
#include "Common/UnicodeString.h"
#include "LogicalFile/Parameter.h"

#include "ModCharString.h"
#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

namespace Parameter
{
	//	CONST
	//	Schema::Parameter::Prefix -- Parameterのキーに添付されるプレフィックス
	//
	//	NOTES

	const char* const Prefix = "Schema_";

	//	TYPEDEF
	//	Schema::Parameter::KeyType -- Parameterのキーに使われる型
	//
	//	NOTES

	typedef	LogicalFile::Parameter::Key		KeyType;

#define _SYDNEY_SCHEMA_PARAMETER_KEY(key_) Parameter::KeyType(key_, 0)
#define _SYDNEY_SCHEMA_FORMAT_KEY(key_, arg_) \
									Parameter::KeyType(key_, arg_)

	//	TYPEDEF
	//	Schema::Parameter::ValueType -- Parameterのバリュー(文字列)に使われる型
	//
	//	NOTES

	typedef	ModUnicodeString	ValueType;

#define _SYDNEY_SCHEMA_PARAMETER_VALUE(value_) value_

	namespace Key
	{

#ifdef USE_PHYSICALLOG_AREA

		//	CONST
		//	Schema::Parameter::Key::LogArea -- ファイルIDで物理ログのエリアを設定するキー文字列
		//
		//	NOTES

		const char* const			LogArea		= "LogArea[%d]";
#endif
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_PARAMETER_H

//
// Copyright (c) 2000, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
