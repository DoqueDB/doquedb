// #T1#
// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModStemDLL.h -- Definition in order to use DLL of the strange declared formalization library
// 
// Copyright (c) 1999-2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModStemDLL_H__
#define	__ModStemDLL_H__

#include "ModConfig.h"

// #T2#
// MACRO
// ModStemDLL --
// From DLL of strange declared formalization library function and object
// Import (export) it does
//
// NOTES

#if defined(MOD_EXPORT_STEM_DLL)
#define	ModStemDLL			ModDLLExport
#elif defined(MOD_IMPORT_STEM_DLL)
#define	ModStemDLL			ModDLLImport
#else
#define	ModStemDLL
#endif

#if defined(MOD_EXPORT_XXSTEM_DLL)
#define	ModXXStemDLL			ModDLLExport
#elif defined(MOD_IMPORT_XXSTEM_DLL)
#define	ModXXStemDLL			ModDLLImport
#else
#define	ModXXStemDLL
#endif

// #T3#
#endif	// __ModStemDLL_H__

//
// Copyright (c) 1999-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
