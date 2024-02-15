// #T1#
// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
//	Type.cpp -- Implement file of Type class
// 
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
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


#include "LibUna/UnaNameSpace.h"
#include "LibUna/Type.h"
#include "ModUnicodeString.h"

_UNA_USING

using namespace Type;

namespace {

	ModUnicodeChar OperateStringMap [][3] = {
		{ ModUnicodeChar('='), 0 }
		,{ ModUnicodeChar('|'), ModUnicodeChar('='), 0 }
		,{ ModUnicodeChar('&'), ModUnicodeChar('='), 0 }
		,{ ModUnicodeChar('^'), ModUnicodeChar('='), 0 }
		,{ 0, 0 }
	};

// #T2#
} // end of namespace


#ifdef OBSOLETE
// #T3#
// FUNCTION pubic
//	CalcType::getOperateString --
//
// NOTES
//
// ARGUMENTS
//	unsigned int
//		
//
// RETURN
//	void
//
// EXCEPTIONS
ModUnicodeString
CalcType::getOperateString(Value value_)
{
	int value = static_cast<int>(value_);
	value -= static_cast<int>(Equal);
	if ( Equal <= value && value <= XorEqual ) {
		return ModUnicodeString(OperateStringMap[value_], 2);
	}
	return ModUnicodeString(OperateStringMap[AndEqual], 2);
}
#endif

// #T4#
// FUNCTION pubic
//	CalcType::
//
// NOTES
//
// ARGUMENTS
//	unsigned int
//		
//
// RETURN
//	void
//
// EXCEPTIONS
CalcType::Value
CalcType::getOperateValue(const ModUnicodeString& value_)
{
	int i = 0;
	while ( OperateStringMap[i][0] != 0 ) {
		if ( value_ == OperateStringMap[i] ) {
			return static_cast<Value>(i + Equal);
		}
		++i;
	}
	return Unknown;
}

//
// Copyright (c) 2004-2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
