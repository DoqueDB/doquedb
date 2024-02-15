// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedWindowLocationListIterator.h -- Window用順序付き文書内出現位置リストの反復子
// 
// Copyright (c) 1997, 1999, 2001, 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedWindowLocationListIterator_H__
#define __ModInvertedWindowLocationListIterator_H__

#include "ModInvertedLocationListIterator.h"

//
// CLASS
// ModInvertedWindowLocationListIterator
//
// NOTES
//
class
ModInvertedWindowLocationListIterator : public ModInvertedLocationListIterator
{
public:
	// typedef
	typedef ModInvertedLocationListIterator	LocationIterator;
	typedef ModVector<ModInvertedLocationListIterator*> LocationIterators;

	ModInvertedWindowLocationListIterator(
		ModInvertedQueryInternalNode* node);
	~ModInvertedWindowLocationListIterator();

	void initialize(ModSize min_, ModSize max_, ModBoolean flag_);
	void pushIterator(LocationIterator* i)
	{
		iterators.pushBack(i);
	}
	void reserve(ModSize n)
	{
		iterators.reserve(n);
	}
	ModSize getSize() const
	{
		return iterators.getSize();
	}
	
	virtual	void next() = 0;
	void reset();
	ModBoolean isEnd() const;
	ModSize getLocation();

	// マッチする部分の末尾位置を返す
	ModSize getEndLocation();

	void release()
	{
		LocationIterators::Iterator i = iterators.begin();
		for (; i != iterators.end(); ++i)
			(*i)->release();
		iterators.erase(iterators.begin(), iterators.end());
		LocationIterator::release();
	}

	//******************************************************
	// 'Location' is the class for word location holder
	//  which holds word location
	//******************************************************
	// 先頭位置・終端位置 格納用クラス
	//	UnorderedOperatorLocationListIterator::PriorityQueue からも
	//	使えるようにするため、このクラスは public とする
	class Location
	{
	public:
		Location(){ location = endLocation = 0;}
		Location(ModSize _location,ModSize _endLocation)
		{
			location = _location,endLocation = _endLocation;
			index = 0;
		}
		Location(ModSize _location,ModSize _endLocation,ModSize _index)
		{
			location = _location,endLocation = _endLocation;
			index = _index;
		}
		const ModSize getLocation() const{return location;}
		const ModSize getEndLocation() const{return endLocation;}
		const ModSize getIndex() const{return index;}
		void setLocation(ModSize _location){location = _location;}
		void setEndLocation(ModSize _endLocation){endLocation = _endLocation;}
		void setIndex(ModSize _index){index = _index;}
	private:
		ModSize location;
		ModSize endLocation;
		ModSize	index;
	};
protected:	
	Location min,max;
private:
	// 各要素の文書内位置情報
protected:
	ModBoolean	flag;
	ModSize	numberOfIterators;
	LocationIterators	iterators;
	// 先頭要素と末尾要素の頭と頭の間の文字数の下限
	ModSize				minimalDistance;
	// 先頭要素と末尾要素の頭と頭の間の文字数の上限
	ModSize				maximalDistance;
};

//
// FUNCTION
// ModInvertedWindowLocationListIterator::ModInvertedWindowLocationListIterator -- コンストラクタ
//
// NOTES
//
inline
ModInvertedWindowLocationListIterator::
ModInvertedWindowLocationListIterator(
	ModInvertedQueryInternalNode* node)
	: LocationIterator(node)
{
}

//
// FUNCTION
// ModInvertedWindowLocationListIterator::
// ~ModInvertedWindowLocationListIterator -- デストラクタ
//
// NOTES
//
inline
ModInvertedWindowLocationListIterator::
~ModInvertedWindowLocationListIterator()
{
}

//
// FUNCTION
// ModInvertedWindowLocationListIterator::initialize -- 初期化
//
// NOTES
//
inline void
ModInvertedWindowLocationListIterator::initialize(
	ModSize minimalDistance_, ModSize maximalDistance_, ModBoolean flag_)
{
	minimalDistance = minimalDistance_;
	maximalDistance = maximalDistance_;
	flag = flag_;
	
	// init the initial position of iterators.
	numberOfIterators = iterators.end() - iterators.begin();
}

//
// FUNCTION
// ModInvertedWindowLocationListIterator::reset -- 先頭に戻る
//
// NOTES
//
inline void
ModInvertedWindowLocationListIterator::reset()
{
	LocationIterators::Iterator i = iterators.begin();
	while( i != iterators.end())
		(*i++)->reset();
	next();
}
//
// FUNCTION
// ModInvertedWindowLocationListIterator::isEnd -- 末尾か調べる
//
// NOTES
//
inline ModBoolean
ModInvertedWindowLocationListIterator::isEnd() const
{
	return (ModBoolean)(max.getEndLocation() == 0);
}

//
// FUNCTION
// ModInvertedWindowLocationListIterator::getLocation -- 位置の取得
//
// NOTES
// 1番前にあるものの位置を返す。
//
inline ModSize
ModInvertedWindowLocationListIterator::getLocation()
{
	return min.getLocation();
}
//
// FUNCTION
// ModInvertedWindowLocationListIterator::getEndLocation -- マッチする部分の末尾位置を返す
//
// NOTES
// tailIterator の getEndLocation() の結果を返す
//
inline ModSize
ModInvertedWindowLocationListIterator::getEndLocation()
{
	return max.getEndLocation();
}

#endif	// __ModInvertedWindowLocationListIterator_H__

//
// Copyright (c) 1997, 1999, 2001, 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
