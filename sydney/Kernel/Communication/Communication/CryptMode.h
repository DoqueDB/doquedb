// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// CryptMode.h -- 暗号モード用
// 
// Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMUNICATION_CRYPTMODE_H
#define __TRMEISTER_COMMUNICATION_CRYPTMODE_H

#include "Communication/Module.h"

_TRMEISTER_BEGIN
namespace Communication
{
	// 暗号アルゴリズム
	struct CryptMode
	{
		enum Value
		{
			// なし
			Unknown			= 0,

			// Mask
			MaskMasterID	= 0x0000FFFF,	// マスターID取得用
			MaskAlgorithm	= 0x00FF0000,	// 暗号アルゴリズム取得用
		};
	};
}
_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_CRYPTMODE_H
//
//	Copyright (c) 2006, 2008, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
