// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_MemFun1.h --
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

template <class _R_, class _T_, class _A_ _CLASS_F>
class _CLASSNAME(MemberFunction1)
{
	_TYPEDEF(_R_ (_T_::*_F_)(_ARGUMENT(_A_)) _CONST)
public:
	_CLASSNAME(MemberFunction1)(_F_ func, _ARGUMENT(_A_) arg1)
		: m_func(func), m_arg1(arg1)
	{}
	_R_ operator()(_T_* obj) const
	{
		RETURN (obj->*m_func)(m_arg1);
	}
	_R_ operator()(_T_& obj) const
	{
		RETURN (obj.*m_func)(m_arg1);
	}
	// Definition for ObjectPointer
	template <class _X_>
	_R_ operator()(const ObjectPointer<_X_>& x_) const
	{
		RETURN (x_.get()->*m_func)(m_arg1);
	}

private:
	_F_ m_func;
	_ARGUMENT(_A_) m_arg1;
};

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_, class _A_>
struct _WRAPPER(_CLASSNAME(MemberFunction1))
	: public _MemFun<_R_>::_CLASSNAME(MemberFunction1)<_R_, _T_, _R_ (_T_::*)(_ARGUMENT(_A_)) _CONST>
{
	typedef _R_ (_T_::*Function)(_ARGUMENT(_A_)) _CONST;
	explicit _WRAPPER(_CLASSNAME(MemberFunction1))(Function func)
		: _MemFun<_R_>::_CLASSNAME(MemberFunction1)<_R_, _T_, _R_ (_T_::*)(_ARGUMENT(_A_)) _CONST>(func)
	{}
};
#else
#define _WRAPPER(x) x
#endif

// TEMPLATE FUNCTION
//	MemberFunction -- std::mem_fun_1
//
// TEMPLATE ARGUMENTS
// 	class _R_
//		Return value type
//	class _T_
//		Class in which the method is defined
//	class _A_
//		First argument type
//
// NOTES
//	Argument is always bound, which is different from std::mem_fun_1

template <class _R_, class _T_, class _A_>
_WRAPPER(_CLASSNAME(MemberFunction1))<_R_, _T_, _A_>
MemberFunction(_R_ (_T_::* func)(_ARGUMENT(_A_)) _CONST, _ARGUMENT(_A_) arg)
{
	return _WRAPPER(_CLASSNAME(MemberFunction1))<_R_, _T_, _A_>(func, arg);
}

#undef _WRAPPER

//
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
