// -*-Mode: C++; tab-width: 4; c-basic-offmap: 4;-*-
// vi:map ts=4 sw=4:
//
// FunctionMap.h -- スキーマオブジェクトのマップを表すクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_FUNCTION_MAP_H
#define	__SYDNEY_SCHEMA_FUNCTION_MAP_H

#include "Schema/ObjectMap.h"
#include "Schema/Function.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	CLASS public
//	Schema::FunctionMap --
//
//	NOTES

class FunctionMap
	: public ObjectMap<Function, FunctionPointer>
{
public:
	FunctionMap();

	// FunctionMapのIterationからオブジェクトを除外するのに使用する比較関数
	static bool omitByID(Function* pFunction_, Object::ID::Value iID_);
};

//	FUNCTION public
//	Schema::FunctionMap::omitByID -- IDでIterationから除外するオブジェクトを探すための比較関数
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Function* pFunction_
//			比較対象のオブジェクト
//		Schema::Object::ID::Value iID_
//			条件となる値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
inline bool
FunctionMap::
omitByID(Function* pFunction_, Object::ID::Value iID_)
{
	return (pFunction_->getID() == iID_);
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif // __SYDNEY_SCHEMA_FUNCTION_MAP_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
