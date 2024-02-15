// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Module.h -- Execution モジュールのソースコードを書くために使用するマクロ定義
// 
// Copyright (c) 2000, 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_MODULE_H
#define __SYDNEY_EXECUTION_MODULE_H

#include "Common/Common.h"
#include "Common/Internal.h"

#ifdef SYD_DLL
#ifdef SYD_EXECUTION_EXPORT_FUNCTION
#define SYD_EXECUTION_FUNCTION	SYD_EXPORT
#else
#define SYD_EXECUTION_FUNCTION	SYD_IMPORT
#endif
#ifdef SYD_EXECUTION_EXPORT_DATA
#define SYD_EXECUTION_DATA		SYD_EXPORT
#else
#define SYD_EXECUTION_DATA		SYD_IMPORT
#endif
#else
#define	SYD_EXECUTION_FUNCTION
#define	SYD_EXECUTION__DATA
#endif

#define	_SYDNEY_EXECUTION_BEGIN	namespace Execution {
#define	_SYDNEY_EXECUTION_END	}
#define	_SYDNEY_EXECUTION_USING	using namespace Execution;

#endif	//__SYDNEY_EXECUTION_MODULE_H

//
//	Copyright (c) 2000, 2002, 2003, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
