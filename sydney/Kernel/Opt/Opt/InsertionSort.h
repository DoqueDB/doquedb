// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/InsertionSort.h --
// 
// Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_INSERTIONSORT_H
#define __SYDNEY_OPT_INSERTIONSORT_H

#include "Opt/Module.h"

#include "Common/Object.h"

_SYDNEY_BEGIN
_SYDNEY_OPT_BEGIN

template <class Iterator_, class Comparator_>
class InsertionSort
	: public Common::Object
{
public:
	// constructor
	InsertionSort(const Iterator_& cFirst_,
				  const Iterator_& cLast_,
				  Comparator_ cComparator_)
		: m_cFirst(cFirst_),
		  m_cLast(cLast_),
		  m_cComparator(cComparator_)
	{}

	// destructor
	~InsertionSort() {}

	// sort container by insertionsort
#ifdef MOD_CONF_ITERATOR_VALUETYPE_DEFINITION
	void sort()
	{
		if (m_cFirst != m_cLast) {
			for (Iterator_ i = m_cFirst + 1; i != m_cLast; ++i) {
				Iterator_ j = i;
				typename Iterator_::ValueType tmp = *i;
				while (j != m_cFirst && m_cComparator(tmp, *(j-1))) {
					*j = *(j-1);
					--j;
				}
				*j = tmp;
			}
		}
	}
#else
	void sort()
	{
		sort(ModValueType(m_cFirst));
	}
	template <class ValueType_>
	void sort(ValueType_* pDummy_)
	{
		if (m_cFirst != m_cLast) {
			for (Iterator_ i = m_cFirst + 1; i != m_cLast; ++i) {
				Iterator_ j = i;
				ValueType_ tmp = *i;
				while (j != m_cFirst && m_cComparator(tmp, *(j-1))) {
					*j = *(j-1);
					--j;
				}
				*j = tmp;
			}
		}
	}
#endif

protected:
private:
	const Iterator_& m_cFirst;
	const Iterator_& m_cLast;
	Comparator_ m_cComparator;
	int m_iLimit;
};

// TEMPLATE FUNCTION public
//	Opt::InsertionSort -- insertionsort
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
insertionSort(Iterator_ cFirst_, Iterator_ cLast_, Comparator_ cComparator_)
{
	Opt::InsertionSort<Iterator_, Comparator_>(cFirst_, cLast_, cComparator_).sort();
}

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_INSERTIONSORT_H

//
//	Copyright (c) 2008, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
