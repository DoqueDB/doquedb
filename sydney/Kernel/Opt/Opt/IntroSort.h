// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/IntroSort.h --
// 
// Copyright (c) 2008, 2009, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_INTROSORT_H
#define __SYDNEY_OPT_INTROSORT_H

#include "Opt/Module.h"
#include "Opt/HeapSort.h"
#include "Opt/InsertionSort.h"

#include "Common/Object.h"

#include "Os/Math.h"

#ifndef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
#include "ModAlgorithm.h"
#endif

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
template <class Iterator_, class Comparator_>
class IntroSort
	: public Common::Object
{
public:
	// constructor
	IntroSort(const Iterator_& cFirst_,
			  const Iterator_& cLast_,
			  Comparator_ cComparator_)
		: m_cFirst(cFirst_),
		  m_cLast(cLast_),
		  m_cComparator(cComparator_)
	{}

	// destructor
	~IntroSort() {}

	// sort container by introsort
	void sort();
	void sort(Iterator_ cLimit_);

protected:
private:
	// size threshold to apply heapsort
	enum
	{
		SizeThreshold = 16
	};

	// main loop
	void loop(Iterator_ cFirst_,
			  Iterator_ cLast_,
			  int iDepthLimit_);
	// main loop with limit option
	Iterator_ loop(Iterator_ cFirst_,
				   Iterator_ cLast_,
				   int iDepthLimit_,
				   Iterator_ cLimit_);
	// partition for quick sort
	Iterator_ partition(Iterator_ cFirst_,
						Iterator_ cLast_,
						Iterator_ cMiddle_);
	// get median
	Iterator_ median(Iterator_ cFirst_,
					 Iterator_ cMiddle_,
					 Iterator_ cLast_);
	// heap threshold
	int heapThreshold();

	const Iterator_& m_cFirst;
	const Iterator_& m_cLast;
	Comparator_ m_cComparator;
};

// TEMPLATE FUNCTION public
//	Opt::IntroSort<Iterator_, Comparator_>::sort -- sort container by introsort
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
void
IntroSort<Iterator_, Comparator_>::
sort()
{
	if (m_cFirst != m_cLast) {
		loop(m_cFirst, m_cLast, heapThreshold());
	}
}

template <class Iterator_, class Comparator_>
inline
void
IntroSort<Iterator_, Comparator_>::
sort(Iterator_ cLimit_)
{
	if (m_cFirst != m_cLast) {
		if (cLimit_ < m_cLast) {
			Iterator_ cLast =
				loop(m_cFirst, m_cLast, heapThreshold(), cLimit_);
		} else {
			sort();
		}
	}
}

// TEMPLATE FUNCTION private
//	Opt::IntroSort<Iterator_, Comparator_>::loop -- main loop
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	int iDepthLimit_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
void
IntroSort<Iterator_, Comparator_>::
loop(Iterator_ cFirst_,
	 Iterator_ cLast_,
	 int iDepthLimit_)
{
	while (cLast_ - cFirst_ > SizeThreshold) {
		if (iDepthLimit_ == 0) {
			Opt::heapSort(cFirst_, cLast_, m_cComparator);
			return;
		}
		--iDepthLimit_;
		int n = cLast_ - cFirst_;
		Iterator_ p = partition(cFirst_,
								cLast_,
								median(cFirst_ + n/4,
									   cFirst_ + n/2,
									   cFirst_ + n/2 + n/4));
		loop(p, cLast_, iDepthLimit_);
		cLast_ = p;
	}
	Opt::insertionSort(cFirst_, cLast_, m_cComparator);
}

// TEMPLATE FUNCTION private
//	Opt::IntroSort<Iterator_, Comparator_>::loop -- main loop with limit option
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	int iDepthLimit_
//	Iterator_ cLimit_
//	
// RETURN
//	Iterator_
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
Iterator_
IntroSort<Iterator_, Comparator_>::
loop(Iterator_ cFirst_,
	 Iterator_ cLast_,
	 int iDepthLimit_,
	 Iterator_ cLimit_)
{
	Iterator_ cResult = cLast_;
	while (cLast_ - cFirst_ > SizeThreshold) {
		if (iDepthLimit_ == 0) {
			Opt::heapSort(cFirst_, cLast_, m_cComparator);
			return cLast_;
		}
		--iDepthLimit_;
		int n = cLast_ - cFirst_;
		Iterator_ p = partition(cFirst_,
								cLast_,
								median(cFirst_ + n/4,
									   cFirst_ + n/2,
									   cFirst_ + n/2 + n/4));
		if (p < cLimit_) {
			cResult = loop(p, cLast_, iDepthLimit_, cLimit_);
		} else {
			cResult = p;
		}
		cLast_ = p;
	}
	Opt::insertionSort(cFirst_, cResult, m_cComparator);
	return cResult;
}

