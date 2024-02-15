// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/ObjectSet.h --
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_UTILITY_OBJECTSET_H
#define __SYDNEY_PLAN_UTILITY_OBJECTSET_H

#include "Plan/Utility/Module.h"
#include "Plan/Utility/Algorithm.h"

#include "Plan/Declaration.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"
#include "Opt/Sort.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Column;
	class File;
	class Table;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_UTILITY_BEGIN

//////////////////////////////////////////////////////////////////////
// TEMPLATE CLASS
//	ObjectSet -- a set of object without duplication of same instances
//
// TEMPLATE ARGUMENT
//	class Object_
//	class Less_
//
// NOTES
template <class Object_, class Less_>
class ObjectSet
	: private VECTOR<Object_>
{
public:
	typedef VECTOR<Object_> Super;
	typedef ObjectSet<Object_, Less_> This;
	typedef Less_ Comparator;

	typedef typename Super::ITERATOR Iterator;
	typedef typename Super::CONSTITERATOR ConstIterator;

	using Super::isEmpty;
	using Super::getSize;
	using Super::reserve;
	using Super::erase;
	using Super::clear;
	using Super::begin;
	using Super::end;
	using Super::getFront;
	using Super::getBack;

	// constructor
	ObjectSet();
	ObjectSet(const This& cOther_);
	explicit ObjectSet(const Super& cVector_);
	ObjectSet(ConstIterator first_, ConstIterator last_);

	// destructor
	~ObjectSet();

	// operator
	This& operator=(const This& cOther_)
	{
		if (this != &cOther_) {
			Super::operator=(cOther_);
		}
		return *this;
	}
	bool operator==(const This& cOther_) const;
	bool operator!=(const This& cOther_) const;
	bool operator<(const This& cOther_) const;

	// add object
	This& addObject(const Object_& cObject_) {return add(cObject_);}
	This& add(const Object_& cObject_);
	This& add(const Super& cVector_);
	This& add(ConstIterator first_, ConstIterator last_);

	// remove object
	This& remove(const Object_& cObject_);
	This& remove(const Super& cVector_);
	This& removeAll();

	// check existense
	bool isContainingObject(const Object_& cObject_) const {return isContaining(cObject_);}
	bool isContaining(const Object_& cObject_) const;
	// check including
	bool isContaining(const This& cOther_) const;
	// check overlapping
	bool isContainingAny(const Super& cOther_) const;
	bool isContainingAny(const This& cOther_) const;

	// set operations
	void intersect(const This& cOther_, This& cResult_) const;
	void intersect(const This& cOther_);

	void merge(const This& cOther_, This& cResult_) const;
	void merge(const This& cOther_);
	void setMerged(const This& cOther1_, const This& cOther2_);

	void difference(const This& cOther_, This& cResult_) const;
	void difference(const This& cOther_);

	// divide a set into intersect and difference
	void divide(const This& cOther_, This& cIntersect_, This& cDifference_) const;

	// get as a vector
	Super& getVector() {return *this;}
	const Super& getVector() const {return *this;}

	// TEMPLATE FUNCTION
	//	Plan::Utility::ObjectSet::find -- search for an object
	//
	// TEMPLATE ARGUMENTS
	//	class ValueType_
	//
	// NOTES
	//	ValueType_ have to be comparable with Object_ by Less_
	//
	// ARGUMENTS
	//	const ValueType_& cValue_
	//
	// RETURN
	//	ConstIterator or Iterator
	//
	// EXCEPTIONS
	template <class ValueType_>
	ConstIterator find(const ValueType_& cValue_) const
	{
		ConstIterator iterator =
			LOWERBOUND(begin(), end(), cValue_, Less_());
		if (iterator != end() && !Less_()(cValue_, *iterator))
			return iterator;
		return end();
	}
	template <class ValueType_>
	Iterator find(const ValueType_& cValue_)
	{
		Iterator iterator =
			LOWERBOUND(begin(), end(), cValue_, Less_());
		if (iterator != end() && !Less_()(cValue_, *iterator))
			return iterator;
		return end();
	}

	// TEMPLATE FUNCTION
	//	Plan::Utility::ObjectSet::foreachElement -- apply function for each element
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//
	// RETURN
	//	Function_
	//
	// EXCEPTIONS
	template <class Function_>
	Function_
	foreachElement(Function_ function_) const
	{
		if (getSize() == 1) {
			function_(*begin());
			return function_;
		}
		return FOREACH(begin(), end(), function_);
	}

	// TEMPLATE FUNCTION
	//	Plan::Utility::ObjectSet::foreachElement_i -- apply function for each element(with counter)
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//
	// RETURN
	//	Function_
	//
	// EXCEPTIONS
	template <class Function_>
	Function_
	foreachElement_i(Function_ function_) const
	{
		if (getSize() == 1) {
			function_(*begin(), 0);
			return function_;
		}
		return FOREACH_i(begin(), end(), function_);
	}

	// TEMPLATE FUNCTION
	//	Plan::Utility::ObjectSet::isAll -- check isAll
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	template <class Function_>
	bool
	isAll(Function_ function_) const
	{
		if (getSize() == 1) {
			return function_(*begin());
		}
		return Opt::IsAll(begin(), end(), function_);
	}

	// TEMPLATE FUNCTION
	//	Plan::Utility::ObjectSet::isAny -- check isAny
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//
	// RETURN
	//	bool
	//
	// EXCEPTIONS
	template <class Function_>
	bool
	isAny(Function_ function_) const
	{
		if (getSize() == 1) {
			return function_(*begin());
		}
		return Opt::IsAny(begin(), end(), function_);
	}

	// TEMPLATE FUNCTION
	//	Plan::Utility::ObjectSet::findElement -- find by function
	//
	// TEMPLATE ARGUMENTS
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Function_ function_
	//
	// RETURN
	//	ConstIterator
	//
	// EXCEPTIONS
	template <class Function_>
	ConstIterator
	findElement(Function_ function_) const
	{
		return Opt::Find(begin(), end(), function_);
	}

	// TEMPLATE FUNCTION
	//	Plan::Utility::ObjectSet::mapElement -- map by function
	//
	// TEMPLATE ARGUMENTS
	//	class Container_
	//	class Function_
	//
	// NOTES
	//
	// ARGUMENTS
	//	Container& cContainer_
	//	Function_ function_
	//
	// RETURN
	//	void
	//
	// EXCEPTIONS
	template <class Container_, class Function_>
	void
	mapElement(Container_& cContainer_,
			   Function_ function_) const
	{
		Opt::MapContainer(begin(), end(), cContainer_, function_);
	}

protected:
private:
	using Super::insert;

	typedef void (This::* Member)(ConstIterator iterator_,
								  This& cResult_) const;
	typedef void (This::* Member2)(ConstIterator first_,
								   ConstIterator last_,
								   This& cResult_) const;
	typedef void (This::* Member3)(Iterator& iterator0_,
								   ConstIterator iterator1_);
	typedef void (This::* Member4)(Iterator first_,
								   Iterator last_);
	typedef void (This::* Member5)(ConstIterator first_,
								   ConstIterator last_);

	// general method to apply some operations to two ObjectSet instances
	void setOperator(const This& cOther_, This& cResult_,
					 Member equal_, Member less_, Member greater_,
					 Member2 rest0_ = 0, Member2 rest1_ = 0) const;
	// general method to apply some operations to two ObjectSet instances (apply to this)
	void setOperator(const This& cOther_,
					 Member3 equal_, Member3 less_, Member3 greater_,
					 Member4 rest0_ = 0, Member5 rest1_ = 0);

	//////////////////////////////////////////////
	// member functions for setOperator(Member)
	// add an object refered by an iterator to an ObjectSet
	void insert(ConstIterator iterator_, This& cResult_) const;

	//////////////////////////////////////////////
	// member functions for setOperator(Member2)
	// add objects refered by a pair of iterators to an ObjectSet
	void insert(ConstIterator first_, ConstIterator last_, This& cResult_) const;

	//////////////////////////////////////////////
	// member functions for setOperator(Member3)
	// increment iterator
	void increment(Iterator& iterator0_, ConstIterator iterator1_);
	// add an object to the place refered by an iterator
	void insert(Iterator& iterator0_, ConstIterator iterator1_);
	// erase an object at the place refered by an iterator
	void erase(Iterator& iterator0_, ConstIterator iterator1_);

	//////////////////////////////////////////////
	// member functions for setOperator(Member4)
	// erase objects refered by a pair of iterators
	void erase(Iterator first_, Iterator last_);

	//////////////////////////////////////////////
	// member functions for setOperator(Member5)
	// add objects refered by a pair of iterators
	void insert(ConstIterator first_, ConstIterator last_);
};

