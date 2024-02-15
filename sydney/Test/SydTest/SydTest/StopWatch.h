// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StopWatch.h -- 排他制御つきの計時クラス
// 
// Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_SYDTEST_STOPWATCH_H
#define __SYDNEY_SYDTEST_STOPWATCH_H

#include "Common/Internal.h"
#include "ModTime.h"
#include "ModTimeSpan.h"
#include "Map.h"
#include "ModCriticalSection.h"

_SYDNEY_BEGIN

namespace SydTest
{

//
//	CLASS
//	SydTest::Watch -- StopWatchの補助クラス
//
//	NOTES
//
//
struct _Watch
{
	_Watch() {}; // mapの構成上必要
	ModTimeSpan dLap;
	ModTime     dStart;
};

//
//	CLASS
//	SydTest::StopWatch -- 排他制御つきの計時クラス
//
//	NOTES
//
//
class StopWatch
{
public:	
	bool start(ModUnicodeString pszLabel_);
	bool stop(ModUnicodeString pszLabel_);
	const ModTimeSpan showTotalTime(ModUnicodeString pszLabel_);
	const ModTimeSpan showCurrentLapTime(ModUnicodeString pszLabel_);
	void reset(ModUnicodeString pszLabel_);
	bool clear(ModUnicodeString pszLabel_);

	bool startAll();
	bool stopAll();
#ifdef DEBUG
	void showAllWatches();
#endif

private:
	Map<ModUnicodeString, _Watch> m_cMap;
	ModCriticalSection m_cCS;
};

} // end of namespace Sydtest

_SYDNEY_END

#endif //__SYDNEY_SYDTEST_STOPWATCH_H

//
//	Copyright (c) 2000, 2001, 2002, 2007, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
