// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModCommonDLL.h -- 汎用ライブラリーの DLL を使用するための定義
// 
// Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModCommonDLL_H__
#define	__ModCommonDLL_H__

#include "ModConfig.h"

//	MACRO
//	ModCommonDLL --
//		汎用ライブラリーの DLL から関数やオブジェクトを
//		インポート(エクスポート)する
//
//	NOTES

#if defined(MOD_EXPORT_COMMON_DLL)
#define	ModCommonDLL			ModDLLExport
#elif defined(MOD_IMPORT_COMMON_DLL)
#define	ModCommonDLL			ModDLLImport
#else
#define	ModCommonDLL
#endif

#endif	// __ModCommonDLL_H__

//
// Copyright (c) 1999, 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
