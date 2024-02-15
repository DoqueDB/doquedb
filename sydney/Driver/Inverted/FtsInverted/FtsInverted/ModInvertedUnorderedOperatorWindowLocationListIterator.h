// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedUnorderedOperatorWindowLocationListIterator.h 
// -- Window用順序付き文書内出現位置リストの反復子
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

#ifndef	__ModInvertedUnorderedOperatorWindowLocationListIterator_H__
#define __ModInvertedUnorderedOperatorWindowLocationListIterator_H__

#include "ModInvertedWindowLocationListIterator.h"
#include "ModAlgorithm.h"

class

#define INDEXMASK 0x80000000

ModInvertedUnorderedOperatorWindowLocationListIterator:
public ModInvertedWindowLocationListIterator {
public:
	ModInvertedUnorderedOperatorWindowLocationListIterator(
		ModInvertedQueryInternalNode* node);
	
	void initialize(
			ModSize minimalDistance_, 
			ModSize maximalDistance_,
			ModBoolean dummy = ModFalse);
	void reserve(ModSize n)
	{
		queue.reserve(n);
		lastqueue.reserve(n);
		ModInvertedWindowLocationListIterator::reserve(n);
	}

	void next();

	void release()
	{
		queue.release();
		lastqueue.release();
		ModInvertedWindowLocationListIterator::release();
	}
	
protected:
	//
	// priorityQueue are helper class for
	// ModInvertedUnorderedOperatorWindowLocationListIterator
	//
	class PriorityQueue
	{
		struct Indexcount
		{
			ModSize n;			// number of index
			ModSize	r;			// reference count
			ModSize	pos;		// index position
		};
	public:
		static ModBoolean compare(const Location & x, const Location & y)
		{
			return (ModBoolean)(x.getLocation() > y.getLocation());
		}
		PriorityQueue(ModSize maxsize) : indexCount(0) {
			heapmax = maxsize;heapsize = 0;data.reserve(heapmax);}
		~PriorityQueue(){delete indexCount;}
		void reserve(ModSize n) {
			if (indexCount) delete indexCount;
			numberOfElements = n;
			indexCount = new Indexcount[numberOfElements];
			for(ModSize i = 0 ; i < numberOfElements ; i++)
				indexCount[i].n = indexCount[i].r = indexCount[i].pos = 0;
		}
		void release() {
			data.erase(data.begin(), data.end());
			for(ModSize i = 0 ; i < numberOfElements ; i++)
				indexCount[i].n = indexCount[i].r = indexCount[i].pos = 0;
			heapsize = 0;
		}
		// protocol
		void add(Location value)
		{
			if(heapsize + 1 >= heapmax )
			{
				// reserve ModVector's area
				heapmax += 32;
				data.reserve(heapmax);
			}
			ModSize pos = heapsize++;
			while(pos > 0 && compare(value,data[(pos - 1)/2]) == ModFalse){
				if(pos >= data.getSize())
					data.pushBack(data[(pos - 1)/2]);
				else
					data[pos] = data[(pos - 1)/2];
				pos = (pos - 1)/2;
			}
			if(pos >= data.getSize())
				data.pushBack(value);
			else
				data[pos] = value;
			++(indexCount[value.getIndex() - 1].n);
		}
		ModBoolean	isEmpty() const{return (ModBoolean)(heapsize == 0);}
		Location getMin() const{return data[0];}
		void removeMin()
		{
			if(heapsize > 0)
			{
				if(indexCount[data[0].getIndex()- 1].n > 0)
					--(indexCount[data[0].getIndex() - 1].n);

				data[0] = data[--heapsize];
				data.popBack();
				ModMakeHeap(data.begin(),data.end(),compare);
				//
				for(ModSize i = 0 ;  i < numberOfElements; i++)
					indexCount[i].r = indexCount[i].pos = 0;
			}
		}
		ModBoolean find(ModSize index){
			return indexCount[index-1].n ? ModTrue:ModFalse;}
		ModBoolean findA(ModSize index,Location &t)
		{
			for(int i = 0 ; i < (int)data.getSize(); i++)
			{
				if(data[i].getIndex() == index)
				{
					t = data[i];
					return ModTrue;
				}
			}
			return ModFalse;
		}
		ModBoolean find(ModSize index,Location &t)
		{
			if(indexCount[index-1].n > indexCount[index-1].r + 1)
			{
				ModSize i;
				for(i = indexCount[index-1].pos + 1 ; i < data.getSize() ; i++)
				{
					if(data[i].getIndex() == index)
					{
						if(data[i].getLocation() == t.getLocation())
							continue;
						t = data[i];
						++indexCount[index - 1].r;
						indexCount[index - 1].pos = i;
						return ModTrue;
					}
				}
			}
			else
				indexCount[index - 1].r = indexCount[index-1].n;
			return ModFalse;
		}
	private:
		ModSize	numberOfElements;
		Indexcount *indexCount;
		ModVector<Location> data;
		ModSize		heapmax;
		ModSize		heapsize;
	};
	PriorityQueue queue,lastqueue;

private:
	enum { _MAX_DISTANCE = 0x7fffffff };
};

//	FUNCTION
//	ModInvertedUnorderedOperatorWindowLocationListIterator::ModInvertedUnorderedOperatorWindowLocationListIterator -- コンストラクタ

