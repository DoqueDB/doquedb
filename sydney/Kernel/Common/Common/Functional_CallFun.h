// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_CallFun.h --
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
// mem_fun for no arguments
/////////////////////////////////
#define _CLASSNAME(x) _##x
#include "Common/Functional_CallFun0.h"
#undef _CLASSNAME

/////////////////////////////////
// mem_fun for one argument
/////////////////////////////////
#define _ARGUMENT(x) x

#define _CLASSNAME(x) _##x
#include "Common/Functional_CallFun1.h"
#undef _CLASSNAME

#undef _ARGUMENT

#define _ARGUMENT(x) x&

#define _CLASSNAME(x) _Ref##x
#include "Common/Functional_CallFun1.h"
#undef _CLASSNAME

#undef _ARGUMENT

/////////////////////////////////
// mem_fun for two arguments
/////////////////////////////////

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _##x
#include "Common/Functional_CallFun2.h"
#undef _CLASSNAME

#undef _ARGUMENT2

#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref2##x
#include "Common/Functional_CallFun2.h"
#undef _CLASSNAME

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _Ref1##x
#include "Common/Functional_CallFun2.h"
#undef _CLASSNAME

#undef _ARGUMENT2

#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref1Ref2##x
#include "Common/Functional_CallFun2.h"
#undef _CLASSNAME

#undef _ARGUMENT1
#undef _ARGUMENT2

/////////////////////////////////
// mem_fun for three arguments
/////////////////////////////////

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3##x
#include "Common/Functional_CallFun3.h"
#undef _CLASSNAME

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
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3
#undef _ARGUMENT4

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x
#define _ARGUMENT4(x) x&

#define _CLASSNAME(x) _Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT3

#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3Ref4##x
#include "Common/Functional_CallFun4.h"
#undef _CLASSNAME

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3
#undef _ARGUMENT4

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
