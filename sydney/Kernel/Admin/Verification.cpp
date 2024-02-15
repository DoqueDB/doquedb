// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Verification.cpp -- 整合性検査関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "Admin/Verification.h"
#include "Admin/Operation.h"
#include "Admin/Utility.h"

#include "Checkpoint/Daemon.h"
#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/ErrorMessageManager.h"
#include "Common/IntegerData.h"
#include "Common/NullData.h"
#include "Common/ResultSetMetaData.h"
#include "Common/SQLData.h"
#include "Common/StringData.h"
#include "Common/UnsignedIntegerData.h"
#include "Exception/BadArgument.h"
#include "Exception/DatabaseChanged.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/IndexNotFound.h"
#include "Exception/ReadOnlyDatabase.h"
#include "Exception/ReadOnlyTransaction.h"
#include "Exception/TableNotFound.h"
#include "Schema/Database.h"
#include "Schema/File.h"
#include "Schema/Hold.h"
#include "Schema/Index.h"
#include "Schema/Manager.h"
#include "Schema/Table.h"
#include "Server/Session.h"
#include "Statement/VerifyStatement.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_ADMIN_USING

namespace
{

namespace _Verification
{
	// SQL 文に指定された整合性検査オプションを得る
	Verification::Treatment::Value
	getOption(const Statement::VerifyStatement& stmt);
}

//	FUNCTION
//	$$$::_Verification::getOption --
//		SQL 文に指定された整合性検査オプションを得る
//
//	NOTES
//
//	ARGUMENTS
//		Statement::VerifyStatement&	stmt
//			整合性検査の SQL 文を表すクラス
//
//	RETURN
//		得られた整合性検査オプションを表す
//		Admin::Verification::Treatment::Value の論理和
//
//	EXCEPTIONS
//		なし

inline
Verification::Treatment::Value
_Verification::getOption(const Statement::VerifyStatement& stmt)
{
	Verification::Treatment::Value treatment = Verification::Treatment::None;

	if (stmt.isCorrect())
		treatment |= Verification::Treatment::Correct;
	if (stmt.isContinue())
		treatment |= Verification::Treatment::Continue;
	if (stmt.isCascade())
		treatment |= Verification::Treatment::Cascade;
	if (stmt.isVerbose())
		treatment |= Verification::Treatment::Verbose;
	if (stmt.isValue())
		treatment |= Verification::Treatment::Data;

	return treatment;
}

}

//	FUNCTION
//	Admin::Verification::verify -- 整合性検査を行う
//
//	NOTES
//		SQL 文で指定されたオブジェクトの整合性検査を行う
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を行うトランザクションのトランザクション記述子
//		Statement::VerifyStatement&	stmt
//			整合性検査の SQL 文を表すクラス
//		ModUnicodeString&	dbName
//			整合性検査の対象が存在するデータベースの名前
//		Admin::Verification::Progress&	result
//			整合性検査の結果が設定されるクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Verification::
verify(Trans::Transaction& trans,
	   Server::Session* pSession_,
	   const Statement::VerifyStatement& stmt,
	   const ModUnicodeString& dbName, Progress& result)
{
#ifdef ADMIN_RETURN_INTERMEDIATE_RESULT
	if (result.getConnection().getMasterID() >= 1)
		// verify結果のColumnMetaDataを返す
		result.putMetaData();
#endif
	switch (stmt.getSchemaType()) {
	case Statement::VerifyStatement::SchemaType::Database:
		verifyDatabase(trans, pSession_, stmt, dbName, result);	break;
	case Statement::VerifyStatement::SchemaType::Table:
		verifyTable(trans, pSession_, stmt, dbName, result);	break;
	case Statement::VerifyStatement::SchemaType::Index:
		verifyIndex(trans, pSession_, stmt, dbName, result);	break;
	}
}

