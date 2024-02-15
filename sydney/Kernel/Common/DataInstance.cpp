// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// DataInstance.cpp -- データ関連の関数定義
// 
// Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "Common/Assert.h"
#include "Common/DataInstance.h"
#include "Common/IntegerData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/Integer64Data.h"
#include "Common/UnsignedInteger64Data.h"
#include "Common/FloatData.h"
#include "Common/DataArrayData.h"
#include "Common/DateData.h"
#include "Common/DateTimeData.h"
#ifdef OBSOLETE
#include "Common/TimeData.h"
#endif
#include "Common/DecimalData.h"
#include "Common/DoubleData.h"
#include "Common/StringData.h"
#include "Common/BinaryData.h"
#include "Common/NullData.h"
#include "Common/BitSet.h"
#include "Common/ObjectIDData.h"
#include "Common/LanguageData.h"
#include "Common/WordData.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

//	FUNCTION
//	Common::DataInstance::create -- Dataクラスのインスタンスを確保する
//
//	NOTES
//
//	ARUMENTS
//		Common::DataType::Type		eType__
//			インスタンスを確保するデータクラスの種別
//
//	RETURN
//		0 以外の値
//			得られたインスタンスを格納する領域の先頭アドレス
//		0
//			存在しないデータタイプが指定された
//
//	EXCEPTIONS

Common::Data*
Common::DataInstance::create(DataType::Type eType_)
{
	Data* pObject = 0;
	
	switch (eType_)
	{
	case DataType::Integer:
		pObject = new IntegerData;
		break;
	case DataType::UnsignedInteger:
		pObject = new UnsignedIntegerData;
		break;
	case DataType::Integer64:
		pObject = new Integer64Data;
		break;
	case DataType::UnsignedInteger64:
		pObject = new UnsignedInteger64Data;
		break;
	case DataType::Float:
		pObject = new FloatData;
		break;
	case DataType::Double:
		pObject = new DoubleData;
		break;
	case DataType::Decimal:
		pObject = new DecimalData;
		break;
	case DataType::String:
		pObject = new StringData;
		break;
	case DataType::Date:
		pObject = new DateData;
		break;
	case DataType::DateTime:
		pObject = new DateTimeData;
		break;
	case DataType::Binary:
		pObject = new BinaryData;
		break;
	case DataType::BitSet:
		pObject = new BitSet;
		break;
	case DataType::ObjectID:
		pObject = new ObjectIDData;
		break;
	case DataType::Language:
		pObject = new LanguageData;
		break;
	case DataType::Word:
		pObject = new WordData;
		break;
	case DataType::Array:
		pObject = new DataArrayData;
		break;
	case DataType::Data:
	case DataType::Undefined:
		// Nothing to be created for now
		pObject = 0;
		break;
	default:
		; _TRMEISTER_ASSERT(false);
		break;
	}

	return pObject;
}

//	FUNCTION
//	Common::DataInstance::create -- Dataクラスのインスタンスを確保する
//
//	NOTES
//		For array type, result data has element data type
//
//	ARUMENTS
//		const Common::SQLData&	cType__
//			インスタンスを確保するデータに対応するSQLデータ型SQL data type corresponding to data that secures instance
//
//	RETURN
//		0 以外の値
//			得られたインスタンスを格納する領域の先頭アドレス
//		0
//			存在しないSQLデータ型が指定された
//
//	EXCEPTIONS

Common::Data*
Common::DataInstance::
create(const SQLData& cType_)
{
	if (cType_.isArrayType()) {
		return new DataArrayData;
	}

	Data* pObject = 0;

	switch (cType_.getType()) {
	case SQLData::Type::NoType:
		break;
	case SQLData::Type::Char:
	case SQLData::Type::CLOB:
		pObject = new StringData(StringData::EncodingForm::Unknown /* 無変換 */,
								 cType_.getCollation());
		break;
	case SQLData::Type::NChar:
	case SQLData::Type::NText:
	case SQLData::Type::Fulltext:
	case SQLData::Type::NCLOB:
		pObject = new StringData(StringData::EncodingForm::UCS2,
								 cType_.getCollation());
		break;
	case SQLData::Type::Int:
		pObject = new IntegerData;
		break;
	case SQLData::Type::BigInt:
		pObject = new Integer64Data;
		break;
	case SQLData::Type::Float:
		//...
		// YET
		// precision(= getLength())の結果によりFloatDataと使い分けるべき
		pObject = new DoubleData;
		break;
	case SQLData::Type::DateTime:
	case SQLData::Type::Timestamp:
		pObject = new DateTimeData;
		break;
	case SQLData::Type::UniqueIdentifier:
		pObject = new StringData;
		break;
	case SQLData::Type::Binary:
	case SQLData::Type::Image:
	case SQLData::Type::BLOB:
		pObject = new BinaryData;
		break;
	case SQLData::Type::Language:
		pObject = new LanguageData;
		break;
	case SQLData::Type::Decimal:
		pObject = new DecimalData(cType_.getLength(),cType_.getScale());
		break;
	case SQLData::Type::Date:
		pObject = new DateData;
		break;
#ifdef OBSOLETE
	case SQLData::Type::Time:
		pObject = new TimeData;
		break;
#endif
	case SQLData::Type::UInt:
		pObject = new UnsignedIntegerData;
		break;
	case SQLData::Type::Word:
		pObject = new WordData;
		break;
	default:
		; _TRMEISTER_ASSERT(false);
		break;
	}
	return pObject;
}

//
//	Copyright (c) 2004, 2005, 2006, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
