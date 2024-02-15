// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Manager.h -- 共通ライブラリマネージャー関連のクラス定義、関数宣言
// 
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
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

#ifndef __TRMEISTER_COMMON_MANAGER_H
#define	__TRMEISTER_COMMON_MANAGER_H

#include "Common/Module.h"

class ModUnicodeString;

_TRMEISTER_BEGIN
_TRMEISTER_COMMON_BEGIN

//	NAMESPACE
//	Common::Manager -- 共通ライブラリマネージャー全体の管理を行う名前空間
//
//	NOTES

namespace Manager
{
	// 初期化を行う
	SYD_COMMON_FUNCTION
	void
	initialize(const ModUnicodeString& regParent);
	// 後処理を行う
	SYD_COMMON_FUNCTION
	void
	terminate();
}

_TRMEISTER_COMMON_END
_TRMEISTER_END

#endif	// __TRMEISTER_COMMON_MANAGER_H

//
// Copyright (c) 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

