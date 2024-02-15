// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Bind2.h --
//		Included by Functional_Bind.h
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

template <class _R_, class _T_, class _O_, class _A1_, class _A2_ _CLASS_F>
class _CLASSNAME(BindMember2)
{
	_TYPEDEF(_R_ (_O_::*_F_)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)) _CONST)
public:
	_CLASSNAME(BindMember2)(_F_ func, _O_* obj, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
		: m_func(func), m_obj(obj), m_arg1(arg1), m_arg2(arg2)
	{}
	_R_ operator()(_T_ obj) const
	{
		RETURN (m_obj->*m_func)(obj, m_arg1, m_arg2);
	}

private:
	_F_ m_func;
	_O_* m_obj;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
};

#ifdef _STATIC
template <class _R_, class _T_, class _A1_, class _A2_ _CLASS_F>
class _CLASSNAME(BindStatic2)
{
	_TYPEDEF(_R_ (*_F_)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)))
public:
	_CLASSNAME(BindStatic2)(_F_ func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
		: m_func(func), m_arg1(arg1), m_arg2(arg2)
	{}
	_R_ operator()(_T_ obj) const
	{
		RETURN (*m_func)(obj, m_arg1, m_arg2);
	}

private:
	_F_ m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
};
#endif

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_, class _O_, class _A1_, class _A2_>
struct _WRAPPER(_CLASSNAME(BindMember2))
	: public _Bind<_R_>::_CLASSNAME(BindMember2)<_R_, _T_, _O_, _A1_, _A2_, _R_ (_O_::*)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)) _CONST>
{
	typedef _R_ (_O_::*Function)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)) _CONST;
	_WRAPPER(_CLASSNAME(BindMember2))(Function func, _O_* obj, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
		: _Bind<_R_>::_CLASSNAME(BindMember2)<_R_, _T_, _O_, _A1_, _A2_, _R_ (_O_::*)(_T_, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2) _CONST>(func, obj, arg1, arg2)
	{}
};

#ifdef _STATIC
template <class _R_, class _T_, class _A1_, class _A2_>
struct _WRAPPER(_CLASSNAME(BindStatic2))
	: public _Bind<_R_>::_CLASSNAME(BindStatic2)<_R_, _T_, _A1_, _A2_, _R_ (*)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_))>
{
	typedef _R_ (*Function)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_));
	_WRAPPER(_CLASSNAME(BindStatic2)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
		: _Bind<_R_>::_CLASSNAME(BindStatic2)<_R_, _T_, _A1_, _A2_, _R_ (*)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_))>(func, obj, arg1, arg2)
	{}
};
#endif
#else
#define _WRAPPER(x) x
#endif

// TEMPLATE FUNCTION
//	Bind -- bind arguments
//
// TEMPLATE ARGUMENTS
// 	class _R_
//		Return value type
//	class _T_
//		Class in which the method is defined
//
// NOTES

template <class _R_, class _T_, class _O_, class _A1_, class _A2_>
_WRAPPER(_CLASSNAME(BindMember2))<_R_, _T_, _O_, _A1_, _A2_>
Bind(_R_ (_O_::* func)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)) _CONST, _O_* obj, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
{
	return _WRAPPER(_CLASSNAME(BindMember2))<_R_, _T_, _O_, _A1_, _A2_>(func, obj, arg1, arg2);
}

#ifdef _STATIC
template <class _R_, class _T_, class _A1_, class _A2_>
_WRAPPER(_CLASSNAME(BindStatic2))<_R_, _T_, _A1_, _A2_>
Bind(_R_ (*func)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)), _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
{
	return _WRAPPER(_CLASSNAME(BindStatic2))<_R_, _T_, _A1_, _A2_>(func, arg1, arg2);
}
#endif

#undef _WRAPPER

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
