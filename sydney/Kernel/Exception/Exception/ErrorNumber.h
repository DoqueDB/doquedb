// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ErrorNumber.h -- 例外番号の型宣言
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

#ifndef	__TRMEISTER_EXCEPTION_ERROR_NUMBER_H
#define	__TRMEISTER_EXCEPTION_ERROR_NUMBER_H

#include "Exception/Module.h"

_TRMEISTER_BEGIN
_TRMEISTER_EXCEPTION_BEGIN

//	TYPEDEF
//	Exception::ErrorNumber::Type -- 例外番号の型
//
//	NOTES

namespace ErrorNumber
{
	typedef unsigned int Type;
}

_TRMEISTER_EXCEPTION_END
_TRMEISTER_END

#endif	// __TRMEISTER_EXCEPTION_ERROR_NUMBER_H

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
