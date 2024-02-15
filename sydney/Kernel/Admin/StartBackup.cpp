// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// StartBackup.cpp -- バックアップの開始関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2013, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Admin/StartBackup.h"
#include "Admin/Operation.h"
#include "Admin/Utility.h"

#include "Checkpoint/Daemon.h"
#include "Checkpoint/TimeStamp.h"
#include "Common/UnsignedInteger64Data.h"
#include "Exception/AlreadyStartBackup.h"
#include "Exception/BadTransaction.h"
#include "Exception/DatabaseChanged.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/NotSupported.h"
#include "Schema/Database.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Server/Session.h"
#include "Trans/AutoLatch.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

namespace
{
}

//	FUNCTION public
//	Admin::StartBackup::execute -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Statement::Object*	stmt
//			START BACKUP 文を表すオブジェクト
//		Utility::PathList&	pathList
//			バックアップを開始するデータベースの実体が
//			格納されているディレクトリのリストを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
StartBackup::execute(const Statement::StartBackupStatement* stmt,
					 Utility::PathList& pathList)
{
	; _SYDNEY_ASSERT(stmt);

	// 初期化を行う

	initialize(*stmt);

	// メタデータベース、データベース表と
	// バックアップするデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Schema::Database* database = Schema::Database::getLocked(
		_trans, _dbName,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport);

	if (!database)

		// 指定されたデータベース名が取得されてから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, _dbName);

	if (database->getID() != m_pSession->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		m_pSession->setDatabaseInfo(database->getID(), database->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// トランザクションで操作するデータベースを設定する
	//
	//【注意】	すぐに論理ログを記録しなくても、
	//			スキーマ操作関数で論理ログファイルを
	//			参照しに行く可能性があるので、
	//			操作対象のデータベースのスキーマ情報を取得したら、
	//			すぐトランザクションに設定すること

	_trans.setLog(*database);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*database);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(_trans, *database, stmt, m_pSession);

	if (!_discarding && !database->isReadOnly() &&
		(_trans.isNoVersion() ||
		 _trans.getIsolationLevel() !=
		 Trans::Transaction::IsolationLevel::Serializable))

		// DISCARD SNAPSHOT を指定せずに READ WRITE なデータベースを
		// バックアップするとき、バックアップを開始する
		// トランザクションが版管理するトランザクションで、
		// アイソレーションレベルが SERIALIZABLE である必要がある

		_SYDNEY_THROW0(Exception::BadTransaction);

	if (!database->isReadOnly())

		// バックアップするデータベースが READ ONLY でなければ、
		// 論理ログを記録するために、
		// データベース用の論理ログファイルをロックする

		Schema::Manager::SystemTable::hold(
			_trans, Schema::Hold::Target::LogicalLog,
			Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadWrite,
			0, Trans::Log::File::Category::Database, database);

	if (!_trans.isNoVersion())

		// 版管理するトランザクションがバックアップを開始するとき、
		// バックアップするデータベースを
		// データベースを操作するためにロックする

		Schema::Manager::SystemTable::hold(
			_trans, Schema::Hold::Target::Database,
			Lock::Name::Category::Database, Schema::Hold::Operation::ReadOnly,
			database->getID());

	// バックアップするデータベースに定義されている
	// エリアのディレクトリ、データ格納ディレクトリ、
	// ログ格納ディレクトリ、システム表格納ディレクトリを求める

	Utility::makePathList(_trans, *database, pathList);

	// バックアップ中はチェックポイント処理
	// およびバージョンファイルの同期処理を禁止する

	Checkpoint::Daemon::disable();

	bool bNeedToEnd = false;
	try {
		// バックアップするデータベースに対して、
		// バックアップの開始を指示する

		database->startBackup(_trans, !stmt->isRecovery());
		bNeedToEnd = true;

		// トランザクションが指定されたデータベースの
		// バックアップを開始することを記憶しておく

		if (!Utility::Backup::enter(_dbName, _trans))

			// バックアップしようとしたデータベースは、
			// すでに他のトランザクションがバックアップ中である

			_SYDNEY_THROW1(Exception::AlreadyStartBackup, _dbName);

		// バックアップの開始を表す論理ログを記録する

		writeLog(*database);

	} catch (...) {

		if (bNeedToEnd) {
			try {
				database->endBackup(_trans);
			} catch (...) {
				// データベースはすでに利用不可になっているはず
				// なので何もしない
			}
		}

		Checkpoint::Daemon::enable();

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Admin::StartBackup::initialize -- 初期化する
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

void
StartBackup::initialize(const Statement::StartBackupStatement& stmt)
{
	// どのようなバックアップを行うか

	_type = static_cast<
		Statement::StartBackupStatement::BackupType>(stmt.getType());

	if (_type == Statement::StartBackupStatement::Unknown)
		// 未指定はFULLを意味する
		_type = Statement::StartBackupStatement::Full;

	else if (_type != Statement::StartBackupStatement::Full)

		// 現在は全バックアップしかできない

		_SYDNEY_THROW0(Exception::NotSupported);
	
	// DISCARD SNAPSHOT が指定されたか

	_discarding =
		(static_cast<
		 Statement::StartBackupStatement::VersionType>(stmt.getVersion()) ==
		 Statement::StartBackupStatement::DiscardSnapshot);
}

//	FUNCTION private
//	Admin::StartBackup::writeLog -- バックアップの開始を表す論理ログを記録する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			バックアップの対象であるデータベースのスキーマオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
StartBackup::writeLog(Schema::Database& database)
{
	if (!database.isReadOnly()) {

		// READ ONLY でマウントされたデータベースでなければ、論理ログを記録する

		// まず、論理ログデータを生成する

		Schema::LogData logData(Schema::LogData::Category::StartBackup);

		// 前回と前々回のチェックポイント処理終了時のタイムスタンプを記録する

		const Lock::LogicalLogName& lockName = _trans.getLogInfo(
			Trans::Log::File::Category::Database).getLockName();

		logData.addData(new Common::UnsignedInteger64Data(
						Checkpoint::TimeStamp::getMostRecent(lockName)));
		logData.addData(new Common::UnsignedInteger64Data(
						Checkpoint::TimeStamp::getSecondMostRecent(lockName)));

		// DISCARD SNAPSHOT が指定されていない、
		// すなわち、バックアップを開始したトランザクションが
		// 参照する版にリストア可能にするときは、
		// バックアップを開始したトランザクションの
		// 開始時のタイムスタンプを記録する
		//
		//【注意】	DISCARD SNAPSHOT が指定されているとき、
		//			不正なタイムスタンプを記録する

		logData.addData(new Common::UnsignedInteger64Data((_discarding) ?
						Trans::IllegalTimeStamp : _trans.getBirthTimeStamp()));

		// 生成した論理ログをバックアップする
		// データベース用の論理ログファイルに追加する

		Trans::AutoLatch latch(_trans, lockName);
		_trans.storeLog(Trans::Log::File::Category::Database, logData);
		_trans.flushLog(Trans::Log::File::Category::Database);
	}
}

//
//	Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2013, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
