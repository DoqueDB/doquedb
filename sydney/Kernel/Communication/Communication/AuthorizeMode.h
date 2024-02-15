// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AuthorizeMode.h -- 認証方式
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_AUTHORIZEMODE_H
#define __TRMEISTER_COMMUNICATION_AUTHORIZEMODE_H

#include "Communication/Module.h"

_TRMEISTER_BEGIN
namespace Communication
{
	// 認証方式に関する定数定義
	struct AuthorizeMode
	{
		enum Value
		{
			// 認証なし
			None			= 0,

			// パスワード認証
			Password		= 0x01000000,

			// Mask
			MaskMasterID	= 0x0000FFFF,	// Mask to get master ID
			MaskMode		= 0x0F000000,	// Mask to get authorize mode

			// サポートされている認証方式
			Supported		= Password
		};
	};
}
_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_AUTHORIZEMODE_H
//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
