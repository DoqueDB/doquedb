// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ModInvertedFile.cpp --
// 
// Copyright (c) 2002, 2009, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/Module.h"
#include "Inverted/ModInvertedFile.h"
#include "ModParameter.h"

//
//	VARIABLE public static
//	Inverted::ModInvertedFile::debugLevel
//
ModUInt32 ModInvertedFile::debugLevel = 0;

//
//	FUNCTION public static
//	Inverted::ModInvertedFile::getUInt32FromModParameter - MODパラメータからUInt32の取得
//
//	NOTES
//
//	ARGUMENTS
//	const char* key_
//		パラメータキー
//	const ModUInt32 default_
//		デフォルト値
//
//	RETURN
//	ModUInt32
//		パラメータ値
//
//	EXCEPTIONS
//
ModUInt32
ModInvertedFile::getUInt32FromModParameter(const char* key_, const ModUInt32 default_)
{
	ModUInt32 value(default_);
	ModParameter parameter;
	parameter.getUnsignedInteger(value, key_);
	return value;
}

//
//	Copyright (c) 2002, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