// TEMPLATE FUNCTION private
//	Opt::IntroSort<Iterator_, Comparator_>::partition -- partition for quick sort
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Iterator_ cMiddle_
//	
// RETURN
//	Iterator_
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
Iterator_
IntroSort<Iterator_, Comparator_>::
partition(Iterator_ cFirst_,
		  Iterator_ cLast_,
		  Iterator_ cMiddle_)
{
	typename Iterator_::ValueType cPivot = *cMiddle_;
	while (true) {
		while (m_cComparator(*cFirst_, cPivot)) ++cFirst_;
		--cLast_;
		while (m_cComparator(cPivot, *cLast_)) --cLast_;
		if (cFirst_ < cLast_) {
			Opt::Swap(cFirst_, cLast_);
			++cFirst_;
		} else {
			break;
		}
	}
	return cFirst_;
}

// TEMPLATE FUNCTION private
//	Opt::IntroSort<Iterator_, Comparator_>::median -- get median
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cMiddle_
//	Iterator_ cLast_
//	
// RETURN
//	Iterator_
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
Iterator_
IntroSort<Iterator_, Comparator_>::
median(Iterator_ cFirst_,
	   Iterator_ cMiddle_,
	   Iterator_ cLast_)
{
	if (m_cComparator(*cMiddle_, *cFirst_)) {
		// mid < first
		if (m_cComparator(*cLast_, *cMiddle_)) {
			// last < mid < first
			return cMiddle_;
		} else if (m_cComparator(*cLast_, *cFirst_)) {
			// mid <= last < first
			return cLast_;
		} else {
			// mid < first <= last
			return cFirst_;
		}
	} else if (m_cComparator(*cLast_, *cMiddle_)) {
		// last < mid
		if (m_cComparator(*cLast_, *cFirst_)) {
			// last < first <= mid
			return cFirst_;
		} else {
			// first <= last < mid
			return cLast_;
		}
	} else {
		// first <= mid <= last
		return cMiddle_;
	}
}

// TEMPLATE FUNCTION private
//	Opt::IntroSort<Iterator_, Comparator_>::heapThreshold -- heap threshold
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
int
IntroSort<Iterator_, Comparator_>::
heapThreshold()
{
	return static_cast<int>(::floor(Os::Math::log(m_cLast - m_cFirst) / Os::Math::log(2)));
}

// TEMPLATE FUNCTION public
//	Opt::IntroSort -- introsort
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Comparator_ cComparator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
void
introSort(Iterator_ cFirst_, Iterator_ cLast_, Comparator_ cComparator_)
{
	Opt::IntroSort<Iterator_, Comparator_>(cFirst_, cLast_, cComparator_).sort();
}

template <class Iterator_, class Comparator_>
inline
void
introSort(Iterator_ cFirst_, Iterator_ cLimit_, Iterator_ cLast_, Comparator_ cComparator_)
{
	Opt::IntroSort<Iterator_, Comparator_>(cFirst_, cLast_, cComparator_).sort(cLimit_);
}

#else /* if !defined(MOD_CONF_ITERATOR_VALUETYPE_DEFINITION) */

// TEMPLATE FUNCTION public
//	Opt::IntroSort -- introsort
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	Iterator_ cFirst_
//	Iterator_ cLast_
//	Comparator_ cComparator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
void
introSort(Iterator_ cFirst_, Iterator_ cLast_, Comparator_ cComparator_)
{
	ModSort(cFirst_, cLast_, cComparator_);
}

template <class Iterator_, class Comparator_>
inline
void
introSort(Iterator_ cFirst_, Iterator_ cLimit_, Iterator_ cLast_, Comparator_ cComparator_)
{
	ModPartialSort(cFirst_, cLimit_, cLast_, cComparator_);
}
#endif

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_INTROSORT_H

//
//	Copyright (c) 2008, 2009, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
