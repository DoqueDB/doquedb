// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Module.h -- ロックモジュールのソースコードを書くために使用するマクロ定義
// 
// Copyright (c) 2000, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_LOCK_MODULE_H
#define	__SYDNEY_LOCK_MODULE_H

//【注意】	本モジュールで定義するクラスについては、
//			領域的にも速度的にも無駄なので
//			本当に必要でない限り Common::Object を継承しない

#include "Common/Common.h"
#include "Common/Internal.h"

#define	_SYDNEY_LOCK_BEGIN	namespace Lock {
#define	_SYDNEY_LOCK_END	}

#define _SYDNEY_LOCK_USING	using namespace Lock;

#ifdef SYD_DLL
#ifdef SYD_LOCK_EXPORT_FUNCTION
#define SYD_LOCK_FUNCTION	SYD_EXPORT
#else
#define SYD_LOCK_FUNCTION	SYD_IMPORT
#endif
#else
#define	SYD_LOCK_FUNCTION
#endif

#endif	// __SYDNEY_LOCK_MODULE_H

//
// Copyright (c) 2000, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
