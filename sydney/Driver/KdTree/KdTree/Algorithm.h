// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Algorithm.h -- 
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_KDTREE_ALGORITHM_H
#define __SYDNEY_KDTREE_ALGORITHM_H

#include "KdTree/Module.h"

_SYDNEY_BEGIN
_SYDNEY_KDTREE_BEGIN

//
//	NAMEKDTREE
//	KdTree::Algorithm
//
//	NOTES
//	基本的にはModと同じだが、less ではなく compare
//
namespace Algorithm
{
	//
	//	TEMPLATE FUNCTION
	//	KdTree::Algorithm::lowerBound
	//
	//	NOTES
	//
	template <class Iterator, class Compare, class ValueType>
	Iterator lowerBound(Iterator first_, Iterator last_,
						ValueType& value_, const Compare& compare_)
	{
		int length = static_cast<int>(last_ - first_);
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
	//	KdTree::Algorithm::upperBound
	//
	//	NOTES
	//
	template <class Iterator, class Compare, class ValueType>
	Iterator upperBound(Iterator first_, Iterator last_,
						ValueType& value_, const Compare& compare_)
	{
		int length = static_cast<int>(last_ - first_);
		while (length > 0)
		{
			int halfIndex = length / 2;
			Iterator middle(first_ + halfIndex);

			if (compare_(value_, *middle)  < 0)
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

_SYDNEY_KDTREE_END
_SYDNEY_END

#endif //__SYDNEY_KDTREE_ALGORITHM_H

//
//	Copyright (c) 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
