// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Accumulator0.h --
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

template <class _R_, class _T_>
class _CLASSNAME(CallAccumulator)
{
	typedef _RETURN(_R_) (*Function)(_T_);
public:
	_CLASSNAME(CallAccumulator)(Function func, _R_ init) : m_func(func), m_val(init) {}
	void operator()(_T_ obj)
	{
		m_val += (m_func)(obj);
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_R_ m_val;
};

template <class _R_, class _T_>
_CLASSNAME(CallAccumulator)<_R_, _T_>
Accumulator(_RETURN(_R_) (*func)(_T_), _R_ init)
{
	return _CLASSNAME(CallAccumulator)<_R_, _T_>(func, init);
}

////////////////////////////////
// Accumulator using operator<
////////////////////////////////

template <class _R_, class _T_>
class _CLASSNAME(CallGetMax)
{
	typedef _RETURN(_R_) (*Function)(_T_);
public:
	_CLASSNAME(CallGetMax)(Function func, _R_ init) : m_func(func), m_val(init) {}
	void operator()(_T_ obj)
	{
		_RETURN(_R_) val = (m_func)(obj);
		if (m_val < val) m_val = val;
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_R_ m_val;
};

template <class _R_, class _T_>
_CLASSNAME(CallGetMax)<_R_, _T_>
GetMax(_RETURN(_R_) (*func)(_T_), _R_ init)
{
	return _CLASSNAME(CallGetMax)<_R_, _T_>(func, init);
}

template <class _R_, class _T_>
class _CLASSNAME(CallGetMin)
{
	typedef _RETURN(_R_) (*Function)(_T_);
public:
	_CLASSNAME(CallGetMin)(Function func, _R_ init) : m_func(func), m_val(init) {}
	void operator()(_T_ obj)
	{
		_RETURN(_R_) val = (m_func)(obj);
		if (val < m_val) m_val = val;
	}

	_R_ getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_R_ m_val;
};

template <class _R_, class _T_>
_CLASSNAME(CallGetMin)<_R_, _T_>
GetMin(_RETURN(_R_) (*func)(_T_), _R_ init)
{
	return _CLASSNAME(CallGetMin)<_R_, _T_>(func, init);
}

#ifndef _ONLY_REF
/////////////////////////////////
// Accumulator using operator&&
/////////////////////////////////

template <class _T_>
class _CLASSNAME(CallIsAll)
{
	typedef bool (*Function)(_T_);
public:
	_CLASSNAME(CallIsAll)(Function func) : m_func(func), m_val(true) {}
	void operator()(_T_ obj)
	{
		m_val = m_val && (m_func)(obj);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	bool m_val;
};

template <class _T_>
_CLASSNAME(CallIsAll)<_T_>
IsAll(bool (*func)(_T_))
{
	return _CLASSNAME(CallIsAll)<_T_>(func);
}

/////////////////////////////////
// Accumulator using operator||
/////////////////////////////////

template <class _T_>
class _CLASSNAME(CallIsAny)
{
	typedef bool (*Function)(_T_);
public:
	_CLASSNAME(CallIsAny)(Function func) : m_func(func), m_val(false) {}
	void operator()(_T_ obj)
	{
		m_val = m_val || (m_func)(obj);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	bool m_val;
};

template <class _T_>
_CLASSNAME(CallIsAny)<_T_>
IsAny(bool (*func)(_T_))
{
	return _CLASSNAME(CallIsAny)<_T_>(func);
}
#endif // _ONLY_REF

//
//	Copyright (c) 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
