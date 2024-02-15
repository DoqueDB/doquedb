// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModAssert.h -- ModAssert の定義
// 
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModAssert_H__
#define __ModAssert_H__

//	MACRO
//	ModAssert -- ::assert() のかわりのためのマクロ
//
//	NOTES
//		MOD_DEBUG が定義され、かつ ModDebug::check が真のとき、
//		与えられた式を評価する
//		評価結果が偽のとき、ログにメッセージが出力される
//		このとき、ModDebug::assertingException が真であれば、例外が発生し、
//		偽であれば、アボートする

#ifdef MOD_DEBUG
#include "ModDebug.h"
#include "ModError.h"
#include "ModOsDriver.h"
#ifdef DEBUG
#define ModAssert(expression)											\
{																		\
	if (ModDebug::check && !(expression)) {								\
		ModErrorHandle::assertCheck();									\
		if (ModDebug::assertingException) {								\
			ModException	e;											\
			e.setError(ModModuleOs, ModCommonErrorAssert,				\
					   ModErrorLevelFatal, ModFalse);					\
			ModErrorMessage << e.setMessage() << ModEndl;				\
			ModThrow(ModModuleOs, ModCommonErrorAssert, ModErrorLevelFatal); \
		} else {														\
			ModException	e;											\
			e.setError(ModModuleOs, ModCommonErrorAssert,				\
					   ModErrorLevelFatal, ModFalse);					\
			ModErrorMessage << e.setMessage() << ModEndl;				\
			ModOsDriver::Process::abort();								\
		}																\
	}																	\
}
#else
#define ModAssert(expression)											\
{																		\
	if (ModDebug::check && !(expression)) {								\
		if (ModDebug::assertingException) {								\
			ModThrow(ModModuleOs, ModCommonErrorAssert, ModErrorLevelFatal); \
		} else {														\
			ModException	e;											\
			e.setError(ModModuleOs,	ModCommonErrorAssert,				\
					   ModErrorLevelFatal, ModFalse);					\
			ModErrorMessage << e.setMessage() << ModEndl;				\
			ModOsDriver::Process::abort();								\
		}																\
	}																	\
}
#endif	// DEBUG
#else
#define ModAssert(expression)	{ }
#endif	// MOD_DEBUG

#endif	// __ModAssert_H__

//
// Copyright (c) 1997, 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
