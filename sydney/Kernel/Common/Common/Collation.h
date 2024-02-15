// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collation.h -- 比較方法
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_COLLATION_H
#define __TRMEISTER_COMMON_COLLATION_H

#include "Common/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//
//	CLASS
//	Collation -- 比較方法を表すクラス
//
//	NOTES
//
namespace Collation
{
	//	ENUM
	//  Common::Collation::Type::Value -- 比較方法
	//
	//  NOTES

	struct Type
	{
		enum Value
		{
			// 指定なし
			Implicit = 0,

			// PAD SPACE(比較時に末尾の空白を無視する)
			PadSpace,

			// NO PAD(比較時に末尾の空白を無視しない)
			NoPad,

			ValueNum
		};
	};
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_COLLATION_H

//
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
