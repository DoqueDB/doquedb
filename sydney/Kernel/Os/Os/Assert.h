// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Assert.h -- アサート関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_ASSERT_H
#define	__TRMEISTER_OS_ASSERT_H

#ifdef SYD_OS_WINDOWS
// #include <assert.h>
#endif
#ifdef SYD_OS_POSIX
// #include <assert.h>
#endif

#include "Os/Module.h"

//	MACRO
//	_TRMEISTER_ASSERT -- 式を評価した結果が偽であれば、実行を中断する
//
//	NOTES

#define	_TRMEISTER_ASSERT(expr)		assert(expr)

#endif	// __TRMEISTER_OS_ASSERT_H

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
