// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FakeError.h --
// 
// Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SERVER_FAKEERROR_H
#define __SYDNEY_SERVER_FAKEERROR_H

#include "Server/Module.h"
#include "Common/UnicodeString.h"
#include "Exception/FakeError.h"
#include "Exception/Unexpected.h"
#include "Exception/Cancel.h"
#include "Exception/ConnectionRanOut.h"
#include "ModException.h"

#ifdef SYD_FAKE_ERROR
#define _SERVER_FAKE_ERROR(func) \
{ \
	_SYDNEY_FAKE_ERROR("Server::" #func, Exception::Unexpected(moduleName, srcFile, __LINE__)); \
	_SYDNEY_FAKE_ERROR("Server::" #func, ModException()); \
	_SYDNEY_FAKE_ERROR("Server::" #func, 0); \
	_SYDNEY_FAKE_ERROR("Server::" #func, Exception::Cancel(moduleName, srcFile, __LINE__)); \
	_SYDNEY_FAKE_ERROR("Server::" #func, Exception::ConnectionRanOut(moduleName, srcFile, __LINE__)); \
}
#else
#define _SERVER_FAKE_ERROR(func)
#endif

#endif // __SYDNEY_SERVER_FAKEERROR_H

//
//	Copyright (c) 2002, 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
