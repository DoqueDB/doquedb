// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Meta.cpp -- システム表関連の関数定義
// 
// Copyright (c) 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Schema/Meta.h"
#include "Schema/ObjectID.h"
#include "LogicalFile/ObjectID.h"

#include "ModPair.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace 
{
	typedef ModPair<Common::DataType::Type, Common::DataType::Type> _Types;

	_Types _vecFieldType[] =
	{
		_Types(Common::DataType::Undefined, Common::DataType::Undefined), // Unknown
		_Types(LogicalFile::ObjectID().getType(), Common::DataType::Undefined), // FileOID
		_Types(ObjectID().getType(), Common::DataType::Undefined), // ObjectID
		_Types(ObjectID().getType(), Common::DataType::Undefined), // ParentID
		_Types(Common::DataType::String, Common::DataType::Undefined), // Name
		_Types(Common::DataType::UnsignedInteger, Common::DataType::Undefined), // Timestamp
		_Types(ObjectID().getType(), Common::DataType::Undefined), // ID
		_Types(Common::DataType::Array, ObjectID().getType()), // IDArray
		_Types(Common::DataType::Integer, Common::DataType::Undefined), // Integer
		_Types(Common::DataType::UnsignedInteger, Common::DataType::Undefined), // UnsignedInteger
		_Types(Common::DataType::String, Common::DataType::Undefined), // String
		_Types(Common::DataType::Array, Common::DataType::String), // StringArray
		_Types(Common::DataType::Binary, Common::DataType::Undefined), // Binary
		_Types(Common::DataType::Integer64, Common::DataType::Undefined), // BigInt
		_Types(Common::DataType::Array, Common::DataType::UnsignedInteger), // UnsignedIntegerArray
		_Types(Common::DataType::Undefined, Common::DataType::Undefined), // ValueNum
	};
}

//	FUNCTION public
//	Meta::getFieldType -- システム表のフィールドに対応する型を得る
//
//	NOTES

Common::DataType::Type
Meta::
getFieldType(Meta::MemberType::Value eType_)
{
	return _vecFieldType[eType_].first;
}

//	FUNCTION public
//	Meta::getFieldElementType -- システム表のフィールドに対応する配列の要素型を得る
//
//	NOTES

Common::DataType::Type
Meta::
getFieldElementType(Meta::MemberType::Value eType_)
{
	return _vecFieldType[eType_].second;
}

//
// Copyright (c) 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
