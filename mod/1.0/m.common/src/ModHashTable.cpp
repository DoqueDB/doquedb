// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModHashTable.cpp -- ハッシュ表関連の関数定義
// 
// Copyright (c) 1999, 2023 Ricoh Company, Ltd.
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


#include "ModHashTable.h"

// 静的クラスメンバーの定義

const ModSize	ModHashTableBase::_primeTable[] =
{
	1,			7,			17,			53,			97,
	193,		389,		769,		1543,		3079,
	6151,		12289,		24593,		49157,		98317,		
	196613,		393241,		786433,		1572869,	3145739,
	6291469,	12582917,	25165843,	50331653,	100663319,
	201326611,	402653189,	805306457, 	1610612741,	3221225473
};

const ModSize	ModHashTableBase::_primeTableSize =
					sizeof(ModHashTableBase::_primeTable) / sizeof(ModSize);

//	FUNCTION public
//	ModHashTableBase::verifySize --
//		ハッシュ表の大きさとして与えられた値を適切な値に直す
//
//	NOTES
//		ハッシュ表の大きさは素数であることが望ましいため、
//		この関数を用いて指定された大きさを適切な値にする
//
//	ARGUMENTS
//		ModSize				n
//			ハッシュ表の大きさとして与えられた値
//
//	RETURN
//		適切なハッシュ表の大きさ
//
//	EXCEPTIONS
//		なし

// static
ModSize
ModHashTableBase::verifySize(ModSize n)
{
	const ModSize* last = _primeTable + _primeTableSize;
	const ModSize* pos = ModLowerBound(_primeTable, last, n);
	return *((pos == last) ? last - 1 : pos);
}

//
// Copyright (c) 1999, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
