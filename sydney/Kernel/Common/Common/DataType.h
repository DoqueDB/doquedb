// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataType.h -- データ型関連のクラス定義、関数宣言
// 
// Copyright (c) 1999, 2002, 2004, 2006, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_DATATYPE_H
#define __TRMEISTER_COMMON_DATATYPE_H

#include "Common/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

namespace DataType
{
	//	ENUM
	//	Common::DataType::Type -- データ型を表す列挙型
	//
	//	NOTES
	//	These are permanent values except for MaxScalar,
	//	So a new value has to be added to the front of MaxScalar.
	//
	enum Type
	{
		Data =					1000,

		MinScalar,
		//-------------------------------
		Integer =				MinScalar,
		UnsignedInteger,
		Integer64,
		UnsignedInteger64,
		String,
		Float,
		Double,
		Decimal,
		Date,
		DateTime,
		Binary,
		BitSet,
		ObjectID,
		Language,
		ColumnMetaData,
		Word,
		SearchTerm,				// This is declared in Kernel/Utility.
		//-------------------------------
		MaxScalar,

		Array =					2000,
		
		Null =					3000,

		Default =				4000,
		
		Undefined =				9999
	};
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif //__TRMEISTER_COMMON_DATATYPE_H

//
//	Copyright (c) 1999, 2002, 2004, 2006, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
