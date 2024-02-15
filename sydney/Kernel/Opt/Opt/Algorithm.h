// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/Algorithm.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_ALGORITHM_H
#define __SYDNEY_OPT_ALGORITHM_H

#include "boost/function.hpp"

#include "Opt/Module.h"

#ifdef SYD_USE_LARGE_VECTOR
#include "Common/LargeVector.h"
#endif
#include "Common/VectorMap.h"

#include "ModAlgorithm.h"
#include "ModAutoPointer.h"
#include "ModHashMap.h"
#include "ModOstream.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

//////////////////////
// Functions
//////////////////////

// TEMPLATE CLASS
//	Opt::DoNothingImpl --
//
// TEMPLATE ARGUMENTS
//	class Value_
//
// NOTES
template <class Value_>
class DoNothingImpl
{
public:
	DoNothingImpl() {}
	~DoNothingImpl() {}

	void operator()(const Value_* p) const {}
};

// TEMPLATE FUNCTION public
//	Opt::DoNothing -- 
//
// TEMPLATE ARGUMENTS
//	class Value_
//	
// NOTES
//
// ARGUMENTS
//	const Value_* p
//	
// RETURN
//	DoNothingImpl<Value_>
//
// EXCEPTIONS

template <class Value_>
DoNothingImpl<Value_>
DoNothing(const Value_* p)
{
	return DoNothingImpl<Value_>();
}

//////////////////////
// Comparators
//////////////////////

//	CLASS
//	Opt::CaseInsensitiveComparator -- string comparator (case insensitive)
//
//	NOTES
class CaseInsensitiveComparator
{
public:
	CaseInsensitiveComparator() {}
	~CaseInsensitiveComparator() {}

	// case insensitive less operator
	bool operator()(const ModUnicodeString& cstrName1_,
					const ModUnicodeString& cstrName2_) const
	{
		return cstrName1_.compare(cstrName2_, ModFalse /* case insensitive */) < 0;
	}
};

// TEMPLATE CLASS
//	Opt::EqualComparator -- == comparator
//
// TEMPLATE ARGUMENTS
//	class V_
//
// NOTES
template <class V_>
class EqualComparator
{
public:
	EqualComparator(V_ v_) : m_v(v_) {}
	~EqualComparator() {}

	bool operator()(V_ v_) const
	{
		return v_ == m_v;
	}
private:
	V_ m_v;
};

// TEMPLATE CLASS
//	Opt::ValidPointerFilter -- pointer validation
//
// TEMPLATE ARGUMENTS
//	class V_
//
// NOTES
template <class V_>
class ValidPointerFilter
{
public:
	ValidPointerFilter() {}
	~ValidPointerFilter() {}

	bool operator()(V_* v_) const
	{
		return v_ != 0;
	}
};

// TEMPLATE CLASS
//	Opt::LogicalAnd -- logical and
//
// TEMPLATE ARGUMENTS
//	class V_
//
// NOTES
template <class V_>
class LogicalAnd
{
public:
	LogicalAnd(boost::function<bool(V_*)> function1_,
			   boost::function<bool(V_*)> function2_)
		: m_function1(function1_),
		  m_function2(function2_)
	{}
	~LogicalAnd() {}

	bool operator()(V_* p_)
	{return m_function1(p_) && m_function2(p_);}
private:
	boost::function<bool(V_*)> m_function1;
	boost::function<bool(V_*)> m_function2;
};

//////////////////////
// VECTOR utility
//////////////////////

// TEMPLATE FUNCTION
//	Opt::ExpandContainer -- expand container size to the size
//
// TEMPLATE ARGUMENTS
//	class Container_
//
// NOTES
//
// ARGUMENTS
//	Container_& cTarget_
//	int iSize_
//
// RETURN
//	Nothing
//
// EXCEPTIONS
template <class Container_>
inline
void
ExpandContainer(Container_& cTarget_, int iSize_)
{
	if (static_cast<int>(cTarget_.getSize()) < iSize_) {
		Container_ cFill(iSize_ - cTarget_.getSize());
		cTarget_.insert(cTarget_.end(), cFill.begin(), cFill.end());
	}
}

