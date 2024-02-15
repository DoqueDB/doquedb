// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModInvertedOrderedDistanceLocationListIterator.h -- 文書内出現位置リストの反復子
// 
// Copyright (c) 1999, 2002, 2023 Ricoh Company, Ltd.
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

#ifdef SYD_INVERTED // SYDNEY 対応
#include "SyDefault.h"
#include "SyReinterpretCast.h"
#endif

#include "ModInvertedOrderedDistanceLocationListIterator.h"

#ifdef DEBUG
//
// VARIABLE
// ModInvertedOrderedDistanceLocationListIterator::countRawNext -- 次項取得の回数
//
/*static*/ int
ModInvertedOrderedDistanceLocationListIterator::countRawNext = 0;
#endif


//
// Copyright (c) 1999, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
