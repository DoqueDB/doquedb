// -*-Mode: C++; tab-width: 4; -*-
// vi:set ts=4 sw=4:	
//
// ModInvertedUncompressedLocationListIterator.h -- 圧縮された文書内出現位置リストの反復子
// 
// Copyright (c) 1997, 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedUncompressedLocationListIterator_H__
#define __ModInvertedUncompressedLocationListIterator_H__

#include "ModOs.h"
#include "ModInvertedTypes.h"
#include "ModInvertedLocationListIterator.h"

#ifdef SYD_INVERTED // SYDNEY 対応
#include "ModUnicodeString.h"
#endif

//
// 圧縮された位置エントリに対する反復子
// 
class
ModInvertedUncompressedLocationListIterator :
	public  ModInvertedLocationListIterator
{
public:
    ModInvertedUncompressedLocationListIterator(const ModInvertedLocationList&);
    ~ModInvertedUncompressedLocationListIterator();

    void next();
	void reset();
    ModBoolean isEnd() const;
	ModSize getLocation();

	void setLength(const ModSize);
	ModSize getLength();
	ModSize getEndLocation();

	void release() {}

private:
	const ModInvertedLocationList& vector;
	ModInvertedLocationList::ConstIterator iterator;
	ModInvertedLocationList::ConstIterator endIterator;
	ModSize length;
};


inline
ModInvertedUncompressedLocationListIterator::ModInvertedUncompressedLocationListIterator(
	const ModInvertedLocationList& vector_)
	:
	vector(vector_), iterator(vector.begin()), endIterator(vector.end()), length(0)
{}

inline
ModInvertedUncompressedLocationListIterator::~ModInvertedUncompressedLocationListIterator()
{}

inline void
ModInvertedUncompressedLocationListIterator::next()
{
	++iterator;
}

inline void
ModInvertedUncompressedLocationListIterator::reset()
{
	iterator = vector.begin();
}

inline ModBoolean
ModInvertedUncompressedLocationListIterator::isEnd() const
{
	if (iterator == endIterator) {
		return ModTrue;
	}
	return ModFalse;
}

inline ModSize
ModInvertedUncompressedLocationListIterator::getLocation()
{
	return *iterator;
}

inline void
ModInvertedUncompressedLocationListIterator::setLength(const ModSize length_)
{
	length = length_;
}

inline ModSize
ModInvertedUncompressedLocationListIterator::getLength()
{
	return length;
}

inline ModSize
ModInvertedUncompressedLocationListIterator::getEndLocation()
{
	return *iterator + length;
}

#endif	__ModInvertedUncompressedLocationListIterator_H__

//
// Copyright (c) 1997, 2002, 2004, 2005, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
