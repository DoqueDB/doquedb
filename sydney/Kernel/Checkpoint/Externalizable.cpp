// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Externalizable.cpp -- シリアル化可能なオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2007, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Checkpoint";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Checkpoint/Externalizable.h"
#include "Checkpoint/LogData.h"
#include "Checkpoint/Manager.h"

#include "Common/Assert.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING

namespace
{
}

//	FUNCTION private
//	Checkpoint::Manager::Externalizable::initialize --
//		マネージャーのうち、シリアル化可能なオブジェクト関連を初期化する
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

// static
void
Manager::Externalizable::initialize()
{
	// チェックポイント処理関連のシリアル化可能なオブジェクトを
	// 確保するための関数を共通ライブラリに登録する

	Common::Externalizable::insertFunction(
		Common::Externalizable::CheckpointClasses,
			Checkpoint::Externalizable::getClassInstance);
}

//	FUNTION private
//	Checkpoint::Manager::Externalizable::terminate --
//		マネージャーのうち、シリアル可能なオブジェクト関連を終了処理する
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

// static
void
Manager::Externalizable::terminate()
{}

//	FUNCTION public
//	Checkpoint::Externalizable::getClassInstance --
//		指定された種別のシリアル化可能なオブジェクトを確保する
//
//	NOTES
//
//	ARGUMENTS
//		int					classID
//			確保するオブジェクトの種別を表す値
//
//	RETURN
//		確保したシリアル化可能なオブジェクトを格納する領域の先頭アドレス
//
//	EXCEPTIONS

// static
Common::Externalizable*
Externalizable::
getClassInstance(int classID)
{
	; _SYDNEY_ASSERT(classID >= Common::Externalizable::CheckpointClasses);

	Common::Externalizable*	object = 0;

	Externalizable::Category::Value	category =
		static_cast<Externalizable::Category::Value>(
			classID - Common::Externalizable::CheckpointClasses);
		
	switch (category) {
	case Category::CheckpointLogData:
		object = new Log::CheckpointData();				break;
	case Category::CheckpointDatabaseLogData_0:
		object = new Log::CheckpointDatabaseData(
			Trans::Log::Data::VersionNumber::First);	break;
	case Category::CheckpointDatabaseLogData_1:
		object = new Log::CheckpointDatabaseData(
			Trans::Log::Data::VersionNumber::Second);	break;
	case Category::CheckpointDatabaseLogData_2:
		object = new Log::CheckpointDatabaseData(
			Trans::Log::Data::VersionNumber::Third);	break;
	case Category::CheckpointSystemLogData_0:
		object = new Log::CheckpointSystemData(
			Trans::Log::Data::VersionNumber::First);	break;
	case Category::CheckpointSystemLogData_1:
		object = new Log::CheckpointSystemData(
			Trans::Log::Data::VersionNumber::Second);	break;
	case Category::FileSynchronizeBeginLogData:
		object = new Log::FileSynchronizeBeginData();	break;
	case Category::FileSynchronizeEndLogData:
		object = new Log::FileSynchronizeEndData();		break;
	default:
		; _SYDNEY_ASSERT(false);
	}

	return object;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2007, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
