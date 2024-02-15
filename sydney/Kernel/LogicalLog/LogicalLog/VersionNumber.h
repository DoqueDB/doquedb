// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VersionNumber.h -- 論理ログファイルのバージョン番号の定義
// 
// Copyright (c) 2001, 2009, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_LOGICALLOG_VERSIONNUMBER_H
#define __SYDNEY_LOGICALLOG_VERSIONNUMBER_H

#include "LogicalLog/Module.h"

_SYDNEY_BEGIN
_SYDNEY_LOGICALLOG_BEGIN

//	NAMESPACE
//	LogicalLog::VersionNumber --
//		論理ログファイルのバージョン番号に関する名前空間
//
//	NOTES

namespace VersionNumber
{
	//	ENUM
	//	LogicalLog::VersionNumber::Value --
	//		論理ログファイルのバージョン番号を表す値の列挙型
	//
	//	NOTES

	enum Value
	{
		// 不明
		Unknown =			-1,
		// 最初
		First =				0,
		// 2番目
		Second,
		// 3番目
		Third,
		// 値の数
		ValueNum,
		// 現在
		Current =			ValueNum - 1
	};
}

_SYDNEY_LOGICALLOG_END
_SYDNEY_END

#endif //__SYDNEY_LOGICALLOG_VERSIONNUMBER_H

//
//	Copyright (c) 2001, 2009, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