//	FUNCTION
//	Admin::Verification::verifyDatabase -- データベースの整合性検査を行う
//
//	NOTES
//		SQL 文で指定されたデータベース
//		およびデータベースに存在する表と索引の整合性検査を行う
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を行うトランザクションのトランザクション記述子
//	   Server::Session* pSession_
//			整合性検査を行うセッションのセッション記述子
//		Statement::VerifyStatement&	stmt
//			整合性検査の SQL 文を表すクラス
//		ModUnicodeString&	dbName
//			整合性検査の対象であるデータベースの名前
//		Admin::Verification::Progress&	result
//			整合性検査の結果が設定されるクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::BadArgument
//			SQL 文で指定されたデータベース名と dbName が一致しない
//		Exception::DatabaseNotFound
//			整合性検査するデータベースが存在しない
//		Exception::ReadOnlyDatabase
//			読取専用のデータベースに対して、
//			見つかった矛盾を訂正する整合性検査は行えない
//		Excetpion::ReadOnlyTransaction
//			読取専用トランザクションで、
//			見つかった矛盾を訂正する整合性検査は行えない

void
Verification::
verifyDatabase(Trans::Transaction& trans,
			   Server::Session* pSession_,
			   const Statement::VerifyStatement& stmt,
			   const ModUnicodeString& dbName, Progress& result)
{
	; _SYDNEY_ASSERT(stmt.getSchemaType() ==
					 Statement::VerifyStatement::SchemaType::Database);

	// バージョンファイルの同期を実行不可にする

	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::FileSynchronizer);

	// 与えられた SQL 文から検査の対象となるデータベースの名前を取得する

	const Schema::Object::Name& name = stmt.getName();

	// SQL 文に指定されたオプションを取得する

	Verification::Treatment::Value treatment = _Verification::getOption(stmt);

	if (treatment & Treatment::Correct &&
		trans.getCategory() == Trans::Transaction::Category::ReadOnly)

		// 読取専用トランザクションで整合性検査を行っているときは、
		// 見つかった矛盾は訂正できない

		_SYDNEY_THROW0(Exception::ReadOnlyTransaction);

	// メタデータベース、データベース表と
	// 検査するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	const Schema::Hold::Operation::Value operation =
		((treatment & Treatment::Correct) ?
		 Schema::Hold::Operation::ReadWrite :
		 Schema::Hold::Operation::ReadForImport);

	Schema::Database* database = Schema::Database::getLocked(
		trans, dbName,
		Lock::Name::Category::Tuple, operation,
		Lock::Name::Category::Tuple, operation);
	if (!database)

		// 指定されたデータベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, dbName);

	if (database->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(database->getID(), database->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// トランザクションで操作するデータベースを設定する
	//
	//【注意】	すぐに論理ログを記録しなくても、
	//			スキーマ操作関数で論理ログファイルを
	//			参照しに行く可能性があるので、
	//			操作対象のデータベースのスキーマ情報を取得したら、
	//			すぐトランザクションに設定すること

	trans.setLog(*database);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*database);

	// 得られたデータベースの名前と SQL 文中の名前が一致することを確認する
	//
	//【注意】	現状では、セッション中で処理可能なデータベースは
	//			ひとつに限定されているため、
	//			検査するデータベースの名前は呼び出し側から指示される
	//
	//			この制限がなくなったとき、
	//			検査するデータベース名は SQL 文中から取得する必要がある

	if (database->getName() != name)

		// 一致しなかった

		_SYDNEY_THROW0(Exception::BadArgument);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(trans, *database, &stmt, pSession_);

	if (treatment & Treatment::Correct && database->isReadOnly())

		// 検査するデータベースが読取専用であれば、
		// 見つかった矛盾を訂正することはできない

		_SYDNEY_THROW1(Exception::ReadOnlyDatabase, dbName);

	// まず、データベース自身に関する整合性検査を行う
	{
	Progress progress(result.getConnection());
	database->verify(progress, trans, treatment);
	result += progress;

	switch (progress.getStatus()) {
	case Status::Consistent:

		// データベース自身に矛盾がなかった

		if (operation == Schema::Hold::Operation::ReadWrite)

			// データベース表の検査するデータベースを格納する
			// タプルのロックを必要があれば変換する

			Schema::Manager::SystemTable::convert(
				trans, Schema::Hold::Target::MetaTuple,
				Lock::Name::Category::Tuple, operation,
				Lock::Name::Category::Tuple,
				Schema::Hold::Operation::ReadOnly, database->getID());
		break;

	case Status::Inconsistent:

		// データベース自身に矛盾が見つかったときは、
		// それを構成する表や索引の整合性検査はあきらめる

		return;
	}
	}

	Lock::Mode::Value mode;
	Lock::Duration::Value duration;

	// 検査するデータベースをロックする

	if (trans.getAdequateLock(
			Lock::Name::Category::Database, Lock::Name::Category::Table,
			!(treatment & Treatment::Correct), mode, duration))
		trans.lock(Lock::DatabaseName(database->getID()), mode, duration);

	// データベース中の表ごとに処理していく

	ModVector<Schema::Table*> tables = database->getTable(trans);
	const ModSize n = tables.getSize();
	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(tables[i]);

		// 検査する表をロックする

		if (trans.getAdequateLock(
				Lock::Name::Category::Table, Lock::Name::Category::Table,
				!(treatment & Treatment::Correct), mode, duration))
			trans.lock(Lock::TableName(database->getID(), tables[i]->getID()),
					   mode, duration);

		// 表と表についている索引の整合性検査を行う

		Progress progress(result.getConnection());
		tables[i]->verify(progress, trans, treatment);
		result += progress;

		switch (progress.getStatus()) {
		case Status::Consistent:

			// 表と表についている索引に矛盾はなかった

			if (mode == Lock::Mode::X)

				// 表に対するロックを必要があれば変換する

				trans.convertLock(
					Lock::TableName(database->getID(), tables[i]->getID()),
					Lock::Mode::X, duration, Lock::Mode::S);
			break;

		case Status::Inconsistent:

			// 表に訂正できない矛盾が見つかった

			if (!(treatment & Treatment::Continue))

				// できる限り検査を続ける指定がされていないので、ここでやめる

				return;
		}
	}
}

