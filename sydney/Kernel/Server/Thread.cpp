// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Thread.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Server";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "Server/Thread.h"
#ifdef OBSOLETE
#include "Os/Event.h"
#endif

_SYDNEY_USING
_SYDNEY_SERVER_USING

//
//	FUNCTION public
//	Server::Thread::Thread -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Thread::Thread()
{
}

//
//	FUNCTION public
//	Server::Thread::~Thread -- デストラクタ
//
//	NOTES
//	デストラクタ。
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Thread::~Thread()
{
}

#ifdef OBSOLETE
//
//	FUNCTION public
//	Server::Thread::waitEvent -- 待機イベントがシグナル化されるまで待つ
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Thread::waitEvent()
{
	m_cEvent.wait();
}

//
//	FUNCTION public
//	Server::Thread::setEvent -- 待機イベントをシグナル化する
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Thread::setEvent()
{
	m_cEvent.set();
}
#endif

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