// TEMPLATE FUNCTION
//	Opt::ExpandContainer -- expand container size to the size
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Value_
//
// NOTES
//
// ARGUMENTS
//	Container_& cTarget_
//	int iSize_
//	const Value_& cValue_
//
// RETURN
//	Nothing
//
// EXCEPTIONS
template <class Container_, class Value_>
inline
void
ExpandContainer(Container_& cTarget_, int iSize_, const Value_& cValue_)
{
	if (static_cast<int>(cTarget_.getSize()) < iSize_) {
		Container_ cFill(iSize_ - cTarget_.getSize(), cValue_);
		cTarget_.insert(cTarget_.end(), cFill.begin(), cFill.end());
	}
}

// TEMPLATE FUNCTION public
//	Opt::MapContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Container_& cContainer_
//	Function_ function_
//	
// RETURN
//	void
//
// EXCEPTIONS

template <class Container_, class Function_>
inline
void
MapContainer(Container_& cContainer_, Function_ function_)
{
	typename Container_::Iterator iterator = cContainer_.begin();
	const typename Container_::Iterator last = cContainer_.end();
	for (; iterator != last; ++iterator) {
		*iterator = function_(*iterator);
	}
}

// TEMPLATE FUNCTION public
//	Opt::MapContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Container2_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator cFirst_
//	Iterator cLast_
//	Container2_& cContainer2_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Iterator_, class Container2_, class Function_>
inline
void
MapContainer(Iterator_ cFirst_, Iterator_ cLast_, Container2_& cContainer2_, Function_ function_)
{
	for (; cFirst_ != cLast_; ++cFirst_) {
		cContainer2_.pushBack(function_(*cFirst_));
	}
}

// TEMPLATE FUNCTION public
//	Opt::MapContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Container2_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Container_& cContainer_
//	Container2_& cContainer2_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Container2_, class Function_>
inline
void
MapContainer(Container_& cContainer_, Container2_& cContainer2_, Function_ function_)
{
	MapContainer(cContainer_.begin(),
				 cContainer_.end(),
				 cContainer2_,
				 function_);
}

// TEMPLATE FUNCTION public
//	Opt::MapContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Container2_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Container_& cContainer_
//	Container2_& cContainer2_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Container2_, class Function_>
inline
void
MapContainer(const Container_& cContainer_, Container2_& cContainer2_, Function_ function_)
{
	MapContainer(cContainer_.begin(),
				 cContainer_.end(),
				 cContainer2_,
				 function_);
}

// TEMPLATE FUNCTION public
//	Opt::FilterContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Container_& cContainer_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Function_>
inline
void
FilterContainer(Container_& cContainer_, Function_ function_)
{
	typename Container_::Iterator iterator = cContainer_.begin();
	typename Container_::Iterator last = cContainer_.end();
	while (iterator != last) {
		if (function_(*iterator)) {
			++iterator;
		} else {
			iterator = cContainer_.erase(iterator);
			last = cContainer_.end();
		}
	}
}

// TEMPLATE FUNCTION public
//	Opt::FilterContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Container2_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Container_& cContainer_
//	Container2_& cContainer2_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Container2_, class Function_>
inline
void
FilterContainer(Container_& cContainer_, Container2_& cContainer2_, Function_ function_)
{
	typename Container_::ConstIterator iterator = cContainer_.begin();
	const typename Container_::ConstIterator last = cContainer_.end();
	for (; iterator != last; ++iterator) {
		if (function_(*iterator)) {
			cContainer2_.pushBack(*iterator);
		}
	}
}

// TEMPLATE FUNCTION public
//	Opt::FilterContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Container2_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Container_& cContainer_
//	Container2_& cContainer2_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Container2_, class Function_>
inline
void
FilterContainer(const Container_& cContainer_, Container2_& cContainer2_, Function_ function_)
{
	typename Container_::ConstIterator iterator = cContainer_.begin();
	const typename Container_::ConstIterator last = cContainer_.end();
	for (; iterator != last; ++iterator) {
		if (function_(*iterator)) {
			cContainer2_.pushBack(*iterator);
		}
	}
}

// TEMPLATE FUNCTION public
//	Opt::FilterOutContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Container_& cContainer_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Function_>
inline
void
FilterOutContainer(Container_& cContainer_, Function_ function_)
{
	typename Container_::Iterator iterator = cContainer_.begin();
	typename Container_::Iterator last = cContainer_.end();
	while (iterator != last) {
		if (function_(*iterator) == false) {
			++iterator;
		} else {
			iterator = cContainer_.erase(iterator);
			last = cContainer_.end();
		}
	}
}

