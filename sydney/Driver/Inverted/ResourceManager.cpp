// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ResourceManager.cpp --
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
const char moduleName[] = "Inverted";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Inverted/ResourceManager.h"
#include "ModOstrStream.h"

_SYDNEY_USING
_SYDNEY_INVERTED_USING

//
//	FUNCTION public static
//	Inverted::ResourceManager::makeParameterName -- パラメータ名を得る
//
//	NOTES
//
//	ARGUMENTS
//	const char* pszName_
//		パラメータ名のリソースID以外の部分
//	ModUInt32 uiID_
//		リソースID
//
//	RETURN
//	ModCharString
//		リソースIDを付加したパラメータ名
//
//	EXCEPTIONS
//
ModCharString
ResourceManager::makeParameterName(const char* pszName_, ModUInt32 uiID_)
{
	ModOstrStream cStream;
	cStream << pszName_ << uiID_;
	return cStream.getString();
}

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
