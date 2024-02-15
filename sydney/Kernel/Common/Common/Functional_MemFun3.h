// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_MemFun3.h --
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

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_ _CLASS_F>
class _CLASSNAME(MemberFunction3)
{
	_TYPEDEF(_R_ (_T_::*_F_)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST)
public:
	_CLASSNAME(MemberFunction3)(_F_ func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3)
		: m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3)
	{}
	_R_ operator()(_T_* obj) const
	{
		RETURN (obj->*m_func)(m_arg1, m_arg2, m_arg3);
	}
	_R_ operator()(_T_& obj) const
	{
		RETURN (obj.*m_func)(m_arg1, m_arg2, m_arg3);
	}
	// Definition for ObjectPointer
	template <class _X_>
	_R_ operator()(const ObjectPointer<_X_>& x_) const
	{
		RETURN (x_.get()->*m_func)(m_arg1, m_arg2, m_arg3);
	}

private:
	_F_ m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_ARGUMENT3(_A3_) m_arg3;
};

#ifdef SYD_VOID_NOT_RETURN // for compilers which reject 'return void'
#define _WRAPPER(x) _wrapper##x
template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
struct _WRAPPER(_CLASSNAME(MemberFunction3))
	: public _MemFun<_R_>::_CLASSNAME(MemberFunction3)<_R_, _T_, _R_ (_T_::*)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST>
{
	typedef _R_ (_T_::*Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST;
	explicit _WRAPPER(_CLASSNAME(MemberFunction3))(Function func)
		: _MemFun<_R_>::_CLASSNAME(MemberFunction3)<_R_, _T_, _R_ (_T_::*)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST>(func)
	{}
};
#else
#define _WRAPPER(x) x
#endif

// TEMPLATE FUNCTION
//	MemberFunction -- std::mem_fun_1 extension for 3 arguments
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
//
// NOTES
//	Argument is always bound, which is different from std::mem_fun_1

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
_WRAPPER(_CLASSNAME(MemberFunction3))<_R_, _T_, _A1_, _A2_, _A3_>
MemberFunction(_R_ (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3)
{
	return _WRAPPER(_CLASSNAME(MemberFunction3))<_R_, _T_, _A1_, _A2_, _A3_>(func, arg1, arg2, arg3);
}

#undef _WRAPPER

//
// Copyright (c) 2007, 2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