// TEMPLATE FUNCTION public
//	Opt::FilterOutContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Container2_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Container_& cContainer_
//	Container2_& cContainer2_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Container2_, class Function_>
inline
void
FilterOutContainer(Container_& cContainer_, Container2_& cContainer2_, Function_ function_)
{
	typename Container_::ConstIterator iterator = cContainer_.begin();
	const typename Container_::ConstIterator last = cContainer_.end();
	for (; iterator != last; ++iterator) {
		if (function_(*iterator) == false) {
			cContainer2_.pushBack(*iterator);
		}
	}
}

// TEMPLATE FUNCTION public
//	Opt::FilterOutContainer -- 
//
// TEMPLATE ARGUMENTS
//	class Container_
//	class Container2_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Container_& cContainer_
//	Container2_& cContainer2_
//	Function_ function_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Container_, class Container2_, class Function_>
inline
void
FilterOutContainer(const Container_& cContainer_, Container2_& cContainer2_, Function_ function_)
{
	typename Container_::ConstIterator iterator = cContainer_.begin();
	const typename Container_::ConstIterator last = cContainer_.end();
	for (; iterator != last; ++iterator) {
		if (function_(*iterator) == false) {
			cContainer2_.pushBack(*iterator);
		}
	}
}

//////////////////////
// Iterators
//////////////////////

// TEMPLATE FUNCTION public
//	Opt::Find -- find last element which satisfies a function
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	
// RETURN
//	Iterator_
//
// EXCEPTIONS

template <class Iterator_, class Function_>
inline
Iterator_
Find(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_)
{
	for (; cFirst_ != cLast_; ++cFirst_) {
		if (function_(*cFirst_)) {
			return cFirst_;
		}
	}
	return cLast_;
}

// TEMPLATE FUNCTION public
//	Opt::FindLast -- find last element which satisfies a function
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	
// RETURN
//	Iterator_
//
// EXCEPTIONS

template <class Iterator_, class Function_>
inline
Iterator_
FindLast(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_)
{
	Iterator_ cResult = cLast_;
	for (; cFirst_ != cLast_; ++cFirst_) {
		if (function_(*cFirst_)) {
			cResult = cFirst_;
		}
	}
	return cResult;
}

// TEMPLATE FUNCTION public
//	Opt::FindMin -- find element returning minimum value with specified function
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Iterator_
//
// EXCEPTIONS

template <class Iterator_, class Value_, class Function_>
inline
Iterator_
FindMin(Iterator_ cFirst_, Iterator_ cLast_, const Value_& cInit_, Function_ function_)
{
	Iterator_ cResult = cLast_;
	Value_ cMin(cInit_);
	for (; cFirst_ != cLast_; ++cFirst_) {
		Value_ cValue(function_(*cFirst_));
		if (cValue < cMin) {
			cMin = cValue;
			cResult = cFirst_;
		}
	}
	return cResult;
}

// TEMPLATE FUNCTION public
//	Opt::FindMax -- find element returning maximum value with specified function
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Value_
//
// EXCEPTIONS

template <class Iterator_, class Value_, class Function_>
inline
Iterator_
FindMax(Iterator_ cFirst_, Iterator_ cLast_, const Value_& cInit_, Function_ function_)
{
	Iterator_ cResult = cLast_;
	Value_ cMax(cInit_);
	for (; cFirst_ != cLast_; ++cFirst_) {
		Value_ cValue(function_(*cFirst_));
		if (cMax < cValue) {
			cMax = cValue;
			cResult = cFirst_;
		}
	}
	return cResult;
}

// TEMPLATE FUNCTION public
//	Opt::IsAllKey -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
bool
IsAllKey(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		if (function_((*iterator).first) == false) {
			return false;
		}
	}
	return true;
}

// TEMPLATE FUNCTION public
//	Opt::IsAllValue -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
bool
IsAllValue(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		if (function_((*iterator).second) == false) {
			return false;
		}
	}
	return true;
}

// TEMPLATE FUNCTION public
//	Opt::IsAllMap -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
bool
IsAllMap(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		if (function_((*iterator).first, (*iterator).second) == false) {
			return false;
		}
	}
	return true;
}

