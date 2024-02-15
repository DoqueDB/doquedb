// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ClassID.h -- クラス ID 関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_CLASSID_H
#define __TRMEISTER_COMMON_CLASSID_H

#include "Common/Module.h"
#include "Common/Externalizable.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

namespace ClassID
{
	//	ENUM
	//	Common::ClassID::Value -- シリアル化可能クラスのクラスIDを表す列挙型
	//
	//	NOTES
	//	These are permanent values,
	//	so a new value has to be added to the end of the list.
	//
	enum Value
	{
		StatusClass	=					Externalizable::CommonClasses,
		IntegerDataClass,
		UnsignedIntegerDataClass,
		Integer64DataClass,
		UnsignedInteger64DataClass,
		FloatDataClass,
		DoubleDataClass,
		DecimalDataClass,
		StringDataClass,
		DateDataClass,
		DateTimeDataClass,
		IntegerArrayDataClass,
		UnsignedIntegerArrayDataClass,
		StringArrayDataClass,
		DataArrayDataClass,
		BinaryDataClass,
		NullDataClass,
		ExceptionClass,
		ParameterClass,
		BitSetClass,
		CompressedStringDataClass,
		CompressedBinaryDataClass,
#ifdef OBSOLETE
		CompressedStreamStringDataClass,
		CompressedStreamBinaryDataClass,
#endif
		ObjectIDDataClass,
		RequestClass,
		LanguageDataClass,
		SQLDataClass,
		ColumnMetaDataClass,
		ResultSetMetaDataClass,
		WordDataClass,
		ErrorLevelClass,
		DefaultDataClass,
		SearchTermDataClass				// This is declared in Kernel/Utility.
	};
}

//	FUNCTION
//	Common::getClassInstance -- シリアル化可能クラスのインスタンスを得る
//
//	NOTES

SYD_COMMON_FUNCTION
Common::Externalizable*
getClassInstance(int iClassID_);

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_CLASSID_H

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
