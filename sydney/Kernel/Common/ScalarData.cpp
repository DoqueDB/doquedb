// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ScalarData.cpp -- スカラーデータ共通の基底クラス
// 
// Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Common";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Common/ScalarData.h"

_TRMEISTER_USING
_TRMEISTER_COMMON_USING

namespace
{
}

//	FUNCTION public
//	Common::ScalarData::ScalarData -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataType::Type	type
//			生成するスカラデータのデータ型
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

ScalarData::ScalarData(DataType::Type type)
	: Data(type)
{}

//	FUNCTION public
//	Common::ScalarData::~ScalarData -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

ScalarData::~ScalarData()
{}

//	FUNCTION public
//	Common::ScalarData::isScalar -- スカラデータか調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			スカラデータである
//
//	EXCEPTIONS
//		なし

bool
ScalarData::isScalar() const
{
	return true;
}

//
//	Copyright (c) 1999, 2001, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
