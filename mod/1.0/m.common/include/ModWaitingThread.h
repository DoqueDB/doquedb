// -*-Mode: C++; tab-width: 4;-*-
// vi:set ts=4 sw=4:	
//
// ModWaitingThread.h -- ModWaitingThread のクラス定義
// 
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__ModWaitingThread_H__
#define __ModWaitingThread_H__

#include "ModCommonDLL.h"
#include "ModThread.h"

class ModSyncBase;

//【注意】	ライブラリ外に公開しないクラスなので dllexport しない

class ModWaitingThread
{
public:
	ModWaitingThread();

	ModSyncBase* waitingTarget;
	ModThreadId threadId;
	int count;
};

inline
ModWaitingThread::ModWaitingThread() :
	waitingTarget(0), count(0)
{
	this->threadId = ModThisThread::self();
}

#endif	// __ModWaitingThread_H__

//
// Copyright (c) 1998, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
