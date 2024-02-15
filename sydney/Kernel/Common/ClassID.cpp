// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ClassID.cpp -- クラス ID 関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/ClassID.h"
#include "Common/Status.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/FloatData.h"
#include "Common/DoubleData.h"
#include "Common/DecimalData.h"
#include "Common/StringData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#include "Common/IntegerArrayData.h"
#include "Common/UnsignedIntegerArrayData.h"
#include "Common/StringArrayData.h"
#include "Common/DataArrayData.h"
#include "Common/BinaryData.h"
#include "Common/NullData.h"
#include "Common/DefaultData.h"
#include "Common/ExceptionObject.h"
#include "Common/Parameter.h"
#include "Common/BitSet.h"
#include "Common/CompressedStringData.h"
#include "Common/CompressedBinaryData.h"
#ifdef OBSOLETE
#include "Common/CompressedStreamStringData.h"
#include "Common/CompressedStreamBinaryData.h"
#endif
#include "Common/ObjectIDData.h"
#include "Common/Request.h"
#include "Common/LanguageData.h"
#include "Common/SQLData.h"
#include "Common/ColumnMetaData.h"
#include "Common/ResultSetMetaData.h"
#include "Common/WordData.h"
#include "Common/ErrorLevel.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//	FUNCTION
//	Common::getClassInstance -- シリアル化可能クラスのインスタンスを確保する
//
//	NOTES
//
//	ARUMENTS
//		int		iClassID_
//			インスタンスを確保するシリアル化可能クラスのクラス ID
//
//	RETURN
//		0 以外の値
//			得られたインスタンスを格納する領域の先頭アドレス
//		0
//			存在しないクラス ID が指定された
//
//	EXCEPTIONS

Common::Externalizable*
Common::getClassInstance(int iClassID_)
{
	Externalizable* pObject = 0;
	
	switch (iClassID_)
	{
	case ClassID::StatusClass:
		pObject = new Status;
		break;
	case ClassID::IntegerDataClass:
		pObject = new IntegerData;
		break;
	case ClassID::UnsignedIntegerDataClass:
		pObject = new UnsignedIntegerData;
		break;
	case ClassID::Integer64DataClass:
		pObject = new Integer64Data;
		break;
	case ClassID::UnsignedInteger64DataClass:
		pObject = new UnsignedInteger64Data;
		break;
	case ClassID::FloatDataClass:
		pObject = new FloatData;
		break;
	case ClassID::DoubleDataClass:
		pObject = new DoubleData;
		break;
	case ClassID::DecimalDataClass:
		pObject = new DecimalData;
		break;
	case ClassID::StringDataClass:
		pObject = new StringData;
		break;
	case ClassID::DateDataClass:
		pObject = new DateData;
		break;
	case ClassID::DateTimeDataClass:
		pObject = new DateTimeData;
		break;
	case ClassID::IntegerArrayDataClass:
		pObject = new IntegerArrayData;
		break;
	case ClassID::UnsignedIntegerArrayDataClass:
		pObject = new UnsignedIntegerArrayData;
		break;
	case ClassID::StringArrayDataClass:
		pObject = new StringArrayData;
		break;
	case ClassID::DataArrayDataClass:
		pObject = new DataArrayData;
		break;
	case ClassID::BinaryDataClass:
		pObject = new BinaryData;
		break;
	case ClassID::NullDataClass:
		pObject = new NullData;
		break;
	case ClassID::DefaultDataClass:
		pObject = new DefaultData;
		break;
	case ClassID::ExceptionClass:
		pObject = new ExceptionObject;
		break;
	case ClassID::ParameterClass:
		pObject = new Parameter;
		break;
	case ClassID::BitSetClass:
		pObject = new BitSet;
		break;
	case ClassID::CompressedStringDataClass:
		// serializeで伸張したものをやりとりするので
		// StringDataを作る
		pObject = new StringData;
		break;
	case ClassID::CompressedBinaryDataClass:
		pObject = new CompressedBinaryData;
		break;
#ifdef OBSOLETE
	case ClassID::CompressedStreamStringDataClass:
		pObject = new CompressedStringData;
		break;
	case ClassID::CompressedStreamBinaryDataClass:
		pObject = new CompressedBinaryData;
		break;
#endif
	case ClassID::ObjectIDDataClass:
		pObject = new ObjectIDData;
		break;
	case ClassID::RequestClass:
		pObject = new Request;
		break;
	case ClassID::LanguageDataClass:
		pObject = new LanguageData;
		break;
	case ClassID::SQLDataClass:
		pObject = new SQLData;
		break;
	case ClassID::ColumnMetaDataClass:
		pObject = new ColumnMetaData;
		break;
	case ClassID::ResultSetMetaDataClass:
		pObject = new ResultSetMetaData;
		break;
	case ClassID::WordDataClass:
		pObject = new WordData;
		break;
	case ClassID::ErrorLevelClass:
		pObject = new ErrorLevel;
		break;
	}

	return pObject;
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
