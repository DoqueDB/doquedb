// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TimeStamp.h --	タイムスタンプに関する処理を行うクラス関連の
//					クラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_TIMESTAMP_H
#define	__SYDNEY_CHECKPOINT_TIMESTAMP_H

#include "Checkpoint/Module.h"

_SYDNEY_BEGIN

namespace Lock
{
	class FileName;
	class LogicalLogName;
}
namespace Trans
{
	class TimeStamp;
}

_SYDNEY_CHECKPOINT_BEGIN

//	CLASS
//	Checkpoint::TimeStamp --
//		チェックポイント処理のうち、タイムスタンプに関する処理を行うクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class TimeStamp
{
	friend class Executor;
public:
	// 前回のチェックポイント処理が終了したときのタイムスタンプを得る
	SYD_CHECKPOINT_FUNCTION
	static const Trans::TimeStamp&	getMostRecent();
	// あるデータベースに関する
	// 前回のチェックポイント処理が終了したときのタイムスタンプを得る
	SYD_CHECKPOINT_FUNCTION
	static const Trans::TimeStamp&
	getMostRecent(const Lock::FileName& lockName);
	SYD_CHECKPOINT_FUNCTION
	static const Trans::TimeStamp&
	getMostRecent(const Lock::LogicalLogName& lockName);

	// 前々回のチェックポイント処理が終了したときのタイムスタンプを得る	
	SYD_CHECKPOINT_FUNCTION
	static const Trans::TimeStamp&	getSecondMostRecent();
	// あるデータベースに関する
	// 前々回のチェックポイント処理が終了したときのタイムスタンプを得る	
	SYD_CHECKPOINT_FUNCTION
	static const Trans::TimeStamp&
	getSecondMostRecent(const Lock::FileName& lockName);
	SYD_CHECKPOINT_FUNCTION
	static const Trans::TimeStamp&
	getSecondMostRecent(const Lock::LogicalLogName& lockName);

	// あるデータベースに関する
	// チェックポイント処理が終了したときのタイムスタンプを新たに設定する
	SYD_CHECKPOINT_FUNCTION
	static void
	assign(const Lock::LogicalLogName& lockName,
		   const Trans::TimeStamp& v, bool synchronized);

private:
	// チェックポイント処理が終了したときのタイムスタンプを新たに設定する
	static void
	assign(const Trans::TimeStamp& v, bool synchronized);
};

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_TIMESTAMP_H

//
// Copyright (c) 2000, 2001, 2002, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
