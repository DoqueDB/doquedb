// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogicalLog.h --	論理ログファイルに関する処理を行うクラス関連の
//					クラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_LOGICALLOG_H
#define	__SYDNEY_CHECKPOINT_LOGICALLOG_H

#include "Checkpoint/Module.h"

_SYDNEY_BEGIN

namespace Trans
{
	namespace Log
	{
		class File;
	}
}

_SYDNEY_CHECKPOINT_BEGIN

//	CLASS
//	Checkpoint::LogicalLog --
//		チェックポイント処理のうち、論理ログファイルに関する処理を行うクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class LogicalLog
{
	friend class Executor;
private:
	// 既存の論理ログファイルのうち、
	// 必要なものにチェックポイント処理の終了を表す論理ログを記録する
	static void
	store(bool persisted, bool terminating);
	// ある論理ログファイルに
	// チェックポイント処理の終了を表す論理ログを必要であれば記録する
	static bool
	store(Trans::Log::File& logFile, bool persisted, bool terminating);
};

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_LOGICALLOG_H

//
// Copyright (c) 2001, 2002, 2004, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