///////////////////////////////////////
// implementation of ObjectSet
///////////////////////////////////////

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::ObjectSet -- constructor
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	inline
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>::
ObjectSet()
	: Super()
{}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::ObjectSet -- constructor
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	inline
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>::
ObjectSet(const This& cOther_)
	: Super(cOther_)
{}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::ObjectSet -- constructor
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const Super& cVector_
//	
// RETURN
//	inline
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>::
ObjectSet(const Super& cVector_)
	: Super(cVector_)
{
	// sort elements
	SORT(begin(), end(), Less_());
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::ObjectSet -- constructor
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	ConstIterator first_
//	ConstIterator last_
//	
// RETURN
//	inline
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>::
ObjectSet(ConstIterator first_, ConstIterator last_)
	: Super()
{
	assign(first_, last_);
	// sort elements
	SORT(begin(), end(), Less_());
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::~ObjectSet -- destructor
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	inline
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>::
~ObjectSet()
{
	clear();
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::operator== -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Object_, class Less_>
bool
ObjectSet<Object_, Less_>::
operator==(const This& cOther_) const
{
	if (this == &cOther_) return true;
	if (getSize() == cOther_.getSize()) {
		ConstIterator iterator0 = begin();
		ConstIterator last0 = end();
		ConstIterator iterator1 = cOther_.begin();
		ConstIterator last1 = cOther_.end();
		for (; iterator0 != last0 && iterator1 != last1; ++iterator0, ++iterator1)
			if (Less_()(*iterator0, *iterator1)
				|| Less_()(*iterator1, *iterator0)) return false;
		return true;
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::operator!= -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
bool
ObjectSet<Object_, Less_>::
operator!=(const This& cOther_) const
{
	return !(*this == cOther_);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::operator< -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Object_, class Less_>
bool
ObjectSet<Object_, Less_>::
operator<(const This& cOther_) const
{
	if (this != &cOther_) {
		ConstIterator iterator0 = begin();
		ConstIterator last0 = end();
		ConstIterator iterator1 = cOther_.begin();
		ConstIterator last1 = cOther_.end();
		for (; iterator0 != last0 && iterator1 != last1;) {
			if (Less_()(*iterator0, *iterator1))
				return true;
			else if (Less_()(*iterator1, *iterator0)) {
				return false;
			} else {
				++iterator0;
				++iterator1;
			}
		}
		// if any elements of other is left, this < other
		return (iterator1 != last1);
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::add -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const Object_& cObject_
//	
// RETURN
//	ObjectSet<Object_, Less_>&
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>&
ObjectSet<Object_, Less_>::
add(const Object_& cObject_)
{
	// keep sorted
	Iterator iterator =
		LOWERBOUND(begin(), end(), cObject_, Less_());
	if (iterator == end() || Less_()(cObject_, *iterator))
		insert(iterator, cObject_);

	return *this;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::add -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const Super& cVector_
//	
// RETURN
//	ObjectSet<Object_, Less_>&
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>&
ObjectSet<Object_, Less_>::
add(const Super& cVector_)
{
	if (int n = cVector_.getSize()) {
		reserve(getSize() + n);
		ConstIterator iterator = cVector_.begin();
		do {
			add(*iterator);
			++iterator;

		} while (--n > 0);
	}
	return *this;
}

// FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::add -- 
//
// NOTES
//
// ARGUMENTS
//	ConstIterator first_
//	ConstIterator last_
//	
// RETURN
//	ObjectSet<Object_, Less_>&
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>&
ObjectSet<Object_, Less_>::
add(ConstIterator first_, ConstIterator last_)
{
	for (; first_ != last_; ++first_) {
		add(*first_);
	}
	return *this;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::remove -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const Object_& cObject_
//	
// RETURN
//	ObjectSet<Object_, Less_>&
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>&
ObjectSet<Object_, Less_>::
remove(const Object_& cObject_)
{
	Iterator iterator =
		LOWERBOUND(begin(), end(), cObject_, Less_());
	if (iterator != end() && !Less_()(cObject_, *iterator))
		erase(iterator);

	return *this;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::remove -- remove elements in a vector
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const Super& cVector_
//	
// RETURN
//	ObjectSet<Object_, Less_>&
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>&
ObjectSet<Object_, Less_>::
remove(const Super& cVector_)
{
	if (SIZE n = cVector_.getSize()) {
		reserve(getSize() + n);
		ConstIterator iterator = cVector_.begin();
		do {
			remove(*iterator);
			++iterator;

		} while (--n > 0);
	}
	return *this;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::removeAll -- remove all the elements
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ObjectSet<Object_, Less_>&
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
ObjectSet<Object_, Less_>&
ObjectSet<Object_, Less_>::
removeAll()
{
	Super::erase(begin(), end());
	return *this;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::isContaining -- check existense
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const Object_& cObject_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
bool
ObjectSet<Object_, Less_>::
isContaining(const Object_& cObject_) const
{
	return BINARYSEARCH(begin(), end(), cObject_, Less_());
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::isContaining -- check inclusion
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Object_, class Less_>
bool
ObjectSet<Object_, Less_>::
isContaining(const This& cOther_) const
{
	if (this == &cOther_) return true;

	// If this is smaller than cOther_, it can't include
	SIZE iThisSize = getSize();
	SIZE iOtherSize = cOther_.getSize();
	if (iOtherSize > iThisSize)
		return false;

	if (iThisSize && iOtherSize) {
		ConstIterator iterator0 = begin();
		ConstIterator iterator1 = cOther_.begin();
		do {
			if (Less_()(*iterator0, *iterator1)) {
				++iterator0;
				--iThisSize;

			} else if (Less_()(*iterator1, *iterator0)) {
				// there is one element that is smaller than this -> not including
				break;

			} else {
				++iterator0;
				++iterator1;
				--iThisSize;
				--iOtherSize;
			}
		} while (iOtherSize && iOtherSize <= iThisSize);
	}
	return (iOtherSize == 0);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::isContainingAny -- check overlapping
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const Super& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Object_, class Less_>
bool
ObjectSet<Object_, Less_>::
isContainingAny(const Super& cOther_) const
{
	if (this == &cOther_) return true;

	ConstIterator iterator = cOther_.begin();
	ConstIterator last = cOther_.end();
	for (; iterator != last; ++iterator) {
		if (isContaining(*iterator)) return true;
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::isContainingAny -- check overlapping
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

template <class Object_, class Less_>
bool
ObjectSet<Object_, Less_>::
isContainingAny(const This& cOther_) const
{
	if (this == &cOther_) return true;

	ConstIterator iterator0 = begin();
	ConstIterator last0 = end();
	ConstIterator iterator1 = cOther_.begin();
	ConstIterator last1 = cOther_.end();
	for (; iterator0 != last0 && iterator1 != last1;) {
		if (Less_()(*iterator0, *iterator1))
			++iterator0;
		else if (Less_()(*iterator1, *iterator0))
			++iterator1;
		else
			// found
			return true;
	}
	return false;
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::intersect -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	This& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
intersect(const This& cOther_, This& cResult_) const
{
	// put to result only when both elements are same
	cResult_.reserve(MIN(getSize(), cOther_.getSize()));
	setOperator(cOther_, cResult_, &This::insert, 0, 0);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::intersect -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
intersect(const This& cOther_)
{
	// erase if this is less than other
	// proceed iterator if this is greater than other
	// if this is left, erase rest elements
	setOperator(cOther_, &This::increment, &This::erase, 0,
				&This::erase, 0);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::merge -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	This& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
merge(const This& cOther_, This& cResult_) const
{
	// put smaller elements to result
	cResult_.reserve(getSize() + cOther_.getSize());
	setOperator(cOther_, cResult_, &This::insert, &This::insert, &This::insert, &This::insert, &This::insert);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::merge -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
merge(const This& cOther_)
{
	// insert other when this is larger than other
	// proceed iterator when this is not larger than other
	// add all the left elements in other
	setOperator(cOther_, &This::increment, &This::increment, &This::insert,
				0, &This::insert);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::setMerged -- set merged result
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther1_
//	const This& cOther2_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
setMerged(const This& cOther1_, const This& cOther2_)
{
	cOther1_.merge(cOther2_, *this);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::difference -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	This& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
difference(const This& cOther_, This& cResult_) const
{
	cResult_.reserve(getSize());
	setOperator(cOther_, cResult_, 0, &This::insert, 0, &This::insert, 0);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::difference -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
difference(const This& cOther_)
{
	// erase if this == other
	// proceed iterator if this is smaller than other
	// do nothing if this is greater than other
	setOperator(cOther_, &This::erase, &This::increment, 0);
}

// TEMPLATE FUNCTION public
//	Utility::ObjectSet<Object_, Less_>::divide -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	This& cIntersect_
//	This& cDifference_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
void
ObjectSet<Object_, Less_>::
divide(const This& cOther_, This& cIntersect_, This& cDifference_) const
{
	cIntersect_.clear();
	cDifference_.clear();
	cIntersect_.reserve(MIN(getSize(), cOther_.getSize()));
	cDifference_.reserve(getSize());

	ConstIterator iterator0 = begin();
	ConstIterator last0 = end();
	ConstIterator iterator1 = cOther_.begin();
	ConstIterator last1 = cOther_.end();
	for (; iterator0 != last0 && iterator1 != last1;) {
		if (Less_()(*iterator0, *iterator1)) {
			insert(iterator0, cDifference_);
			++iterator0;

		} else if (Less_()(*iterator1, *iterator0)) {
			++iterator1;

		} else {
			insert(iterator0, cIntersect_);
			++iterator0;
			++iterator1;
		}
	}
	// all the left elements in this is added to difference
	insert(iterator0, last0, cDifference_);
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::setOperator -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	This& cResult_
//	Member equal_
//	Member less_
//	Member greater_
//	Member2 rest0_
//	Member2 rest1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
void
ObjectSet<Object_, Less_>::
setOperator(const This& cOther_, This& cResult_,
			Member equal_, Member less_, Member greater_,
			Member2 rest0_, Member2 rest1_) const
{
	cResult_.clear();
	ConstIterator iterator0 = begin();
	ConstIterator last0 = end();
	ConstIterator iterator1 = cOther_.begin();
	ConstIterator last1 = cOther_.end();
	for (; iterator0 != last0 && iterator1 != last1;) {

		if (Less_()(*iterator0, *iterator1)) {
			// this < other
			if (less_) (this->*less_)(iterator0, cResult_);
			++iterator0;
		} else if (Less_()(*iterator1, *iterator0)) {
			// this > other
			if (greater_) (this->*greater_)(iterator1, cResult_);
			++iterator1;
		} else {
			// this == other
			if (equal_) (this->*equal_)(iterator0, cResult_);
			++iterator0;
			++iterator1;
		}
	}
	// apply to all the left elements in this
	if (rest0_) (this->*rest0_)(iterator0, last0, cResult_);
	// apply to all the left elements in other
	if (rest1_) (this->*rest1_)(iterator1, last1, cResult_);
}

// FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::setOperator -- 
//
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	Member3 equal_
//	Member3 less_
//	Member3 greater_
//	Member4 rest0_
//	Member5 rest1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
void
ObjectSet<Object_, Less_>::
setOperator(const This& cOther_,
			Member3 equal_, Member3 less_, Member3 greater_,
			Member4 rest0_, Member5 rest1_)
{
	Iterator iterator0 = begin();
	ConstIterator iterator1 = cOther_.begin();
	ConstIterator last1 = cOther_.end();
	for (; iterator0 != end() && iterator1 != last1;) {

		if (Less_()(*iterator0, *iterator1)) {
			// this < other
			if (less_) (this->*less_)(iterator0, iterator1);

		} else if (Less_()(*iterator1, *iterator0)) {
			// this > other
			if (greater_) (this->*greater_)(iterator0, iterator1);
			++iterator1;

		} else {
			// this == other
			if (equal_) (this->*equal_)(iterator0, iterator1);
			++iterator1;
		}
	}
	// apply to all the left elements in this
	if (rest0_) (this->*rest0_)(iterator0, end());
	// apply to all the left elements in other
	if (rest1_) (this->*rest1_)(iterator1, last1);
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::insert -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	ConstIterator iterator_
//	This& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
insert(ConstIterator iterator_, This& cResult_) const
{
	cResult_.pushBack(*iterator_);
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::insert -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	ConstIterator first_
//	ConstIterator last_
//	This& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
insert(ConstIterator first_, ConstIterator last_, This& cResult_) const
{
	if (first_ != last_)
		cResult_.insert(cResult_.end(), first_, last_);
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::increment -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	Iterator& iterator0_
//	ConstIterator iterator1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
increment(Iterator& iterator0_, ConstIterator iterator1_)
{
	++iterator0_;
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::insert -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	Iterator& iterator0_
//	ConstIterator iterator1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
insert(Iterator& iterator0_, ConstIterator iterator1_)
{
	// put the object at iterator and proceed the iterator
	iterator0_ = Super::insert(iterator0_, *iterator1_);
	++iterator0_;
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::erase -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	Iterator& iterator0_
//	ConstIterator iterator1_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
erase(Iterator& iterator0_, ConstIterator iterator1_)
{
	iterator0_ = erase(iterator0_);
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::insert -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	ConstIterator first_
//	ConstIterator last_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
insert(ConstIterator first_, ConstIterator last_)
{
	if (first_ != last_)
		insert(end(), first_, last_);
}

// TEMPLATE FUNCTION private
//	Utility::ObjectSet<Object_, Less_>::erase -- 
//
// TEMPLATE ARGUMENTS
//	class Object_
//	class Less_
//	
// NOTES
//
// ARGUMENTS
//	Iterator first_
//	Iterator last_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Object_, class Less_>
inline
void
ObjectSet<Object_, Less_>::
erase(Iterator first_, Iterator last_)
{
	Super::erase(first_, last_);
}

///////////////////////////////////////
// various ObjectSets
///////////////////////////////////////

typedef ObjectSet< Interface::IRelation*, ReferingLess<Interface::IRelation> > RelationSet;
typedef ObjectSet< Scalar::Field*, ReferingLess<Scalar::Field> > FieldSet;
typedef ObjectSet< Interface::IFile*, ReferingLess<Interface::IFile> > FileSet;
typedef ObjectSet< Relation::RowElement*, ReferingLess<Relation::RowElement> > RowElementSet;
typedef ObjectSet< Interface::IScalar*, ReferingLess<Interface::IScalar> > ScalarSet;
typedef ObjectSet< Interface::IPredicate*, Utility::ReferingLess<Interface::IPredicate> > PredicateSet;
typedef ObjectSet< Schema::File*, SchemaLess<Schema::File> > SchemaFileSet;
typedef ObjectSet< Schema::Table*, SchemaLess<Schema::Table> > SchemaTableSet;
typedef ObjectSet< Schema::Column*, SchemaLess<Schema::Column> > SchemaColumnSet;
typedef ObjectSet< int, LESS<int> > IntSet;
typedef ObjectSet< unsigned int, LESS<unsigned int> > UIntSet;

_SYDNEY_PLAN_UTILITY_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_UTILITY_OBJECTSET_H

//
//	Copyright (c) 2003, 2004, 2005, 2006, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
