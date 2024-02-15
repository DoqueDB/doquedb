// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedWindowLocationListIterator.h -- Window用順序付き文書内出現位置リストの反復子
// 
// Copyright (c) 1997, 1999, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedOrderedSimpleLocationListIterator_H__
#define __ModInvertedOrderedSimpleLocationListIterator_H__

#include "ModInvertedOrderedOperatorWindowLocationListIterator.h"

class
ModInvertedOrderedSimpleWindowLocationListIterator:
public ModInvertedOrderedOperatorWindowLocationListIterator{
public:

	ModInvertedOrderedSimpleWindowLocationListIterator(
		ModInvertedQueryInternalNode* node)
		: ModInvertedOrderedOperatorWindowLocationListIterator(node) {}

	void initialize(
		ModSize minimalDistance_,
		ModSize maximalDistance_)
	{
		ModInvertedOrderedOperatorWindowLocationListIterator::initialize(
			minimalDistance_ + 1,
			maximalDistance_ + 1,
			ModTrue);
	}
	
};

#endif	__ModInvertedOrderedSimpleLocationListIterator_H__

//
// Copyright (c) 1997, 1999, 2001, 2002, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
