// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Functional_Accumulator3.h --
//		Included by Functional_Accumulator.h
//		Don't include directly
// 
// Copyright (c) 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
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

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
class _CLASSNAME(Accumulator3)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST;
public:
	_CLASSNAME(Accumulator3)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_val(init) {}
	void operator()(_T_* obj)
	{
		m_val += (obj->*m_func)(m_arg1, m_arg2, m_arg3);
	}
	void operator()(_T_& obj)
	{
		m_val += (obj.*m_func)(m_arg1, m_arg2, m_arg3);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val += (obj.get()->*m_func)(m_arg1, m_arg2, m_arg3);
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_ARGUMENT3(_A3_) m_arg3;
	_R_ m_val;
};

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
_CLASSNAME(Accumulator3)<_R_, _T_, _A1_, _A2_, _A3_>
Accumulator(_RETURN(_R_) (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init)
{
	return _CLASSNAME(Accumulator3)<_R_, _T_, _A1_, _A2_, _A3_>(func, arg1, arg2, arg3, init);
}

////////////////////////////////
// Accumulator using operator<
////////////////////////////////

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
class _CLASSNAME(GetMax3)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST;
public:
	_CLASSNAME(GetMax3)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_val(init) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)(m_arg1, m_arg2, m_arg3);
		if (m_val < val) m_val = val;
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)(m_arg1, m_arg2, m_arg3);
		if (m_val < val) m_val = val;
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)(m_arg1, m_arg2, m_arg3);
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
	_ARGUMENT3(_A3_) m_arg3;
	_R_ m_val;
};

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
_CLASSNAME(GetMax3)<_R_, _T_, _A1_, _A2_, _A3_>
GetMax(_RETURN(_R_) (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init)
{
	return _CLASSNAME(GetMax3)<_R_, _T_, _A1_, _A2_, _A3_>(func, arg1, arg2, arg3, init);
}

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
class _CLASSNAME(GetMin3)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST;
public:
	_CLASSNAME(GetMin3)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_val(init) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)(m_arg1, m_arg2, m_arg3);
		if (val < m_val) m_val = val;
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)(m_arg1, m_arg2, m_arg3);
		if (val < m_val) m_val = val;
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)(m_arg1, m_arg2, m_arg3);
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
	_ARGUMENT3(_A3_) m_arg3;
	_R_ m_val;
};

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
_CLASSNAME(GetMin3)<_R_, _T_, _A1_, _A2_, _A3_>
GetMin(_RETURN(_R_) (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init)
{
	return _CLASSNAME(GetMin3)<_R_, _T_, _A1_, _A2_, _A3_>(func, arg1, arg2, arg3, init);
}

////////////////////////////////
// Accumulator using merger
////////////////////////////////

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
class _CLASSNAME(GetMerged3)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST;
	typedef _R_ (*Merger)(_R_, _R_);

public:
	_CLASSNAME(GetMerged3)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init, Merger merger) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_val(init), m_merger(merger) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)(m_arg1, m_arg2, m_arg3);
		m_val = (*m_merger)(m_val, val);
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)(m_arg1, m_arg2, m_arg3);
		m_val = (*m_merger)(m_val, val);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)(m_arg1, m_arg2, m_arg3);
		m_val = (*m_merger)(m_val, val);
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_ARGUMENT3(_A3_) m_arg3;
	_R_ m_val;
	Merger m_merger;
};

template <class _R_, class _T_, class _A1_, class _A2_, class _A3_>
_CLASSNAME(GetMerged3)<_R_, _T_, _A1_, _A2_, _A3_>
GetMerged(_RETURN(_R_) (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3, _R_ init, _R_ (*merger)(_R_, _R_))
{
	return _CLASSNAME(GetMerged3)<_R_, _T_, _A1_, _A2_, _A3_>(func, arg1, arg2, arg3, init, merger);
}

#ifndef _ONLY_REF
/////////////////////////////////
// Accumulator using operator&&
/////////////////////////////////

template <class _T_, class _A1_, class _A2_, class _A3_>
class _CLASSNAME(IsAll3)
{
	typedef bool (_T_::* Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST;
public:
	_CLASSNAME(IsAll3)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_val(true) {}
	void operator()(_T_* obj)
	{
		m_val = m_val && (obj->*m_func)(m_arg1, m_arg2, m_arg3);
	}
	void operator()(_T_& obj)
	{
		m_val = m_val && (obj.*m_func)(m_arg1, m_arg2, m_arg3);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val = m_val && (obj.get()->*m_func)(m_arg1, m_arg2, m_arg3);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_ARGUMENT3(_A3_) m_arg3;
	bool m_val;
};

template <class _T_, class _A1_, class _A2_, class _A3_>
_CLASSNAME(IsAll3)<_T_, _A1_, _A2_, _A3_>
IsAll(bool (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3)
{
	return _CLASSNAME(IsAll3)<_T_, _A1_, _A2_, _A3_>(func, arg1, arg2, arg3);
}

/////////////////////////////////
// Accumulator using operator||
/////////////////////////////////

template <class _T_, class _A1_, class _A2_, class _A3_>
class _CLASSNAME(IsAny3)
{
	typedef bool (_T_::* Function)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST;
public:
	_CLASSNAME(IsAny3)(Function func, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3) : m_func(func), m_arg1(arg1), m_arg2(arg2), m_arg3(arg3), m_val(false) {}
	void operator()(_T_* obj)
	{
		m_val = m_val || (obj->*m_func)(m_arg1, m_arg2, m_arg3);
	}
	void operator()(_T_& obj)
	{
		m_val = m_val || (obj.*m_func)(m_arg1, m_arg2, m_arg3);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val = m_val || (obj.get()->*m_func)(m_arg1, m_arg2, m_arg3);
	}

	bool getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT1(_A1_) m_arg1;
	_ARGUMENT2(_A2_) m_arg2;
	_ARGUMENT3(_A3_) m_arg3;
	bool m_val;
};

template <class _T_, class _A1_, class _A2_, class _A3_>
_CLASSNAME(IsAny3)<_T_, _A1_, _A2_, _A3_>
IsAny(bool (_T_::* func)(_ARGUMENT1(_A1_), _ARGUMENT2(_A2_), _ARGUMENT3(_A3_)) _CONST, _ARGUMENT1(_A1_) arg1, _ARGUMENT2(_A2_) arg2, _ARGUMENT3(_A3_) arg3)
{
	return _CLASSNAME(IsAny3)<_T_, _A1_, _A2_, _A3_>(func, arg1, arg2, arg3);
}

#endif // _ONLY_REF

//
//	Copyright (c) 2007, 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
