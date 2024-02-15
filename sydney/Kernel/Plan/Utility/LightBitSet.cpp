// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/LightBitSet.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Utility";
}

#include "SyDefault.h"

#include "Plan/Utility/LightBitSet.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_UTILITY_USING

namespace
{
	// CONST local
	//	$$$::_BitPerUnit --
	//
	// NOTES

	const int _BitPerUnit = sizeof(LightBitSet::UnitType) * 8;

	// FUNCTION local
	//	$$$::_getUnitPosition -- bit position -> unit position
	//
	// NOTES
	//
	// ARGUMENTS
	//	LightBitSet::Position iPos_
	//	
	// RETURN
	//	LightBitSet::Position
	//
	// EXCEPTIONS

	LightBitSet::Position
	_getUnitPosition(LightBitSet::Position iPos_)
	{
		return iPos_ / _BitPerUnit;
	}

	// FUNCTION local
	//	$$$::_setBitOnUnit -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	LightBitSet::UnitType& cUnit_
	//	LightBitSet::Position iPos_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_setBitOnUnit(LightBitSet::UnitType& cUnit_,
				  LightBitSet::Position iPos_)
	{
		cUnit_ |= (static_cast<LightBitSet::UnitType>(1) << (iPos_ % _BitPerUnit));
	}

	// FUNCTION local
	//	$$$::_testBitOnUnit -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const LightBitSet::UnitType& cUnit_
	//	LightBitSet::Position iPos_
	//	
	// RETURN
	//	bool
	//
	// EXCEPTIONS

	bool
	_testBitOnUnit(const LightBitSet::UnitType& cUnit_,
				   LightBitSet::Position iPos_)
	{
		return (cUnit_ & (static_cast<LightBitSet::UnitType>(1) << (iPos_ % _BitPerUnit)));
	}

	void
	_addString(ModUnicodeOstrStream& stream_,
			   const LightBitSet::UnitType& cUnit_,
			   LightBitSet::Position iBase_)
	{
		if (cUnit_) {
			LightBitSet::UnitType cBit(1);
			for (int i = 0; i < _BitPerUnit; ++i, cBit <<= 1) {
				if (cUnit_ & cBit) {
					if (stream_.isEmpty() == ModFalse) stream_ << ',';
					stream_ << (iBase_ + i);
				}
			}
		}
	}
}

// FUNCTION public
//	Utility::LightBitSet::set -- set a bit
//
// NOTES
//
// ARGUMENTS
//	Position iPos_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
LightBitSet::
set(Position iPos_)
{
	Position iUnitPos = _getUnitPosition(iPos_);
	Opt::ExpandContainer(m_vecUnit, iUnitPos + 1, static_cast<UnitType>(0));
	_setBitOnUnit(m_vecUnit[iUnitPos], iPos_);
}

// FUNCTION public
//	Utility::LightBitSet::test -- test a bit
//
// NOTES
//
// ARGUMENTS
//	Position iPos_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
LightBitSet::
test(Position iPos_) const
{
	Position iUnitPos = _getUnitPosition(iPos_);
	return iUnitPos < m_vecUnit.GETSIZE() && _testBitOnUnit(m_vecUnit[iUnitPos], iPos_);
}

// FUNCTION public
//	Utility::LightBitSet::operator== -- comparison
//
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
LightBitSet::
operator==(const This& cOther_) const
{
	bool bResult = true;
	if (this != &cOther_) {
		if (m_vecUnit.GETSIZE() != cOther_.m_vecUnit.GETSIZE()) {
			bResult = false;
		} else {
			VECTOR<UnitType>::CONSTITERATOR iterator0 = m_vecUnit.begin();
			const VECTOR<UnitType>::CONSTITERATOR last0 = m_vecUnit.end();
			VECTOR<UnitType>::CONSTITERATOR iterator1 = cOther_.m_vecUnit.begin();
			const VECTOR<UnitType>::CONSTITERATOR last1 = cOther_.m_vecUnit.end();
			for (; iterator0 != last0 && iterator1 != last1; ++iterator0, ++iterator1) {
				if (*iterator0 != *iterator1) {
					bResult = false;
					break;
				}
			}
		}
	}
	return bResult;
}

// FUNCTION public
//	Utility::LightBitSet::operator< -- comparison
//
// NOTES
//
// ARGUMENTS
//	const This& cOther_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
LightBitSet::
operator<(const This& cOther_) const
{
	bool bResult = false;
	if (this != &cOther_) {
		if (m_vecUnit.GETSIZE() != cOther_.m_vecUnit.GETSIZE()) {
			bResult = (m_vecUnit.GETSIZE() < cOther_.m_vecUnit.GETSIZE());
		} else {
			; _SYDNEY_ASSERT(m_vecUnit.GETSIZE() == cOther_.m_vecUnit.GETSIZE());

			VECTOR<UnitType>::CONSTITERATOR iterator0 = m_vecUnit.end();
			const VECTOR<UnitType>::CONSTITERATOR first0 = m_vecUnit.begin();
			VECTOR<UnitType>::CONSTITERATOR iterator1 = cOther_.m_vecUnit.end();
			const VECTOR<UnitType>::CONSTITERATOR first1 = cOther_.m_vecUnit.begin();
			while (iterator0 != first0 && iterator1 != first1) {
				--iterator0;
				--iterator1;
				if (*iterator0 != *iterator1) {
					bResult = (*iterator0 < *iterator1);
					break;
				}
			}
		}
	}
	return bResult;
}

// FUNCTION public
//	Utility::LightBitSet::getString -- get as string
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
LightBitSet::
getString() const
{
	ModUnicodeOstrStream stream;
	VECTOR<UnitType>::CONSTITERATOR iterator = m_vecUnit.begin();
	const VECTOR<UnitType>::CONSTITERATOR last = m_vecUnit.end();
	Position iBase = 0;
	for (; iterator != last; ++iterator) {
		_addString(stream, *iterator, iBase);
		iBase += _BitPerUnit;
	}
	return stream.getString();
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
