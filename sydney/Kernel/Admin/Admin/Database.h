// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Database.h -- データベースの回復関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_ADMIN_DATABASE_H
#define	__SYDNEY_ADMIN_DATABASE_H

#include "Admin/Module.h"
#include "Admin/Mount.h"

#include "Checkpoint/Database.h"
#include "Trans/LogFile.h"

#include "ModAutoPointer.h"
#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Database;
}
namespace Trans
{
	namespace Log
	{
		class Data;
	}

	class Transaction;
	class TimeStamp;
}

_SYDNEY_ADMIN_BEGIN
_SYDNEY_ADMIN_RECOVERY_BEGIN

//	CLASS
//	Admin::Recovery::Database -- データベースの回復処理を行うクラス
//
//	NOTES

class Database
	: public	Common::Object
{
public:
	// コンストラクター
	Database(Schema::Database& database);
	Database(Schema::Database& database, const Trans::TimeStamp& starting);
	// デストラクター
	~Database();

	// スキーマデータベースの回復処理の基点となる
	// 論理ログのログシーケンス番号を求める
	SYD_ADMIN_FUNCTION
	bool
	findStartLSN(Trans::Transaction& trans,
				 Checkpoint::Database::UnavailableMap& unavailableDatabase);
	// 通常のデータベースの回復処理の基点となる
	// 論理ログのログシーケンス番号を求める
	SYD_ADMIN_FUNCTION
	void
	findStartLSN(Trans::Transaction& trans);
	// データベースのマウント時の回復処理の基点となる
	// 論理ログのログシーケンス番号を求める
	SYD_ADMIN_FUNCTION
	void
	findStartLSN(Trans::Transaction& trans, Mount::Option::Value option,
				 Boolean::Value& unmounted, Boolean::Value& backup,
				 Trans::TimeStamp& mostRecent);

	// データベースの回復処理として、すべての更新操作を UNDO する
	SYD_ADMIN_FUNCTION
	void
	undoAll(Trans::Transaction& trans);
	// データベースの回復処理として、すべての更新操作を REDO する
	SYD_ADMIN_FUNCTION
	void
	redoAll(Trans::Transaction& trans);
	// データベースの回復処理として、更新操作を REDO する
	SYD_ADMIN_FUNCTION
	Database*
	redo(Trans::Transaction& trans);
	// データベースの回復処理として、更新操作の REDO をすべて終えたか
	bool
	isRedone() const;

	// 回復するデータベースに対する更新を表す論理ログが
	// 記録されている論理ログファイルを操作するための情報
	Schema::Database&		_database;
	// REDO する操作を表す論理ログのログシーケンス番号
	Trans::Log::LSN			_lsn;
	// REDO する操作を表す論理ログのログデータ
	ModAutoPointer<const Trans::Log::Data>	_logData;

	// 回復を開始する時点のタイムスタンプ
	Trans::TimeStamp		_starting;
	// 回復処理の開始時点にリカバリ済である
	bool					_recovered;
	// 初めて処理しようとしているか
	bool					_first;
	// ロールフォワードリカバリか
	bool					_rollforward;
	// マウントか
	bool					_mount;

	// 回復するデータベースを更新したトランザクションのうち、
	// 回復処理を開始する時点に実行中でその後コミットされないものの
	// 回復処理を開始する時点の直前に記録した論理ログのログシーケンス番号
	ModVector<Trans::Log::LSN>	_undoLSN;
	// 回復するデータベースを更新したトランザクションのうち、
	// 回復処理を開始する時点以降に実行された
	// UNDO 不能な操作を表す論理ログのログシーケンス番号
	ModVector<Trans::Log::LSN>	_redoLSN;
	// 回復するデータベースを更新したトランザクションのうち、
	// 回復処理を開始する時点以降にコミットされたものの
	// 開始を表す論理ログのログシーケンス番号
	ModVector<Trans::Log::LSN>	_committedLSN;
	// 回復するデータベースを更新したトランザクションのうち、
	// 回復処理を開始する時点以降にコミットされたもので実行され、
	// ロールバックされた SQL 文の直前に実行された
	// SQL 文の終了を表す論理ログのログシーケンス番号
	ModVector<Trans::Log::LSN>	_noRedoLSN;
	// 回復するデータベースを更新したトランザクションのうち、
	// 回復処理を開始する時点以降にコミットまたはロールバックされたものの
	// 開始を表す論理ログのログシーケンス番号
	ModVector<Trans::Log::LSN>	_finishedLSN;
	
	// 回復するデータベースを更新したトランザクションのうち、
	// 実行中のものの開始を表す論理ログのログシーケンス番号
	ModVector<Trans::Log::LSN>	_runningLSN;
	// 最後にマスターから受け取った論理ログのログシーケンス番号
	Trans::Log::LSN				_lastMasterLSN;

	// 非同期レプリケーションのスレーブデータベースか
	bool					_slave;
};

//	FUNCTION public
//	Admin::Recovery::Database::isRedone --
//		データベースの回復処理として、更新操作の REDO をすべて終えたか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			終えた
//		false
//			終えていない
//
//	EXCEPTIONS
//		なし

inline
bool
Database::isRedone() const
{
	return _committedLSN.isEmpty() && _redoLSN.isEmpty();
}

_SYDNEY_ADMIN_RECOVERY_END
_SYDNEY_ADMIN_END
_SYDNEY_END

#endif	// __SYDNEY_ADMIN_DATABASE_H

//
// Copyright (c) 2001, 2002, 2003, 2004, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
