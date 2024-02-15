// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReplacementPriority.h -- バッファの破棄されにくさを表すクラス定義、関数宣言
// 
// Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_BUFFER_REPLACEMENTPRIORITY_H
#define	__SYDNEY_BUFFER_REPLACEMENTPRIORITY_H

#include "Buffer/Module.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

#ifdef DUMMY
#else
//	CLASS
//	Buffer::ReplacementPriority -- バッファの破棄されにくさを表すクラス
//
//	NOTES

struct ReplacementPriority
{
	//	ENUM
	//	Buffer::ReplacementPriority::Value --
	//		バッファの棄てられにくさを表す値の列挙型
	//
	//	NOTES

	typedef unsigned char	Value;
	enum
	{
		// 破棄されやすい
		Low =			0,
		// 通常
		Middle,
		// 破棄されにくい
		High,
		// 値の数
		Count,
		// 不明
		Unknown =		Count
		, ValueNum = Count
	};
};
#endif

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_REPLACEMENTPRIORITY_H

//
// Copyright (c) 2000, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
