// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional.h --
// 
// Copyright (c) 2003, 2004, 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_FUNCTIONAL_H
#define __TRMEISTER_COMMON_FUNCTIONAL_H

#ifndef __SY_DEFAULT_H
#error require #include "SyDefault.h"
#endif

#include "SyTypeName.h"
#include "Common/Module.h"
#include "Common/ObjectPointer.h"

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

// ModAlgorithmで足りないものを補完するテンプレートクラス

namespace Functional
{

//////////////////////
// MemberFunction
//////////////////////

#ifdef SYD_VOID_NOT_RETURN // return voidと書けない処理系での定義

#define _CLASS_F , class _F_
#define _TYPEDEF(x)

// 返り値によって異なるテンプレート定義
template <class _V_>
struct _MemFun
{
#define RETURN return
#include "Common/Functional_MemFun.h"
#undef RETURN
};
template <>
struct _MemFun<void>
{
#define RETURN
#include "Common/Functional_MemFun.h"
#undef RETURN
};

#undef _CLASS_F
#undef _TYPEDEF

#else // return voidと書ける処理系での定義

#define _CLASS_F
#define _TYPEDEF(x) typedef x;

#define RETURN return
#include "Common/Functional_MemFun.h"
#undef RETURN

#undef _CLASS_F
#undef _TYPEDEF

#endif

//////////////////////
// Function
//////////////////////

#ifdef SYD_VOID_NOT_RETURN // return voidと書けない処理系での定義

#define _CLASS_F , class _F_
#define _TYPEDEF(x)

// 返り値によって異なるテンプレート定義
template <class _V_>
struct _CallFun
{
#define RETURN return
#include "Common/Functional_CallFun.h"
#undef RETURN
};
template <>
struct _CallFun<void>
{
#define RETURN
#include "Common/Functional_CallFun.h"
#undef RETURN
};

#undef _CLASS_F
#undef _TYPEDEF

#else // return voidと書ける処理系での定義

#define _CLASS_F
#define _TYPEDEF(x) typedef x;

#define RETURN return
#include "Common/Functional_CallFun.h"
#undef RETURN

#undef _CLASS_F
#undef _TYPEDEF

#endif

//////////////////////
// Bind
//////////////////////

#ifdef SYD_VOID_NOT_RETURN // return voidと書けない処理系での定義

#define _CLASS_F , class _F_
#define _TYPEDEF(x)

// 返り値によって異なるテンプレート定義
template <class _V_>
struct _Bind
{
#define RETURN return
#include "Common/Functional_Bind.h"
#undef RETURN
};
template <>
struct _Bind<void>
{
#define RETURN
#include "Common/Functional_Bind.h"
#undef RETURN
};

#undef _CLASS_F
#undef _TYPEDEF

#else // return voidと書ける処理系での定義

#define _CLASS_F
#define _TYPEDEF(x) typedef x;

#define RETURN return
#include "Common/Functional_Bind.h"
#undef RETURN

#undef _CLASS_F
#undef _TYPEDEF

#endif

//////////////////////
// Comparator
//////////////////////

#include "Common/Functional_Comparator.h"

//////////////////////
// Accumulator
//////////////////////

#include "Common/Functional_Accumulator.h"

//////////////////////
// IsAll, IsAny
//////////////////////

template <class _I_, class _Function_>
bool
IsAll(_I_ first, _I_ last, _Function_ function)
{
	bool b = true;
	if (first != last)
		for (; first != last; ++first)
			if (!function(*first)) {
				b = false;
				break;
			}
	return b;
}

template <class _I_, class _Function_>
bool
IsAny(_I_ first, _I_ last, _Function_ function)
{
	bool b = false;
	if (first != last)
		for (; first != last; ++first)
			if (function(*first)) {
				b = true;
				break;
			}
	return b;
}

//////////////////////
// MaxElement, MinElement
//////////////////////

template <class _I_, class _Comp_>
_I_
MinElement(_I_ first, _I_ last, _Comp_ comp)
{
	_I_ result = first;
	if (first != last)
		for (++first; first != last; ++first)
			if (comp(*first, *result)) result = first;
	return result;
}

template <class _I_, class _Comp_>
_I_
MaxElement(_I_ first, _I_ last, _Comp_ comp)
{
	_I_ result = first;
	if (first != last)
		for (++first; first != last; ++first)
			if (comp(*result, *first)) result = first;
	return result;
}

} // namespace Functional

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif // __TRMEISTER_COMMON_FUNCTIONAL_H

//
//	Copyright (c) 2003, 2004, 2005, 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