// TEMPLATE FUNCTION public
//	Opt::IsAll -- check if all the elements satisfies a function
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Iterator_, class Function_>
inline
bool
IsAll(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_)
{
	for (; cFirst_ != cLast_; ++cFirst_) {
		if (function_(*cFirst_) == false) {
			return false;
		}
	}
	return true;
}

// TEMPLATE FUNCTION public
//	Opt::IsAll -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
bool
IsAll(const Collection_& cCollection_, Function_ function_)
{
	return IsAll(cCollection_.begin(),
				 cCollection_.end(),
				 function_);
}

// TEMPLATE FUNCTION public
//	Opt::IsAll -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
bool
IsAll(Collection_& cCollection_, Function_ function_)
{
	return IsAll(cCollection_.begin(),
				 cCollection_.end(),
				 function_);
}

// TEMPLATE FUNCTION public
//	Opt::IsAnyKey -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
bool
IsAnyKey(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		if (function_((*iterator).first) == true) {
			return true;
		}
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Opt::IsAnyValue -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
bool
IsAnyValue(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		if (function_((*iterator).second) == true) {
			return true;
		}
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Opt::IsAnyMap -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
bool
IsAnyMap(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		if (function_((*iterator).first, (*iterator).second) == true) {
			return true;
		}
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Opt::IsAny -- check if any element satisfies a function
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Iterator_, class Function_>
inline
bool
IsAny(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_)
{
	for (; cFirst_ != cLast_; ++cFirst_) {
		if (function_(*cFirst_) == true) {
			return true;
		}
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Opt::IsAny -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
bool
IsAny(const Collection_& cCollection_, Function_ function_)
{
	return IsAny(cCollection_.begin(),
				 cCollection_.end(),
				 function_);
}

// TEMPLATE FUNCTION public
//	Opt::IsAny -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
bool
IsAny(Collection_& cCollection_, Function_ function_)
{
	return IsAny(cCollection_.begin(),
				 cCollection_.end(),
				 function_);
}

// TEMPLATE FUNCTION public
//	Opt::GetMin -- get minimum value
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Value_
//
// EXCEPTIONS

template <class Iterator_, class Value_, class Function_>
inline
Value_
GetMin(Iterator_ cFirst_, Iterator_ cLast_, Value_ cInit_, Function_ function_)
{
	for (; cFirst_ != cLast_; ++cFirst_) {
		Value_ cValue(function_(*cFirst_));
		if (cValue < cInit_) cInit_ = cValue;
	}
	return cInit_;
}

// TEMPLATE FUNCTION public
//	Opt::GetMin -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Value_
//
// EXCEPTIONS

template <class Collection_, class Value_, class Function_>
inline
Value_
GetMin(const Collection_& cCollection_, Value_ cInit_, Function_ function_)
{
	return GetMin(cCollection_.begin(),
				  cCollection_.end(),
				  cInit_,
				  function_);
}

// TEMPLATE FUNCTION public
//	Opt::GetMin -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Value_
//
// EXCEPTIONS

template <class Collection_, class Value_, class Function_>
inline
Value_
GetMin(Collection_& cCollection_, Value_ cInit_, Function_ function_)
{
	return GetMin(cCollection_.begin(),
				  cCollection_.end(),
				  cInit_,
				  function_);
}

// TEMPLATE FUNCTION public
//	Opt::GetMax -- get maximum value
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Value_
//
// EXCEPTIONS

template <class Iterator_, class Value_, class Function_>
inline
Value_
GetMax(Iterator_ cFirst_, Iterator_ cLast_, Value_ cInit_, Function_ function_)
{
	for (; cFirst_ != cLast_; ++cFirst_) {
		Value_ cValue(function_(*cFirst_));
		if (cValue > cInit_) cInit_ = cValue;
	}
	return cInit_;
}

// TEMPLATE FUNCTION public
//	Opt::GetMax -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Value_
//
// EXCEPTIONS

template <class Collection_, class Value_, class Function_>
inline
Value_
GetMax(const Collection_& cCollection_, Value_ cInit_, Function_ function_)
{
	return GetMax(cCollection_.begin(),
				  cCollection_.end(),
				  cInit_,
				  function_);
}

// TEMPLATE FUNCTION public
//	Opt::GetMax -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Value_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Value_ cInit_
//	Function_ function_
//	
// RETURN
//	Value_
//
// EXCEPTIONS

template <class Collection_, class Value_, class Function_>
inline
Value_
GetMax(Collection_& cCollection_, Value_ cInit_, Function_ function_)
{
	return GetMax(cCollection_.begin(),
				  cCollection_.end(),
				  cInit_,
				  function_);
}

//////////////////////
// ForEach
//////////////////////

// TEMPLATE FUNCTION public
//	Opt::ForEachKey -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
Function_
ForEachKey(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		function_((*iterator).first);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEachKey -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Map_& cMap_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
Function_
ForEachKey(Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		function_((*iterator).first);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEachValue -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
Function_
ForEachValue(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		function_((*iterator).second);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEachValue -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Map_& cMap_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
Function_
ForEachValue(Map_& cMap_, Function_ function_)
{
	typename Map_::Iterator iterator = cMap_.begin();
	const typename Map_::Iterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		function_((*iterator).second);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEachMap -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Map_& cMap_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
Function_
ForEachMap(const Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		function_((*iterator).first, (*iterator).second);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEachMap -- 
//
// TEMPLATE ARGUMENTS
//	class Map_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Map_& cMap_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Map_, class Function_>
inline
Function_
ForEachMap(Map_& cMap_, Function_ function_)
{
	typename Map_::ConstIterator iterator = cMap_.begin();
	const typename Map_::ConstIterator last = cMap_.end();
	for (; iterator != last; ++iterator) {
		function_((*iterator).first, (*iterator).second);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEach -- 
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Iterator_, class Function_>
inline
Function_
ForEach(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_)
{
	for (; cFirst_ != cLast_; ++cFirst_)
		function_(*cFirst_);
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEach -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
Function_
ForEach(const Collection_& cCollection_, Function_ function_)
{
	return ForEach(cCollection_.begin(),
				   cCollection_.end(),
				   function_);
}

// TEMPLATE FUNCTION public
//	Opt::ForEach -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
Function_
ForEach(Collection_& cCollection_, Function_ function_)
{
	return ForEach(cCollection_.begin(),
				   cCollection_.end(),
				   function_);
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_i -- 
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Iterator_, class Function_>
inline
Function_
ForEach_i(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_)
{
	for (int idx = 0; cFirst_ != cLast_; ++cFirst_, ++idx) {
		function_(*cFirst_, idx);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_i -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
Function_
ForEach_i(const Collection_& cCollection_, Function_ function_)
{
	return ForEach_i(cCollection_.begin(),
					 cCollection_.end(),
					 function_);
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_i -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
Function_
ForEach_i(Collection_& cCollection_, Function_ function_)
{
	return ForEach_i(cCollection_.begin(),
					 cCollection_.end(),
					 function_);
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_r -- 
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Iterator_, class Function_>
inline
Function_
ForEach_r(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_)
{
	if (cFirst_ != cLast_) {
		do {
			--cLast_;
			function_(*cLast_);
		} while (cFirst_ != cLast_);
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_r -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
Function_
ForEach_r(const Collection_& cCollection_, Function_ function_)
{
	return ForEach_r(cCollection_.begin(),
					 cCollection_.end(),
					 function_);
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_r -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Function_ function_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_>
inline
Function_
ForEach_r(Collection_& cCollection_, Function_ function_)
{
	return ForEach_r(cCollection_.begin(),
					 cCollection_.end(),
					 function_);
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_if -- 
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	class Filter_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	Filter_ filter_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Iterator_, class Function_, class Filter_>
inline
Function_
ForEach_if(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_, Filter_ filter_)
{
	for (; cFirst_ != cLast_; ++cFirst_) {
		if (filter_(*cFirst_)) {
			function_(*cFirst_);
		}
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_if -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	class Filter_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Function_ function_
//	Filter_ filter_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_, class Filter_>
inline
Function_
ForEach_if(const Collection_& cCollection_, Function_ function_, Filter_ filter_)
{
	return ForEach_if(cCollection_.begin(),
					  cCollection_.end(),
					  function_,
					  filter_);
}

// TEMPLATE FUNCTION public
//	Opt::ForEach_if -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	class Filter_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Function_ function_
//	Filter_ filter_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_, class Filter_>
inline
Function_
ForEach_if(Collection_& cCollection_, Function_ function_, Filter_ filter_)
{
	return ForEach_if(cCollection_.begin(),
					  cCollection_.end(),
					  function_,
					  filter_);
}

// TEMPLATE FUNCTION public
//	Opt::Join -- 
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Function_
//	class Delimiter_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Function_ function_
//	Delimiter_ delimiter_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Iterator_, class Function_, class Delimiter_>
inline
Function_
Join(Iterator_ cFirst_, Iterator_ cLast_, Function_ function_, Delimiter_ delimiter_)
{
	if (cFirst_ != cLast_) {
		function_(*cFirst_);
		for (++cFirst_; cFirst_ != cLast_; ++cFirst_) {
			delimiter_();
			function_(*cFirst_);
		}
	}
	return function_;
}

// TEMPLATE FUNCTION public
//	Opt::Join -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	class Delimiter_
//	
// NOTES
//
// ARGUMENTS
//	const Collection_& cCollection_
//	Function_ function_
//	Delimiter_ delimiter_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_, class Delimiter_>
inline
Function_
Join(const Collection_& cCollection_, Function_ function_, Delimiter_ delimiter_)
{
	return Join(cCollection_.begin(),
				cCollection_.end(),
				function_,
				delimiter_);
}

// TEMPLATE FUNCTION public
//	Opt::Join -- 
//
// TEMPLATE ARGUMENTS
//	class Collection_
//	class Function_
//	class Delimiter_
//	
// NOTES
//
// ARGUMENTS
//	Collection_& cCollection_
//	Function_ function_
//	Delimiter_ delimiter_
//	
// RETURN
//	Function_
//
// EXCEPTIONS

template <class Collection_, class Function_, class Delimiter_>
inline
Function_
Join(Collection_& cCollection_, Function_ function_, Delimiter_ delimiter_)
{
	return Join(cCollection_.begin(),
				cCollection_.end(),
				function_,
				delimiter_);
}

//////////////////////
// Iterators
//////////////////////

// TEMPLATE FUNCTION public
//	Opt::Swap -- swap two iterators value
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cIterator0_
//	Iterator_ cIterator1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class Iterator_>
inline
void
Swap(Iterator_ cIterator0_, Iterator_ cIterator1_)
{
	typename Iterator_::ValueType cTmp = *cIterator0_;
	*cIterator0_ = *cIterator1_;
	*cIterator1_ = cTmp;
}
#else
template <class Iterator_, class ValueType_>
inline
void
Swap(Iterator_ cIterator0_, Iterator_ cIterator1_, ValueType_* pDummy_)
{
	ValueType_ cTmp = *cIterator0_;
	*cIterator0_ = *cIterator1_;
	*cIterator1_ = cTmp;
}
template <class Iterator_>
inline
void
Swap(Iterator_ cIterator0_, Iterator_ cIterator1_)
{
	Swap(cIterator0_, cIterator1_, ModValueType(cIterator0_));
}
#endif

/////////////////////////////////////////
// Other macros
/////////////////////////////////////////

#ifndef AUTOPOINTER
#define AUTOPOINTER ModAutoPointer
#define BINARYSEARCH ModBinarySearch
#define CONSTITERATOR ConstIterator
#define FOREACH Opt::ForEach
#define FOREACH_i Opt::ForEach_i
#define FOREACH_r Opt::ForEach_r
#define FOREACH_if Opt::ForEach_if
#define GETBACK getBack
#define POPBACK popBack
#define POPFRONT popFront
#define GETSIZE getSize
#define HASHMAP ModHashMap
#define HASHER ModHasher
#define ISEMPTY isEmpty
#define ITERATOR Iterator
#define LESS ModLess
#define LOWERBOUND ModLowerBound
#define MAKEPAIR ModMakePair
#define MAP Common::VectorMap
#define MAX ModMax
#define MIN ModMin
#define OSTREAM ModOstream
#define OSTRSTREAM ModUnicodeOstrStream
#define PAIR ModPair
#define PUSHBACK pushBack
#define PUSHFRONT pushFront
#define SHORTVECTOR ModVector
#define SIZE ModSize
#define STRING ModUnicodeString
#define SWAP ModSwap
#ifdef SYD_USE_LARGE_VECTOR
#define VECTOR Common::LargeVector
#else
#define VECTOR ModVector
#endif
#endif

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_ALGORITHM_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
