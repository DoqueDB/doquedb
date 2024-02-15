// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AreaCategory.cpp -- エリア関連の関数定義
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#include "Schema/AreaCategory.h"
#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	//
	// CONST local
	// _eSuperValueTable --
	//		あるエリアカテゴリーとそれで指定がないときに
	//		代わりに使うエリアカテゴリーを対応付ける
	//
	// NOTES

	const AreaCategory::Value _eSuperValueTable[AreaCategory::ValueNum] =
	{
		AreaCategory::Default,			// <- Default
		AreaCategory::Default,			// <- LogicalLog
		AreaCategory::Default,			// <- PhysicalLog
		AreaCategory::Default,			// <- Table
		AreaCategory::Default,			// <- Heap
		AreaCategory::Default,			// <- Index
		AreaCategory::Default			// <- FullText
	};
} // namespace

//	FUNCTION public
//		Schema::AreaCategory::getSuperValue --
//			あるカテゴリーで指定がないときに代わりに使うカテゴリーを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eValue_
//			対象のカテゴリー
//
//	RETURN
//		代わりに使うカテゴリー
//
//	EXCEPTIONS

// static
AreaCategory::Value
AreaCategory::
getSuperValue(Value eValue_)
{
	; _SYDNEY_ASSERT(eValue_ >= 0 && eValue_ < ValueNum);

	return _eSuperValueTable[eValue_];
}

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