//	FUNCTION
//	Admin::Verification::verifyTable -- 表の整合性検査を行う
//
//	NOTES
//		SQL 文で指定された表および表についている索引の整合性検査を行う
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を行うトランザクションのトランザクション記述子
//	   Server::Session* pSession_
//			整合性検査を行うセッションのセッション記述子
//		Statement::VerifyStatement&	stmt
//			整合性検査の SQL 文を表すクラス
//		ModUnicodeString&	dbName
//			整合性検査の対象である表が存在するデータベースの名前
//		Admin::Verification::Progress&	result
//			整合性検査の結果が設定されるクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::DatabaseNotFound
//			整合性検査する表が存在するデータベースが存在しない
//		Exception::ReadOnlyDatabase
//			読取専用のデータベースに対して、
//			見つかった矛盾を訂正する整合性検査は行えない
//		Excetpion::ReadOnlyTransaction
//			読取専用トランザクションで、
//			見つかった矛盾を訂正する整合性検査は行えない
//		Exception::TableNotFound
//			整合性検査する表が存在しない

void
Verification::
verifyTable(Trans::Transaction& trans,
			Server::Session* pSession_,
			const Statement::VerifyStatement& stmt,
			const ModUnicodeString& dbName, Progress& result)
{
	; _SYDNEY_ASSERT(stmt.getSchemaType() ==
					 Statement::VerifyStatement::SchemaType::Table);

	// バージョンファイルの同期を実行不可にする

	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::FileSynchronizer);

	// 与えられた SQL 文から検査の対象となる表の名前を取得する

	const Schema::Object::Name& name = stmt.getName();

	// SQL 文に指定されたオプションを取得する

	Verification::Treatment::Value treatment = _Verification::getOption(stmt);

	// メタデータベース、データベース表と
	// 検査する表が存在するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Schema::Database* database = Schema::Database::getLocked(
		trans, dbName,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport);
	if (!database)

		// 指定されたデータベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, dbName);

	if (database->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(database->getID(), database->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// トランザクションで操作するデータベースを設定する
	//
	//【注意】	すぐに論理ログを記録しなくても、
	//			スキーマ操作関数で論理ログファイルを
	//			参照しに行く可能性があるので、
	//			操作対象のデータベースのスキーマ情報を取得したら、
	//			すぐトランザクションに設定すること

	trans.setLog(*database);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*database);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(trans, *database, &stmt, pSession_);

	if (treatment & Treatment::Correct) {
		if (trans.getCategory() == Trans::Transaction::Category::ReadOnly)

			// 読取専用トランザクションで整合性検査を行っているときは、
			// 見つかった矛盾は訂正できない

			_SYDNEY_THROW0(Exception::ReadOnlyTransaction);
		if (database->isReadOnly())

			// 検査する表が存在するデータベースが読取専用であれば、
			// 見つかった矛盾を訂正することはできない

			_SYDNEY_THROW1(Exception::ReadOnlyDatabase, dbName);
	}

	// 検査する表を表すクラスを取得する

	Schema::Table* table = database->getTable(name, trans);
	if (!table)

		// 指定された表名を取得してから、
		// これまでに表が破棄されている

		_SYDNEY_THROW2(Exception::TableNotFound, name, dbName);

	Lock::Mode::Value mode;
	Lock::Duration::Value duration;

	// 検査する表が存在するデータベースをロックする

	if (trans.getAdequateLock(
			Lock::Name::Category::Database, Lock::Name::Category::Table,
			!(treatment & Treatment::Correct), mode, duration))
		trans.lock(Lock::DatabaseName(database->getID()), mode, duration);

	// 検査する表をロックする

	if (trans.getAdequateLock(
			Lock::Name::Category::Table, Lock::Name::Category::Table,
			!(treatment & Treatment::Correct), mode, duration))
		trans.lock(Lock::TableName(database->getID(), table->getID()),
				   mode, duration);

	// 表およびその表についている索引の整合性検査を行う

	table->verify(result, trans, treatment);

	switch (result.getStatus()) {
	case Status::Consistent:

		// 表と表についている索引に矛盾はなかった

		if (mode == Lock::Mode::X)

			// 表に対するロックを必要があれば変換する

			trans.convertLock(
				Lock::TableName(database->getID(), table->getID()),
				Lock::Mode::X, duration, Lock::Mode::S);
	}
}