inline
ModInvertedUnorderedOperatorWindowLocationListIterator::
ModInvertedUnorderedOperatorWindowLocationListIterator(
	ModInvertedQueryInternalNode* node)
	: ModInvertedWindowLocationListIterator(node),
	  queue(32), lastqueue(32)
{}

//
// implementation for ModInvertedUnorderedOperatorWindowLocationListIterator
//
inline void
ModInvertedUnorderedOperatorWindowLocationListIterator::initialize(
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
	
	for(ModSize i = 0; i < numberOfIterators; i++)
		queue.add
			(
			Location(iterators[i]->getLocation(),
					 iterators[i]->getEndLocation(),
					 i + 1)
			);
	next();
}
//
// FUNCTION
// ModInvertedUnorderedOperatorWindowLocationListIterator::next -- 次の位置に進む
//
// NOTES
// 現在位置の次の距離の制約条件を満たす位置に自分を進める。
//
inline void
ModInvertedUnorderedOperatorWindowLocationListIterator::next()
{
	ModSize distance;
	Location t;
	ModSize i,j;
	while( 1 )
	{
		t = max = min = queue.getMin();
		for( i = j = 0 ; i < numberOfIterators; i++)
		{
			if( min.getIndex() == i + 1)
				continue;

			if(t.getIndex() & INDEXMASK)
			{
				 if((t.getIndex() & ~INDEXMASK) != i + 1)
					 continue;
			}
			if(iterators[i]->isEnd() == ModFalse)	// 2002/10/10追加
				t = Location(
					iterators[i]->getLocation(),
					iterators[i]->getEndLocation(),
					i + 1);
			else
				lastqueue.findA(i + 1 ,t);

			if(queue.find( i + 1 , t) == ModTrue)		//iterators[i]->isEnd()がtrueの場合は、ここで、最後のiterator位置が探索される
				t.setIndex(t.getIndex() | INDEXMASK);

			distance = t.getLocation() - (flag?min.getLocation():min.getEndLocation()) + 1;
			if(distance <= maximalDistance )
			{
				++j;
				if(max.getLocation() < t.getLocation())
					max = t;
				else if(max.getLocation() == t.getLocation())
				{
					if(max.getEndLocation() == t.getEndLocation())
						j = 0;
				}
				if( j == numberOfIterators - 1)
				{
					distance = max.getLocation() - (flag?min.getLocation():min.getEndLocation()) + 1;
					if( (int)minimalDistance > (int)distance ) 
						j = 0;
					break;
				}
			}
			else
			{
				if(t.getIndex() & INDEXMASK)
				{
					--i;
					continue;
				}
				break;
			}
		}

		if( j < numberOfIterators - 1 )
		{
			if((int)distance < (int)minimalDistance) {
				i = (t.getIndex() & ~INDEXMASK) - 1;
				if(numberOfIterators > 2)
				{
					// iteratorのLocationがmaxとmin以外のものを求める
					int m = (max.getIndex() & ~INDEXMASK) - 1;
					int k = min.getIndex() - 1;
					for(int j = 0 ; j < (int)numberOfIterators; ++j)
					{
						if(j != m && j != k)
						{
							i = j;
							break;
						}
					}
				}
			}
			else
			{
				queue.removeMin();
				i = min.getIndex() - 1;
				if(queue.find( i + 1) == ModTrue)
					continue;
			}
		}
		else
			i = (max.getIndex() & ~INDEXMASK) - 1;

		if( j < numberOfIterators - 1 || !(max.getIndex() & INDEXMASK))
		{
			if( iterators[i]->isEnd() == ModFalse)		// 2002/10/10追加
			{
				t = Location(							// 2002/10/10追加
					iterators[i]->getLocation(),		// A. nextする前の直前の位置をtに保存する
					iterators[i]->getEndLocation(),
					i + 1);								
				iterators[i]->next();
				if(iterators[i]->isEnd() == ModTrue)
					lastqueue.add(t);					// A で保存した位置情報をqueueに入れる　2002/10/10追加
			}
			if(iterators[i]->isEnd() == ModTrue)
			{
				queue.removeMin();
				if( queue.find( i + 1 ) == ModFalse)
				{
					max = min = Location(0,0,i + 1);
					break;
				}
				i = min.getIndex() - 1;
				if(iterators[i]->isEnd() == ModFalse)
				{
					t = Location(							// 2002/10/10追加
						iterators[i]->getLocation(),		// A. nextする前の直前の位置をtに保存する
						iterators[i]->getEndLocation(),
						i + 1);								
					iterators[i]->next();
					if(iterators[i]->isEnd() == ModTrue)
						lastqueue.add(t);					// A で保存した位置情報をqueueに入れる　2002/10/10追加
				}
			}
			if(iterators[i]->isEnd() == ModFalse)
				queue.add
				(
					Location(
						iterators[i]->getLocation(),
						iterators[i]->getEndLocation(),
						i + 1)
				);
		}
		if( j == numberOfIterators - 1)
			break;
	}
}

#endif	// __ModInvertedUnorderedOperatorWindowLocationListIterator_H__

//
// Copyright (c) 1997, 1999, 2001, 2002, 2005, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
