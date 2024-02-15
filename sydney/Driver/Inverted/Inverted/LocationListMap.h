// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LocationListMap.h --
// 
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_INVERTED_LOCATIONLISTMAP_H
#define __SYDNEY_INVERTED_LOCATIONLISTMAP_H

#include "Inverted/Module.h"
#include "ModInvertedTokenizer.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CLASS
//	Inverted::LocationListMap --
//
//	NOTES
//
//
class LocationListMap : public ModInvertedLocationListMap
{
public:
	// コンストラクタ
	LocationListMap();
	// デストラクタ
	virtual ~LocationListMap();
};

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_LOCATIONLISTMAP_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
