// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:
//
// ModNormType.h -- ModNormType のクラス定義
// 
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
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
#ifndef	__ModNormType_H_
#define __ModNormType_H_

#include "ModCommonDLL.h"
#include "ModUnicodeChar.h"
#include "ModNormDLL.h"

class ModUnicodeString;

template <class KeyType, class MappedType, class Hasher> class ModHashMap;

const ModSize ModNormUnicodeCharMax = 65536;

typedef ModHashMap<ModSize, ModUnicodeChar, ModHasher<ModSize> >
	ModNormCombiMap;


#endif // __ModNormType_H_
//
// Copyright (c) 2000-2009, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
