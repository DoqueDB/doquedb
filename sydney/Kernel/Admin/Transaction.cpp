// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.cpp -- トランザクションの回復関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

namespace
{
const char srcFile[] = __FILE__;
const char moduleName[] = "Admin";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#ifndef SYD_COVERAGE
#include "Admin/Debug.h"
#endif
#include "Admin/Transaction.h"
#include "Admin/Utility.h"

#include "Common/Assert.h"

#include "Execution/Executor.h"
#include "Execution/Program.h"
#include "Opt/Optimizer.h"

#include "Schema/Database.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"

#include "LogicalFile/FileDriverManager.h"
#include "LogicalFile/LogData.h"

#include "Trans/Transaction.h"

#include "Exception/Object.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING
_SYDNEY_ADMIN_RECOVERY_USING

namespace
{
}

//	FUNCTION private
//	Admin::Recovery::Transaction::undoTuple --
//		タプルに対する更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルに対する更新操作を UNDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を UNDO するための論理ログデータ
//		ModUnicodeString&	dbName
//			更新されたタプルを保持する表が存在するデータベースの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::undoTuple(Trans::Transaction& trans,
								 const Trans::Log::Data& logData,
								 const ModUnicodeString& dbName)
{
	// UNDO する操作の対象であるデータベースのスキーマ情報を取得する

	Schema::Database* database = Schema::Database::get(
		Schema::Database::getID(dbName, trans), trans);
	; _SYDNEY_ASSERT(database);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*database);

	// 実際に操作を UNDO する

	undoTuple(trans, logData, *database);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::undoTuple --
//		タプルに対する更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルに対する更新操作を UNDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を UNDO するための論理ログデータ
//		Schema::Database&	database
//			更新されたタプルを保持する表が存在する
//			データベースのスキーマオブジェクトを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::undoTuple(Trans::Transaction& trans,
								 const Trans::Log::Data& logData,
								 Schema::Database& database)
{
	; _SYDNEY_ASSERT(
		logData.getCategory() == Trans::Log::Data::Category::TupleModify);
#ifndef SYD_COVERAGE

	// 必要があれば、UNDO する操作についてログを出力する

	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Undo [" << database.getName() << "] : " << logData.toString()
		<< ModEndl;
#endif
	//
	//【注意】	論理ログを記録しない
	//

	Execution::Program	program;
	bool saved = trans.setNoLog(true);

	try {
		// UNDO する操作を実行するためのプログラムをオプティマイザに生成させる

		Opt::Optimizer().rollback(
			&database, &program, &logData, &trans);

		// エグゼキュータを起動し、生成したプログラムを実行させる

		Execution::Executor().execute(program);

	} catch (...) {

		(void) trans.setNoLog(saved);
		_SYDNEY_RETHROW;
	}

	(void) trans.setNoLog(saved);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::undoTuple --
//		タプルに対する更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルに対する更新操作を UNDO する
//			トランザクションのトランザクション記述子
//		Common::DataArrayData&	undoLog
//			更新操作を UNDO するための UNDO ログ
//		ModUnicodeString&	dbName
//			更新されたタプルを保持する表が存在するデータベースの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::undoTuple(Trans::Transaction& trans,
								 const Common::DataArrayData& undoLog,
								 const ModUnicodeString& dbName)
{
	if (undoLog.getCount()) {

		// UNDO する操作の対象であるデータベースのスキーマ情報を取得する

		Schema::Database* database = Schema::Database::get(
			Schema::Database::getID(dbName, trans), trans);
		; _SYDNEY_ASSERT(database);

		// キャッシュが破棄されないようにデータベースをオープンしておき、
		// スコープから抜けた時点で自動的にクローズされるようにする

		Utility::AutoDatabaseCloser autoCloser(*database);

		// 実際に操作を UNDO する

		undoTuple(trans, undoLog, *database);
	}
}

