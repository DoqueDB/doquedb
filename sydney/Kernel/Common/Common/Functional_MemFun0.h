// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_MemFun0.h --
//		Included by Functional_MemFun.h
//		Don't include directly
// 
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
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
class _CLASSNAME(MemberFunction)
{
	_TYPEDEF(_R_ (_T_::*_F_)() _CONST)
public:
	explicit _CLASSNAME(MemberFunction)(_F_ func)
		: m_func(func)
	{}
	_R_ operator()(_T_* obj) const
	{
		RETURN (obj->*m_func)();
	}
	_R_ operator()(_T_& obj) const
	{
		RETURN (obj.*m_func)();
	}
	// Definition for ObjectPointer
	template <class _X_>
	_R_ operator()(const ObjectPointer<_X_>& x_) const
	{
		RETURN (x_.get()->*m_func)();
	}

private:
	_F_ m_func;
};

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_>
struct _WRAPPER(_CLASSNAME(MemberFunction))
	: public _MemFun<_R_>::_CLASSNAME(MemberFunction)<_R_, _T_, _R_ (_T_::*)() _CONST>
{
	typedef _R_ (_T_::*Function)() _CONST;
	explicit _WRAPPER(_CLASSNAME(MemberFunction))(Function func)
		: _MemFun<_R_>::_CLASSNAME(MemberFunction)<_R_, _T_, _R_ (_T_::*)() _CONST>(func)
	{}
};
#else
#define _WRAPPER(x) x
#endif

// TEMPLATE FUNCTION
//	MemberFunction -- std::mem_fun
//
// TEMPLATE ARGUMENTS
// 	class _R_
//		Return value type
//	class _T_
//		Class in which the method is defined
//
// NOTES

template <class _R_, class _T_>
_WRAPPER(_CLASSNAME(MemberFunction))<_R_, _T_>
MemberFunction(_R_ (_T_::* func)() _CONST)
{
	return _WRAPPER(_CLASSNAME(MemberFunction))<_R_, _T_>(func);
}

#undef _WRAPPER

//
//	Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