//	FUNCTION
//	Admin::Verification::verifyIndex -- 索引の整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			整合性検査を行うトランザクションのトランザクション記述子
//	   Server::Session* pSession_
//			整合性検査を行うセッションのセッション記述子
//		Statement::VerifyStatement&	stmt
//			整合性検査の SQL 文を表すクラス
//		ModUnicodeString&	dbName
//			整合性検査の対象である索引が存在するデータベースの名前
//		Admin::Verification::Progress&	result
//			整合性検査の結果が設定されるクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::DatabaseNotFound
//			整合性検査する索引が存在するデータベースが存在しない
//		Exception::IndexNotFound
//			整合性検査する索引が存在しない
//		Exception::ReadOnlyDatabase
//			読取専用のデータベースに対して、
//			見つかった矛盾を訂正する整合性検査は行えない
//		Excetpion::ReadOnlyTransaction
//			読取専用トランザクションで、
//			見つかった矛盾を訂正する整合性検査は行えない

void
Verification::
verifyIndex(Trans::Transaction& trans,
			Server::Session* pSession_,
			const Statement::VerifyStatement& stmt,
			const ModUnicodeString& dbName, Progress& result)
{
	; _SYDNEY_ASSERT(stmt.getSchemaType() ==
					 Statement::VerifyStatement::SchemaType::Index);

	// バージョンファイルの同期を実行不可にする

	Checkpoint::Daemon::AutoDisabler
		disabler(Checkpoint::Daemon::Category::FileSynchronizer);

	// 与えられた SQL 文から検査の対象となる索引の名前を取得する

	const Schema::Object::Name& name = stmt.getName();

	// SQL 文に指定されたオプションを取得する

	Verification::Treatment::Value treatment = _Verification::getOption(stmt);

	// メタデータベース、データベース表と
	// 検査する索引が存在するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Schema::Database* database = Schema::Database::getLocked(
		trans, dbName,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport,
		Lock::Name::Category::Tuple, Schema::Hold::Operation::ReadForImport);
	if (!database)

		// 指定されたデータベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, dbName);

	if (database->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(database->getID(), database->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// トランザクションで操作するデータベースを設定する
	//
	//【注意】	すぐに論理ログを記録しなくても、
	//			スキーマ操作関数で論理ログファイルを
	//			参照しに行く可能性があるので、
	//			操作対象のデータベースのスキーマ情報を取得したら、
	//			すぐトランザクションに設定すること

	trans.setLog(*database);

	// キャッシュが破棄されないようにデータベースをオープンしておき、
	// スコープから抜けた時点で自動的にクローズされるようにする

	Utility::AutoDatabaseCloser autoCloser(*database);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(trans, *database, &stmt, pSession_);

	if (treatment & Treatment::Correct) {
		if (trans.getCategory() == Trans::Transaction::Category::ReadOnly)

			// 読取専用トランザクションで整合性検査を行っているときは、
			// 見つかった矛盾は訂正できない

			_SYDNEY_THROW0(Exception::ReadOnlyTransaction);
		if (database->isReadOnly())

			// 検査する索引が存在するデータベースが読取専用であれば、
			// 見つかった矛盾を訂正することはできない

			_SYDNEY_THROW1(Exception::ReadOnlyDatabase, dbName);
	}

	// 検査する索引を表すクラスを取得する

	Schema::Index* index = Schema::Index::get(name, database, trans);
	if (!index)

		// 指定された索引名を取得してから、
		// これまでに索引が破棄されている

		_SYDNEY_THROW2(Exception::IndexNotFound, name, dbName);

	Lock::Mode::Value mode;
	Lock::Duration::Value duration;

	// 検査する索引が存在するデータベースをロックする

	if (trans.getAdequateLock(
			Lock::Name::Category::Database, Lock::Name::Category::Table,
			!(treatment & Treatment::Correct), mode, duration))
		trans.lock(Lock::DatabaseName(database->getID()), mode, duration);

	// 検査する索引がついている表をロックする

	if (trans.getAdequateLock(
			Lock::Name::Category::Table, Lock::Name::Category::Table,
			!(treatment & Treatment::Correct), mode, duration))
		trans.lock(Lock::TableName(database->getID(), index->getTableID()),
				   mode, duration);

	// 索引の整合性検査を行う

	index->verify(result, trans, treatment);

	switch (result.getStatus()) {
	case Status::Consistent:

		// 索引に矛盾はなかった

		if (mode == Lock::Mode::X)

			// 索引がついている表に対するロックを必要があれば、変換する

			trans.convertLock(
				Lock::TableName(database->getID(), index->getTableID()),
				Lock::Mode::X, duration, Lock::Mode::S);
	}
}

//	FUNCTION public
//	Admin::Verification::Progress::operator += -- += 演算子
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress&	v
//			自分自身に追加する経過を表すクラス
//
//	RETURN
//		追加後の自分自身
//
//	EXCEPTIONS

Verification::Progress&
Verification::
Progress::operator +=(const Progress& v)
{
	if (getStatus() < v.getStatus())

		// これまでの整合性検査結果を設定する

		_status = v.getStatus();

	if (v.getSchemaObjectName().getLength())

		// 検査中のスキーマオブジェクトの名称を上書きする

		setSchemaObjectName(v.getSchemaObjectName());

#ifdef ADMIN_RETURN_INTERMEDIATE_RESULT
	if (!m_pConnection) {
#else
	// 詳細な説明を追加する

	const Common::DataArrayData& src = v.getDescription();
	const int n = src.getCount();

	for (int i = 0; i < n; ++i)
		_description.pushBack(src.getElement(i)->copy());
#endif
#ifdef ADMIN_RETURN_INTERMEDIATE_RESULT
	}
#endif

	return *this;
}

//	FUNCTION public
//	Admin::Verification::Progress::pushDescription -- 詳細な説明を追加する
//
//	NOTES
//
//	ARGUMENTS
//		char*				module
//			説明を追加する検査を行ったモジュールの名称が
//			格納された領域の先頭アドレス
//		char*				file
//			説明を設定した場所の Sydney のファイルの名前
//		unsigned int		line
//			説明を設定した場所の Sydney のファイルの行番号
//		ModUnicodeString&	path
//			説明を追加する検査の対象を表す絶対パス名
//		Admin::Verification::Status::Value	status
//			説明を追加する検査の結果で、Admin::Verification::Status::Corrected
//			または Admin::Verification::Status::Inconsistent を指定する
//		Admin::Verification::Progress::Message&	message
//			指定されたとき
//				追加する説明メッセージ
//			指定されないとき
//				説明メッセージとして空文字列を追加する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Verification::Progress::
pushDescription(const char* module,
				const char* file, unsigned int line,
				const ModUnicodeString& path, Status::Value status)
{
	pushDescription(module, file, line, path, status, ModUnicodeString());
}

void
Verification::Progress::
pushDescription(const char* module,
				const char* file, unsigned int line,
				const ModUnicodeString& path, Status::Value status,
				const Message& message)
{
	ModUnicodeChar buf[1024];
	pushDescription(
		module, file, line, path, status,
		Common::ErrorMessageManager::makeErrorMessage(
			buf, message.getErrorNumber(), message.getErrorMessageArgument()));
}

namespace
{
	// Verify結果を表す表の名前
	ModUnicodeString _cstrVerifyTable("VerifyResult");
	// Verify結果を構成する列の数
	const int _nVerifyTuple = 7;
	// Verify結果の型
	const Common::SQLData _cVerifyColumnType[] =
	{
		Common::SQLData(Common::SQLData::Type::NChar, Common::SQLData::Flag::Unlimited, -1, 0),
		Common::SQLData(Common::SQLData::Type::NChar, Common::SQLData::Flag::Unlimited, -1, 0),
		Common::SQLData(Common::SQLData::Type::UInt, Common::SQLData::Flag::Fixed, 4, 0),
		Common::SQLData(Common::SQLData::Type::NChar, Common::SQLData::Flag::Unlimited, -1, 0),
		Common::SQLData(Common::SQLData::Type::NChar, Common::SQLData::Flag::Unlimited, -1, 0),
		Common::SQLData(Common::SQLData::Type::Int, Common::SQLData::Flag::Fixed, 4, 0),
		Common::SQLData(Common::SQLData::Type::NChar, Common::SQLData::Flag::Unlimited, -1, 0),
	};
	// Verify結果の列名
	const ModUnicodeString _cstrVerifyColumnName[] =
	{
		ModUnicodeString("module"),
		ModUnicodeString("file"),
		ModUnicodeString("line"),
		ModUnicodeString("name"),
		ModUnicodeString("path"),
		ModUnicodeString("status"),
		ModUnicodeString("message"),
	};
}

void
Verification::Progress::
pushDescription(const char* module,
				const char* file, unsigned int line,
				const ModUnicodeString& path, Status::Value status,
				const ModUnicodeString& message)
{
#ifdef DEBUG
	switch (status) {
	case Status::Consistent:
	case Status::Corrected:
	case Status::Correctable:
	case Status::Inconsistent:
	case Status::Interrupted:
	case Status::Aborted:
		break;
	default:

		// 指定できない検査結果である

		; _SYDNEY_ASSERT(false);
	}
#endif
#ifdef ADMIN_RETURN_INTERMEDIATE_RESULT
	// 指定された情報をそれぞれ要素とする配列を生成し、クライアントに返す
	Common::DataArrayData data;
	Common::DataArrayData* tuple = &data;
#else
	// 指定された情報をそれぞれ要素とする配列を生成する

	Common::DataArrayData*	tuple = new Common::DataArrayData();
	; _SYDNEY_ASSERT(tuple);
	Common::DataArrayData::Pointer p(tuple);
#endif

	tuple->reserve(_nVerifyTuple);
	if (status == Status::Consistent) {
		tuple->pushBack(Common::DataArrayData::Pointer(
							Common::NullData::getInstance()));
		tuple->pushBack(Common::DataArrayData::Pointer(
							Common::NullData::getInstance()));
		tuple->pushBack(Common::DataArrayData::Pointer(
							Common::NullData::getInstance()));
	} else {
		tuple->pushBack(Common::DataArrayData::Pointer(
							new Common::StringData(ModUnicodeString(module))));
		tuple->pushBack(Common::DataArrayData::Pointer(
							new Common::StringData(ModUnicodeString(file))));
		tuple->pushBack(Common::DataArrayData::Pointer(
							new Common::UnsignedIntegerData(line)));
	}
	tuple->pushBack(Common::DataArrayData::Pointer(
						new Common::StringData(getSchemaObjectName())));
	tuple->pushBack(Common::DataArrayData::Pointer(
						new Common::StringData(path)));
	tuple->pushBack(Common::DataArrayData::Pointer(
						new Common::IntegerData(status)));
	tuple->pushBack(Common::DataArrayData::Pointer(
						new Common::StringData(message)));

#ifdef ADMIN_RETURN_INTERMEDIATE_RESULT
	// コネクションに書き出す

	if (m_pConnection) {
		m_pConnection->writeObject(tuple);
		m_pConnection->flush();
	} else {
		// 詳細な説明を格納する配列の末尾に、生成した配列を要素として追加する
		Common::DataArrayData*	tuple = new Common::DataArrayData();
		; _SYDNEY_ASSERT(tuple);
		*tuple = data;					// ポインターのコピー
		Common::DataArrayData::Pointer p(tuple);
		_description.pushBack(p);
	}
#else
	// 詳細な説明を格納する配列の末尾に、生成した配列を要素として追加する

	_description.pushBack(p);
#endif

	// これまでの整合性検査の結果を、与えられた情報にしたがって変更する

	if (getStatus() < status)
		_status = status;
}

// 返り値のColumnMetaDataをクライアントに返す
void
Verification::Progress::
putMetaData()
{
	Common::ResultSetMetaData cMetaData;
	cMetaData.reserve(_nVerifyTuple);
	for (int i = 0; i < _nVerifyTuple; ++i) {
		Common::ColumnMetaData cColumnMetaData(_cVerifyColumnType[i]);
		cColumnMetaData.setColumnName(_cstrVerifyColumnName[i]);
		cColumnMetaData.setColumnAliasName(_cstrVerifyColumnName[i]);
		cColumnMetaData.setTableName(_cstrVerifyTable);
		cColumnMetaData.setTableAliasName(_cstrVerifyTable);
		cColumnMetaData.setNotSearchable();
		cColumnMetaData.setReadOnly();

		cMetaData.pushBack(cColumnMetaData);
	}

	if (m_pConnection) {
		m_pConnection->writeObject(&cMetaData);
		m_pConnection->flush();
	}
}

//
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
