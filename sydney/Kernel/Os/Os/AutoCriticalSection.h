// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoCriticalSection.h --
//		自動クリティカルセクション関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
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

#ifndef	__TRMEISTER_OS_AUTOCRITICALSECTION_H
#define	__TRMEISTER_OS_AUTOCRITICALSECTION_H

#include "Os/Module.h"
#include "Os/AutoSynchronization.h"
#include "Os/CriticalSection.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	TYPEDEF
//	Os::AutoCriticalSection --
//		オブジェクト生成時に自動的にクリティカルセクションをロックし、
//		破棄時に自動的にロックをはずすクラス
//
//	NOTES

typedef	AutoSynchronization<CriticalSection>	AutoCriticalSection;

//	TYPEDEF
//	Os::AutoTryCriticalSection --
//		オブジェクト生成時に自動的にクリティカルセクションのロックを試み、
//		破棄時にロックしてれば、自動的にはずすクラス
//
//	NOTES

typedef	AutoTrySynchronization<CriticalSection>	AutoTryCriticalSection;

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_AUTOCRITICALSECTION_H

//
// Copyright (c) 2000, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

