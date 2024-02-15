// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Bind1.h --
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

template <class _R_, class _T_, class _O_, class _A_ _CLASS_F>
class _CLASSNAME(BindMember1)
{
	_TYPEDEF(_R_ (_O_::*_F_)(_T_, _ARGUMENT(_A_)) _CONST)
public:
	_CLASSNAME(BindMember1)(_F_ func, _O_* obj, _ARGUMENT(_A_) arg)
		: m_func(func), m_obj(obj), m_arg(arg)
	{}
	_R_ operator()(_T_ obj) const
	{
		RETURN (m_obj->*m_func)(obj, m_arg);
	}

private:
	_F_ m_func;
	_O_* m_obj;
	_ARGUMENT(_A_) m_arg;
};

#ifdef _STATIC
template <class _R_, class _T_, class _A_ _CLASS_F>
class _CLASSNAME(BindStatic1)
{
	_TYPEDEF(_R_ (*_F_)(_T_, _ARGUMENT(_A_)))
public:
	_CLASSNAME(BindStatic1)(_F_ func, _ARGUMENT(_A_) arg)
		: m_func(func), m_arg(arg)
	{}
	_R_ operator()(_T_ obj) const
	{
		RETURN (*m_func)(obj, m_arg);
	}

private:
	_F_ m_func;
	_ARGUMENT(_A_) m_arg;
};
#endif

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_, class _O_, class _A_>
struct _WRAPPER(_CLASSNAME(BindMember1))
	: public _Bind<_R_>::_CLASSNAME(BindMember1)<_R_, _T_, _O_, _A_, _R_ (_O_::*)(_T_, _ARGUMENT(_A_)) _CONST>
{
	typedef _R_ (_O_::*Function)(_T_, _ARGUMENT(_A_)) _CONST;
	_WRAPPER(_CLASSNAME(BindMember1))(Function func, _O_* obj, _ARGUMENT(_A_) arg)
			 : _Bind<_R_>::_CLASSNAME(BindMember1)<_R_, _T_, _O_, _A_, _R_ (_O_::*)(_T_, _ARGUMENT(_A_)) _CONST>(func, obj, arg)
	{}
};
#ifdef _STATIC
template <class _R_, class _T_, class _A_>
struct _WRAPPER(_CLASSNAME(BindStatic1))
	: public _Bind<_R_>::_CLASSNAME(BindStatic1)<_R_, _T_, _A_, _R_ (*)(_T_, _ARGUMENT(_A_))>
{
	typedef _R_ (*Function)(_T_, _ARGUMENT(_A_));
	_WRAPPER(_CLASSNAME(BindStatic1)(Function func, _ARGUMENT(_A_) arg)
		: _Bind<_R_>::_CLASSNAME(BindStatic1)<_R_, _T_, _A_, _R_ (*)(_T_, _ARGUMENT(_A_))>(func, obj, arg)
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

template <class _R_, class _T_, class _O_, class _A_>
_WRAPPER(_CLASSNAME(BindMember1))<_R_, _T_, _O_, _A_>
Bind(_R_ (_O_::* func)(_T_, _ARGUMENT(_A_)) _CONST, _O_* obj, _ARGUMENT(_A_) arg)
{
	return _WRAPPER(_CLASSNAME(BindMember1))<_R_, _T_, _O_, _A_>(func, obj, arg);
}
#ifdef _STATIC
template <class _R_, class _T_, class _A_>
_WRAPPER(_CLASSNAME(BindStatic1))<_R_, _T_, _A_>
Bind(_R_ (*func)(_T_, _ARGUMENT(_A_)), _ARGUMENT(_A_) arg)
{
	return _WRAPPER(_CLASSNAME(BindStatic1))<_R_, _T_, _A_>(func, arg);
}
#endif

#undef _WRAPPER

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
