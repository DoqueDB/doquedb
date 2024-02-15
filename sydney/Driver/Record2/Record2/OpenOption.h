// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h -- 可変長レコードファイルオープンオプションのヘッダーファイル
// 
// Copyright (c) 2000, 2002, 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_RECORD2_OPENOPTION_H
#define __SYDNEY_RECORD2_OPENOPTION_H

#include "Common/Common.h"
#include "FileCommon/OpenOption.h"

_SYDNEY_BEGIN

namespace Record2
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
			SortOrder = LogicalFile::OpenOption::DriverNumber::Record,
			ValueNum
		};
	}
}

#define _SYDNEY_RECORD2_OPEN_PARAMETER_KEY(key_)              Record2::OpenOption::Parameter::KeyType((key_))
#define _SYDNEY_RECORD2_OPEN_PARAMETER_FORMAT_KEY(key_, arg_) Record2::OpenOption::Parameter::KeyType((key_), (arg_))
#define _SYDNEY_RECORD2_OPEN_PARAMETER_VALUE(value_)          (value_)

// ソート順を指定するパラメータ
namespace SortOrder
{

//
//	CONST
//	Record2::OpenOption::SortOrder::Key --
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


} // end of namespace OpenOption

} // end of namespace Record2

_SYDNEY_END

#endif // __SYDNEY_RECORD2_OPENOPTION_H

//
//	Copyright (c) 2000, 2002, 2006, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
