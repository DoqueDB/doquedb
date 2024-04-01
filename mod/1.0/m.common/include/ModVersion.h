// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModVersion.h -- ModVersionのクラス定義
// 
// Copyright (c) 2023, 2024 Ricoh Company, Ltd.
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

#ifndef	__ModVersion_H__
#define __ModVersion_H__

#include "ModUnicodeString.h"

//
// MODバージョン定義
//
#define MOD_VERSION	"1.1.0+001"

//
// CLASS
// ModVersion -- バージョン取得クラス
//
// NOTES
// MODのバージョンを取得する
//

class ModVersion
{
public:
	static ModUnicodeString getVersion();
};

//
// FUNCTION
// ModVersion::getVersion -- バージョンを取得する
//
// NOTES
// バージョンを取得する
//
// ARGUMENTS
// なし
//
// RETURN
// なし
//
// EXCEPTIONS
// なし
//
inline
ModUnicodeString
ModVersion::getVersion()
{
	return ModUnicodeString(MOD_VERSION);
}

#endif	// __ModVersion_H__

//
// Copyright (c) 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
