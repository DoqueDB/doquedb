// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Algorithm.h -- 
// 
// Copyright (c) 2005, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BITMAP_ALGORITHM_H
#define __SYDNEY_BITMAP_ALGORITHM_H

#include "Bitmap/Module.h"

_SYDNEY_BEGIN
_SYDNEY_BITMAP_BEGIN

//
//	NAMESPACE
//	Bitmap::Algorithm
//
//	NOTES
//	基本的にはModと同じ
//
namespace Algorithm
{
	//
	//	TEMPLATE FUNCTION
	//	Bitmap::Algorithm::lowerBound
	//
	//	NOTES
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
	//	Bitmap::Algorithm::upperBound
	//
	//	NOTES
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

_SYDNEY_BITMAP_END
_SYDNEY_END

#endif //__SYDNEY_BITMAP_ALGORITHM_H

//
//	Copyright (c) 2005, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
