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
class _CLASSNAME(Accumulator1)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT(_A_)) _CONST;
public:
	_CLASSNAME(Accumulator1)(Function func, _ARGUMENT(_A_) arg, _R_ init) : m_func(func), m_arg(arg), m_val(init) {}
	void operator()(_T_* obj)
	{
		m_val += (obj->*m_func)(m_arg);
	}
	void operator()(_T_& obj)
	{
		m_val += (obj.*m_func)(m_arg);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val += (obj.get()->*m_func)(m_arg);
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
_CLASSNAME(Accumulator1)<_R_, _T_, _A_>
Accumulator(_RETURN(_R_) (_T_::* func)(_ARGUMENT(_A_)) _CONST, _ARGUMENT(_A_) arg, _R_ init)
{
	return _CLASSNAME(Accumulator1)<_R_, _T_, _A_>(func, arg, init);
}

////////////////////////////////
// Accumulator using operator<
////////////////////////////////

template <class _R_, class _T_, class _A_>
class _CLASSNAME(GetMax1)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT(_A_)) _CONST;
public:
	_CLASSNAME(GetMax1)(Function func, _ARGUMENT(_A_) arg, _R_ init) : m_func(func), m_arg(arg), m_val(init) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)(m_arg);
		if (m_val < val) m_val = val;
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)(m_arg);
		if (m_val < val) m_val = val;
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)(m_arg);
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
_CLASSNAME(GetMax1)<_R_, _T_, _A_>
GetMax(_RETURN(_R_) (_T_::* func)(_ARGUMENT(_A_)) _CONST, _ARGUMENT(_A_) arg, _R_ init)
{
	return _CLASSNAME(GetMax1)<_R_, _T_, _A_>(func, arg, init);
}

template <class _R_, class _T_, class _A_>
class _CLASSNAME(GetMin1)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT(_A_)) _CONST;
public:
	_CLASSNAME(GetMin1)(Function func, _ARGUMENT(_A_) arg, _R_ init) : m_func(func), m_arg(arg), m_val(init) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)(m_arg);
		if (val < m_val) m_val = val;
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)(m_arg);
		if (val < m_val) m_val = val;
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)(m_arg);
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
_CLASSNAME(GetMin1)<_R_, _T_, _A_>
GetMin(_RETURN(_R_) (_T_::* func)(_ARGUMENT(_A_)) _CONST, _ARGUMENT(_A_) arg, _R_ init)
{
	return _CLASSNAME(GetMin1)<_R_, _T_, _A_>(func, arg, init);
}

////////////////////////////////
// Accumulator using merger
////////////////////////////////

template <class _R_, class _T_, class _A_>
class _CLASSNAME(GetMerged1)
{
	typedef _RETURN(_R_) (_T_::* Function)(_ARGUMENT(_A_)) _CONST;
	typedef _R_ (*Merger)(_R_, _R_);

public:
	_CLASSNAME(GetMerged1)(Function func, _ARGUMENT(_A_) arg, _R_ init, Merger merger) : m_func(func), m_arg(arg), m_val(init), m_merger(merger) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)(m_arg);
		m_val = (*m_merger)(m_val, val);
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)(m_arg);
		m_val = (*m_merger)(m_val, val);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)(m_arg);
		m_val = (*m_merger)(m_val, val);
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_ARGUMENT(_A_) m_arg;
	_R_ m_val;
	Merger m_merger;
};

template <class _R_, class _T_, class _A_>
_CLASSNAME(GetMerged1)<_R_, _T_, _A_>
GetMerged(_RETURN(_R_) (_T_::* func)(_ARGUMENT(_A_)) _CONST, _ARGUMENT(_A_) arg, _R_ init, _R_ (*merger)(_R_, _R_))
{
	return _CLASSNAME(GetMerged1)<_R_, _T_, _A_>(func, arg, init, merger);
}

#ifndef _ONLY_REF
/////////////////////////////////
// Accumulator using operator&&
/////////////////////////////////

template <class _T_, class _A_>
class _CLASSNAME(IsAll1)
{
	typedef bool (_T_::* Function)(_ARGUMENT(_A_)) _CONST;
public:
	_CLASSNAME(IsAll1)(Function func, _ARGUMENT(_A_) arg) : m_func(func), m_arg(arg), m_val(true) {}
	void operator()(_T_* obj)
	{
		m_val = m_val && (obj->*m_func)(m_arg);
	}
	void operator()(_T_& obj)
	{
		m_val = m_val && (obj.*m_func)(m_arg);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val = m_val && (obj.get()->*m_func)(m_arg);
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
_CLASSNAME(IsAll1)<_T_, _A_>
IsAll(bool (_T_::* func)(_ARGUMENT(_A_)) _CONST, _ARGUMENT(_A_) arg)
{
	return _CLASSNAME(IsAll1)<_T_, _A_>(func, arg);
}

/////////////////////////////////
// Accumulator using operator||
/////////////////////////////////

template <class _T_, class _A_>
class _CLASSNAME(IsAny1)
{
	typedef bool (_T_::* Function)(_ARGUMENT(_A_)) _CONST;
public:
	_CLASSNAME(IsAny1)(Function func, _ARGUMENT(_A_) arg) : m_func(func), m_arg(arg), m_val(false) {}
	void operator()(_T_* obj)
	{
		m_val = m_val || (obj->*m_func)(m_arg);
	}
	void operator()(_T_& obj)
	{
		m_val = m_val || (obj.*m_func)(m_arg);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val = m_val || (obj.get()->*m_func)(m_arg);
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
_CLASSNAME(IsAny1)<_T_, _A_>
IsAny(bool (_T_::* func)(_ARGUMENT(_A_)) _CONST, _ARGUMENT(_A_) arg)
{
	return _CLASSNAME(IsAny1)<_T_, _A_>(func, arg);
}

#endif // _ONLY_REF

//
//	Copyright (c) 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
