// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_MemFun4.h --
//		Included by Functional_MemFun.h
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

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_, class _A4_ _CLASS_F>
class _CLASSNAME(MemberFunction4)
{
	_TYPEDEF(_R_ (_T_::*_F_)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_), _ARGUMENT4(_A4_)) _CONST)
public:
	_CLASSNAME(MemberFunction4)(_F_ func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _ARGUMENT4(_A4_) arg4)
		: m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_arg4(arg4)
	{}
	_R_ operator()(_T_* obj) const
	{
		RETURN (obj->*m_func)(m_arg1, m_arg2, m_arg3, m_arg4);
	}
	_R_ operator()(_T_& obj) const
	{
		RETURN (obj.*m_func)(m_arg1, m_arg2, m_arg3, m_arg4);
	}
	// Definition for ObjectPointer
	template <class _X_>
	_R_ operator()(const ObjectPointer<_X_>& x_) const
	{
		RETURN (x_.get()->*m_func)(m_arg1, m_arg2, m_arg3, m_arg4);
	}

private:
	_F_ m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_ARGUMENT3(_A3_) m_arg3;
	_ARGUMENT4(_A4_) m_arg4;
};

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_, class _A1_, class _A2_, class _A3_, class _A4_>
struct _WRAPPER(_CLASSNAME(MemberFunction4))
	: public _MemFun<_R_>::_CLASSNAME(MemberFunction4)<_R_, _T_, _R_ (_T_::*)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_), _ARGUMENT4(_A4_)) _CONST>
{
	typedef _R_ (_T_::*Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_), _ARGUMENT4(_A4_)) _CONST;
	explicit _WRAPPER(_CLASSNAME(MemberFunction4))(Function func)
		: _MemFun<_R_>::_CLASSNAME(MemberFunction4)<_R_, _T_, _R_ (_T_::*)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_), _ARGUMENT4(_A4_)) _CONST>(func)
	{}
};
#else
#define _WRAPPER(x) x
#endif

// TEMPLATE FUNCTION
//	MemberFunction -- std::mem_fun_1 extension for 4 arguments
//
// TEMPLATE ARGUMENTS
// 	class _R_
//		Return value type
//	class _T_
//		Class in which the method is defined
//	class _A1_
//		First argument type
//	class _A2_
//		Second argument type
//	class _A3_
//		Third argument type
//	class _A4_
//		Forth argument type
//
// NOTES
//	Argument is always bound, which is different from std::mem_fun_1

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_, class _A4_>
_WRAPPER(_CLASSNAME(MemberFunction4))<_R_, _T_, _A1_, _A2_, _A3_, _A4_>
MemberFunction(_R_ (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_), _ARGUMENT4(_A4_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _ARGUMENT4(_A4_) arg4)
{
	return _WRAPPER(_CLASSNAME(MemberFunction4))<_R_, _T_, _A1_, _A2_, _A3_, _A4_>(func, arg1, arg2, arg3, arg4);
}

#undef _WRAPPER

//
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
