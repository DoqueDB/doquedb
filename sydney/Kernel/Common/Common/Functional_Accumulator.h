// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Accumulator.h --
//		Functional.h からインクルードされる
//		直接インクルードしてはいけない
// 
// Copyright (c) 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

///////////////////////////////
// without arguments
///////////////////////////////
#define _RETURN(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_Accumulator0.h"
#include "Common/Functional_CallAccumulator0.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Accumulator0.h"
#undef _CLASSNAME
#undef _CONST

#undef _RETURN

#define _RETURN(x) const x&
#define _ONLY_REF

#define _CLASSNAME(x) _##x##Ref
#define _CONST
#include "Common/Functional_Accumulator0.h"
#include "Common/Functional_CallAccumulator0.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator0.h"
#undef _CLASSNAME
#undef _CONST

#undef _RETURN
#undef _ONLY_REF

///////////////////////////////
// one argument
///////////////////////////////
#define _RETURN(x) x

#define _ARGUMENT(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_Accumulator1.h"
#include "Common/Functional_CallAccumulator1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Accumulator1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

#define _ARGUMENT(x) x&

#define _CLASSNAME(x) _Ref##x
#define _CONST
#include "Common/Functional_Accumulator1.h"
#include "Common/Functional_CallAccumulator1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef##x
#define _CONST const
#include "Common/Functional_Accumulator1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

#undef _RETURN

#define _RETURN(x) const x&
#define _ONLY_REF

#define _ARGUMENT(x) x

#define _CLASSNAME(x) _##x##Ref
#define _CONST
#include "Common/Functional_Accumulator1.h"
#include "Common/Functional_CallAccumulator1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

#define _ARGUMENT(x) x&

#define _CLASSNAME(x) _Ref##x##Ref
#define _CONST
#include "Common/Functional_Accumulator1.h"
#include "Common/Functional_CallAccumulator1.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator1.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT

#undef _RETURN
#undef _ONLY_REF

///////////////////////////////
// two arguments
///////////////////////////////
#define _RETURN(x) x

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref2##x
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef2##x
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _Ref1##x
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1##x
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref1Ref2##x
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref2##x
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#undef _RETURN

#define _RETURN(x) const x&
#define _ONLY_REF

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _##x##Ref
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref2##x##Ref
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef2##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x

#define _CLASSNAME(x) _Ref1##x##Ref
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x&

#define _CLASSNAME(x) _Ref1Ref2##x##Ref
#define _CONST
#include "Common/Functional_Accumulator2.h"
#include "Common/Functional_CallAccumulator2.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref2##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator2.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2

#undef _RETURN
#undef _ONLY_REF

///////////////////////////////
// three arguments
///////////////////////////////
#define _RETURN(x) x

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _##x
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2##x
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef2##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
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
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3##x
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef3##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3##x
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef2Ref3##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3##x
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref3##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2##x
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref2##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3##x
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref2Ref3##x
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#undef _RETURN

#define _RETURN(x) const x&
#define _ONLY_REF

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _Const##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref2##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef2##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x

#define _CLASSNAME(x) _Ref1Ref2##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref2##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref3##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef3##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref2Ref3##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef2Ref3##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref3##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref3##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#define _ARGUMENT1(x) x&
#define _ARGUMENT2(x) x&
#define _ARGUMENT3(x) x&

#define _CLASSNAME(x) _Ref1Ref2Ref3##x##Ref
#define _CONST
#include "Common/Functional_Accumulator3.h"
#include "Common/Functional_CallAccumulator3.h"
#undef _CLASSNAME
#undef _CONST

#define _CLASSNAME(x) _ConstRef1Ref2Ref3##x##Ref
#define _CONST const
#include "Common/Functional_Accumulator3.h"
#undef _CLASSNAME
#undef _CONST

#undef _ARGUMENT1
#undef _ARGUMENT2
#undef _ARGUMENT3

#undef _RETURN
#undef _ONLY_REF

//
//	Copyright (c) 2004, 2005, 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
