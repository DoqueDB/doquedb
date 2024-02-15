// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StatisticsReporter.h -- 統計情報出力スレッドに関するクラス定義、関数宣言
// 
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_BUFFER_STATISTICSREPORTER_H
#define	__SYDNEY_BUFFER_STATISTICSREPORTER_H

#include "Buffer/Module.h"
#include "Buffer/DaemonThread.h"

_SYDNEY_BEGIN
_SYDNEY_BUFFER_BEGIN

//	CLASS
//	Buffer::StatisticsReporter -- 使用済領域解放スレッドを表すクラス
//
//	NOTES
//		使用済領域解放スレッドとは、
//		バッファプールが使用されなくなった期間に応じて、
//		使用済の領域を解放していく常駐型スレッドである

class StatisticsReporter
	: public	DaemonThread
{
public:
	// コンストラクター
	StatisticsReporter(unsigned int timeout);
	// デストラクター
	~StatisticsReporter();

private:
	// スレッドが繰り返し実行する関数
	void
	repeatable();
};

//	FUNCTION public
//	Buffer::StatisticsReporter::~StatisticsReporter --
//		デストラクター
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
StatisticsReporter::~StatisticsReporter()
{}

_SYDNEY_BUFFER_END
_SYDNEY_END

#endif	// __SYDNEY_BUFFER_STATISTICSREPORTER_H

//
// Copyright (c) 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
