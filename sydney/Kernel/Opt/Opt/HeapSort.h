// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/HeapSort.h --
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

#ifndef __SYDNEY_OPT_HEAPSORT_H
#define __SYDNEY_OPT_HEAPSORT_H

#include "Opt/Module.h"
#include "Opt/Algorithm.h"
#include "Opt/InsertionSort.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

template <class Iterator_, class Comparator_>
class HeapSort
	: public Common::Object
{
public:
	// constructor
	HeapSort(const Iterator_& cFirst_,
			 const Iterator_& cLast_,
			 Comparator_ cComparator_)
		: m_cFirst(cFirst_),
		  m_cLast(cLast_),
		  m_cComparator(cComparator_)
	{}

	// destructor
	~HeapSort() {}

	// sort container using heap
	void sort();

protected:
private:
	// size threshold to apply insertionsort
	enum
	{
		SizeThreshold = 4
	};

	// pop minimum element
	void pop(int iMax_);
	// adjust heap in upward direction
	void adjustUp(int iPosition_);
	// adjust heap in down direction
	void adjustDown(int iPosition_, int iMax_);

	// get iterator of Container by position
	Iterator_ get(int iPosition_);

	const Iterator_& m_cFirst;
	const Iterator_& m_cLast;
	Comparator_ m_cComparator;
};

// TEMPLATE FUNCTION public
//	Opt::HeapSort<Iterator_, Comparator_>::sort -- sort container using heap
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
HeapSort<Iterator_, Comparator_>::
sort()
{
	int n = m_cLast - m_cFirst;
	if (n > SizeThreshold) {
		// position: 1..n
		for (int i = 2; i <= n; ++i) {
			// adjust heap structure in upward direction
			adjustUp(i);
		}
		for (int i = n; i > 1; --i) {
			// pop minimum element
			pop(i);
		}
	} else {
		Opt::insertionSort(m_cFirst, m_cLast, m_cComparator);
	}
}

// TEMPLATE FUNCTION private
//	Opt::HeapSort<Iterator_, Comparator_>::pop -- pop minimum element
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	int iMax_
//	
// RETURN
//	void
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
void
HeapSort<Iterator_, Comparator_>::
pop(int iMax_)
{
	// swap first element with last element
	Opt::Swap(get(1), get(iMax_));
	// adjust heap structure in downward direction
	adjustDown(1, iMax_ - 1);
}

// TEMPLATE FUNCTION private
//	Opt::HeapSort<Iterator_, Comparator_>::adjustUp -- adjust heap in upward direction
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
void
HeapSort<Iterator_, Comparator_>::
adjustUp(int iPosition_)
{
	if (iPosition_ > 1) {
		int iParent = iPosition_ / 2;
		if (m_cComparator(*get(iPosition_), *get(iParent))) {
			// child is smaller than parent -> swap
			Opt::Swap(get(iPosition_), get(iParent));
			// adjust parent
			adjustUp(iParent);
		}
	}
}

// TEMPLATE FUNCTION private
//	Opt::HeapSort<Iterator_, Comparator_>::adjustDown -- adjust heap in down direction
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	int iMax_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
void
HeapSort<Iterator_, Comparator_>::
adjustDown(int iPosition_, int iMax_)
{
	int iLeft = iPosition_ * 2;
	if (iLeft <= iMax_) {
		int iChild = iLeft;
		if (iLeft < iMax_) {
			int iRight = iLeft + 1;
			if (!m_cComparator(*get(iLeft), *get(iRight))) {
				// if right is smaller than left, use right
				iChild = iRight;
			}
		} else if (iLeft == iMax_) {
			iChild = iLeft;
		}
		if (!m_cComparator(*get(iPosition_), *get(iChild))) {
			// child is smaller than parent -> swap
			Opt::Swap(get(iChild), get(iPosition_));
			// adjust child
			adjustDown(iChild, iMax_);
		}
	}
}

// TEMPLATE FUNCTION private
//	Opt::HeapSort<Iterator_, Comparator_>::get -- get iterator of Container by position
//
// TEMPLATE ARGUMENTS
//	class Iterator_
//	class Comparator_
//	
// NOTES
//
// ARGUMENTS
//	int iPosition_
//	
// RETURN
//	typename Iterator_::Iterator
//
// EXCEPTIONS

template <class Iterator_, class Comparator_>
inline
Iterator_
HeapSort<Iterator_, Comparator_>::
get(int iPosition_)
{
	return m_cLast - iPosition_;
}

// TEMPLATE FUNCTION public
//	Opt::heapSort -- heap sort
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
heapSort(Iterator_ cFirst_, Iterator_ cLast_, Comparator_ cComparator_)
{
	Opt::HeapSort<Iterator_, Comparator_>(cFirst_, cLast_, cComparator_).sort();
}

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_HEAPSORT_H

//
//	Copyright (c) 2008, 2009, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
