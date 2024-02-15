// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Assert.h -- assert
// 
// Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_ASSERT_H
#define __TRMEISTER_COMMON_ASSERT_H

#ifdef DEBUG
#include "ModOsDriver.h"
#include "Common/Message.h"

//
//	DEFINE
//	SydAssert -- アサート
//
//	NOTES
//	アサート。例外的にマクロとする
//
#define SydAssert(expression) \
{ \
	if (!(expression)) \
	{ \
		SydErrorMessage \
			<< "Assertion failed. (" #expression ")" << ModEndl; \
		ModOsDriver::Process::abort(); \
	} \
}
#else
#define SydAssert(expression)
#endif

//	DEFINE
//	_TRMEISTER_ASSERT -- アサート
//
//	NOTES
//		このマクロを使用するソースファイルには、
//		そのファイル名を値として持つ文字配列定数 srcFile が
//		定義されている必要がある

#define	_TRMEISTER_ASSERT(expression)	SydAssert(expression)
#define	_SYDNEY_ASSERT _TRMEISTER_ASSERT
	  
#endif //__TRMEISTER_COMMON_ASSERT_H

//
//	Copyright (c) 2000, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
