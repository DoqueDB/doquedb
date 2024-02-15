// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedFileCapsule.h -- 転置ファイルカプセルの定義
// 
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModInvertedFileCapsule_H__
#define __ModInvertedFileCapsule_H__

#include "ModInvertedTypes.h"

//
// CLASS
// ModInvertedFileCapsuleBooleanSearch -- ブーリアン検索カプセル
//
// NOTES
// 転置ファイルのブーリアン検索を行うためのリクエストカプセルである。
//
class ModInvertedFileCapsuleBooleanSearch
{
public:
	static const ModInvertedQueryValidateMode defaultValidateMode;
	static const ModInvertedQueryEvaluateMode defaultEvaluateMode;
};

//
// CLASS
// ModInvertedFileCapsuleRankingSearch -- ランキング検索カプセル
//
// NOTES
// 転置ファイルのランキング検索を行うためのリクエストカプセルである。
//
class ModInvertedFileCapsuleRankingSearch
{
public:
	static const ModInvertedQueryValidateMode defaultValidateMode;
	static const ModInvertedQueryEvaluateMode defaultEvaluateMode;
};

//
// CLASS
// ModInvertedFileCapsuleGetMatchLocations -- 照合位置取得カプセル
//
// NOTES
// 転置ファイルの照合位置取得を行うためのリクエストカプセルである。
//
class ModInvertedFileCapsuleGetMatchLocations
{
public:
	static const ModInvertedQueryValidateMode defaultValidateMode;
	static const ModInvertedQueryEvaluateMode defaultEvaluateMode;
};

#endif	// __ModInvertedFileCapsule_H__

//
// Copyright (c) 1997, 1998, 1999, 2000, 2001, 2002, 2003, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
