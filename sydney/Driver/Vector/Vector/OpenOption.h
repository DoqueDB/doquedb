// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h -- ベクタファイルオープンオプションのヘッダーファイル
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_VECTOR_OPENOPTION_H
#define __SYDNEY_VECTOR_OPENOPTION_H

#include "Common/Common.h"
#include "FileCommon/OpenOption.h"

_SYDNEY_BEGIN

namespace Vector
{

namespace OpenOption
{

//------------------------
//	OpenParameter の型変更に関するマクロ
//------------------------

namespace Parameter {
	typedef	LogicalFile::Parameter::Key		KeyType;
	typedef	ModUnicodeString				ValueType;

	namespace KeyNumber {
		enum Value {
			SortOrder = LogicalFile::OpenOption::DriverNumber::Vector,
			SearchFieldNumber,
			SearchFieldIndex,
			SearchValue,
			SearchOpe,
			ValueNum
		};
	}
	namespace Operator {
		enum Value {
			Equals = 0,
			GreaterThan,
			GreaterThanEquals,
			LessThan,
			LessThanEquals,
			EqualsToNull,
			Like,
			ValueNum
		};
	}
}

#define _SYDNEY_VECTOR_OPEN_PARAMETER_KEY(key_)              Vector::OpenOption::Parameter::KeyType((key_))
#define _SYDNEY_VECTOR_OPEN_PARAMETER_FORMAT_KEY(key_, arg_) Vector::OpenOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_VECTOR_OPEN_PARAMETER_VALUE(value_)          (value_)


// 次に挙げるオープンオプションはFileCommon::OpenOptionで定義されている。
// OpenMode, ReadSubMode, SearchFieldNumber, SearchFieldIndex,
// SearchValue, SearchOpe,
// FieldSelect, TargetFieldNumber, TargetFieldIndex, GetByBitSet.
	
namespace SortOrder
{

//
//	CONST
//	Vector::OpenOption::SortOrder::Key --
//		ソート順を指定するパラメータのキー
//
//	NOTES
//	ソート順を指定するパラメータのキー。
//	オブジェクト ID の昇順に取得する場合には false を、
//	オブジェクト ID の降順に取得する場合には true を指定する。
//
//const char*	const Key = "SortOrder";
const int Key = Parameter::KeyNumber::SortOrder;

} // end of namespace SortOrder

namespace SearchFieldNumber
{
//
//	CONST
//	Vector::OpenOption::SearchFieldNumber::Key --
//		取得するフィールドの個数を指定するパラメータのキー
//
//	NOTES
//	取得するフィールドの個数を指定するパラメータのキー。
//	intで指定する。
//
//const char* const Key = "SearchFieldNumber";
const int Key = Parameter::KeyNumber::SearchFieldNumber;

} // end of namespace SearchFieldNumber

namespace SearchFieldIndex
{
//
//	CONST
//	Vector::OpenOption::SearchfieldIndex::Key --
//		取得するフィールドのIDを指定するパラメータのキー
//
//	NOTES
//	取得するフィールドのIDを指定するパラメータのキー。
//	intで指定する。
//
//const char* const Key = "SearchFieldIndex";
const int Key = Parameter::KeyNumber::SearchFieldIndex;

} // end of namespace SearchFieldIndex

namespace SearchValue
{
//
//	CONST
//	Vector::OpenOption::SearchValue::Key --
//		SearchModeで取得するベクタキーを指定するパラメータのキー
//
//	NOTES
//	SearchModeで取得するベクタキーを指定するパラメータのキー。
//	intで指定する。
//
//const char* const Key = "SearchValue";
const int Key = Parameter::KeyNumber::SearchValue;

} // end of namespace SearchValue

namespace SearchOpe
{
//
//	CONST
//	Vector::OpenOption::SortOpe::Key --
//
//	NOTES
//
//const char* const Key = "SearchOpe";
const int Key = Parameter::KeyNumber::SearchOpe;

//
//	CONST
//	Vector::OpenOption::SortOpe::Equals --
//
//	NOTES
//
//const char* const Equals = "EQ";
const int Equals = Parameter::Operator::Equals;

} // end of namespace SearchOpe

} // end of namespace OpenOption

} // end of namespace Vector

_SYDNEY_END

#endif // __SYDNEY_VECTOR_OPENOPTION_H

//
//	Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
