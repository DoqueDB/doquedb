// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ListIterator.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "FullText2";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "FullText2/ListIterator.h"
#include "FullText2/LocationListIterator.h"

_SYDNEY_USING
_SYDNEY_FULLTEXT2_USING

namespace
{
}

//
//	FUNCTION public
//	FullText2::ListIterator::ListIterator -- コンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListIterator::ListIterator()
{
}

//
//	FUNCTION public
//	FullText2::ListIterator::~ListIterator -- デストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListIterator::~ListIterator()
{
}

//
//	FUNCTION public
//	FullText2::ListIterator::ListIterator -- コピーコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//	const FullText2::ListIterator& src_
//		コピー元
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
ListIterator::ListIterator(const ListIterator& src_)
{
}

//
//	FUNCTION public
//	FullText2::ListIterator::getLocationListIterator
//		-- 位置情報へのイテレータを得る
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	FullText2::LocationListIterator::AutoPointer
//		位置情報へのイテレータへのポインタ
//
//	EXCEPTIONS
//
LocationListIterator::AutoPointer
ListIterator::getLocationListIterator()
{
	return LocationListIterator::AutoPointer();
}

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//

