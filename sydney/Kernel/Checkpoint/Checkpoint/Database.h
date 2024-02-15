// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Database.h --	データベースに関する処理を行うクラス関連の
//					クラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_CHECKPOINT_DATABASE_H
#define	__SYDNEY_CHECKPOINT_DATABASE_H

#include "Checkpoint/Module.h"

#include "Lock/Name.h"
#include "Schema/ObjectID.h"
#include "Trans/TimeStamp.h"

#include "ModAlgorithm.h"
#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_CHECKPOINT_BEGIN

//	NAMESPACE
//	Checkpoint::Database --
//		チェックポイント処理のうち、データベースに関する処理を行うクラス
//
//	NOTES

class Database
{
	friend class Executor;
	friend class LogicalLog;
public:
	//	CLASS
	//	Checkpoint::Database::UnavailableInfo --
	//		利用不可なデータベースに関する情報を表すクラス
	//
	//	NOTES

	struct UnavailableInfo
	{
		// 利用不可になった時点に取得したタイムスタンプ
		Trans::TimeStamp	_t;
		// 利用不可な論理ファイルのスキーマオブジェクトID
		ModVector<Schema::ObjectID::Value>	_files;
	};

	//	TYPEDEF
	//	Checkpoint::Database::UnavailableMap --
	//		利用不可なデータベースの名前とそれを回復するときの
	//		開始時点のタイムスタンプ値を管理するためのマップ
	//
	//	NOTES

	typedef	ModMap<Schema::ObjectID::Value, UnavailableInfo, ModLess<Schema::ObjectID::Value> >	UnavailableMap;

	// 複数のデータベースの利用可能性を設定する
	SYD_CHECKPOINT_FUNCTION
	static void
	setAvailability(const UnavailableMap& map);
	// データベースの利用可能性を設定する
	SYD_CHECKPOINT_FUNCTION
	static bool
	setAvailability(Schema::ObjectID::Value dbID, bool v,
					const Trans::TimeStamp& timestamp = Trans::TimeStamp());
	// 論理ファイルの利用可能性を設定する
	SYD_CHECKPOINT_FUNCTION
	static bool
	setAvailability(Schema::ObjectID::Value dbID,
					Schema::ObjectID::Value fileID, bool v);
	static bool
	setAvailability(const Lock::FileName& lockName, bool v);
	// システムデータベースを含むすべてのデータベースが利用可能か調べる
	SYD_CHECKPOINT_FUNCTION
	static bool
	isAvailable();
	// データベースが利用可能か調べる
	SYD_CHECKPOINT_FUNCTION
	static bool
	isAvailable(Schema::ObjectID::Value dbID);
	// 論理ファイルが利用可能か調べる
	SYD_CHECKPOINT_FUNCTION
	static bool
	isAvailable(Schema::ObjectID::Value dbID,
				Schema::ObjectID::Value fileID);
	static bool
	isAvailable(const Lock::FileName& lockName);

private:
	// 利用不可なデータベースの一覧を得る
	static const UnavailableMap&
	getUnavailable();
	// 利用不可なデータベースを回復するときの開始時点のタイムスタンプを設定する
	static void
	setStartRecoveryTime(Trans::Transaction& trans);
};

//	FUNCTION public
//	Checkpoint::Database::setAvailability -- 論理ファイルの利用可能性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Lock::FileName&		lockName
//			利用可能性を設定する論理ファイルのロック名
//		bool				v
//			true
//				論理ファイルを利用可能にする
//			false
//				論理ファイルを利用不可にする
//
//	RETURN
//		true
//			設定前の論理ファイルは利用可能だった
//		false
//			設定前の論理ファイルは利用不可だった
//
//	EXCEPTIONS

// static
inline
bool
Database::setAvailability(const Lock::FileName& lockName, bool v)
{
	return setAvailability(
		static_cast<Schema::ObjectID::Value>(lockName.getDatabasePart()),
		static_cast<Schema::ObjectID::Value>(lockName.getFilePart()), v);
}

//	FUNCTION public
//	Checkpoint::Database::isAvailable -- 論理ファイルの利用可能性を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Lock::FileName&		lockName
//			利用可能性を調べる論理ファイルのロック名
//
//	RETURN
//		true
//			論理ファイルは利用可能である
//		false
//			論理ファイルは利用不可である
//
//	EXCEPTIONS

// static
inline
bool
Database::isAvailable(const Lock::FileName& lockName)
{
	return isAvailable(
		static_cast<Schema::ObjectID::Value>(lockName.getDatabasePart()),
		static_cast<Schema::ObjectID::Value>(lockName.getFilePart()));
}

_SYDNEY_CHECKPOINT_END
_SYDNEY_END

#endif	// __SYDNEY_CHECKPOINT_DATABASE_H

//
// Copyright (c) 2001, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
