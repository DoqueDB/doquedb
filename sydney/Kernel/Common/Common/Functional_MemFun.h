// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_MemFun.h --
//		Functional.h から複数回インクルードされる
//		直接インクルードしてはいけない
// 
// Copyright (c) 2003, 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
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
// mem_fun for no arguments
/////////////////////////////////
#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_MemFun0.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_MemFun0.h"
#undef _CLASSNAME
#undef _CONST

/////////////////////////////////
// mem_fun for one argument
/////////////////////////////////
#define _ARGUMENT(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_MemFun1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_MemFun1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

#define _ARGUMENT(x) x&

#define _CLASSNAME(x) _Ref##x
#define _CONST
#include "Common/Functional_MemFun1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _RefConst##x
#define _CONST const
#include "Common/Functional_MemFun1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

/////////////////////////////////
// mem_fun for two arguments
/////////////////////////////////

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2

#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref2##x
#define _CONST
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref2Const##x
#define _CONST const
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _Ref1##x
#define _CONST
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Const##x
#define _CONST const
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2

#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref1Ref2##x
#define _CONST
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref2Const##x
#define _CONST const
#include "Common/Functional_MemFun2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

/////////////////////////////////
// mem_fun for three arguments
/////////////////////////////////

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3##x
#define _CONST
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2##x
#define _CONST
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref2Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3##x
#define _CONST
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref2Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
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
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3##x
#define _CONST
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2##x
#define _CONST
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref2Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3##x
#define _CONST
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref2Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

/////////////////////////////////
// mem_fun for four arguments
/////////////////////////////////

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x
#define _ARGUMENT4(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref2Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref2Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
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
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref2Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref2Ref3Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3
#undef _ARGUMENT4

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x
#define _ARGUMENT4(x) x&

#define _CLASSNAME(x) _Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref3Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref2Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref2Ref3Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref3Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref2Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3Ref4##x
#define _CONST
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Ref1Ref2Ref3Ref4Const##x
#define _CONST const
#include "Common/Functional_MemFun4.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3
#undef _ARGUMENT4

//
//	Copyright (c) 2003, 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
