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
class _CLASSNAME(Accumulator)
{
	typedef _RETURN(_R_) (_T_::* Function)() _CONST;
public:
	_CLASSNAME(Accumulator)(Function func, _R_ init) : m_func(func), m_val(init) {}
	void operator()(_T_* obj)
	{
		m_val += (obj->*m_func)();
	}
	void operator()(_T_& obj)
	{
		m_val += (obj.*m_func)();
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val += (obj.get()->*m_func)();
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
_CLASSNAME(Accumulator)<_R_, _T_>
Accumulator(_RETURN(_R_) (_T_::* func)() _CONST, _R_ init)
{
	return _CLASSNAME(Accumulator)<_R_, _T_>(func, init);
}

////////////////////////////////
// Accumulator using operator<
////////////////////////////////

template <class _R_, class _T_>
class _CLASSNAME(GetMax)
{
	typedef _RETURN(_R_) (_T_::* Function)() _CONST;
public:
	_CLASSNAME(GetMax)(Function func, _R_ init) : m_func(func), m_val(init) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)();
		if (m_val < val) m_val = val;
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)();
		if (m_val < val) m_val = val;
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)();
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
_CLASSNAME(GetMax)<_R_, _T_>
GetMax(_RETURN(_R_) (_T_::* func)() _CONST, _R_ init)
{
	return _CLASSNAME(GetMax)<_R_, _T_>(func, init);
}

template <class _R_, class _T_>
class _CLASSNAME(GetMin)
{
	typedef _RETURN(_R_) (_T_::* Function)() _CONST;
public:
	_CLASSNAME(GetMin)(Function func, _R_ init) : m_func(func), m_val(init) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)();
		if (val < m_val) m_val = val;
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)();
		if (val < m_val) m_val = val;
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)();
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
_CLASSNAME(GetMin)<_R_, _T_>
GetMin(_RETURN(_R_) (_T_::* func)() _CONST, _R_ init)
{
	return _CLASSNAME(GetMin)<_R_, _T_>(func, init);
}

////////////////////////////////
// Accumulator using merger
////////////////////////////////

template <class _R_, class _T_>
class _CLASSNAME(GetMerged)
{
	typedef _RETURN(_R_) (_T_::* Function)() _CONST;
	typedef _R_ (*Merger)(_R_, _R_);

public:
	_CLASSNAME(GetMerged)(Function func, _R_ init, Merger merger) : m_func(func), m_val(init), m_merger(merger) {}
	void operator()(_T_* obj)
	{
		_RETURN(_R_) val = (obj->*m_func)();
		m_val = (*m_merger)(m_val, val);
	}
	void operator()(_T_& obj)
	{
		_RETURN(_R_) val = (obj.*m_func)();
		m_val = (*m_merger)(m_val, val);
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		_RETURN(_R_) val = (obj.get()->*m_func)();
		m_val = (*m_merger)(m_val, val);
	}

	_RETURN(_R_) getVal()
	{
		return m_val;
	}
private:
	Function m_func;
	_R_ m_val;
	Merger m_merger;
};

template <class _R_, class _T_>
_CLASSNAME(GetMerged)<_R_, _T_>
GetMerged(_RETURN(_R_) (_T_::* func)() _CONST, _R_ init, _R_ (*merger)(_R_, _R_))
{
	return _CLASSNAME(GetMerged)<_R_, _T_>(func, init, merger);
}

#ifndef _ONLY_REF
/////////////////////////////////
// Accumulator using operator&&
/////////////////////////////////

template <class _T_>
class _CLASSNAME(IsAll)
{
	typedef bool (_T_::* Function)() _CONST;
public:
	_CLASSNAME(IsAll)(Function func) : m_func(func), m_val(true) {}
	void operator()(_T_* obj)
	{
		m_val = m_val && (obj->*m_func)();
	}
	void operator()(_T_& obj)
	{
		m_val = m_val && (obj.*m_func)();
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val = m_val && (obj.get()->*m_func)();
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
_CLASSNAME(IsAll)<_T_>
IsAll(bool (_T_::* func)() _CONST)
{
	return _CLASSNAME(IsAll)<_T_>(func);
}

/////////////////////////////////
// Accumulator using operator||
/////////////////////////////////

template <class _T_>
class _CLASSNAME(IsAny)
{
	typedef bool (_T_::* Function)() _CONST;
public:
	_CLASSNAME(IsAny)(Function func) : m_func(func), m_val(false) {}
	void operator()(_T_* obj)
	{
		m_val = m_val || (obj->*m_func)();
	}
	void operator()(_T_& obj)
	{
		m_val = m_val || (obj.*m_func)();
	}
	template <class _X_>
	void operator()(const ObjectPointer<_X_>& obj)
	{
		m_val = m_val || (obj.get()->*m_func)();
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
_CLASSNAME(IsAny)<_T_>
IsAny(bool (_T_::* func)() _CONST)
{
	return _CLASSNAME(IsAny)<_T_>(func);
}
#endif // _ONLY_REF

//
//	Copyright (c) 2007, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
