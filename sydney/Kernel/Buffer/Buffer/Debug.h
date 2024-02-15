// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Debug.h --	バッファ管理モジュールのデバッグに使用するマクロ定義
// 
// Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_BUFFER_DEBUG_H
#define	__SYDNEY_BUFFER_DEBUG_H

#include "Buffer/Module.h"

#include "Common/Message.h"

#define	_SYDNEY_BUFFER_DEBUG_MESSAGE							\
		_SYDNEY_MESSAGE(										\
			"Buffer_MessageOutputDebug",						\
			Common::MessageStreamBuffer::LEVEL_DEBUG)

#endif	// __SYDNEY_BUFFER_DEBUG_H

//
// Copyright (c) 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
