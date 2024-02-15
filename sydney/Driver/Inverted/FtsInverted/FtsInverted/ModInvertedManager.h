// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedManager.h -- ModInvertedManager のクラス定義
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedManager_H__
#define __ModInvertedManager_H__

#include "ModDefaultManager.h"
#include "ModInvertedTypes.h"

#ifdef MOD_SELF_MEMORY_MANAGEMENT_OFF
#define MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
#endif

#ifndef MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
#define MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF
#endif

#ifdef MOD_INVERTED_SELF_MEMORY_MANAGEMENT_OFF

class
ModInvertedObject {
public:
	typedef ModInvertedDocumentID DocumentID;
	typedef ModInvertedDataUnit Unit;
};

#else

typedef ModDefaultManager ModInvertedManager;

class
ModInvertedObject : public ModObject<ModInvertedManager> {
public:
	typedef ModInvertedDocumentID DocumentID;
	typedef ModInvertedDataUnit Unit;
};

#endif

#endif	// __ModInvertedManager_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
