// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SyTypes.h -- TRMeister の基本型を定義する
// 
// Copyright (c) 2001, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SY_TYPES_H
#define __SY_TYPES_H

#include "SyNameSpace.h"

_TRMEISTER_BEGIN

namespace Boolean
{
	enum Value
	{
		False =		0,
		True,
		Unknown,
		ValueNum
	};
}

_TRMEISTER_END

#endif //__SY_TYPES_H

//
// Copyright (c) 2001, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
