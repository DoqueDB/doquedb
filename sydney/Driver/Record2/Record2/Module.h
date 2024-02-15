// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Module.h --
//		Macro definition used to write record file driver's source code
// 
// Copyright (c) 2001, 2002, 2006, 2007, 2023 Ricoh Company, Ltd.
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

#include "SyDLL.h"
#include "Common/Internal.h"

#ifndef __SYDNEY_RECORD2_MODULE_H
#define __SYDNEY_RECORD2_MODULE_H

#ifndef SYD_RECORD2_EXPORT_FUNCTION
#define SYD_RECORD2_EXPORT_FUNCTION
#endif

#define _SYDNEY_RECORD2_BEGIN	namespace Record2 {
#define _SYDNEY_RECORD2_END	}
#define _SYDNEY_RECORD2_USING	using namespace Record2;

// DLL Export
#ifdef SYD_DLL
#ifdef SYD_RECORD2_EXPORT_FUNCTION
#define SYD_RECORD2_FUNCTION SYD_EXPORT
#else
#define SYD_RECORD2_FUNCTION SYD_IMPORT
#endif
#else
#define SYD_RECORD2_FUNCTION
#endif

#ifndef SYD_RELEASEBUILD
#define SYD_RECORD2_FUNCTION_TESTEXPORT SYD_RECORD2_FUNCTION
#else
#define SYD_RECORD2_FUNCTION_TESTEXPORT
#endif


#endif // __SYDNEY_RECORD2_MODULE_H

//
// Copyright (c) 2001, 2002, 2006, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
