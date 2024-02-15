// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Protocol.h -- プロトコルバージョンに関する定数定義
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

#ifndef __TRMEISTER_COMMUNICATION_PROTOCOL_H
#define __TRMEISTER_COMMUNICATION_PROTOCOL_H

#include "Communication/Module.h"

_TRMEISTER_BEGIN
namespace Communication
{
	// プロトコルバージョン
	struct Protocol
	{
		typedef unsigned int Value;
		enum
		{
			Version1 = 0,	// v14 互換モード
			Version2,		// v15 初期バージョン
			Version3,		// v15 互換モード
			Version4,		// HasMoreData対応
			Version5,		// getDatabaseProductVersion対応

			ValueNum,
			CurrentVersion = ValueNum - 1
		};
	};

} // namespace Communication
_TRMEISTER_END

#endif // __TRMEISTER_COMMUNICATION_PROTOCOL_H

//
//	Copyright (c) 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
