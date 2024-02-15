// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LogData.cpp -- 論理ログデータ関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2014, 2023 Ricoh Company, Ltd.
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

#include "Checkpoint/LogData.h"

#include "Common/Assert.h"

#include "ModArchive.h"

_SYDNEY_USING
_SYDNEY_CHECKPOINT_USING
_SYDNEY_CHECKPOINT_LOG_USING

namespace
{
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointData::serialize -- クラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
CheckpointData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	Data::serialize(archiver);

	// 自分自身をシリアル化する

	if (archiver.isStore())
		archiver << _mostRecent << _synchronized;
	else
		archiver >> _mostRecent >> _synchronized;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::serialize --
//		クラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
CheckpointDatabaseData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	CheckpointData::serialize(archiver);

	// 自分自身をシリアル化する

	if (archiver.isStore()) {
		unsigned int n = _transInfo.getSize();
		archiver << n;

		if (n) {
			ModVector<Trans::Log::LSN> tmp(n);

			for (unsigned int i = 0; i < n; ++i)
				tmp[i] = _transInfo[i]._beginLSN;
			(void) archiver.writeArchive(&*tmp.begin(), n);

			for (unsigned int i = 0; i < n; ++i)
				tmp[i] = _transInfo[i]._lastLSN;
			(void) archiver.writeArchive(&*tmp.begin(), n);

			if (getVersionNumber() >= VersionNumber::Second)
				for (unsigned int i = 0; i < n; ++i)
					archiver << _transInfo[i]._preparedXID;
		}

		if (getVersionNumber() >= VersionNumber::Third)
			archiver << _terminated;
		
	} else {
		unsigned int n;
		archiver >> n;

		if (n) {
			_transInfo.assign(n);
			ModVector<Trans::Log::LSN> tmp(n);

			(void) archiver.readArchive(&*tmp.begin(), n);
			for (unsigned int i = 0; i < n; ++i)
				_transInfo[i]._beginLSN = tmp[i];

			(void) archiver.readArchive(&*tmp.begin(), n);
			for (unsigned int i = 0; i < n; ++i)
				_transInfo[i]._lastLSN = tmp[i];

			if (getVersionNumber() >= VersionNumber::Second)
				for (unsigned int i = 0; i < n; ++i)
					archiver >> _transInfo[i]._preparedXID;
		}
		
		if (getVersionNumber() >= VersionNumber::Third)
			archiver >> _terminated;
	}
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointDatabaseData::getClassID --
//		このクラスのクラス ID を得る
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

int
CheckpointDatabaseData::getClassID() const
{
	; _SYDNEY_ASSERT(getVersionNumber() >= VersionNumber::First &&
					 getVersionNumber() <= VersionNumber::Third);

	const Checkpoint::Externalizable::Category::Value category[] =
	{
		Checkpoint::Externalizable::Category::CheckpointDatabaseLogData_0,
		Checkpoint::Externalizable::Category::CheckpointDatabaseLogData_1,
		Checkpoint::Externalizable::Category::CheckpointDatabaseLogData_2
	};

	return category[getVersionNumber()] +
		Common::Externalizable::CheckpointClasses;
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::serialize --
//		クラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
CheckpointSystemData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	CheckpointData::serialize(archiver);

	// 自分自身をシリアル化する

	unsigned int n;

	if (archiver.isStore()) {
		n = _unavailableDatabase.getSize();

		archiver << _terminated << _unavailable << n;

		Database::UnavailableMap::Iterator	ite(_unavailableDatabase.begin());
		const Database::UnavailableMap::Iterator&
			end = _unavailableDatabase.end();

		for (; ite != end; ++ite)
			archiver << (*ite).first << (*ite).second._t;

		if (getVersionNumber() >= VersionNumber::Second) {
			n = _heurCompletionInfo.getSize();

			archiver << n;

			for (unsigned int i = 0; i < n; ++i) {
				archiver << _heurCompletionInfo[i]._id;
				{
				int tmp = _heurCompletionInfo[i]._decision;
				archiver << tmp;
				}
			}
		}
	} else {
		archiver >> _terminated >> _unavailable >> n;

		for (unsigned int i = 0; i < n; ++i) {
			Database::UnavailableMap::ValueType	pair;

			archiver >> pair.first >> pair.second._t;

			(void) _unavailableDatabase.insert(pair);
		}

		if (getVersionNumber() >= VersionNumber::Second) {
			archiver >> n;

			for (unsigned int i = 0; i < n; ++i) {
				Trans::Branch::HeurCompletionInfo elm;

				archiver >> elm._id;
				{
				int tmp;
				archiver >> tmp;
				elm._decision =
					static_cast<Trans::Branch::HeurDecision::Value>(tmp);
				}
				
				_heurCompletionInfo.pushBack(elm);
			}
		}
	}
}

//	FUNCTION public
//	Checkpoint::Log::CheckpointSystemData::getClassID --
//		このクラスのクラス ID を得る
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

int
CheckpointSystemData::getClassID() const
{
	; _SYDNEY_ASSERT(getVersionNumber() >= VersionNumber::First &&
					 getVersionNumber() <= VersionNumber::Second);

	const Checkpoint::Externalizable::Category::Value category[] =
	{
		Checkpoint::Externalizable::Category::CheckpointSystemLogData_0,
		Checkpoint::Externalizable::Category::CheckpointSystemLogData_1
	};

	return category[getVersionNumber()] +
		Common::Externalizable::CheckpointClasses;
}

//	FUNCTION public
//	Checkpoint::Log::FileSynchronizeEndData::serialize --
//		クラスをシリアル化する
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
FileSynchronizeEndData::serialize(ModArchive& archiver)
{
	// まず、親クラスをシリアル化する

	ModificationData::serialize(archiver);

	// 自分自身をシリアル化する

	archiver(_modified);
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2007, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
