// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AutoSemaphore.h --
//		自動セマフォ関連のクラス定義、関数宣言
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

#ifndef	__TRMEISTER_OS_AUTOSEMAPHORE_H
#define	__TRMEISTER_OS_AUTOSEMAPHORE_H

#include "Os/Module.h"
#include "Os/AutoSynchronization.h"
#include "Os/Semaphore.h"

_TRMEISTER_BEGIN
_TRMEISTER_OS_BEGIN

//	TYPEDEF
//	Os::AutoSemaphore --
//		オブジェクトの生成時に自動的にセマフォの値を 1 減らし、
//	　	破棄時に自動的に値を 1 増やすクラス
//
//	NOTES

typedef	AutoSynchronization<Semaphore>		AutoSemaphore;

//	TYPEDEF
//	Os::AutoTrySemaphore --
//		オブジェクトの生成時に自動的にセマフォの値が 0 より大きいか調べ、
//	　	破棄時に値を 1 減らしていれば、自動的に 1 増やすクラス
//
//	NOTES

typedef	AutoTrySynchronization<Semaphore>	AutoTrySemaphore;

_TRMEISTER_OS_END
_TRMEISTER_END

#endif	// __TRMEISTER_OS_AUTOSEMAPHORE_H

//
// Copyright (c) 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
