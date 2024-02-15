// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Bind.h --
//		Functional.h から複数回インクルードされる
//		直接インクルードしてはいけない
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

/////////////////////////////////
// bind for no arguments
/////////////////////////////////
#define _CLASSNAME(x) _##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind0.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Bind0.h"
#undef _CLASSNAME
#undef _CONST

/////////////////////////////////
// bind for one argument
/////////////////////////////////
#define _ARGUMENT(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind1.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Bind1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

#define _ARGUMENT(x) x&

#define _CLASSNAME(x) _Ref##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind1.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _RefConst##x
#define _CONST const
#include "Common/Functional_Bind1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

/////////////////////////////////
// bind for two arguments
/////////////////////////////////

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2

#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref2##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref2Const##x
#define _CONST const
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _Ref1##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref1Const##x
#define _CONST const
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2

#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref1Ref2##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref1Ref2Const##x
#define _CONST const
#include "Common/Functional_Bind2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

/////////////////////////////////
// bind for three arguments
/////////////////////////////////

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref3Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref2Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref2Ref3Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref1Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref1Ref3Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref1Ref2Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3##x
#define _CONST
#define _STATIC
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST
#undef _STATIC

#define _CLASSNAME(x) _Ref1Ref2Ref3Const##x
#define _CONST const
#include "Common/Functional_Bind3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