//	FUNCTION private
//	Admin::Recovery::Transaction::undoTuple --
//		タプルに対する更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルに対する更新操作を UNDO する
//			トランザクションのトランザクション記述子
//		Common::DataArrayData&	undoLog
//			更新操作を UNDO するための UNDO ログ
//		Schema::Database&	database
//			更新されたタプルを保持する表が存在する
//			データベースのスキーマオブジェクトを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::undoTuple(Trans::Transaction& trans,
								 const Common::DataArrayData& undoLog,
								 Schema::Database& database)
{
	if (undoLog.getCount()) {

		//
		//【注意】	論理ログを記録しない
		//

		Execution::Program	program;
		bool saved = trans.setNoLog(true);

		try {
			// UNDO する操作を実行するためのプログラムを
			// オプティマイザに生成させる
			
			Opt::Optimizer().undo(&database, &program, &undoLog, &trans);

			// エグゼキュータを起動し、生成したプログラムを実行させる
			
			Execution::Executor().execute(program);

		} catch (...) {

			(void) trans.setNoLog(saved);
			_SYDNEY_RETHROW;
		}

		(void) trans.setNoLog(saved);
	}
}

#ifdef OBSOLETE // データベース名でREDOすることはない

//	FUNCTION private
//	Admin::Recovery::Transaction::redoTuple -- タプルに対する操作を REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルに対する更新操作を REDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を REDO するための論理ログデータ
//		ModUnicodeString&	dbName
//			更新されたタプルを保持する表が存在するデータベースの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::redoTuple(Trans::Transaction& trans,
								 const Trans::Log::Data& logData,
								 const ModUnicodeString& dbName)
{
	// REDO する操作の対象であるデータベースのスキーマ情報を取得する

	Schema::Database* database =
		Schema::Database::get(Schema::Database::getID(dbName, trans), trans);
	; _SYDNEY_ASSERT(database);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*database);

	// 実際に操作を REDO する

	redoTuple(trans, logData, *database);
}
#endif

