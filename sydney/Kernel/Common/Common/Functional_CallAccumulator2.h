// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Accumulator1.h --
//		Included by Functional_Accumulator.h
//		Don't include directly
// 
// Copyright (c) 2008, 2023 Ricoh Company, Ltd.
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

/////////////////////////////////
// Accumulator using operator+=
/////////////////////////////////

template <class _R_, class _T_, class _A1_, class _A2_>
class _CLASSNAME(CallAccumulator2)
{
	typedef _RETURN(_R_) (*Function)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_));
public:
	_CLASSNAME(CallAccumulator2)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _R_ init) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_val(init) {}
	void operator()(_T_ obj)
	{
		m_val += (*m_func)(obj, m_arg1, m_arg2);
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_R_ m_val;
};

template <class _R_, class _T_, class _A1_, class _A2_>
_CLASSNAME(CallAccumulator2)<_R_, _T_, _A1_, _A2_>
Accumulator(_RETURN(_R_) (*func)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)), _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _R_ init)
{
	return _CLASSNAME(CallAccumulator2)<_R_, _T_, _A1_, _A2_>(func, arg1, arg2, init);
}

////////////////////////////////
// Accumulator using operator<
////////////////////////////////

template <class _R_, class _T_, class _A1_, class _A2_>
class _CLASSNAME(CallGetMax2)
{
	typedef _RETURN(_R_) (*Function)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_));
public:
	_CLASSNAME(CallGetMax2)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _R_ init) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_val(init) {}
	void operator()(_T_ obj)
	{
		_RETURN(_R_) val = (*m_func)(obj, m_arg1, m_arg2);
		if (m_val < val) m_val = val;
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_R_ m_val;
};

template <class _R_, class _T_, class _A1_, class _A2_>
_CLASSNAME(CallGetMax2)<_R_, _T_, _A1_, _A2_>
GetMax(_RETURN(_R_) (*func)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)), _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _R_ init)
{
	return _CLASSNAME(CallGetMax2)<_R_, _T_, _A1_, _A2_>(func, arg1, arg2, init);
}

template <class _R_, class _T_, class _A1_, class _A2_>
class _CLASSNAME(CallGetMin2)
{
	typedef _RETURN(_R_) (*Function)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_));
public:
	_CLASSNAME(CallGetMin2)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _R_ init) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_val(init) {}
	void operator()(_T_ obj)
	{
		_RETURN(_R_) val = (*m_func)(obj, m_arg1, m_arg2);
		if (val < m_val) m_val = val;
	}

	_R_ getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_R_ m_val;
};

template <class _R_, class _T_, class _A1_, class _A2_>
_CLASSNAME(CallGetMin2)<_R_, _T_, _A1_, _A2_>
GetMin(_RETURN(_R_) (*func)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)), _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _R_ init)
{
	return _CLASSNAME(CallGetMin2)<_R_, _T_, _A1_, _A2_>(func, arg1, arg2, init);
}

#ifndef _ONLY_REF
/////////////////////////////////
// Accumulator using operator&&
/////////////////////////////////

template <class _T_, class _A1_, class _A2_>
class _CLASSNAME(CallIsAll2)
{
	typedef bool (*Function)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_));
public:
	_CLASSNAME(CallIsAll2)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_val(true) {}
	void operator()(_T_ obj)
	{
		m_val = m_val && (*m_func)(obj, m_arg1, m_arg2);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	bool m_val;
};

template <class _T_, class _A1_, class _A2_>
_CLASSNAME(CallIsAll2)<_T_, _A1_, _A2_>
IsAll(bool (*func)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)), _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
{
	return _CLASSNAME(CallIsAll2)<_T_, _A1_, _A2_>(func, arg1, arg2);
}

/////////////////////////////////
// Accumulator using operator||
/////////////////////////////////

template <class _T_, class _A1_, class _A2_>
class _CLASSNAME(CallIsAny2)
{
	typedef bool (*Function)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_));
public:
	_CLASSNAME(CallIsAny2)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_val(false) {}
	void operator()(_T_ obj)
	{
		m_val = m_val || (*m_func)(obj, m_arg1, m_arg2);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	bool m_val;
};

template <class _T_, class _A1_, class _A2_>
_CLASSNAME(CallIsAny2)<_T_, _A1_, _A2_>
IsAny(bool (*func)(_T_, _ARGUMENT1(_A1_), _ARGUMENT2(_A2_)), _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2)
{
	return _CLASSNAME(CallIsAny2)<_T_, _A1_, _A2_>(func, arg1, arg2);
}

#endif // _ONLY_REF

//
//	Copyright (c) 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
