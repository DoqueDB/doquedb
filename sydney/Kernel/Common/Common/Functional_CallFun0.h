// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_CallFun0.h --
//		Included by Functional_CallFun.h
//		Don't include directly
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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

template <class _R_, class _T_ _CLASS_F>
class _CLASSNAME(CallFunction)
{
	_TYPEDEF(_R_ (*_F_)(_T_))
public:
	explicit _CLASSNAME(CallFunction)(_F_ func)
		: m_func(func)
	{}
	_R_ operator()(_T_ obj) const
	{
		RETURN (*m_func)(obj);
	}

private:
	_F_ m_func;
};

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_>
struct _WRAPPER(_CLASSNAME(CallFunction))
	: public _CallFun<_R_>::_CLASSNAME(CallFunction)<_R_, _T_, _R_ (*)(_T_)>
{
	typedef _R_ (*Function)(_T_);
	explicit _WRAPPER(_CLASSNAME(CallFunction))(Function func)
		: _CallFun<_R_>::_CLASSNAME(CallFunction)<_R_, _T_, _R_ (*)(_T_)>(func)
	{}
};
#else
#define _WRAPPER(x) x
#endif

// TEMPLATE FUNCTION
//	CallFunction -- call function
//
// TEMPLATE ARGUMENTS
// 	class _R_
//		Return value type
//	class _T_
//		Class of first operand
//
// NOTES

template <class _R_, class _T_>
_WRAPPER(_CLASSNAME(CallFunction))<_R_, _T_>
CallFunction(_R_ (*func)(_T_))
{
	return _WRAPPER(_CLASSNAME(CallFunction))<_R_, _T_>(func);
}

#undef _WRAPPER

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
