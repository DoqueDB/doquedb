// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Monitor.cpp -- タイムアウト機能を実現するためのモニタクラス
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "SydTest";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "ModOsDriver.h"
#include "ModAutoMutex.h"
#include "SydTest/Monitor.h"
#include "SydTest/SydTestException.h"
#include "SydTest/SydTestMessage.h"

#include "Common/SystemParameter.h"
#include "Exception/Object.h"

_SYDNEY_USING

ModCriticalSection SydTest::Monitor::m_cCS;
ModConditionVariable SydTest::Monitor::m_cCV;

using namespace SydTest;

//
//	FUNCTION public
//	SydTest::Monitor::Monitor -- コンストラクタ
//
//	NOTES
//	コンストラクタ。
//
//	ARGUMENTS
//	  const int iTimeout_
//      タイムアウトを秒数で指定する
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	なし
//
Monitor::Monitor()
: m_iTimeout(Common::SystemParameter::getInteger("SydTest_Timeout")),
  m_eStatus(Timeout)
{
	if (isSydTestDebugMessage())
	{
		SydTestDebugMessage << "Timeout = " << m_iTimeout 
			<< " seconds." << ModEndl;
	}
	if (m_iTimeout != 0)
		this->create();
}

//
//	FUNCTION public
//	SydTest::Monitor::~Monitor -- デストラクタ
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
Monitor::~Monitor()
{
//	this->terminate();
}

//
//	FUNCTION public
//	SydTest::Monitor::runnable -- スレッドのための実行関数
//
//	NOTES
//	スレッドのための実行関数
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
void
Monitor::runnable()
{
	while (m_iTimeout!=0) { // Timeoutがなければ何もしない
		// ↓引数がintのときは秒とみなされる
		m_cCV.wait(m_iTimeout);
		{
			ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
			cAuto.lock();
			switch(m_eStatus) {

			case Reset:
				m_eStatus=Timeout;
				break;
			case Timeout:
				if (getStatus() < Common::Thread::Aborting) {
					SydTestErrorMessage << "Timeout!" << ModEndl;
				}
				ModOsDriver::Process::abort();
				break;
			case Terminate:
				cAuto.unlock(); // なぜか明示的にunlockしないとだめらしい
				return;
			}
		}
	}
}

//
//	FUNCTION public
//	SydTest::Monitor::reset -- モニタをリセットする
//
//	NOTES
//	モニタをリセットする
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
void
Monitor::reset()
{
	// Timeoutがなければ何もしない
	if (m_iTimeout==0) return;

	{
		ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
		cAuto.lock();
		if (m_eStatus == Timeout) {
			m_eStatus = Reset;
		}
	}
	m_cCV.signal();
}

//
//	FUNCTION public
//	SydTest::Monitor::terminate -- モニタを終了させる
//
//	NOTES
//	モニタを終了させる
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
void
Monitor::terminate()
{
	// Timeoutがなければ何もしない
	if (m_iTimeout==0) return;

	{
		ModAutoMutex<ModCriticalSection> cAuto(&m_cCS);
		cAuto.lock();
		m_eStatus = Terminate;
	}
	m_cCV.signal();
	this->join();
}

//
//	Copyright (c) 2001, 2002, 2003, 2004, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
