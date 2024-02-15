// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Algorithm.h -- 
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ARRAY_ALGORITHM_H
#define __SYDNEY_ARRAY_ALGORITHM_H

#include "Array/Module.h"

_SYDNEY_BEGIN
_SYDNEY_ARRAY_BEGIN

//
//	NAMESPACE
//	Array::Algorithm
//
//	NOTES
//	This is different from ModAlgorithm.
//	The ModAlgorithm expects for the class of Compare to return true/false.
//	But this expects to return -1/0/1.
//
//	The precondition is that the data is arranged in asecnding order.
//
namespace Algorithm
{
	//
	//	TEMPLATE FUNCTION
	//	Array::Algorithm::lowerBound -- Get min(x | x >= k).
	//
	//	NOTES
	//	This algorithm is equals to
	//	the lower_bound of STL, ModAlgorithm and other than this modules.
	//	But this is DIFFERENT from the lower bound of mathematics.
	//
	//	Mathematically,
	//	when 'k' is treated as the subset which consists of only itself,
	//	maximum lower bound, infimum, means max(x | x <= k),
	//	and minimum upper bound, supremum, means min(x | x >= k).
	//	
	//	The above relation is in the folow.
	//
	//	LB: the abbreviation for 'lowerBound'
	//		The meaning in STL is different from the meaning of mathematics.
	//		So, LB is used in the folow example for avoiding the confusion.
	//	UB: 'upperBound', it is used for the above reason.
	//	X': It means an inversion of X.
	//	
	//	      ..., k-1, k, k, k, k+1, ...
	//	LB' --------->  <---------------- LB
	//	UB' ------------------>  <------- UB
	//	infimum -------------->
	//					<---------------- supremum
	//
	//	      ..., k-1, k, k+1, ...
	//	LB' --------->  <---------- LB
	//	UB' ------------>  <------- UB
	//	infimum -------->
	//					<------- supremum
	//
	//	      ..., k-1, k+1, ...
	//	LB' --------->  <------- LB
	//	UB' --------->  <------- UB
	//	infimum ----->
	//					<------- supremum
	//
	//	LB	supremum	min(x | x >= k)
	//	LB'	--			max(x | x < k)
	//	UB	--			min(x | x > k)
	//	UB'	infimum		max(x | x <= k)
	//
	//	ARGUMENTS
	//	Iterator last_
	//		The end of x is a simbole of the end of the list.
	//		So, this is NOT compared with value_.
	//
	//	RETURN
	//	Iterator
	//		When there are some 'k', return the top of 'k'.
	//		When all x are smaller than 'k',
	//		return the end of x, NOT return any exceptions.
	//
	template <class Iterator, class Compare, class ValueType>
	Iterator lowerBound(Iterator first_, Iterator last_,
						ValueType& value_, const Compare& compare_)
	{
		int length = last_ - first_;
		while (length > 0)
		{
			int halfIndex = length / 2;
			Iterator middle(first_ + halfIndex);

			if (compare_(*middle, value_) < 0)
			{
				first_ = middle;
				++first_;
				length = length - halfIndex - 1;
			}
			else
			{
				length = halfIndex;
			}
		}
		return first_;
	}
	
	//
	//	TEMPLATE FUNCTION
	//	Array::Algorithm::upperBound -- Get min(x | x > k).
	//
	//	NOTES
	//	See lowerBound.
	//
	//	ARGUMENTS
	//	Iterator last_
	//		The end of x is a simbole of the end of the list.
	//		So, this is NOT compared with value_.
	//
	//	RETURN
	//	Iterator
	//		When all x are smaller than 'k',
	//		return the end of x, NOT return any exceptions.
	//
	template <class Iterator, class Compare, class ValueType>
	Iterator upperBound(Iterator first_, Iterator last_,
					  ValueType& value_, const Compare& compare_)
	{
		int length = last_ - first_;
		while (length > 0)
		{
			int halfIndex = length / 2;
			Iterator middle(first_ + halfIndex);

			if (compare_(value_, *middle) < 0)
			{
				length = halfIndex;
			}
			else
			{
				first_ = middle;
				++first_;
				length = length - halfIndex - 1;
			}
		}
		return first_;
	}
}

_SYDNEY_ARRAY_END
_SYDNEY_END

#endif //__SYDNEY_ARRAY_ALGORITHM_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
