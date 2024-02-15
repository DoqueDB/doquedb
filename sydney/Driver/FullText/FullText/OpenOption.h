// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// OpenOption.h -- 全文検索ファイルオープンオプションのヘッダーファイル
// 
// Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT_OPENOPTION_H
#define __SYDNEY_FULLTEXT_OPENOPTION_H

#include "FullText/Module.h"
#include "LogicalFile/OpenOption.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT_BEGIN

namespace OpenOption
{
//------------------------
//	OpenParameter の型変更に関するマクロ
//------------------------

namespace Parameter
{
	namespace KeyNumber
	{
		enum Value
		{
			CreateIndex = LogicalFile::OpenOption::DriverNumber::FullText,
			SortParameter,
			Limit,
			Offset,
			ValueNum
		};
	}
}

#define _SYDNEY_FULLTEXT_OPEN_PARAMETER_KEY(key_)				LogicalFile::Parameter::Key((key_))
#define _SYDNEY_FULLTEXT_OPEN_PARAMETER_FORMAT_KEY(key_, arg_)	LogicalFile::Parameter::Key((key_), (arg_))
#define _SYDNEY_FULLTEXT_OPEN_PARAMETER_VALUE(value_)			(value_)

namespace CreateIndex
{
//
//	CONST
//	FullText::OpenOption::CreateIndex::Key
//		-- 索引の作成時のフラグ
//		   索引の作成時の特別な挿入方法を行なう為のフラグ
//		   OpenOptionへの設定はSchemaで行われる
//
const int Key = Parameter::KeyNumber::CreateIndex;

} // end of namespace CreateIndex

namespace SortParameter
{
//
//	CONST
//	FullText::OpenOption::SortParameter::Key
//		-- ソートキー種別
//		   設定される値はInverted::SortParameter::Value
//
const int Key = Parameter::KeyNumber::SortParameter;

} // end of namespace SortParameter

namespace Limit
{
//
//	CONST
//	FullText::OpenOption::Limit::Key
//		-- 結果取得の上限値
//
const int Key = Parameter::KeyNumber::Limit;

} // end of namespace Limit

namespace Offset
{
//
//	CONST
//	FullText::OpenOption::Offset::Key
//		-- 結果取得の取得開始位置
//
const int Key = Parameter::KeyNumber::Offset;

} // end of namespace Offset

} // end of namespace OpenOption

_SYDNEY_FULLTEXT_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT_OPENOPTION_H

//
//	Copyright (c) 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
