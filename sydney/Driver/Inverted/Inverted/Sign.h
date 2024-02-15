// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Sign.h --
// 
// Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
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
#ifndef __SYDNEY_INVERTED_SIGN_H
#define __SYDNEY_INVERTED_SIGN_H

#include "Inverted/Module.h"

_SYDNEY_BEGIN
_SYDNEY_INVERTED_BEGIN

namespace Sign
{
	// [NOTE] _Insert0, _Delete0 はマージ用小転置で、
	//  _Insert1, _Delete1 は更新用小転置。
	//  ディレクトリ名と異なり、各値の意味は固定である。
	//	参考 Inverted/IndexFile.h
	
	enum Value
	{
		_FullInvert = 1,	// 0x01 - bit位置 0
		_Insert0    = 2,	// 0x02 - bit位置 1
		_Insert1    = 4,	// 0x04 - bit位置 2
		_Delete0    = 8,	// 0x08 - bit位置 3
		_Delete1    = 16,	// 0x10 - bit位置 4

	// 挿入転置と削除転置を識別するためのmask
		INSERT_MASK  = 0x07,
		DELETE_MASK  = 0x18,
	};
}

_SYDNEY_INVERTED_END
_SYDNEY_END

#endif //__SYDNEY_INVERTED_SIGN_H

//
//	Copyright (c) 2004-2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
