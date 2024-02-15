// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Monitor.h -- スクリプトの実行をタイムアウトさせるためのモニタクラス
// 
// Copyright (c) 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_MONITOR_H
#define __SYDNEY_SYDTEST_MONITOR_H

#include "Common/Common.h"
#include "Common/Internal.h"
#include "Common/Thread.h"
#include "ModConditionVariable.h"
#include "ModCriticalSection.h"

_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Monitor -- スクリプトの実行をタイムアウトさせるためのモニタクラス
//
//	NOTES
//
//
class Monitor : public Common::Thread
{
public:

	explicit Monitor(); //コンストラクタ
	virtual ~Monitor(); //デストラクタ
	void reset();
	void terminate();

    enum Status {
	  Timeout=0, 
	  Reset,
	  Terminate // SydTestを安全に終了させるために必要
	};

private:
	void runnable();

	static ModCriticalSection m_cCS;
	static ModConditionVariable m_cCV;
	const int m_iTimeout;
	Status m_eStatus;
};

}

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_MONITOR_H

//
//	Copyright (c) 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
