// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Accumulator1.h --
//		Included by Functional_Accumulator.h
//		Don't include directly
// 
// Copyright (c) 2007, 2008, 2023 Ricoh Company, Ltd.
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

template <class _R_, class _T_, class _A_>
class _CLASSNAME(CallAccumulator1)
{
	typedef _RETURN(_R_) (*Function)(_T_, _ARGUMENT(_A_));
public:
	_CLASSNAME(CallAccumulator1)(Function func, _ARGUMENT(_A_) arg, _R_ init) : m_func(func), m_arg(arg), m_val(init) {}
	void operator()(_T_ obj)
	{
		m_val += (*m_func)(obj, m_arg);
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT(_A_) m_arg;
	_R_ m_val;
};

template <class _R_, class _T_, class _A_>
_CLASSNAME(CallAccumulator1)<_R_, _T_, _A_>
Accumulator(_RETURN(_R_) (*func)(_T_, _ARGUMENT(_A_)), _ARGUMENT(_A_) arg, _R_ init)
{
	return _CLASSNAME(CallAccumulator1)<_R_, _T_, _A_>(func, arg, init);
}

////////////////////////////////
// Accumulator using operator<
////////////////////////////////

template <class _R_, class _T_, class _A_>
class _CLASSNAME(CallGetMax1)
{
	typedef _RETURN(_R_) (*Function)(_T_, _ARGUMENT(_A_));
public:
	_CLASSNAME(CallGetMax1)(Function func, _ARGUMENT(_A_) arg, _R_ init) : m_func(func), m_arg(arg), m_val(init) {}
	void operator()(_T_ obj)
	{
		_RETURN(_R_) val = (*m_func)(obj, m_arg);
		if (m_val < val) m_val = val;
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT(_A_) m_arg;
	_R_ m_val;
};

template <class _R_, class _T_, class _A_>
_CLASSNAME(CallGetMax1)<_R_, _T_, _A_>
GetMax(_RETURN(_R_) (*func)(_T_, _ARGUMENT(_A_)), _ARGUMENT(_A_) arg, _R_ init)
{
	return _CLASSNAME(CallGetMax1)<_R_, _T_, _A_>(func, arg, init);
}

template <class _R_, class _T_, class _A_>
class _CLASSNAME(CallGetMin1)
{
	typedef _RETURN(_R_) (*Function)(_T_, _ARGUMENT(_A_));
public:
	_CLASSNAME(CallGetMin1)(Function func, _ARGUMENT(_A_) arg, _R_ init) : m_func(func), m_arg(arg), m_val(init) {}
	void operator()(_T_ obj)
	{
		_RETURN(_R_) val = (*m_func)(obj, m_arg);
		if (val < m_val) m_val = val;
	}

	_R_ getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT(_A_) m_arg;
	_R_ m_val;
};

template <class _R_, class _T_, class _A_>
_CLASSNAME(CallGetMin1)<_R_, _T_, _A_>
GetMin(_RETURN(_R_) (*func)(_T_, _ARGUMENT(_A_)), _ARGUMENT(_A_) arg, _R_ init)
{
	return _CLASSNAME(CallGetMin1)<_R_, _T_, _A_>(func, arg, init);
}

#ifndef _ONLY_REF
/////////////////////////////////
// Accumulator using operator&&
/////////////////////////////////

template <class _T_, class _A_>
class _CLASSNAME(CallIsAll1)
{
	typedef bool (*Function)(_T_, _ARGUMENT(_A_));
public:
	_CLASSNAME(CallIsAll1)(Function func, _ARGUMENT(_A_) arg) : m_func(func), m_arg(arg), m_val(true) {}
	void operator()(_T_ obj)
	{
		m_val = m_val && (*m_func)(obj, m_arg);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT(_A_) m_arg;
	bool m_val;
};

template <class _T_, class _A_>
_CLASSNAME(CallIsAll1)<_T_, _A_>
IsAll(bool (*func)(_T_, _ARGUMENT(_A_)), _ARGUMENT(_A_) arg)
{
	return _CLASSNAME(CallIsAll1)<_T_, _A_>(func, arg);
}

/////////////////////////////////
// Accumulator using operator||
/////////////////////////////////

template <class _T_, class _A_>
class _CLASSNAME(CallIsAny1)
{
	typedef bool (*Function)(_T_, _ARGUMENT(_A_));
public:
	_CLASSNAME(CallIsAny1)(Function func, _ARGUMENT(_A_) arg) : m_func(func), m_arg(arg), m_val(false) {}
	void operator()(_T_ obj)
	{
		m_val = m_val || (*m_func)(obj, m_arg);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT(_A_) m_arg;
	bool m_val;
};

template <class _T_, class _A_>
_CLASSNAME(CallIsAny1)<_T_, _A_>
IsAny(bool (*func)(_T_, _ARGUMENT(_A_)), _ARGUMENT(_A_) arg)
{
	return _CLASSNAME(CallIsAny1)<_T_, _A_>(func, arg);
}

#endif // _ONLY_REF

//
//	Copyright (c) 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
