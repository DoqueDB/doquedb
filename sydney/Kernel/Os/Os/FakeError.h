// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FakeError.h -- 擬似エラー関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_OS_FAKE_ERROR_H
#define	__TRMEISTER_OS_FAKE_ERROR_H

#include "Os/Module.h"

#include "Exception/FakeError.h"
#include "Exception/SystemCall.h"

#define	_TRMEISTER_OS_FAKE_ERROR(name, e)									\
		_TRMEISTER_FAKE_ERROR(name, e(moduleName, srcFile, __LINE__))

//	MACRO
//	_TRMEISTER_OS_FAKE_ERROR_SYSTEMCALL --
//		条件を満たせば、システムコールエラーの例外を発生する
//
//	NOTES

#define	_TRMEISTER_OS_FAKE_ERROR_SYSTEMCALL(name, func, errno)				\
		_TRMEISTER_FAKE_ERROR(name, Exception::SystemCall(					\
						   moduleName, srcFile, __LINE__, func, errno))

#endif	// __TRMEISTER_OS_FAKE_ERROR_H

//
// Copyright (c) 2001, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
