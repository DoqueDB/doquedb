// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/LightBitSet.h --
// 
// Copyright (c) 2008, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_UTILITY_LIGHTBITSET_H
#define __SYDNEY_PLAN_UTILITY_LIGHTBITSET_H

#include "Plan/Utility/Module.h"

#include "Common/Object.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_UTILITY_BEGIN

//////////////////////////////////////////////////////////////////////
// CLASS
//	LightBitSet -- a set of bitfield
//
// NOTES
//	This class is similar to Common::BitSet.
//	Comparable bitset class is needed.

class LightBitSet
	: public Common::Object
{
public:
	typedef Common::Object Super;
	typedef LightBitSet This;

	typedef unsigned int UnitType;
	typedef unsigned int Position;

	// constructor
	LightBitSet() : Super(), m_vecUnit() {}
	LightBitSet(const LightBitSet& cOther_) : Super(cOther_), m_vecUnit(cOther_.m_vecUnit) {}

	// destructor
	~LightBitSet() {}

	// set a bit
	void set(Position iPos_);

	// test a bit
	bool test(Position iPos_) const;

	// comparison
	bool operator==(const This& cOther_) const;
	bool operator<(const This& cOther_) const;

	// assignment
	This& operator=(const This& cOther_)
	{
		m_vecUnit = cOther_.m_vecUnit;
		return *this;
	}
	// get as string
	ModUnicodeString getString() const;
protected:
private:
	VECTOR<UnitType> m_vecUnit;
};

_SYDNEY_PLAN_UTILITY_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_UTILITY_LIGHTBITSET_H

//
//	Copyright (c) 2008, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
