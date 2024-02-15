// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Bind0.h --
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

template <class _R_, class _T_, class _O_ _CLASS_F>
class _CLASSNAME(BindMember)
{
	_TYPEDEF(_R_ (_O_::*_F_)(_T_) _CONST)
public:
	_CLASSNAME(BindMember)(_F_ func, _O_* obj)
		: m_func(func), m_obj(obj)
	{}
	_R_ operator()(_T_ obj) const
	{
		RETURN (m_obj->*m_func)(obj);
	}

private:
	_F_ m_func;
	_O_* m_obj;
};

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_, class _O_>
struct _WRAPPER(_CLASSNAME(BindMember))
	: public _Bind<_R_>::_CLASSNAME(BindMember)<_R_, _T_, _O_, _R_ (_O_::*)(_T_) _CONST>
{
	typedef _R_ (_O_::*Function)(_T_) _CONST;
	_WRAPPER(_CLASSNAME(BindMember))(Function func, _O_* obj)
		: _Bind<_R_>::_CLASSNAME(BindMember)<_R_, _T_, _O_, _R_ (_O_::*)(_T_) _CONST>(func, obj)
	{}
};
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

template <class _R_, class _T_, class _O_>
_WRAPPER(_CLASSNAME(BindMember))<_R_, _T_, _O_>
Bind(_R_ (_O_::* func)(_T_) _CONST, _O_* obj)
{
	return _WRAPPER(_CLASSNAME(BindMember))<_R_, _T_, _O_>(func, obj);
}

#undef _WRAPPER

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