//	FUNCTION private
//	Admin::Recovery::Transaction::redoTuple -- タプルに対する操作を REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			タプルに対する更新操作を REDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を REDO するための論理ログデータ
//		Schema::Database&	database
//			更新されたタプルを保持する表が存在する
//			データベースのスキーマオブジェクトを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::redoTuple(Trans::Transaction& trans,
								 const Trans::Log::Data& logData,
								 Schema::Database& database)
{
	; _SYDNEY_ASSERT(
		logData.getCategory() == Trans::Log::Data::Category::TupleModify);
#ifndef SYD_COVERAGE

	// 必要があれば、REDO する操作についてログを出力する

	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Redo [" << database.getName() << "] : " << logData.toString()
		<< ModEndl;
#endif
	//
	//【注意】	論理ログを記録しない
	//

	Execution::Program	program;
	bool saved = trans.setNoLog(true);

	try {
		// REDO する操作を実行するためのプログラムをオプティマイザに生成させる
		
		Opt::Optimizer().rollforward(
			&database, &program, &logData, &trans);

		// エグゼキュータを起動し、生成したプログラムを実行させる

		Execution::Executor().execute(program);

	} catch (...) {

		(void) trans.setNoLog(saved);
		_SYDNEY_RETHROW;
	}

	(void) trans.setNoLog(saved);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::undoReorg --
//		スキーマに対する更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			スキーマに対する更新操作を UNDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を UNDO するための論理ログデータ
//		ModUnicodeString&	dbName
//			更新するスキーマオブジェクトが存在するデータベースの名前
//		bool				redone
//			true
//				後で REDO される
//			false
//				後で REDO されない
//	
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::undoReorg(
	Trans::Transaction& trans, const Trans::Log::Data& logData,
	const ModUnicodeString& dbName, bool redone, bool rollforward)
{
 	; _SYDNEY_ASSERT(
		logData.getCategory() == Trans::Log::Data::Category::SchemaModify);
#ifndef SYD_COVERAGE

	// 必要があれば、UNDO する操作についてログを出力する

	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Undo [" << dbName << "] : " << logData.toString() << ModEndl;
#endif
	// スキーマ操作を UNDO する

	Schema::Manager::SystemTable::undo(
		trans, _SYDNEY_DYNAMIC_CAST(const Schema::LogData&, logData),
		dbName, redone, rollforward);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::undoReorg --
//		スキーマに対する更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			スキーマに対する更新操作を UNDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を UNDO するための論理ログデータ
//		Schema::Database&	database
//			更新するスキーマオブジェクトが存在する
//			データベースのスキーマオブジェクトを表すクラス
//		bool				redone
//			true
//				後で REDO される
//			false
//				後で REDO されない
//	
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::undoReorg(
	Trans::Transaction& trans, const Trans::Log::Data& logData,
	Schema::Database& database, bool redone, bool rollforward)
{
	undoReorg(trans, logData, database.getName(), redone, rollforward);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::redoReorg --
//		スキーマに対する更新操作を REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			スキーマに対する更新操作を REDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を REDO するための論理ログデータ
//		ModUnicodeString&	dbName
//			更新するスキーマオブジェクトが存在するデータベースの名前
//		bool rollfoward
//			ロールフォワードのためのREDOか
//	
//	RETURN
//		0 以外の値
//			生成またはマウントしたデータベースの
//			スキーマオブジェクトを格納する領域の先頭アドレス
//		0
//			データベースの生成またはマウント以外の操作を REDO した
//
//	EXCEPTIONS

// static
Recovery::Database*
Recovery::Transaction::redoReorg(Trans::Transaction& trans,
								 const Trans::Log::Data& logData,
								 const ModUnicodeString& dbName,
								 bool rollforward)
{
	; _SYDNEY_ASSERT(
		logData.getCategory() == Trans::Log::Data::Category::SchemaModify);
#ifndef SYD_COVERAGE

	// 必要があれば、REDO する操作についてログを出力する

	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Redo [" << dbName << "] : " << logData.toString() << ModEndl;
#endif
	// スキーマ操作を REDO する

	return Schema::Manager::SystemTable::redo(
		trans, _SYDNEY_DYNAMIC_CAST(const Schema::LogData&, logData),
		dbName, rollforward);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::redoReorg --
//		スキーマに対する更新操作を REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			スキーマに対する更新操作を REDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を REDO するための論理ログデータ
//		Schema::Database&	database
//			更新するスキーマオブジェクトが存在する
//			データベースのスキーマオブジェクトを表すクラス
//		bool rollfoward
//			ロールフォワードのためのREDOか
//	
//	RETURN
//		0 以外の値
//			生成またはマウントしたデータベースの
//			スキーマオブジェクトを格納する領域の先頭アドレス
//		0
//			データベースの生成またはマウント以外の操作を REDO した
//
//	EXCEPTIONS

// static
Recovery::Database*
Recovery::Transaction::redoReorg(Trans::Transaction& trans,
								 const Trans::Log::Data& logData,
								 Schema::Database& database,
								 bool rollforward)
{
	return redoReorg(trans, logData, database.getName(), rollforward);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::undoDriver --
//	   	ドライバーに対する更新操作を UNDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ドライバーに対する更新操作を UNDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を UNDO するための論理ログデータ
//		Schema::Database&	database
//			更新されたドライバーに対応するファイルが存在する
//			データベースのスキーマオブジェクトを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::undoDriver(Trans::Transaction& trans,
								  const Trans::Log::Data& logData,
								  Schema::Database& database)
{
	; _SYDNEY_ASSERT(
		logData.getCategory() == Trans::Log::Data::Category::DriverModify);
#ifndef SYD_COVERAGE

	// 必要があれば、UNDO する操作についてログを出力する

	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Undo [" << database.getName() << "] : " << logData.toString()
		<< ModEndl;
#endif
	// ドライバー操作を UNDO する

	LogicalFile::FileDriverManager::undo(
		trans,
		_SYDNEY_DYNAMIC_CAST(const LogicalFile::LogData&, logData),
		database);
}

//	FUNCTION private
//	Admin::Recovery::Transaction::redoDriver --
//	   	ドライバーに対する更新操作を REDO する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			ドライバーに対する更新操作を REDO する
//			トランザクションのトランザクション記述子
//		Trans::Log::Data&	logData
//			更新操作を REDO するための論理ログデータ
//		Schema::Database&	database
//			更新されたドライバーに対応するファイルが存在する
//			データベースのスキーマオブジェクトを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Recovery::Transaction::redoDriver(Trans::Transaction& trans,
								  const Trans::Log::Data& logData,
								  Schema::Database& database)
{
	; _SYDNEY_ASSERT(
		logData.getCategory() == Trans::Log::Data::Category::DriverModify);
#ifndef SYD_COVERAGE

	// 必要があれば、REDO する操作についてログを出力する

	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Redo [" << database.getName() << "] : " << logData.toString()
		<< ModEndl;
#endif
	// ドライバー操作を REDO する

	LogicalFile::FileDriverManager::redo(
		trans,
		_SYDNEY_DYNAMIC_CAST(const LogicalFile::LogData&, logData),
		database);
}

//
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
