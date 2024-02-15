// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Thread.h --
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

#ifndef __SYDNEY_SERVER_THREAD_H
#define __SYDNEY_SERVER_THREAD_H

#include "Server/Module.h"
#include "Server/Type.h"
#include "Common/Thread.h"
#ifdef OBSOLETE
#include "Os/Event.h"
#endif

_SYDNEY_BEGIN
_SYDNEY_SERVER_BEGIN

//
//	CLASS
//	Server::Thread --
//
//	NOTES
//
//
class Thread : public Common::Thread
{
public:
	//コンストラクタ
	Thread();
	//デストラクタ
	virtual ~Thread();

	//スレッドを停止する
	virtual void stop() = 0;
	//IDを得る
	virtual ID getID() const = 0;

#ifdef OBSOLETE
	//待機イベントがシグナル化されるまで待つ
	void waitEvent();
	//待機イベントをシグナル化する
	void setEvent();

private:
	//待機イベント
	Os::Event m_cEvent;
#endif
};

_SYDNEY_SERVER_END
_SYDNEY_END

#endif //__SYDNEY_SERVER_THREAD_H

//
//	Copyright (c) 2002, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
