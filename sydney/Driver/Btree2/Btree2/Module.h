// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Module.h --
// 
// Copyright (c) 2003, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BTREE2_MODULE_H
#define __SYDNEY_BTREE2_MODULE_H

#include "SyDLL.h"
#include "Common/Internal.h"

#define _SYDNEY_BTREE2_BEGIN	namespace Btree2 {
#define _SYDNEY_BTREE2_END	}
#define _SYDNEY_BTREE2_USING	using namespace Btree2;

// DLL Export
#ifdef SYD_DLL
#ifdef SYD_BTREE2_EXPORT_FUNCTION
#define SYD_BTREE2_FUNCTION SYD_EXPORT
#else
#define SYD_BTREE2_FUNCTION SYD_IMPORT
#endif
#else
#define SYD_BTREE2_FUNCTION
#endif

#endif // __SYDNEY_BTREE2_MODULE_H

//
// Copyright (c) 2003, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
