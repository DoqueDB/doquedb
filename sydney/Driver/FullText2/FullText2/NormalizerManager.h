// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// NormalizerManager.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_FULLTEXT2_NORMALIZERMANAGER_H
#define __SYDNEY_FULLTEXT2_NORMALIZERMANAGER_H

#include "FullText2/Module.h"

#include "ModTypes.h"

#ifdef SYD_USE_UNA_V10
namespace UNA {
class ModNlpAnalyzer;
}
#else
class ModNormalizer;
#endif

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
//	CLASS
//	FullText2::NormalizerManager --
//
//	NOTES
//
//
class NormalizerManager
{
public:
	// コンストラクタ
	NormalizerManager();
	// デストラクタ
	~NormalizerManager();

	// 異表記正規化器を得る
#ifdef SYD_USE_UNA_V10
	static UNA::ModNlpAnalyzer* get(const ModUInt32 normRscId_);
#else
	static ModNormalizer* get(const ModUInt32 normRscId_);
#endif
};

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_NORMALIZERMANAGER_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
