// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedOrderedOperatorWindowLocationListIterator.h
// -- Window用順序付き文書内出現位置リストの反復子 --
// 
// Copyright (c) 1997, 1999, 2001, 2002, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedOrderedOperatorWindowLocationListIterator_H__
#define __ModInvertedOrderedOperatorWindowLocationListIterator_H__

#include "ModInvertedWindowLocationListIterator.h"

class
ModInvertedOrderedOperatorWindowLocationListIterator:
public ModInvertedWindowLocationListIterator {
public:
	ModInvertedOrderedOperatorWindowLocationListIterator(
		ModInvertedQueryInternalNode* node)
		: ModInvertedWindowLocationListIterator(node) {}
	void initialize(
		ModSize minimalDistance_, 
		ModSize maximalDistance_,
		ModBoolean dummy=ModFalse);
	void next();
private:
	ModBoolean findTailAndHead();
	enum { _MAX_DISTANCE = 0x7fffffff };
protected:
};

//	FUNCTION
//	ModInvetedOrderedOperatorWindowLocationListIterator::initialize --初期化
//
//	NOTES
//	内部状態を初期化し、最初の位置に進む

inline void
ModInvertedOrderedOperatorWindowLocationListIterator::initialize(
	ModSize minimalDistance_, 
	ModSize maximalDistance_,
	ModBoolean dummy)
{
	// 最大長は _MAX_DISTANCE まで
	if (minimalDistance_ > _MAX_DISTANCE)
		minimalDistance_ = _MAX_DISTANCE;
	if (maximalDistance_ > _MAX_DISTANCE)
		maximalDistance_ = _MAX_DISTANCE;
	
	ModInvertedWindowLocationListIterator::initialize(
		minimalDistance_, maximalDistance_, dummy);
	next();
}

//
// FUNCTION
// ModInvertedOrderedOperatorWindowLocationListIterator::next -- 次の位置に進む
//
// NOTES
// 現在位置の次の距離の制約条件を満たす位置に自分を進める。
//
inline void
ModInvertedOrderedOperatorWindowLocationListIterator::next()
{
	while(1)
	{
		if(findTailAndHead() == ModFalse)
		{
			max = Location(0,0);
			min = max;
			break;
		}
		ModSize i;
		for(i = 1 ;  i < numberOfIterators - 1 ; i++)
		{
			if(min.getEndLocation() <= iterators[i]->getLocation() && iterators[i]->getEndLocation() <= max.getLocation())
			{
				if(i == 1 || iterators[i]->getLocation() > iterators[i - 1]->getLocation())
					continue;
			}
			if( iterators[i]->getLocation() < min.getEndLocation() )
			{
				iterators[i]->next();
				if(iterators[i]->isEnd() == ModTrue)
				{
					max = Location(0,0);
					min = max;
					return;
				}
				--i;
				continue;
			}
			else
				break;
		}
		if( i >= numberOfIterators - 1)
			break;
	}
}
inline	ModBoolean
ModInvertedOrderedOperatorWindowLocationListIterator::
findTailAndHead()
{
	ModSize distance;
	while(iterators[0]->isEnd() == ModFalse || iterators[numberOfIterators - 1]->isEnd() == ModFalse)// 2002/10/10変更
	{
		if(iterators[0]->isEnd() == ModFalse)							// 2002/10/10追加
			min = Location(iterators[0]->getLocation(),iterators[0]->getEndLocation());
		if(iterators[numberOfIterators - 1]->isEnd() == ModFalse)		// 2002/10/10追加
			max = Location(iterators[numberOfIterators - 1]->getLocation(),iterators[numberOfIterators - 1]->getEndLocation());
		distance = max.getLocation() - (flag?min.getLocation():min.getEndLocation()) + 1;
		if( min.getLocation() < max.getLocation())// 2002/10/10変更
		{
			if((int)distance < (int)minimalDistance)
				goto iterator_next;
			else
			{
				if(iterators[0]->isEnd() == ModFalse)					// 2002/10/10追加
				{
					iterators[0]->next();
					if(iterators[0]->isEnd() == ModTrue)				// 2002/10/10追加
						iterators[numberOfIterators - 1]->next();		// 2002/10/10追加
				}
				else
					iterators[numberOfIterators - 1]->next();			// 2002/10/10追加
			}
		}
		else
		{
iterator_next:
			if(iterators[numberOfIterators - 1]->isEnd() == ModFalse)	// 2002/10/10追加
				iterators[numberOfIterators - 1]->next();
			else														// 2002/10/10追加
				iterators[0]->next();									// 2002/10/10追加
		}

		if( (int)minimalDistance <= (int)distance && (int)distance <= (int)maximalDistance )
			return ModTrue;
	}
	return ModFalse;
}

#endif	// __ModInvertedOrderedOperatorWindowLocationListIterator_H__

//
// Copyright (c) 1997, 1999, 2001, 2002, 2005, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
