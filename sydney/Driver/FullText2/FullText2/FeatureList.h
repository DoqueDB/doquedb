// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FeatureList.h --
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

#ifndef __SYDNEY_FULLTEXT2_FEATURELIST_H
#define __SYDNEY_FULLTEXT2_FEATURELIST_H

#include "FullText2/Module.h"
#include "Common/LargeVector.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_FULLTEXT2_BEGIN

//
// STRUCT
// FullText2::FeatureElement -- 特徴語と重みの組
//
// NOTES
// 特徴語と重みの組のデータ型。
//
struct FeatureElement
{
	FeatureElement() : scale(0.0) {}
	FeatureElement(const ModUnicodeString& term_, double scale_)
		: term(term_), scale(scale_) {}
	
	ModUnicodeString	term;
	double				scale;

	// 比較クラス
	class Greator
	{
	public:
		ModBoolean operator() (const FeatureElement& s1,
							   const FeatureElement& s2) const
			{
				return (s1.scale > s2.scale) ? ModTrue : ModFalse;
			}
	};
};

//
// TYPEDEF
// FullText2::FeatureList -- 特徴語と重みの組のリスト
//
// NOTES
// 特徴語と重みの組のリストのデータ型。
//
typedef Common::LargeVector<FeatureElement> FeatureList;

_SYDNEY_FULLTEXT2_END
_SYDNEY_END

#endif //__SYDNEY_FULLTEXT2_FEATURELIST_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
