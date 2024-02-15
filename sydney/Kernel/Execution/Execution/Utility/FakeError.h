// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FakeError.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_UTILITY_FAKEERROR_H
#define __SYDNEY_EXECUTION_UTILITY_FAKEERROR_H

#ifdef SYD_FAKE_ERROR
#include "Exception/FakeError.h"
#define _EXECUTION_FAKE_ERROR(func, except) \
	_SYDNEY_FAKE_ERROR("Execution::" #func, Exception::except(moduleName, srcFile, __LINE__))
#else
#define _EXECUTION_FAKE_ERROR(func, except)
#endif

#endif // __SYDNEY_EXECUTION_UTILITY_FAKEERROR_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
