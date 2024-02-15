// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Types.h --
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

#ifndef __SYDNEY_INVERTED_TYPES_H
#define __SYDNEY_INVERTED_TYPES_H

#include "Inverted/Module.h"
#include "ModInvertedTypes.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

//
//	CONST
//	Inverted::UndefinedDocumentID -- 無効な文書IDをあらわす数
//
const ModUInt32 UndefinedDocumentID = ModInvertedUpperBoundDocumentID;

//
//	CONST
//	Inverted::DocumentIdMask -- 有効な文書IDの範囲
//
const ModUInt32 DocumentIdMask = 0x7fffffff;

//
//	CONST
//	Inverted::UndefinedRowID -- 無効な文書IDをあらわす数
//
const ModUInt32 UndefinedRowID = 0xffffffff;

//
//	CONST
//	Inverted::UndefinedResourceID -- 無効なリソースIDをあらわす数
//
const ModSize UndefinedResourceID = 0x7fffffff;

//
//	NAMESPACE
//	Inverted::ListType -- リスト種別をあらわす数
//
namespace ListType
{
	const ModUInt32 Short		= 0x00000000;
	const ModUInt32 Middle		= 0x80000000;
	const ModUInt32 Batch		= 0x40000000;

	const ModUInt32 TypeMask	= 0xf0000000;
	const ModUInt32 SizeMask	= 0x0fffffff;
}

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_TYPES_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
