// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Module.h --
//		Ｂ＋木ファイルドライバのソースコードを書くために
//		使用するマクロ定義
// 
// Copyright (c) 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE_MODULE_H
#define __SYDNEY_BTREE_MODULE_H

#include "SyDLL.h"
#include "Common/Internal.h"

#define _SYDNEY_BTREE_BEGIN	namespace Btree {
#define _SYDNEY_BTREE_END	}
#define _SYDNEY_BTREE_USING	using namespace Btree;

// DLL Export
#ifdef SYD_DLL
#ifdef SYD_BTREE_EXPORT_FUNCTION
#define SYD_BTREE_FUNCTION SYD_EXPORT
#else
#define SYD_BTREE_FUNCTION SYD_IMPORT
#endif
#else
#define SYD_BTREE_FUNCTION
#endif

#ifndef SYD_RELEASEBUILD
#define SYD_BTREE_FUNCTION_TESTEXPORT SYD_BTREE_FUNCTION
#else
#define SYD_BTREE_FUNCTION_TESTEXPORT
#endif


#endif // __SYDNEY_BTREE_MODULE_H

//
// Copyright (c) 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
