// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Comparator.h --
//		Functional.h からインクルードされる
//		直接インクルードしてはいけない
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

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_Comparator0.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Comparator0.h"
#undef _CLASSNAME
#undef _CONST

#define _ARGUMENT(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_Comparator1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Comparator1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

#define _ARGUMENT(x) x&

#define _CLASSNAME(x) _Ref##x
#define _CONST
#include "Common/Functional_Comparator1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _RefConst##x
#define _CONST const
#include "Common/Functional_Comparator1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

//
//	Copyright (c) 2003, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
