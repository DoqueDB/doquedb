// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Undo.cpp -- Undo関連の関数定義(Manager::SystemTable)
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Manager.h"
#include "Schema/Undo.h"
#include "Schema/Area.h"
#include "Schema/AreaContent.h"
#include "Schema/Cascade.h"
#include "Schema/Database.h"
#include "Schema/Externalizable.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Function.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/LogData.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/NameParts.h"
#include "Schema/ObjectID.h"
#include "Schema/ObjectSnapshot.h"
#include "Schema/Parameter.h"
#include "Schema/Partition.h"
#include "Schema/PathParts.h"
#include "Schema/Privilege.h"
#include "Schema/Recovery.h"
#include "Schema/Sequence.h"
#include "Schema/SystemTable.h"
#include "Schema/Table.h"
#include "Schema/Utility.h"

#include "Admin/Mount.h"
#include "Admin/Unmount.h"

#include "Common/Assert.h"
#include "Common/Externalizable.h"
#include "Common/Hasher.h"
#include "Common/Message.h"
#include "Common/ObjectPointer.h"
#include "Common/Parameter.h"

#include "Exception/AreaNotFound.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/IndexNotFound.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/ReadOnlyTransaction.h"
#include "Exception/SchemaObjectNotFound.h"
#include "Exception/TableNotFound.h"
#include "Exception/NotBeginTransaction.h"

#include "Trans/Transaction.h"

#include "Lock/Client.h"
#include "Lock/Duration.h"
#include "Lock/Item.h"
#include "Lock/Mode.h"
#include "Lock/Name.h"

#include "ModAlgorithm.h"
#include "ModAutoPointer.h"
#include "ModMap.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

} // namespace $$$

//////////////////////////////////////
//	 Schema::Manager::SystemTable	//
//////////////////////////////////////

//	FUNCTION
//	Schema::Manager::SystemTable::undo --
//		データベースのシステム表の再構成の再実行を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			再実行する操作を記述したログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true
//				後で REDO される
//			false
//				後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::undo(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	if (cTrans_.getCategory() == Trans::Transaction::Category::ReadOnly)

		// 読取専用トランザクションでは、
		// スキーマ操作を UNDO できない

		_SYDNEY_THROW0(Exception::ReadOnlyTransaction);

	// 念のために初期化しておく
	initialize();

#ifdef DEBUG
#ifndef SYD_COVERAGE
	SydSchemaDebugMessage << "UNDO " << cLogData_.toString() << ModEndl;
#endif
#endif

	// ログデータの種類に応じて再構成処理を行う
	switch ( cLogData_.getSubCategory() )
	{

	//=======================================================================
	//	Admin の操作
	//=======================================================================
	case LogData::Category::Mount:

		UndoUtility::undoMount(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;

	case LogData::Category::Unmount:

		UndoUtility::undoUnmount(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;

	case LogData::Category::StartBackup:
	{
		UndoUtility::undoStartBackup(cTrans_, cLogData_, cDatabaseName_,
									 bRollforward_);
		break;
	}
	case LogData::Category::EndBackup:
	{
		UndoUtility::undoEndBackup(cTrans_, cLogData_, cDatabaseName_,
								   bRollforward_);
		break;
	}

	//=======================================================================
	//	Database の操作
	//=======================================================================

	case LogData::Category::CreateDatabase:

		UndoUtility::undoCreateDatabase(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;

	case LogData::Category::DropDatabase:

		UndoUtility::undoDropDatabase(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;

	case LogData::Category::MoveDatabase:
	{
		UndoUtility::undoMoveDatabase(cTrans_, cLogData_, cDatabaseName_,
									  redone_, bRollforward_);
		break;
	}
	case LogData::Category::AlterDatabase:
	case LogData::Category::AlterDatabase_ReadOnly:
	case LogData::Category::AlterDatabase_SetToMaster:
	{
		UndoUtility::undoAlterDatabase(cTrans_, cLogData_, cDatabaseName_,
									   redone_, bRollforward_);
		break;
	}

	//=======================================================================
	//	Database 以下の操作
	//=======================================================================

	case LogData::Category::CreateArea:
	{
		UndoUtility::undoCreateArea(cTrans_, cLogData_, cDatabaseName_,
									redone_, bRollforward_);
		break;
	}
	case LogData::Category::DropArea:
	{
		UndoUtility::undoDropArea(cTrans_, cLogData_, cDatabaseName_,
								  redone_, bRollforward_);
		break;
	}
	case LogData::Category::AlterArea:
	{
		UndoUtility::undoAlterArea(cTrans_, cLogData_, cDatabaseName_,
								   redone_, bRollforward_);
		break;
	}
	case LogData::Category::CreateTable:

		UndoUtility::undoCreateTable(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;

	case LogData::Category::DropTable:
	{
		UndoUtility::undoDropTable(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;
	}
	case LogData::Category::AlterTable:
	{
		UndoUtility::undoAlterTable(cTrans_, cLogData_, cDatabaseName_,
									redone_, bRollforward_);
		break;
	}
	case LogData::Category::RenameTable:
	{
		UndoUtility::undoRenameTable(cTrans_, cLogData_, cDatabaseName_,
									 redone_, bRollforward_);
		break;
	}
	case LogData::Category::AddColumn:
	{
		UndoUtility::undoAddColumn(cTrans_, cLogData_, cDatabaseName_,
								   redone_, bRollforward_);
		break;
	}
	case LogData::Category::AlterColumn:
	{
		UndoUtility::undoAlterColumn(cTrans_, cLogData_, cDatabaseName_,
									 redone_, bRollforward_);
		break;
	}
	case LogData::Category::AddConstraint:
	{
		UndoUtility::undoAddConstraint(cTrans_, cLogData_, cDatabaseName_,
									   redone_, bRollforward_);
		break;
	}
	case LogData::Category::DropConstraint:
	{
		UndoUtility::undoDropConstraint(cTrans_, cLogData_, cDatabaseName_,
										redone_, bRollforward_);
		break;
	}
	case LogData::Category::CreateIndex:

		UndoUtility::undoCreateIndex(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;

	case LogData::Category::DropIndex:

		UndoUtility::undoDropIndex(
			cTrans_, cLogData_, cDatabaseName_, redone_, bRollforward_);
		break;

	case LogData::Category::AlterIndex:
	{
		UndoUtility::undoAlterIndex(cTrans_, cLogData_, cDatabaseName_,
									redone_, bRollforward_);
		break;
	}
	case LogData::Category::RenameIndex:
	{
		UndoUtility::undoRenameIndex(cTrans_, cLogData_, cDatabaseName_,
									 redone_, bRollforward_);
		break;
	}
	case LogData::Category::CreatePrivilege:
	{
		UndoUtility::undoCreatePrivilege(cTrans_, cLogData_, cDatabaseName_,
										 redone_, bRollforward_);
		break;
	}
	case LogData::Category::DropPrivilege:
	{
		UndoUtility::undoDropPrivilege(cTrans_, cLogData_, cDatabaseName_,
									   redone_, bRollforward_);
		break;
	}
	case LogData::Category::AlterPrivilege:
	{
		UndoUtility::undoAlterPrivilege(cTrans_, cLogData_, cDatabaseName_,
										redone_, bRollforward_);
		break;
	}
	case LogData::Category::CreateCascade:
	{
		UndoUtility::undoCreateCascade(cTrans_, cLogData_, cDatabaseName_,
									   redone_, bRollforward_);
		break;
	}
	case LogData::Category::DropCascade:
	{
		UndoUtility::undoDropCascade(cTrans_, cLogData_, cDatabaseName_,
									 redone_, bRollforward_);
		break;
	}
	case LogData::Category::AlterCascade:
	{
		UndoUtility::undoAlterCascade(cTrans_, cLogData_, cDatabaseName_,
									  redone_, bRollforward_);
		break;
	}

	case LogData::Category::CreatePartition:
	{
		UndoUtility::undoCreatePartition(cTrans_, cLogData_, cDatabaseName_,
										 redone_, bRollforward_);
		break;
	}
	case LogData::Category::DropPartition:
	{
		UndoUtility::undoDropPartition(cTrans_, cLogData_, cDatabaseName_,
									   redone_, bRollforward_);
		break;
	}
	case LogData::Category::AlterPartition:
	{
		UndoUtility::undoAlterPartition(cTrans_, cLogData_, cDatabaseName_,
										redone_, bRollforward_);
		break;
	}

	case LogData::Category::CreateFunction:
	{
		UndoUtility::undoCreateFunction(cTrans_, cLogData_, cDatabaseName_,
										redone_, bRollforward_);
		break;
	}
	case LogData::Category::DropFunction:
	{
		UndoUtility::undoDropFunction(cTrans_, cLogData_, cDatabaseName_,
									  redone_, bRollforward_);
		break;
	}

	default:
		// 知らないログデータがきた
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
		break;
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::undoMount -- mount の undo
//
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true
//				後で REDO される
//			false
//				後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoMount(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// 論理ログデータより MOUNT したデータベースの Schema ID を得る
	const ObjectID::Value id = Database::getObjectID(cLogData_);

	// ★注意★
	// Create Databaseのログはシステムとデータベースの両方に記録される
	// システム用の論理ログファイルに対してUNDOが実行される場合はデータベース表に対する操作、
	// データベース用の論理ログファイルに対してUNDOが実行される場合はデータベースに対する操作を行う
	// システム用の論理ログに対してはUNDO情報に関する操作も行う

	if (cDatabaseName_ == Object::Name(NameParts::Database::System)) {

		if (Undo::isEntered(id, Undo::Type::DropDatabase)
			|| Undo::isEntered(id, Undo::Type::Unmount)) {
			if (redone_) {
				// MOUNT したデータベースの DROP または UNMOUNT が
				// UNDO されているおり、MOUNTが後でREDOされるときは、
				// MOUNT が UNDO されたことを記憶しておく
		
				Undo::enter(id, Undo::Type::Mount);
			}
			return;

		} else if (redone_) {

			// MOUNTしたデータベースがUNDO中にDROPまたはUNMOUNTされていないとき、
			// REDOされる場合はこれ以降の処理ですべてのパスを削除から除外するように登録する

			// データベースを構成するOSファイルはALTERなどによって移動したときの
			// 最終的な場所(つまりUNDO開始時にあった場所)を用いる

			ModVector<ModUnicodeString> paths;

			if (!Path::getUndoDatabasePath(id, paths)) {
				// ALTERのUNDOで登録されたパス指定がないときは
				// 移動していないということなのでログに記録されていた
				// パス指定を使用する
				// MOUNTのログデータはCREATEと同じである
				Database::getPath(cLogData_, Database::Log::Create::Path, paths);

				// REDOされる場合はこのパスが最終的なパスになるので
				// パス指定を登録する
				Path::setUndoDatabasePath(id, paths);
			}

			// MOUNTのREDOではファイルが存在していることを前提とした処理が行われるので
			// 消してはいけないパスとしてデータベースに指定されているすべてのパスを除外リストに加える
			// → これ以降、リカバリーが終了するまで削除されなくなる
			const Object::Name& name = Database::getName(cLogData_);
			for (int i = 0; i < Database::Path::Category::ValueNum; ++i) {
				Path::addUnremovablePath(Database::getPath(static_cast<Database::Path::Category::Value>(i),
														   paths, name));
			}

			return;
		}

#ifdef OBSOLETE // データベース単位のログはない
	} else {

		; _SYDNEY_ASSERT(cDatabaseName_ != NameParts::Database::System);

		// ALTER AREA、DROP AREAとそれに伴うALTER TABLE、ALTER INDEX相当の処理の
		// UNDOをする
		// ただしファイルの移動や破棄を伴う処理はしない
		//
		// ★注意★
		// 以前はデータベースのログによるUNDO処理なので常にredoされるはずであるとしていたが
		// Rollbackに失敗するとredoされないときにundoが実行されることがある

		// Mount処理内容を記述するオブジェクトを作る
		Admin::Mount mountDB(cTrans_, cLogData_);
		// Undoする
		mountDB.undo(redone_);
#endif
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::undoUnmount
//		
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true
//				後で REDO される
//			false
//				後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoUnmount(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// Unmount のログはシステムおよびデータベースのログに書かれるように
	// 仕様変更されたのでアサートチェックはしない
	//; _SYDNEY_ASSERT(cDatabaseName_ == NameParts::Database::System);

	if (redone_)
	{
		// 論理ログデータより UNMOUNT したデータベースの名前を得て、
		// UNMOUNT が UNDO されたことを記憶しておく
		ObjectID::Value id = Database::getObjectID(cLogData_);

		// DROPのUNDOに際して親子関係のあるオブジェクトにも
		// UNDO DROPを記録するようにしていたが、データベースにUNDO DROPが記録される場合は
		// その子オブジェクトに対してリカバリー処理は行われないので記録する意味がない。
		// また、データベース以下のオブジェクトに関する記録のキーとしてデータベース名も
		// 使うためにここで記録すると誤った記録となる可能性がある

		// このデータベースの Unmount が UNDO されたことを記憶する
		Undo::enter(id, Undo::Type::Unmount);

		// UNMOUNTのUNDOを記録すると取得できるデータベースから除外されるので
		// データベース表のキャッシュを破棄する
		// ★注意★
		// このUNDO処理が行われるときにはロックの機構によりReadOnlyトランザクションは
		// 存在しないはずなので他のObjectSnapshotの面倒を見る必要はない
		Manager::ObjectSnapshot::get(cTrans_)->clearDatabaseView();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::undoStartBackup
//		
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoStartBackup(Trans::Transaction& cTrans_, const LogData& cLogData_,
				const Object::Name& cDatabaseName_, bool bRollforward_)
{
	// 特に何もしない
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::undoEndBackup
//		
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoEndBackup(Trans::Transaction& cTrans_, const LogData& cLogData_,
			  const Object::Name& cDatabaseName_, bool bRollforward_)
{
	// 特に何もしない
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Database
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true
//				後で REDO される
//			false
//				後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoCreateDatabase(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// 論理ログデータから Schema ID を得る
	const ObjectID::Value id = Database::getObjectID(cLogData_);

	// ★注意★
	// Create Databaseのログはシステムとデータベースの両方に記録される
	// システム用の論理ログファイルに対してUNDOが実行される場合はデータベース表に対する操作、
	// データベース用の論理ログファイルに対してUNDOが実行される場合はデータベースに対する操作を行う
	// システム用の論理ログに対してはUNDO情報に関する操作も行う

	if (cDatabaseName_ == Object::Name(NameParts::Database::System)) {

		if (Undo::isEntered(id, Undo::Type::DropDatabase) ||
			Undo::isEntered(id, Undo::Type::Unmount)) {

			// CREATEしたデータベースがUNDO中にDROPまたはUNMOUNTされており、
			// 後でREDOされる場合はREDO処理を省くのでそれを判断するために
			// UNDOすることを記録しておく

			if (redone_)
				Undo::enter(id, Undo::Type::CreateDatabase);
			// REDOされない場合は何もしない

			return;

		} else if (redone_) {

			// CREATEしたデータベースがUNDO中にDROPまたはUNMOUNTされていないとき、
			// REDOされる場合はこれ以降の処理ですべてのパスを削除から除外するように登録する

			// データベースを構成するOSファイルはALTERなどによって移動したときの
			// 最終的な場所(つまりUNDO開始時にあった場所)を用いる

			ModVector<ModUnicodeString> paths;

			if (!Path::getUndoDatabasePath(id, paths)) {
				// ALTERのUNDOで登録されたパス指定がないときは
				// 移動していないということなのでログに記録されていた
				// パス指定を使用する
				Database::getPath(cLogData_, Database::Log::Create::Path, paths);

				// REDOされる場合はこのパスが最終的なパスになるので
				// パス指定を登録する
				Path::setUndoDatabasePath(id, paths);
			}

			// CREATE DATABASEのREDOではファイルが存在していることを前提とした処理が行われるので
			// 消してはいけないパスとしてデータベースに指定されているすべてのパスを除外リストに加える
			// → これ以降、リカバリーが終了するまで削除されなくなる
			const Object::Name& name = Database::getName(cLogData_);
			for (int i = 0; i < Database::Path::Category::ValueNum; ++i) {
				Path::addUnremovablePath(Database::getPath(static_cast<Database::Path::Category::Value>(i),
														   paths, name));
			}

			// REDOされる場合はデータベースの論理ログファイルに関するUNDOでファイルの破棄を行う
			// REDOされない場合はここでファイルの破棄も行ってしまう
			return;
		}

	}

	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System) ||
		(cDatabaseName_ == Object::Name(NameParts::Database::System) &&
		 !redone_));

	// データベース用の論理ログのUNDOのときはデータベースを構成するファイルに対する操作を行う
	// ★注意★
	// DROPやUNMOUNTがUNDOされているときはCREATEのREDOは行われないはずなので
	// ここでDROPやUNMOUNTがUNDOされているかを調べる必要はない
	// また、システム用の論理ログのUNDOで最終的なパスがUNDO情報として記録されている

	// システムの論理ログのUNDOのときでもREDOされない場合は同様の処理を行う
	// 上記注意に書かれた用件はこの場合でも成り立つ

	// REDOされる場合はシステム用の論理ログにおける処理ですべてのパスが除外リストにあるので
	// 以下の操作はまったく無駄なことをしていることになる
	// REDOされない場合のみ削除するようにする

	if (!redone_) {
		ModVector<ModUnicodeString> paths;
		if (!Path::getUndoDatabasePath(id, paths)) {
			Database::getPath(cLogData_, Database::Log::Create::Path, paths);
		}

		const Object::Name& name = Database::getName(cLogData_);

		for (int i = 0; i < Database::Path::Category::ValueNum; ++i) {
			// すべてのパスを削除
			Path::rmAllRemovable(Database::getPath(static_cast<Database::Path::Category::Value>(i),
												   paths, name));
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::undoDropDatabase
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true
//				後で REDO される
//			false
//				後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoDropDatabase(
	Trans::Transaction& cTrans_, const LogData& data_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// システム用の論理ログファイルに記録されているはず

	; _SYDNEY_ASSERT(
		cDatabaseName_ == Object::Name(NameParts::Database::System));

	if (redone_) {
		// DROPが後でREDOされるとき、UNDOされたことを記憶する
		// データベースに属するエリア、表、索引についてもDROPがUNDOされたことを記憶する

		// 論理ログデータより DROP したデータベースの Schema IDを得る
		Object::ID::Value id = Database::getObjectID(data_);

		// DROPのUNDOに際して親子関係のあるオブジェクトにも
		// UNDO DROPを記録するようにしていたが、データベースにUNDO DROPが記録される場合は
		// その子オブジェクトに対してリカバリー処理は行われないので記録する意味がない。
		// また、データベース以下のオブジェクトに関する記録のキーとしてデータベース名も
		// 使うためにここで記録すると誤った記録となる可能性がある

		// DROP が UNDO されたことを記憶する
		Undo::enter(id, Undo::Type::DropDatabase);

		// DROPのUNDOを記録すると取得できるデータベースから除外されるので
		// データベース表のキャッシュを破棄する
		// ★注意★
		// このUNDO処理が行われるときにはロックの機構によりReadOnlyトランザクションは
		// 存在しないはずなのでObjectSnapshotの面倒を見る必要はない
		Manager::ObjectSnapshot::get(cTrans_)->clearDatabaseView();
	}

	// DROPはUNDO不可な操作なのでREDOされるか否かに関係なく
	// データベースの格納ディレクトリー以下のファイルを破棄する

	// 論理ログデータより DROP したデータベースの格納場所の絶対パス名を得る
	// ★注意★
	// ALTERによりデータベースのパス指定が変更されていても
	// DROPのログに書かれるのは最終的なパス指定なので
	// ALTERでgetUndoDatabasePathを使用する必要はない

	ModVector<ModUnicodeString>	path;
	Database::getPath(data_, Database::Log::Drop::Path, path);

	// DROP したデータベースの格納場所以下の
	// すべての OS ファイル、OS ディレクトリを削除する
	// ただし、削除してはいけないと登録されているファイルは残す
	{
		unsigned int i = 0;
		const Object::Name& dbName = Database::getName(data_);

		do {
			Path::rmAllRemovable(
				(i < path.getSize() && path[i].getLength()) ? path[i] :
				static_cast<ModUnicodeString>(
					  Database::getDefaultPath(
						   static_cast<Database::Path::Category::Value>(i),
						   dbName)));

		} while (++i < Database::Path::Category::ValueNum) ;
	}
	// DROP したデータベースのエリアの格納場所以下の
	// すべての OS ファイル、OS ディレクトリを削除する

	Database::getAreaPath(data_, path);
	{
		ModSize n = path.getSize();
		for (ModSize i = 0; i < n; ++i) {
			Path::rmAllRemovable(path[i]);
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Database
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoMoveDatabase(Trans::Transaction& cTrans_, const LogData& cLogData_,
				 const Object::Name& cDatabaseName_, bool redone_,
				 bool bRollforward_)
{
	using namespace RecoveryUtility;

	// 処理対象のデータベース
	const ObjectID::Value id = Database::getObjectID(cLogData_);

	// databaseのDROPまたはUNMOUNTがUNDOされたか調べる
	if ( Undo::isEntered(id, Undo::Type::DropDatabase) ||
		 Undo::isEntered(id, Undo::Type::Unmount) )
		// どうせ消されるので何もしない
		return;

	// ★注意★
	// MoveDatabaseはシステムとデータベースの両方のログに書かれているはずである
	// システム用の論理ログファイルに対してUNDOが実行される場合はデータベース表に対する操作、
	// データベース用の論理ログファイルに対してUNDOが実行される場合はデータベースに対する操作を行う
	// システム用の論理ログに対してはUNDO情報に関する操作も行う

	if (cDatabaseName_ == Object::Name(NameParts::Database::System)) {

		// REDOされる場合、移動後のパス指定を記録する
		if (redone_) {

			// UNOD されたことを記録する
			Undo::enter(id, Undo::Type::MoveDatabase);

			// 使用されるはずの変更後のパス指定を記録する。
			ModVector<ModUnicodeString> vecPostPath;
			Database::getPath(cLogData_, Database::Log::Move::PostPath, vecPostPath);

			if (Path::setUndoDatabasePath(id, vecPostPath)) {
				// 新規に登録された場合、これが最終的なデータベースパスになる
				// REDOされる場合はこれ以降の処理ですべてのパスを削除から除外するように登録する
				const Object::Name& cName = Database::getName(cLogData_);
				for (int i = 0; i < Database::Path::Category::ValueNum; ++i) {
					Path::addUnremovablePath(Database::getPath(static_cast<Database::Path::Category::Value>(i),
															   vecPostPath, cName));
				}
			}
		}
	} else {

		; _SYDNEY_ASSERT(
			cDatabaseName_ != Object::Name(NameParts::Database::System));

		// REDOされない場合、データベースのログに関するUNDOでファイルを変更前の位置に移動する
		if (!redone_) {
			// ログから変更前、後のパス指定を取得する
			ModVector<ModUnicodeString> vecPrevPath;
			ModVector<ModUnicodeString> vecPostPath;
			Database::getPath(cLogData_, Database::Log::Move::PrevPath, vecPrevPath);
			Database::getPath(cLogData_, Database::Log::Move::PostPath, vecPostPath);

			// デフォルトのパスを使用する場合があるのでデータベース名をログから得ておく
			const Object::Name& cName = Database::getName(cLogData_);
			ModSize n = vecPrevPath.getSize();
			for ( ModSize i = 0; i < n; ++i ) {
				Os::Path cPrevPath(vecPrevPath[i]);
				Os::Path cPostPath(vecPostPath[i]);
				// パスが異なるもののみ処理すればよい
				if (cPrevPath.compare(cPostPath) != Os::Path::CompareResult::Identical) {
					// パス指定が空の場合はデフォルトのパスを使用する
					if (cPrevPath.getLength() == 0) {
						cPrevPath = Database::getDefaultPath(static_cast<Database::Path::Category::Value>(i), cName);
					}
					if (cPostPath.getLength() == 0) {
						cPostPath = Database::getDefaultPath(static_cast<Database::Path::Category::Value>(i), cName);
					}
					// ファイルの移動（すぐに移動、ディレクトリを作成、ファイルを上書き）
#ifdef DEBUG
					SydSchemaDebugMessage << "UNDO MoveDatabase: move file \"" << cPostPath << "\" -> \"" << cPrevPath << "\"" << ModEndl;
#endif
					Utility::File::move(cTrans_, cPostPath, cPrevPath,
										true /* force */,  true /* recovery, overwrite */);
				}
			}
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Database
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoAlterDatabase(Trans::Transaction& cTrans_, const LogData& cLogData_,
				  const Object::Name& cDatabaseName_, bool redone_,
				  bool bRollforward_)
{
	using namespace RecoveryUtility;

	// ★注意★
	// ALTER DATABASE文の論理ログは
	// READ WRITEへの変更のときにシステム専用の論理ログファイルと
	// データベースの論理ログファイルの両方に記録されるので
	// ここでcDatabaseName_のチェックをしない

	// 特に何もしない
}


//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Area
//		データベースの「エリア」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoCreateArea(Trans::Transaction& cTrans_, const LogData& cLogData_,
			   const Object::Name& cDatabaseName_, bool redone_,
			   bool bRollforward_)
{
	using namespace RecoveryUtility;

	ObjectID::Value id = Area::getObjectID(cLogData_);

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリの場合はこれ以上何もしない
	if (bRollforward_) return;

	// Area が Drop されているか調べる
	if ( Undo::isEntered(cDatabaseName_, id, Undo::Type::DropArea) ) {

		if ( redone_ )
			// REDO されるので記録しておく
			Undo::enter(cDatabaseName_, id, Undo::Type::CreateArea);

	} else {

		// CREATE したエリアの DROP または UNMOUNT が
		// UNDO されていないとき、
		// 使用中の表や索引がなければ
		// エリアのディレクトリ以下をすべて破棄する
		// ★注意★
		// 破棄しない場合もUndoAreaPathの設定は行う必要がある

		ModVector<ModUnicodeString> paths;

		// ALTER AREAやALTER TABLEがUNDOされている場合は最終的なパスを使用する
		if (!Path::getUndoAreaPath(cDatabaseName_, id, paths)) {
			// 最終的なパスが登録されていない場合はログに記録されているパスを使用する
			Area::getPath(cLogData_, Area::Log::Path, paths);

			if (redone_) {
				// REDOされる場合はこのパスが最終的なパスになるので
				// 登録する
				Path::setUndoAreaPath(cDatabaseName_, id, paths);
			}
		}
		else if (Undo::isEntered(cDatabaseName_, id, Undo::Type::AlterArea)) {
			// ALTER AREAがUNDOされているのでエリアの参照回数をデクリメントする
			ID::eraseUnremovableAreaID(cDatabaseName_, id);
		}

		ModVector<ModUnicodeString>::Iterator it = paths.begin();
		const ModVector<ModUnicodeString>::Iterator& fin = paths.end();

		// すべてのディレクトリを削除
		for ( ; it != fin; ++it ) {
			// 破棄してよいパスかどうか調べる
			if (Path::isRemovableAreaPath(cDatabaseName_, *it)) {
#ifdef DEBUG
				SydSchemaDebugMessage << "UNDO CreateArea: remove all \"" << *it << "\"" << ModEndl;
#endif
				Utility::File::rmAll(*it);
			}
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Area
//		データベースの「エリア」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoDropArea(Trans::Transaction& cTrans_, const LogData& cLogData_,
			 const Object::Name& cDatabaseName_, bool redone_,
			 bool bRollforward_)
{
	using namespace RecoveryUtility;

	// 論理ログデータより DROP したデータベースの Schema ID を得る

	ObjectID::Value id = Area::getObjectID(cLogData_);

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリの場合はこれ以上何もしない
	if (bRollforward_) return;

	// 論理ログデータより DROP した Area の格納場所の絶対パス名を得る

	ModVector<ModUnicodeString>	paths;
	Area::getPath(cLogData_, Area::Log::Path, paths);

	// MOUNTのUNDOでも使用できるようにファイルの破棄を伴わないUNDO処理を分離する
	undoDropArea(cTrans_, id, paths, cDatabaseName_, redone_);

	if (!redone_) {
		// 後に REDO されないとき、DROP した Area の格納場所以下の
		// すべての OS ファイル、OS ディレクトリを削除する
		ModVector<ModUnicodeString>::ConstIterator it = paths.begin();
		const ModVector<ModUnicodeString>::ConstIterator& fin = paths.end();

		for ( ; it != fin; ++it ) {
			// 破棄してよいパスかどうか調べる
			if (Path::isRemovableAreaPath(cDatabaseName_, *it)) {
#ifdef DEBUG
				SydSchemaDebugMessage << "UNDO DropArea: remove all \"" << *it << "\"" << ModEndl;
#endif
				Utility::File::rmAll(*it);
			}
		}
	}
}

// MOUNTのUNDOでも使用できるようにファイルの破棄を伴わないUNDO処理を分離する
void
Manager::SystemTable::UndoUtility::
undoDropArea(Trans::Transaction& cTrans_, ObjectID::Value areaID_,
			 const ModVector<ModUnicodeString>& paths_,
			 const Object::Name& cDatabaseName_,
			 bool redone_)
{
	using namespace RecoveryUtility;

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, areaID_);

	if (redone_) {
		// このエリアの DROP が UNDO されたことを記憶する
		Undo::enter(cDatabaseName_, areaID_, Undo::Type::DropArea);
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Area
//		データベースの「エリア」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoAlterArea(Trans::Transaction& cTrans_, const LogData& cLogData_,
			  const Object::Name& cDatabaseName_, bool redone_,
			  bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterAreaはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Area::getObjectID(cLogData_);

	if ( Undo::isEntered(cDatabaseName_, id, Undo::Type::DropArea) )
	{
		// どうせ Drop されるから何もしない
		return;
	}

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリの場合はこれ以上何もしない
	if (bRollforward_) return;

	// 変更前、後のパスを取得する
	ModVector<ModUnicodeString> vecPrevPath;
	ModVector<ModUnicodeString> vecPostPath;
	Area::getPath(cLogData_, Area::Log::Alter::PrevPath, vecPrevPath);
	Area::getPath(cLogData_, Area::Log::Alter::PostPath, vecPostPath);

	// ファイルの移動を伴わないUNDO処理を行う
	undoAlterArea(cTrans_, id, vecPrevPath, vecPostPath, cDatabaseName_, redone_);

	if (!redone_) {
		// REDOされない場合ディレクトリーを元に戻す
		ModSize n = vecPrevPath.getSize();
		for ( ModSize i = 0; i < n; ++i ) {
#ifdef DEBUG
			SydSchemaDebugMessage << "UNDO AlterArea: move file \"" << vecPostPath[i] << "\" -> \"" << vecPrevPath[i] << "\"" << ModEndl;
#endif
			Utility::File::move(cTrans_, vecPostPath[i], vecPrevPath[i],
								true /* force */, true /* recovery */);
		}
	}
}

// MOUNTのUNDOでも使用するため、undoAlterAreaの処理のうちファイルの移動を伴わない処理を分離する
void
Manager::SystemTable::UndoUtility::
undoAlterArea(Trans::Transaction& cTrans_,
			  ObjectID::Value id_,
			  const ModVector<ModUnicodeString>& vecPrevPath_,
			  const ModVector<ModUnicodeString>& vecPostPath_,
			  const Object::Name& cDatabaseName_,
			  bool redone_, bool bMount_ /* = false */)
{
	using namespace RecoveryUtility;

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, id_);

	if ( redone_ ) {
		// ALTER AREAが後でREDOされる場合、移動することを記録した上で
		// 最終的なパス情報を残す

		// MOUNTからも呼ばれた場合Dropのチェックをここで行う
		if (bMount_ && Undo::isEntered(cDatabaseName_, id_, Undo::Type::DropArea)) {
			// どうせ Drop されるから何もしない
			return;
		}

		bool bPathReplaced = false;		// Mount中に登録されたものを上書きするならtrue

		// REDO されるので記録しておく
		if (!bMount_) {
			// MOUNT中でない場合、すでにMOUNT中に登録されているパスがあったらそれをまず消去する
			if (Undo::isEntered(cDatabaseName_, id_, Undo::Type::AlterAreaInMount)) {
				Undo::remove(cDatabaseName_, id_, Undo::Type::AlterAreaInMount);
				Path::eraseUndoAreaPath(cDatabaseName_, id_);
				bPathReplaced = true;
			}
			Undo::enter(cDatabaseName_, id_, Undo::Type::AlterArea);
		} else {
			Undo::enter(cDatabaseName_, id_, Undo::Type::AlterAreaInMount);
		}

		// Alter されるパスの最新情報を記録しておく
		if (Path::setUndoAreaPath(cDatabaseName_, id_, vecPostPath_) && !bPathReplaced) {
			// 新規に登録された場合破棄してはいけないパスを持つエリアIDとして登録する
			ID::setUnremovableAreaID(cDatabaseName_, id_);
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true
//				後で REDO される
//			false
//				後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoCreateTable(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;
	// CreateTableはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// ログデータより Schema ID を取得
	const ObjectID::Value id = Table::getObjectID(cLogData_);

	// ログデータよりIDの最大値を取得
	ObjectID::Value iLastID = cLogData_.getID(Table::Log::Create::LastID);

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, iLastID);

	// ロールフォワードリカバリの場合はこれ以上何もしない
	if (bRollforward_) return;

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropTable)) {

		if (redone_)

			// CREATE したテーブルの DROP が UNDO されているとき、
			// CREATE が UNDO されたことを記憶しておく

			Undo::enter(cDatabaseName_, id, Undo::Type::CreateTable);

	} else {

		// CREATE したテーブルの DROP が UNDO されていないとき、
		// テーブルの格納ディレクトリ以下をすべて破棄する

		////////////////////////////////
		// データベースのパス指定を得る
		////////////////////////////////
		ModVector<ModUnicodeString> vecDatabasePath;
		Path::getEffectiveDatabasePath(cTrans_, cLogData_, Table::Log::DatabaseID, Table::Log::DatabasePaths,
									   cDatabaseName_, vecDatabasePath);

		///////////////////
		// エリア指定を得る
		///////////////////
		ModVector<Object::ID::Value> vecAreaID;
		ID::getEffectiveAreaID(cLogData_, Table::Log::Create::AreaIDs, id, cDatabaseName_, vecAreaID);

		// 上記で得られたデータベースのパス指定およびエリアIDをもとに
		// 表で使用しているファイルが格納されているディレクトリーを削除する
		// ★注意★
		// エリアについてはALTER AREAがUNDOされている可能性があるので
		// エリアIDから最終的なパスを取得しなければならない

		// ログデータから表名を得る
		Object::Name tableName;
		Name::getEffectiveName(cLogData_, Table::Log::Name, id, cDatabaseName_, tableName);

		ModVector<Object::ID::Value> vecRemovedAreaID; // 破棄したエリアIDを覚える
		ModSize n = vecAreaID.getSize();
		for (ModSize i = 0; i < n; ++i) {
			Object::ID::Value areaID = vecAreaID[i];
			if (vecRemovedAreaID.find(areaID) != vecRemovedAreaID.end())
				continue;
			vecRemovedAreaID.pushBack(areaID);

			ModVector<ModUnicodeString> vecAreaPath;
			if (areaID != Object::ID::Invalid) {
				// ALTER AREAのUNDOによりパスが登録されているか調べる
				if (!Path::getUndoAreaPath(cDatabaseName_, areaID, vecAreaPath)) {
					// 登録されていないのでログのパス配列を用いる
					Table::getAreaPath(cLogData_, Table::Log::Create::AreaPaths,
									   i, vecAreaPath);
				}
			}

			// データベースのパス指定とエリアのパスから
			// 破棄すべきパス名を得て削除する
#ifdef DEBUG
			SydSchemaDebugMessage << "UNDO CreateTable: remove all \""
								  << Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName) << "\"" << ModEndl;
#endif
			// Drop TableがUndoされておらず、Create TableがUndoされる場合
			// その表の最終的なパス、エリアID、名前によって得られるファイルは無条件に消してよい
			Utility::File::rmAll(Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName));
		}
		if (Undo::isEntered(cDatabaseName_, id, Undo::Type::AlterTable)) {
			// AlterTableがUNDOされているときは破棄してはいけないエリアIDの登録から削除する
			ID::eraseUnremovableAreaID(cDatabaseName_, vecAreaID);
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoDropTable(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// テーブルIDを取得
	const ObjectID::Value id = Table::getObjectID(cLogData_);
	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリの場合はこれ以上何もしない
	if (bRollforward_) return;

	if (redone_) {

		// 子オブジェクトに対してもDropを記録するためにIDリストを得る
		const ModVector<unsigned int>& vecIndexIDs = Table::getIndexID(cLogData_);

		// このテーブルの DROP が UNDO されたことを記録する
		Undo::enter(cDatabaseName_, id, Undo::Type::DropTable);
		// 索引に対してもDROPがUNDOされたことを記録する
		for (unsigned int iIndex = 0; iIndex < vecIndexIDs.getSize(); ++iIndex) {
			Undo::enter(cDatabaseName_, vecIndexIDs[iIndex], Undo::Type::DropIndex);
		}
	}

	// DROPはUNDO不可なのでREDOされるかどうかに関係なく使用しているパスを削除する

	// ログに表および索引に割り当てられているエリアのパスがすべて記録されている
	// DROPされた後のALTER AREAでDROPされたファイルまで移動することはないので
	// ほかの場合とは異なりALTER AREAがUNDOされている場合でもログに記録されているパスを使用する
	// また、データベースも同様だがマウント後のリカバリー処理の場合は
	// ログに記録されているパスではなく名称から取得できるデータベースオブジェクトのパス指定を使用する。

	////////////////////////////////
	// データベースのパス指定を得る
	////////////////////////////////

	ModVector<ModUnicodeString> vecDatabasePath;
	Path::getEffectiveDatabasePathInDrop(cTrans_, cLogData_, Table::Log::DatabaseID, Table::Log::DatabasePaths,
										 cDatabaseName_, vecDatabasePath);

	/////////////////////
	// エリアのパスを得る
	/////////////////////
	ModVector<ModUnicodeString> vecAreaPath;

	// ログデータから表名を得る
	Object::Name tableName(Table::getName(cLogData_));

	// 常にログのパス配列を用いる
	// ログデータに入っているパス配列のサイズが分からないので
	// 結果がfalseになるまで取得とファイルの削除を繰り返す

	int iIndex = 0;
	while (Table::getAreaPath(cLogData_, Table::Log::Drop::AreaPaths,
							  iIndex++, vecAreaPath)) {

		if (!vecAreaPath.isEmpty()) {
			// データベースのパス指定とエリアのパスから
			// 破棄すべきパス名を得て削除する
#ifdef DEBUG
			SydSchemaDebugMessage << "UNDO DropTable: remove all \""
								  << Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName)
								  << "\"" << ModEndl;
#endif
			// Drop tableをUNDOする場合、同じ名前の表が同じ場所にあったらファイルを消せない
			// したがって、消す対象となるパスが登録されているときのみ除外する
			Path::rmAllRemovable(Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName),
								 true /* immediately */, false /* not look parent */);

			// 次の取得のために空にしておく
			vecAreaPath.clear();
		}
	}
	// エリアのパスが空の状態で再びファイルの削除を実行することで
	// エリア指定なしのときのファイルを確実に削除する
#ifdef DEBUG
	SydSchemaDebugMessage << "UNDO DropTable: remove all \""
						  << Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName)
						  << "\"" << ModEndl;
#endif
	// Drop tableをUNDOする場合、同じ名前の表が同じ場所にあったらファイルを消せない
	// したがって、消す対象となるパスが登録されているときのみ除外する
	Path::rmAllRemovable(Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName),
						 true /* immediately */, false /* not look parent */);
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		ObjectID::Value tableID_,
//			テーブルＩＤ
//		const ModVector<ModUnicodeString>& vecPostPaths_
//			変更前パス
//		const ModVector<ModUnicodeString>& vecPrevPaths_
//			変更後パス
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoAlterTable(Trans::Transaction& cTrans_, const LogData& cLogData_,
			   const Object::Name& cDatabaseName_, bool redone_,
			   bool bRollforward_)
{
	using namespace RecoveryUtility;
	// AlterTableはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// テーブル ID を取得
	const ObjectID::Value tableID = Table::getObjectID(cLogData_);

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, tableID);

	// ロールフォワードリカバリの場合はこれ以上何もしない
	if (bRollforward_) return;

	// tableのDROPがUNDOされていると記録されている場合は
	// どうせ消されるので何もしない
	if ( Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable) )
		return;

	if ( redone_ ) {

		// 変更後のエリアのIDをログデータから取得する
		ModVector<Schema::Object::ID::Value> vecPostAreaID;
		Table::getAreaID(cLogData_, Table::Log::Alter::PostAreaIDs, vecPostAreaID);

		// MOUNTでも実行する部分は分離されている
		if (undoAlterTable(cTrans_, tableID, vecPostAreaID, cDatabaseName_, redone_)) {
			// エリアIDが新たに登録されたら後の処理で使うかもしれないので
			// ログのエリアパスを登録しておく
			// ★注意★
			// CREATE INDEX -> ALTER TABLE したものをUNDOするときにINDEXファイルのありかを
			// 知るにはALTER TABLE時点のエリアのパスが得られる必要があるため
			ModSize n = vecPostAreaID.getSize();
			for (ModSize i = 0; i < n; ++i) {
				if (vecPostAreaID[i] != Object::ID::Invalid) {
					ModVector<ModUnicodeString> vecPostAreaPath;
					// エリアに対応するパスが登録されていなければログのパスを登録する
					if (!Path::getUndoAreaPath(cDatabaseName_, vecPostAreaID[i], vecPostAreaPath)) {
						// 登録されていない場合はログから取得して登録する
						Table::getAreaPath(cLogData_, Table::Log::Alter::PostAreaPaths, i, vecPostAreaPath);
						Path::setUndoAreaPath(cDatabaseName_, vecPostAreaID[i], vecPostAreaPath);
					}
				}
			}
		}

	} else {

		// REDO されないので論理ファイルのメソッドを介さずに移動する

		////////////////////////////////////////
		// 最終的なデータベースのパス指定を得る
		////////////////////////////////////////
		ModVector<ModUnicodeString> vecDatabasePath;
		Path::getEffectiveDatabasePath(cTrans_, cLogData_, Table::Log::DatabaseID, Table::Log::DatabasePaths,
									   cDatabaseName_, vecDatabasePath);

		///////////////////////////
		// 最終的なエリア指定を得る
		///////////////////////////
		ModVector<Object::ID::Value> vecPrevAreaID;
		ModVector<Object::ID::Value> vecPostAreaID;
		// すでにALTERのUNDOが登録されている場合、以下の処理で戻した結果が
		// そのエリア指定にあっている必要があるので移動前のエリア指定を最終的なものにする
		ID::getEffectiveAreaID(cLogData_, Table::Log::Alter::PrevAreaIDs, tableID, cDatabaseName_, vecPrevAreaID);
		Table::getAreaID(cLogData_, Table::Log::Alter::PostAreaIDs, vecPostAreaID);

		// 移動前後の表名を設定する
		// (移動前の名称を最終的なものにする)
		Object::Name cPrevName;
		Object::Name cPostName;
		Name::getEffectiveName(cLogData_, Table::Log::Name, tableID, cDatabaseName_, cPrevName);
		cPostName = Table::getName(cLogData_);

		// 移動したファイルを元に戻す
		undoMovedFiles(cTrans_, cLogData_, cDatabaseName_, tableID,
					   cPrevName, cPostName,
					   vecDatabasePath, vecPrevAreaID, vecPostAreaID,
					   Table::Log::Alter::MovedFiles,
					   Table::Log::Alter::PrevAreaPaths,
					   Table::Log::Alter::PostAreaPaths);
	}
}

// MOUNTでも実行する部分は分離されている
bool
Manager::SystemTable::UndoUtility::
undoAlterTable(Trans::Transaction& cTrans_,
			   const ObjectID::Value tableID_,
			   const ModVector<ObjectID::Value>& vecPostArea_,
			   const Object::Name& cDatabaseName_,
			   bool redone_, bool bMount_ /* = false */)
{
	using namespace RecoveryUtility;

	// tableID_に対応するエリア割り当てとして新規に登録されたらtrueを返す
	bool bResult = false;

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, tableID_);

	if (redone_) {

		// MOUNTから呼ばれる場合はここでチェックする
		if (bMount_ && Undo::isEntered(cDatabaseName_, tableID_, Undo::Type::DropTable))
			// どうせ消されるので何もしない
			return bResult;

		// REDO されるので記録する
		if (!bMount_) {
			// MOUNT中でない場合、すでにMOUNT中に登録されているエリアがあったらそれをまず消去する
			if (Undo::isEntered(cDatabaseName_, tableID_, Undo::Type::AlterTableInMount)) {
				Undo::remove(cDatabaseName_, tableID_, Undo::Type::AlterTableInMount);
				// 現在登録されているエリアをUnremovableの登録から除くために取得する
				ModVector<ObjectID::Value> vecArea;
				if (ID::getUndoAreaID(cDatabaseName_, tableID_, vecArea)) {
					ID::eraseUnremovableAreaID(cDatabaseName_, vecArea);
					// 登録されているエリアを消去する
					ID::eraseUndoAreaID(cDatabaseName_, tableID_);
				}
			}
			Undo::enter(cDatabaseName_, tableID_, Undo::Type::AlterTable);
		} else {
			Undo::enter(cDatabaseName_, tableID_, Undo::Type::AlterTableInMount);
		}

		// 処理対象エリアを記録する
		if (ID::setUndoAreaID(cDatabaseName_, tableID_, vecPostArea_)) {
			// 新規にエリアが登録されたらそのエリアIDを破棄対象から除くために登録する
			ID::setUnremovableAreaID(cDatabaseName_, vecPostArea_);
			// 返り値はtrueになる
			bResult = true;
		}
	}
	return bResult;
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoRenameTable(Trans::Transaction& cTrans_, const LogData& cLogData_,
				const Object::Name& cDatabaseName_, bool redone_,
				bool bRollforward_)
{
	using namespace RecoveryUtility;
	// AlterTableはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// テーブル ID を取得
	const ObjectID::Value tableID = Table::getObjectID(cLogData_);

	// Renameの場合はtableのDROPがUNDOされていると記録されていても、
	// 以降の処理をやる必要がある

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, tableID);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	////////////////////////////////////////
	// 最終的なデータベースのパス指定を得る
	////////////////////////////////////////
	ModVector<ModUnicodeString> vecDatabasePath;
	Path::getEffectiveDatabasePath(cTrans_, cLogData_, Table::Log::DatabaseID, Table::Log::DatabasePaths,
								   cDatabaseName_, vecDatabasePath);

	if ( redone_ ) {

		// REDO されるので記録する
		Undo::enter(cDatabaseName_, tableID, Undo::Type::RenameTable);

		// 処理対象の名前を記録する
		ModUnicodeString cTableName = cLogData_.getString(Table::Log::Rename::PostName);
		if (Name::setUndoName(cDatabaseName_, tableID, cTableName)) {
			// 新規に登録された場合は最終的なパスを用いて削除禁止の登録をする

			// 最終的なエリア指定
			ModVector<Object::ID::Value> vecAreaID;
			ID::getEffectiveAreaID(cLogData_, Table::Log::Rename::AreaIDs, tableID, cDatabaseName_, vecAreaID);

			ModSize nArea = vecAreaID.getSize();
			for (ModSize iArea = 0; iArea < nArea; ++iArea) {
				ModVector<ModUnicodeString> vecAreaPath;
				if (!Path::getUndoAreaPath(cDatabaseName_, vecAreaID[iArea], vecAreaPath)) {
					// 登録されていない場合はログから取得する
					// 実際にファイルに適用されるパスを使用する
					Table::getEffectiveAreaPath(cLogData_, Table::Log::Rename::AreaPaths, iArea,
												vecAreaID, vecAreaPath);
					if (!vecAreaPath.isEmpty()) {
						// エリアの指定があればエリアの下のファイルを削除不可にする
						Os::Path cPath = Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, cTableName);
						Path::addUnremovablePath(cPath);
						Path::addUsedPath(cPath);
					}
				}
			}
			// エリア指定がない場合のパスも削除不可にする
			Os::Path cPath = Table::getPath(vecDatabasePath, ModVector<ModUnicodeString>(), cDatabaseName_, cTableName);
			Path::addUnremovablePath(cPath);
			Path::addUsedPath(cPath);
		}

		// 変更後のファイルの名前も取得して登録する
		const Common::DataArrayData& cData = cLogData_.getDataArrayData(Table::Log::Rename::MovedFiles);
		int n = cData.getCount() / 4;
		for (int i = 0; i < n; ++i) {
			ModVector<Object::ID::Value> vecParentID;
			ModVector<Object::ID::Value> vecFileID;
			ModVector<ModUnicodeString> vecPrevFileName;
			ModVector<ModUnicodeString> vecPostFileName;
			Table::getMovedFiles(cData, i, vecParentID, vecFileID,
								 vecPrevFileName, vecPostFileName, true /* rename */);

			; _SYDNEY_ASSERT(vecParentID.getSize() == vecFileID.getSize());
			; _SYDNEY_ASSERT(vecFileID.getSize() == vecPrevFileName.getSize());
			; _SYDNEY_ASSERT(vecFileID.getSize() == vecPostFileName.getSize());

			ModVector<Object::ID::Value>::ConstIterator iteratorParentID = vecParentID.begin();
			const ModVector<Object::ID::Value>::ConstIterator endParentID = vecParentID.end();
			ModVector<Object::ID::Value>::ConstIterator iteratorID = vecFileID.begin();
			const ModVector<Object::ID::Value>::ConstIterator endID = vecFileID.end();
			ModVector<ModUnicodeString>::ConstIterator iteratorName = vecPostFileName.begin();
			const ModVector<ModUnicodeString>::ConstIterator endName = vecPostFileName.end();
			for (; iteratorID != endID; ++iteratorID, ++iteratorParentID, ++iteratorName) {
				if (*iteratorID != Object::ID::Invalid) {
					if (*iteratorParentID != tableID) {
						// if parent's id is not equals to table's id, use parent id
						if (*iteratorParentID == *iteratorID)
							// if parentID == ID, that means this is primary key index name
							Name::setUndoName(cDatabaseName_, *iteratorID, *iteratorName);
						else
							// otherwise, it's filename
							Name::setUndoFileName(cDatabaseName_, *iteratorParentID, *iteratorName);
					} else
						Name::setUndoFileName(cDatabaseName_, *iteratorID, *iteratorName);
				}
			}
		}

	} else {

		// REDO されないので論理ファイルのメソッドを介さずに移動する

		////////////////////////////////////////
		// 最終的なエリア指定、名前指定を得る
		////////////////////////////////////////
		ModVector<Object::ID::Value> vecPrevAreaID;
		ModVector<Object::ID::Value> vecPostAreaID;
		// Area指定を変更するALTERのUNDOがすでに登録されている場合、以下の処理で戻した結果が
		// そのエリア指定にあっている必要があるので移動前のエリア指定を最終的なものにする
		// 移動後はログの指定を使う
		ID::getEffectiveAreaID(cLogData_, Table::Log::Rename::AreaIDs, tableID, cDatabaseName_, vecPrevAreaID);
		Table::getAreaID(cLogData_, Table::Log::Rename::AreaIDs, vecPostAreaID);

		// 名前の変更が繰り返された場合、最終的な名前を用いる
		Object::Name cPrevName;
		Object::Name cPostName;
		Name::getEffectiveName(cLogData_, Table::Log::Rename::PrevName, tableID, cDatabaseName_, cPrevName);
		cPostName = cLogData_.getString(Table::Log::Rename::PostName);

		// 移動したファイルを元に戻す
		undoMovedFiles(cTrans_, cLogData_, cDatabaseName_, tableID,
					   cPrevName, cPostName,
					   vecDatabasePath, vecPrevAreaID, vecPostAreaID,
					   Table::Log::Rename::MovedFiles,
					   Table::Log::Rename::AreaPaths,
					   Table::Log::Rename::AreaPaths);
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoAddColumn(Trans::Transaction& cTrans_, const LogData& cLogData_,
			  const Object::Name& cDatabaseName_, bool redone_,
			  bool bRollforward_)
{
	using namespace RecoveryUtility;
	// AlterTableはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// テーブル ID を取得
	const ObjectID::Value tableID = Table::getObjectID(cLogData_);

	// tableのDROPがUNDOされていると記録されている場合は
	// どうせ消されるので何もしない
	if ( Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable) )
		return;

	// ログデータよりIDの最大値を取得
	ObjectID::Value iLastID = cLogData_.getID(Table::Log::AlterAddColumn::LastID);

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, iLastID);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	// REDO されてもされなくても実際にUNDOする

	////////////////////////////////////////
	// 最終的なデータベースのパス指定を得る
	////////////////////////////////////////
	ModVector<ModUnicodeString> vecDatabasePath;
	Path::getEffectiveDatabasePath(cTrans_, cLogData_, Table::Log::DatabaseID, Table::Log::DatabasePaths,
								   cDatabaseName_, vecDatabasePath);

	///////////////////////////
	// 最終的なエリア指定を得る
	///////////////////////////
	ModVector<Object::ID::Value> vecAreaID;
	// すでにALTERのUNDOが登録されている場合、以下の処理で戻した結果が
	// そのエリア指定にあっている必要があるのでエリア指定を最終的なものにする
	ID::getEffectiveAreaID(cLogData_, Table::Log::AlterAddColumn::AreaIDs, tableID, cDatabaseName_, vecAreaID);

	// 表名を最終的なものに設定する
	Object::Name cTableName;
	Name::getEffectiveName(cLogData_, Table::Log::Name, tableID, cDatabaseName_, cTableName);

	// AddColumnによって作成されたファイルを破棄する
	undoCreatedFiles(cTrans_, cLogData_, cDatabaseName_, tableID,
					 cTableName, vecDatabasePath, vecAreaID,
					 Table::Log::AlterAddColumn::PostFiles,
					 Table::Log::AlterAddColumn::AreaPaths);

	// 中心レコードが退避されていたら戻す
	undoCreatedFiles(cTrans_, cLogData_, cDatabaseName_, tableID,
					 cTableName, vecDatabasePath, vecAreaID,
					 Table::Log::AlterAddColumn::PrevFiles,
					 Table::Log::AlterAddColumn::AreaPaths,
					 Table::Log::AlterAddColumn::TempSuffix);
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoAlterColumn(Trans::Transaction& cTrans_, const LogData& cLogData_,
				const Object::Name& cDatabaseName_, bool redone_,
				bool bRollforward_)
{
	// 何もしない
	;
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoAddConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoAddConstraint(Trans::Transaction& cTrans_,
				  const LogData& cLogData_,
				  const Schema::Object::Name& cDatabaseName_,
				  bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get tableID from the log data
	const ObjectID::Value tableID = Table::getObjectID(cLogData_);

	// When drop table has been undone, there are nothing to do
	if (Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable)) {
		return;
	}

	// get last schema objectID from the log data to ensure the consistency
	ObjectID::Value iLastID = cLogData_.getID(Table::Log::AlterAddConstraint::LastID);
	// set consistency about last ID
	ID::setUsedID(cDatabaseName_, iLastID);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	////////////////////////////////////////
	// get last effective database path
	////////////////////////////////////////
	ModVector<ModUnicodeString> vecDatabasePath;
	Path::getEffectiveDatabasePath(cTrans_, cLogData_, Table::Log::DatabaseID, Table::Log::DatabasePaths,
								   cDatabaseName_, vecDatabasePath);

	/////////////////////////////////////////
	// get last effective area specification
	/////////////////////////////////////////
	ModVector<Object::ID::Value> vecAreaID;
	ID::getEffectiveAreaID(cLogData_, Table::Log::AlterAddConstraint::AreaIDs, tableID,
						   cDatabaseName_, vecAreaID);

	/////////////////////////////
	// get last effective name
	/////////////////////////////
	Object::Name cTableName;
	Name::getEffectiveName(cLogData_, Table::Log::Name, tableID, cDatabaseName_, cTableName);

	// delete created files
	undoCreatedFiles(cTrans_, cLogData_, cDatabaseName_, tableID,
					 cTableName, vecDatabasePath, vecAreaID,
					 Table::Log::AlterAddConstraint::CreatedFiles,
					 Table::Log::AlterAddConstraint::AreaPaths);
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoDropConstraint -- 
//
// NOTES
//	not supported yet
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoDropConstraint(Trans::Transaction& cTrans_,
				   const LogData& cLogData_,
				   const Schema::Object::Name& cDatabaseName_,
				   bool redone_, bool bRollforward_)
{
	// not supported yet, so this method never be called.
	; _SYDNEY_ASSERT(false);
}

// FUNCTION private
//	Schema::Manager::SystemTable::UndoUtility::undoMovedFiles -- 移動したファイルを元に戻す
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Object::Name& cDatabaseName_
//	const Object::Name& cPrevName_
//	const ModVector<ModUnicodeString>& vecDatabasePath_
//	const ModVector<Object::ID::Value>& vecPrevAreaID_
//	const ModVector<Object::ID::Value>& vecPostAreaID_
//	int iLogIndexMovedFiles_
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoMovedFiles(Trans::Transaction& cTrans_, const LogData& cLogData_,
			   const Object::Name& cDatabaseName_, const ObjectID::Value& iTableID_,
			   const Object::Name& cPrevName_, const Object::Name& cPostName_,
			   const ModVector<ModUnicodeString>& vecDatabasePath_,
			   const ModVector<Object::ID::Value>& vecPrevAreaID_,
			   const ModVector<Object::ID::Value>& vecPostAreaID_,
			   int iLogIndexMovedFiles_, int iLogIndexPrevAreaPaths_, int iLogIndexPostAreaPaths_)
{
	using namespace RecoveryUtility;

	// 移動したファイルを元に戻す
	ModSize n = vecPrevAreaID_.getSize();
	; _SYDNEY_ASSERT(vecPostAreaID_.getSize() == n);
	; _SYDNEY_ASSERT(n <= ModSize(AreaCategory::ValueNum));

	bool bIsDifferentName = (cPrevName_ != cPostName_);
	bool bRename = (cLogData_.getSubCategory() == LogData::Category::RenameTable);

	for (ModSize i = 0; i < n; ++i) {
		// 違いのあるカテゴリーについて調べればよい
		// ただし名前の変更があったらすべてに対して実行する
		if (bIsDifferentName || vecPrevAreaID_[i] != vecPostAreaID_[i]) {
			// ログから対応するエリアカテゴリーで移動したファイル名とその親IDを得る
			ModVector<Object::ID::Value> vecParentID;
			ModVector<Object::ID::Value> vecFileID;
			ModVector<ModUnicodeString> vecPrevFileName;
			ModVector<ModUnicodeString> vecPostFileName;
			Table::getMovedFiles(cLogData_.getDataArrayData(iLogIndexMovedFiles_),
								 i, vecParentID, vecFileID, vecPrevFileName, vecPostFileName,
								 bRename);

			ModSize nFiles = vecFileID.getSize();
			; _SYDNEY_ASSERT(vecParentID.getSize() == nFiles);
			; _SYDNEY_ASSERT(vecPrevFileName.getSize() == nFiles);
			; _SYDNEY_ASSERT(vecPostFileName.getSize() == nFiles);
			for (ModSize iFiles = 0; iFiles < nFiles; ++iFiles) {
				Object::ID::Value iParentID = vecParentID[iFiles];
				Object::ID::Value iFileID = vecFileID[iFiles];
				Object::Name cPrevFileName(vecPrevFileName[iFiles]);
				Object::Name cPostFileName(vecPostFileName[iFiles]);

				if (iParentID != iTableID_) {
					// 親がこの表でない場合はALTERにより最終的なエリア指定が登録されているか調べる
					ModVector<Object::ID::Value> vecTmpAreaID;
					if (ID::getUndoAreaID(cDatabaseName_, iParentID, vecTmpAreaID)) {
						// 登録されておりそれがInvalidでなければ移動してはいけない
						if (vecTmpAreaID.getSize() > 0
							&& vecTmpAreaID[0] != Object::ID::Invalid) {
							continue;
						}
					}
					// 移動前のファイル名は最終的なものにする(ParentIDを使う)
					(void)Name::getUndoFileName(cDatabaseName_, iParentID, cPrevFileName);
				} else {
					// 移動前のファイル名は最終的なものにする(FileIDを使う)
					(void)Name::getUndoFileName(cDatabaseName_, iFileID, cPrevFileName);
				}

				// 移動前後のエリアIDから最終的なパス指定が登録されているか調べる
				ModVector<ModUnicodeString> vecPrevAreaPath;
				ModVector<ModUnicodeString> vecPostAreaPath;
				if (!Path::getUndoAreaPath(cDatabaseName_, vecPrevAreaID_[i], vecPrevAreaPath)) {
					// 登録されていない場合はログから取得する
					// 実際にファイルに適用されるパスを使用する
					Table::getEffectiveAreaPath(cLogData_, iLogIndexPrevAreaPaths_, i,
												vecPrevAreaID_, vecPrevAreaPath);
				}
				if (!Path::getUndoAreaPath(cDatabaseName_, vecPostAreaID_[i], vecPostAreaPath)) {
					// 登録されていない場合はログから取得する
					// 実際にファイルに適用されるパスを使用する
					Table::getEffectiveAreaPath(cLogData_, iLogIndexPostAreaPaths_, i,
												vecPostAreaID_, vecPostAreaPath);
				}
				// 移動する
#ifdef DEBUG
				SydSchemaDebugMessage << "UNDO AlterTable: move file \""
									  << Table::getPath(vecDatabasePath_, vecPostAreaPath, cDatabaseName_, cPostName_).addPart(cPostFileName)
									  << "\" -> \""
									  << Table::getPath(vecDatabasePath_, vecPrevAreaPath, cDatabaseName_, cPrevName_).addPart(cPrevFileName)
									  << "\"" << ModEndl;
#endif
				Utility::File::move(cTrans_,
									Table::getPath(vecDatabasePath_, vecPostAreaPath, cDatabaseName_, cPostName_).addPart(cPostFileName),
									Table::getPath(vecDatabasePath_, vecPrevAreaPath, cDatabaseName_, cPrevName_).addPart(cPrevFileName),
									true /* force */, true /* recovery */);
				// 移動前(ログ上は移動後)の親ディレクトリーが空になったら削除する
				Utility::File::rmEmpty(Table::getPath(vecDatabasePath_, vecPostAreaPath, cDatabaseName_, cPostName_));
			}
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::undoCreateIndex
//		データベースの「インデックス」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true
//				後で REDO される
//			false
//				後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoCreateIndex(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;
	// CreateIndexはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// ログデータより Schema ID を取得
	const ObjectID::Value id = Index::getObjectID(cLogData_);

	// ログデータよりIDの最大値を取得
	ObjectID::Value iLastID = id;
	if (cLogData_.getCount() >= Index::Log::Create::Num1) {
		iLastID = cLogData_.getID(Index::Log::Create::LastID);
	}

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, iLastID);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropIndex)) {
		if (redone_)

			// CREATE した Index の DROP が UNDO されているとき、
			// CREATE が UNDO されたことを記憶しておく

			Undo::enter(cDatabaseName_, id, Undo::Type::CreateIndex);

	} else {

		// CREATE した Index の DROP が UNDO されていないとき、
		// 索引の格納ディレクトリ以下をすべて破棄する

		////////////////////////////////
		// データベースのパス指定を得る
		////////////////////////////////
		ModVector<ModUnicodeString> vecDatabasePath;
		Path::getEffectiveDatabasePath(cTrans_, cLogData_, Index::Log::DatabaseID, Index::Log::DatabasePaths,
									   cDatabaseName_, vecDatabasePath);

		///////////////////
		// エリア指定を得る
		///////////////////

		// UNDO情報があったらそちらから得る
		ModVector<Object::ID::Value> vecAreaID;
		ID::getEffectiveAreaID(cLogData_, Index::Log::Create::AreaIDs, id, cDatabaseName_, vecAreaID);

		// 表に対するエリア割り当てを取得する
		// UNDO情報があったらそちらから得る
		Object::ID::Value tableID = Index::getTableID(cLogData_);
		ModVector<Object::ID::Value> vecTableAreaID;
		ID::getEffectiveAreaID(cLogData_, Index::Log::Create::TableAreaIDs, tableID, cDatabaseName_, vecTableAreaID);

		// 索引のエリアカテゴリーを得る
		AreaCategory::Value eAreaCategory = Index::getAreaCategory(cLogData_);

		// 索引に対する割り当てと表に対する割り当てから索引を構成するファイルが
		// 格納されているエリアを得る
		ModVector<Object::ID::Value> vecEffectiveAreaID;
		Index::getEffectiveAreaID(vecAreaID, vecTableAreaID, eAreaCategory, vecEffectiveAreaID);

		// 上記で得られたデータベースのパス指定および実質的に割り当てられているエリアIDをもとに
		// 索引で使用しているファイルが格納されているディレクトリーを削除する
		// ★注意★
		// 索引を構成するファイルを削除した後でそのディレクトリーが
		// 空になったらディレクトリーも破棄する
		// エリアについてはALTER AREAがUNDOされている可能性があるので
		// エリアIDから最終的なパスを取得しなければならない

		// ログから表名を得る
		// 最終的な表名が登録されていたらそれを使う
		Object::Name tableName;
		Name::getEffectiveName(cLogData_, Index::Log::TableName, tableID, cDatabaseName_, tableName);

		// ログから索引を構成するファイル名を取得する
		// 最終的なファイル名が登録されていたらそれを使う
		Object::Name cFileName;
		Name::getEffectiveFileName(cLogData_, Index::Log::FileName, id, cDatabaseName_, cFileName);

		ModSize n = vecEffectiveAreaID.getSize();
		for (ModSize i = 0; i < n; ++i) {
			Object::ID::Value areaID = vecEffectiveAreaID[i];

			ModVector<ModUnicodeString> vecAreaPath;
			if (areaID != Object::ID::Invalid) {
				// ALTER AREAのUNDOによりパスが登録されているか調べる
				if (!Path::getUndoAreaPath(cDatabaseName_, areaID, vecAreaPath)) {
					// 登録されていないのでログのパス配列を用いる
					// ★注意★
					// ALTER TABLEおよびALTER INDEXのUNDOでもエリアのパスを登録しているので
					// 登録されていないならログのパス配列の位置にファイルがあることを意味する
					if (!Index::getAreaPath(cLogData_, Index::Log::Create::AreaPaths,
											i, vecAreaPath)) {
						// ログデータにパスが登録されていないはずがない
						_SYDNEY_THROW0(Exception::LogItemCorrupted);
					}
				}
			}

			// データベースのパス指定とエリアのパスから
			// 破棄すべきパス名を得て削除する
#ifdef DEBUG
			SydSchemaDebugMessage << "UNDO CreateIndex: remove all \""
								  << Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, cFileName)
								  << "\"" << ModEndl;
#endif
			// Drop IndexがUndoされておらず、Create IndexがUndoされる場合
			// その索引の最終的なパス、エリアID、名前によって得られるファイルは無条件に消してよい
			Utility::File::rmAll(Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, cFileName));

			// 索引ファイルを破棄した後にファイルを格納しているディレクトリーが空ならディレクトリーも破棄する
			// ★注意★
			// 索引ファイルを格納するディレクトリーは同じエリアパスを使用して表のパスを得ることで得られる
			
			Os::Path parent(Table::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName));

			// ファイルリストを取得
			ModVector<ModUnicodeString> list;
			if (Utility::File::getFileList(parent, list)) {
				if ( list.getSize() <= 0 ) {
					// 以下にファイルが無ければ削除する
#ifdef DEBUG
					SydSchemaDebugMessage << "UNDO CreateIndex: remove all \"" << parent << "\"" << ModEndl;
#endif
					Utility::File::rmAll(parent);
				}
			}
		}

		if (Undo::isEntered(cDatabaseName_, id, Undo::Type::AlterIndex)) {
			// AlterIndexがUNDOされているときは破棄してはいけないエリアIDの登録から削除する
			ID::eraseUnremovableAreaID(cDatabaseName_, vecAreaID);
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::undoDropIndex
//		データベースの「インデックス」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::UndoUtility::undoDropIndex(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	const ObjectID::Value id = Index::getObjectID(cLogData_);

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (redone_)

		// このインデックスの DROP が UNDO されたことを記憶する

		Undo::enter(cDatabaseName_, id, Undo::Type::DropIndex);

	// DROPはUNDO不可なのでREDOされるかどうかに関係なく使用しているパスを削除する

	// ログに索引に割り当てられているエリアのパスがすべて記録されている
	// DROPされた後のALTER AREAでDROPされたファイルまで移動することはないので
	// ほかの場合とは異なりALTER AREAがUNDOされている場合でもログに記録されているパスを使用する
	// また、データベースも同様だがマウント後のリカバリー処理の場合は
	// ログに記録されているパスではなく名称から取得できるデータベースオブジェクトのパス指定を使用する。

	////////////////////////////////
	// データベースのパス指定を得る
	////////////////////////////////

	ModVector<ModUnicodeString> vecDatabasePath;
	Path::getEffectiveDatabasePathInDrop(cTrans_, cLogData_, Index::Log::DatabaseID, Index::Log::DatabasePaths,
										 cDatabaseName_, vecDatabasePath);

	/////////////////////
	// エリアのパスを得る
	/////////////////////
	ModVector<ModUnicodeString> vecAreaPath;

	// ログデータから表名とファイル名を得る
	// 最終的な表名が登録されていたらそれを使う
	// ファイル名はDropされてから変わることはありえないのでログのものを使う
	Object::ID::Value tableID = Index::getTableID(cLogData_);
	Object::Name tableName;
	Name::getEffectiveName(cLogData_, Index::Log::TableName, tableID, cDatabaseName_, tableName);
	Object::Name fileName(Index::getFileName(cLogData_));

	// 常にログのパス配列を用いる
	// ログデータに入っているパス配列のサイズが分からないので
	// 結果がfalseになるまで取得とファイルの削除を繰り返す

	int iIndex = 0;
	while (Index::getAreaPath(cLogData_, Index::Log::Drop::AreaPaths,
							  iIndex++, vecAreaPath)) {

		if (!vecAreaPath.isEmpty()) {
			// データベースのパス指定とエリアのパスから
			// 破棄すべきパス名を得て削除する
#ifdef DEBUG
			SydSchemaDebugMessage << "UNDO DropIndex: remove all \""
								  << Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, fileName)
								  << "\"" << ModEndl;
#endif
			// Drop tableをUNDOする場合、同じ名前の索引が同じ場所にあったらファイルを消せない
			// したがって、消す対象となるパスが登録されているときのみ除外する
			Path::rmAllRemovable(Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, fileName),
								 true /* immediately */, false /* not look parent */);

			// 次の取得のために空にしておく
			vecAreaPath.clear();
		}
	}
	// エリアのパスが空の状態で再びファイルの削除を実行することで
	// エリア指定なしのときのファイルを確実に削除する
#ifdef DEBUG
	SydSchemaDebugMessage << "UNDO DropIndex: remove all \""
						  << Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, fileName)
						  << "\"" << ModEndl;
#endif
	// Drop tableをUNDOする場合、同じ名前の索引が同じ場所にあったらファイルを消せない
	// したがって、消す対象となるパスが登録されているときのみ除外する
	Path::rmAllRemovable(Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, fileName),
						 true /* immediately */, false /* not look parent */);
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Index
//		データベースの「インデックス」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
// 		const ObjectID::Value indexID_
//			処理対象インデックスの Schema ID
// 		const ModVector<ModUnicodeString>& vecPrevPath_
//			変更前パス
//		const ModVector<ModUnicodeString>& vecPostPath_
//			変更後パス
//		const ModVector<ObjectID::Value>& vecPostArea_
//			変更先エリア ID
//		bool redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoAlterIndex(Trans::Transaction& cTrans_, const LogData& cLogData_,
			   const Object::Name& cDatabaseName_, bool redone_,
			   bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropIndexはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// Schema IDを取得
	const ObjectID::Value indexID = Index::getObjectID(cLogData_);

	// indexのDROPがUNDOされていると記録されている場合は
	// どうせ消されるので何もしない
	if ( Undo::isEntered(cDatabaseName_, indexID, Undo::Type::DropIndex) )
		return;

	// ログに記録されていたIDを使用済みとして登録する
	ID::setUsedID(cDatabaseName_, indexID);

	// ログに記録されていたファイルのスキーマオブジェクトIDを得て使用済みに登録する
	ObjectID::Value fileID = cLogData_.getID(Index::Log::Alter::FileID);
	ID::setUsedID(cDatabaseName_, fileID);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (redone_) {

		// 変更後のエリアのIDをログデータから取得する
		ModVector<Schema::Object::ID::Value> vecPostAreaID;
		Index::getAreaID(cLogData_, Index::Log::Alter::PostAreaID, vecPostAreaID);

		// MOUNTでも実行する処理を分離してある
		if (undoAlterIndex(cTrans_, indexID, vecPostAreaID, cDatabaseName_, redone_)) {
			// エリアIDが新たに登録されたら後の処理で使うかもしれないので
			// ログのエリアパスを登録しておく
			// ★注意★
			// CREATE INDEX -> ALTER INDEX したものをUNDOするときにCREATEのUNDOで
			// INDEXファイルのありかを知るにはALTER INDEX時点のエリアのパスが得られる必要があるため
			ModSize n = vecPostAreaID.getSize();
			for (ModSize i = 0; i < n; ++i) {
				if (vecPostAreaID[i] != Object::ID::Invalid) {
					ModVector<ModUnicodeString> vecPostAreaPath;
					// エリアに対応するパスが登録されていなければログのパスを登録する
					if (!Path::getUndoAreaPath(cDatabaseName_, vecPostAreaID[i], vecPostAreaPath)) {
						// 登録されていない場合はログから取得して登録する
						// PostAreaPathsに記録されているのは表のエリアID割り当ても加味した割り当てに対応するものだが
						// vecPostAreaID[i]がInvalidでないということはこれが索引に割り当てられたエリアのパスである
						Index::getAreaPath(cLogData_, Index::Log::Alter::PostAreaPath, i, vecPostAreaPath);
						Path::setUndoAreaPath(cDatabaseName_, vecPostAreaID[i], vecPostAreaPath);
					}
				}
			}
		}

	} else {

		// REDO されないので論理ファイルのメソッドを介さずに移動する

		////////////////////////////////////////
		// 最終的なデータベースのパス指定を得る
		////////////////////////////////////////
		ModVector<ModUnicodeString> vecDatabasePath;
		Path::getEffectiveDatabasePath(cTrans_, cLogData_, Index::Log::DatabaseID, Index::Log::DatabasePaths,
									   cDatabaseName_, vecDatabasePath);

		///////////////////////////
		// 実質的なエリア指定を得る
		///////////////////////////
		// 常にログに記録されている実質的なエリアIDを得る
		// REDOされない場合、これ以前のUNDO処理もすべてREDOされないものとして行われているはずなので
		// 最終的なID割り当てが登録されている可能性を調べる必要はない
		// -> alter indexが何らかの原因で失敗した後に
		//                        別のalter indexが成功したらREDOされる場合があるので
		//                        最終的なID割り当てを調べる必要はある
		// またファイルを移動するだけなので索引に対するエリア割り当てではなく
		// 表の指定も加味した実質的なエリア指定を使う

		Object::ID::Value tableID = Index::getTableID(cLogData_);

		// 変更前のID指定は最終的なものを使用する
		ModVector<Object::ID::Value> vecTableAreaID;
		ID::getUndoAreaID(cDatabaseName_, tableID, vecTableAreaID);

		ModVector<Object::ID::Value> vecTmpPrevAreaID;
		ModVector<Object::ID::Value> vecPrevAreaID;
		ModVector<Object::ID::Value> vecPostAreaID;
		Index::getAreaID(cLogData_, Index::Log::Alter::PrevEffectiveAreaID, vecTmpPrevAreaID);
		Index::getAreaID(cLogData_, Index::Log::Alter::PostEffectiveAreaID, vecPostAreaID);
		if (!vecTableAreaID.isEmpty())
			Index::getEffectiveAreaID(vecTmpPrevAreaID, vecTableAreaID, Index::getAreaCategory(cLogData_), vecPrevAreaID);
		else
			vecPrevAreaID = vecTmpPrevAreaID;

		// 違いのあるエリアIDについて移動したファイルを元に戻す
		ModSize n = vecPrevAreaID.getSize();
		; _SYDNEY_ASSERT(vecPostAreaID.getSize() == n);

		// 表名とファイル名を得る
		// 最終的な名称が登録されていたらそれを使う
		// ファイル名を得るときは索引Idを使う
		Object::Name tableName;
		Name::getEffectiveName(cLogData_, Index::Log::TableName, tableID, cDatabaseName_, tableName);
		Object::Name fileName;
		Name::getEffectiveFileName(cLogData_, Index::Log::FileName, indexID, cDatabaseName_, fileName);

		for (ModSize i = 0; i < n; ++i) {
			// 実質的な指定に違いのあるカテゴリーについて調べればよい
			if (vecPrevAreaID[i] != vecPostAreaID[i]) {
				// これ以前のUNDOでREDOされるという指定で行われたものはないはずなので
				// UNDO情報を調べる必要はない
				// ログから実質的な移動前後のパス指定を得る
				ModVector<ModUnicodeString> vecPrevAreaPath;
				ModVector<ModUnicodeString> vecPostAreaPath;
				Index::getAreaPath(cLogData_, Index::Log::Alter::PrevAreaPath, i, vecPrevAreaPath);
				Index::getAreaPath(cLogData_, Index::Log::Alter::PostAreaPath, i, vecPostAreaPath);

				// 移動する
#ifdef DEBUG
				SydSchemaDebugMessage << "UNDO AlterIndex: move file \""
									  << Index::getPath(vecDatabasePath, vecPostAreaPath, cDatabaseName_, tableName, fileName)
									  << "\" -> \""
									  << Index::getPath(vecDatabasePath, vecPrevAreaPath, cDatabaseName_, tableName, fileName)
									  << "\"" << ModEndl;
#endif
				Utility::File::move(cTrans_,
									Index::getPath(vecDatabasePath, vecPostAreaPath, cDatabaseName_, tableName, fileName),
									Index::getPath(vecDatabasePath, vecPrevAreaPath, cDatabaseName_, tableName, fileName),
									true /* force */, true /* recovery */);
				// 移動前(ログ上は移動後)の親ディレクトリーが空になったら削除する
				Utility::File::rmEmpty(Index::getPath(vecDatabasePath, vecPostAreaPath, cDatabaseName_, tableName, Object::Name()));
			}
		}
	}
}

// MountのUNDOでも実行する部分を分離する
bool
Manager::SystemTable::UndoUtility::
undoAlterIndex(Trans::Transaction& cTrans_,
			   const ObjectID::Value indexID_,
			   const ModVector<ObjectID::Value>& vecPostArea_,
			   const Object::Name& cDatabaseName_,
			   bool redone_, bool bMount_ /* = false */)
{
	using namespace RecoveryUtility;

	bool bResult = false;

	// ログに記録されていたIDは無条件に使用済みとして登録する
	ID::setUsedID(cDatabaseName_, indexID_);

	if (redone_) {

		// MOUNTからも呼ばれる場合はここでチェックする
		if (bMount_ && Undo::isEntered(cDatabaseName_, indexID_, Undo::Type::DropIndex))
			// indexのDROPがUNDOされていると記録されている場合は
			// どうせ消されるので何もしない
			return bResult;

		// REDO されるので記録する
		if (!bMount_) {
			// MOUNT中でない場合、すでにMOUNT中に登録されているエリアがあったらそれをまず消去する
			if (Undo::isEntered(cDatabaseName_, indexID_, Undo::Type::AlterIndexInMount)) {
				Undo::remove(cDatabaseName_, indexID_, Undo::Type::AlterIndexInMount);
				// 現在登録されているエリアをUnremovableの登録から除くために取得する
				ModVector<ObjectID::Value> vecArea;
				if (ID::getUndoAreaID(cDatabaseName_, indexID_, vecArea)) {
					ID::eraseUnremovableAreaID(cDatabaseName_, vecArea);
					// 登録されているエリアを消去する
					ID::eraseUndoAreaID(cDatabaseName_, indexID_);
				}
			}
			Undo::enter(cDatabaseName_, indexID_, Undo::Type::AlterIndex);
		} else {
			Undo::enter(cDatabaseName_, indexID_, Undo::Type::AlterIndexInMount);
		}

		// 処理対象エリアを記録する
		if (ID::setUndoAreaID(cDatabaseName_, indexID_, vecPostArea_)) {
			// 新規にエリアが登録されたらそのエリアIDを破棄対象から除くために登録する
			ID::setUnremovableAreaID(cDatabaseName_, vecPostArea_);
			bResult = true;
		}
	}
	return bResult;
}

//	FUNCTION
//	Schema::Manager::SystemTable::UndoUtility::Index
//		データベースの「インデックス」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		const Schema::LogData& cLogData_
//			ログデータ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		bool				redone_
//			true	後で REDO される
//			false	後で REDO されない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::UndoUtility::
undoRenameIndex(Trans::Transaction& cTrans_, const LogData& cLogData_,
				const Object::Name& cDatabaseName_, bool redone_,
				bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropIndexはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// Schema IDを取得
	const ObjectID::Value indexID = Index::getObjectID(cLogData_);

	// Renameの場合はindexのDROPがUNDOされていると記録されていても、
	// 以降の処理をやる必要がある

	// ログに記録されていたIDを使用済みとして登録する
	ID::setUsedID(cDatabaseName_, indexID);

	// ログに記録されていたファイルのスキーマオブジェクトIDを得て使用済みに登録する
	ObjectID::Value fileID = cLogData_.getID(Index::Log::Rename::FileID);
	ID::setUsedID(cDatabaseName_, fileID);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	////////////////////////////////////////
	// 最終的なデータベースのパス指定を得る
	////////////////////////////////////////
	ModVector<ModUnicodeString> vecDatabasePath;
	Path::getEffectiveDatabasePath(cTrans_, cLogData_, Index::Log::DatabaseID, Index::Log::DatabasePaths,
								   cDatabaseName_, vecDatabasePath);

	////////////////////////
	// 最終的な表名を得る
	////////////////////////

	// 表のスキーマIDを得る
	Object::ID::Value tableID = Index::getTableID(cLogData_);

	Object::Name tableName;
	Name::getEffectiveName(cLogData_, Index::Log::TableName, tableID, cDatabaseName_, tableName);

	////////////////////////////
	// 変更後のファイル名を得る
	////////////////////////////
	Object::Name cPostFileName(cLogData_.getString(Index::Log::Rename::PostFileName));

	///////////////////////////
	// 実質的なエリア指定を得る
	///////////////////////////
	// 常にログに記録されている実質的なエリアIDを得る
	// またファイルの移動や削除禁止の登録をするだけなので
	// 索引に対するエリア割り当てではなく
	// 表の指定も加味した実質的なエリア指定を使う

	// 表に対する最終的なエリアID指定を得る
	ModVector<Object::ID::Value> vecTableAreaID;
	ID::getUndoAreaID(cDatabaseName_, tableID, vecTableAreaID);

	ModVector<Object::ID::Value> vecTmpAreaID;
	ModVector<Object::ID::Value> vecAreaID;
	Index::getAreaID(cLogData_, Index::Log::Rename::EffectiveAreaID, vecTmpAreaID);
	if (!vecTableAreaID.isEmpty())
		// 表に対する最終的なエリア指定があればその指定で上書きする
		Index::getEffectiveAreaID(vecTmpAreaID, vecTableAreaID, Index::getAreaCategory(cLogData_), vecAreaID);
	else
		// ログの内容をそのまま使う
		vecAreaID = vecTmpAreaID;

	if (redone_) {

		// 変更後の名前をログデータから取得する
		Object::Name cPostName(cLogData_.getString(Index::Log::Rename::PostName));

		// REDO されるので記録する
		Undo::enter(cDatabaseName_, indexID, Undo::Type::RenameIndex);

		// 処理対象の名前を記録する
		if (Name::setUndoName(cDatabaseName_, indexID, cPostName)) {
			// 新規に登録された場合は最終的なパスを用いて削除禁止の登録をする

			// すべてのエリアIDについてパスを削除禁止にする
			ModSize n = vecAreaID.getSize();

			for (ModSize i = 0; i < n; ++i) {
				ModVector<ModUnicodeString> vecAreaPath;
				if (!Path::getUndoAreaPath(cDatabaseName_, vecAreaID[i], vecAreaPath)) {
					// エリアの最終的なパスが登録されていなければログのパスを使用する
					Index::getAreaPath(cLogData_, Index::Log::Rename::AreaPaths, i, vecAreaPath);
				}
				if (!vecAreaPath.isEmpty()) {
					Os::Path cPath = Index::getPath(vecDatabasePath, vecAreaPath,
													cDatabaseName_, tableName, cPostFileName);
					Path::addUnremovablePath(cPath);
					Path::addUsedPath(cPath);
				}
			}
			// エリア指定がない場合のパスも対象とする
			Os::Path cPath = Index::getPath(vecDatabasePath, ModVector<ModUnicodeString>(),
											cDatabaseName_, tableName, cPostFileName);
			Path::addUnremovablePath(cPath);
			Path::addUsedPath(cPath);
		}

		Name::setUndoFileName(cDatabaseName_, indexID, cPostFileName);

	} else {

		// REDO されないので論理ファイルのメソッドを介さずに移動する

		// 変更前のファイル名を得る
		// 名前の変更が繰り返された場合、最終的な名前を用いる
		Object::Name cPrevFileName;
		Name::getEffectiveFileName(cLogData_, Index::Log::Rename::PrevFileName, indexID, cDatabaseName_, cPrevFileName);

		// 名前に変更のある場合のみ実施する
		if (cPrevFileName != cPostFileName) {

			// すべてのエリアIDについて移動したファイルを元に戻す
			ModSize n = vecAreaID.getSize();

			for (ModSize i = 0; i < n; ++i) {
				ModVector<ModUnicodeString> vecAreaPath;
				Index::getAreaPath(cLogData_, Index::Log::Rename::AreaPaths, i, vecAreaPath);

				// 移動する
#ifdef DEBUG
				SydSchemaDebugMessage << "UNDO RenameIndex: move file \""
									  << Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, cPostFileName)
									  << "\" -> \""
									  << Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, cPrevFileName)
									  << "\"" << ModEndl;
#endif
				Utility::File::move(cTrans_,
									Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, cPostFileName),
									Index::getPath(vecDatabasePath, vecAreaPath, cDatabaseName_, tableName, cPrevFileName),
									true /* force */, true /* recovery */);
			}
		}
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoCreatePrivilege -- undo create privilege
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoCreatePrivilege(Trans::Transaction& cTrans_,
					const Schema::LogData& cLogData_,
					const Schema::Object::Name& cDatabaseName_,
					bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// get schema object id from log data
	ObjectID::Value id = Privilege::getID(cLogData_);

	// register the id as used
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	// Check whether the privilege will be dropped by following log data
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPrivilege)) {

		if (redone_) {
			// If this log data will be redone, record it
			Undo::enter(cDatabaseName_, id, Undo::Type::CreatePrivilege);
		}

	} else {
		ModVector<Common::Privilege::Value> vecValue;

		// If grant/revoke is called by following log data, use final value
		// otherwise, use the value gotten from the log data
		if (!PrivilegeValue::getUndoValue(cDatabaseName_, id, vecValue)) {
			// nothing has been registered
			vecValue = Privilege::getValue(cLogData_);

			if (redone_) {
				// If this logdata is redone, register this value
				PrivilegeValue::setUndoValue(cDatabaseName_, id, vecValue);
			}
		}
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoDropPrivilege -- undo drop privilege
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoDropPrivilege(Trans::Transaction& cTrans_,
				  const Schema::LogData& cLogData_,
				  const Schema::Object::Name& cDatabaseName_,
				  bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// get schema object id from log data
	ObjectID::Value id = Privilege::getID(cLogData_);

	// register used id
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	// get privilege value from log dataa
	ModVector<Common::Privilege::Value>	vecValue = Privilege::getValue(cLogData_);

	if (redone_) {
		// register undo information
		Undo::enter(cDatabaseName_, id, Undo::Type::DropPrivilege);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoAlterPrivilege -- undo alter privilege
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoAlterPrivilege(Trans::Transaction& cTrans_,
				   const Schema::LogData& cLogData_,
				   const Schema::Object::Name& cDatabaseName_,
				   bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterPrivilege should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Privilege::getID(cLogData_);
	// register used id
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPrivilege)) {
		// The privilege will be dropped, so do nothing
		return;
	}

	if (redone_) {
		// get privilege value after altering
		ModVector<Common::Privilege::Value> vecPostValue
			= Privilege::getValue(cLogData_, Privilege::Log::Alter::PostPrivilege);
		// register final privilege value
		(void)PrivilegeValue::setUndoValue(cDatabaseName_, id, vecPostValue);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoCreateCascade -- undo create cascade
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoCreateCascade(Trans::Transaction& cTrans_,
					const Schema::LogData& cLogData_,
					const Schema::Object::Name& cDatabaseName_,
					bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// get schema object id from log data
	ObjectID::Value id = Cascade::getObjectID(cLogData_);

	// register the id as used
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	// Check whether the cascade will be dropped by following log data
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropCascade)) {

		if (redone_) {
			// If this log data will be redone, record it
			Undo::enter(cDatabaseName_, id, Undo::Type::CreateCascade);
		}

	} else {
		// YET
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoDropCascade -- undo drop cascade
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoDropCascade(Trans::Transaction& cTrans_,
				  const Schema::LogData& cLogData_,
				  const Schema::Object::Name& cDatabaseName_,
				  bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// get schema object id from log data
	ObjectID::Value id = Cascade::getObjectID(cLogData_);

	// register used id
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (redone_) {
		// register undo information
		Undo::enter(cDatabaseName_, id, Undo::Type::DropCascade);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoAlterCascade -- undo alter cascade
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoAlterCascade(Trans::Transaction& cTrans_,
				   const Schema::LogData& cLogData_,
				   const Schema::Object::Name& cDatabaseName_,
				   bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterCascade should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Cascade::getObjectID(cLogData_);
	// register used id
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropCascade)) {
		// The cascade will be dropped, so do nothing
		return;
	}

	if (redone_) {
		// YET
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoCreatePartition -- undo create partition
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoCreatePartition(Trans::Transaction& cTrans_,
					const Schema::LogData& cLogData_,
					const Schema::Object::Name& cDatabaseName_,
					bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// get schema object id from log data
	ObjectID::Value id = Partition::getObjectID(cLogData_);

	// register the id as used
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	// Check whether the partition will be dropped by following log data
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPartition)) {

		if (redone_) {
			// If this log data will be redone, record it
			Undo::enter(cDatabaseName_, id, Undo::Type::CreatePartition);
		}

	} else {
		// do nothing
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoDropPartition -- undo drop partition
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoDropPartition(Trans::Transaction& cTrans_,
				   const Schema::LogData& cLogData_,
				   const Schema::Object::Name& cDatabaseName_,
				   bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropPartition should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Partition::getObjectID(cLogData_);
	// register used id
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (redone_) {
		// record this partition to be dropped
		Undo::enter(cDatabaseName_, id, Undo::Type::DropPartition);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoAlterPartition -- undo alter partition
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoAlterPartition(Trans::Transaction& cTrans_,
				   const Schema::LogData& cLogData_,
				   const Schema::Object::Name& cDatabaseName_,
				   bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterPartition should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Partition::getObjectID(cLogData_);
	// register used id
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPartition)) {
		// The partition will be dropped, so do nothing
		return;
	}

	if (redone_) {
		// do nothing
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoCreateFunction -- undo create function
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoCreateFunction(Trans::Transaction& cTrans_,
					const Schema::LogData& cLogData_,
					const Schema::Object::Name& cDatabaseName_,
					bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// get schema object id from log data
	ObjectID::Value id = Function::getObjectID(cLogData_);

	// register the id as used
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	// Check whether the function will be dropped by following log data
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropFunction)) {

		if (redone_) {
			// If this log data will be redone, record it
			Undo::enter(cDatabaseName_, id, Undo::Type::CreateFunction);
		}

	} else {
		// YET
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoDropFunction -- undo drop function
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::Object::Name& cDatabaseName_
//	bool redone_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoDropFunction(Trans::Transaction& cTrans_,
				  const Schema::LogData& cLogData_,
				  const Schema::Object::Name& cDatabaseName_,
				  bool redone_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// get schema object id from log data
	ObjectID::Value id = Function::getObjectID(cLogData_);

	// register used id
	ID::setUsedID(cDatabaseName_, id);

	// ロールフォワードリカバリなら、これ以上何もしない
	if (bRollforward_) return;

	if (redone_) {
		// register undo information
		Undo::enter(cDatabaseName_, id, Undo::Type::DropFunction);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::UndoUtility::undoCreatedFiles -- ファイルの作成をUNDOする(addColumn/addConstraintの下請け)
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const LogData& cLogData_
//	const Object::Name& cDatabaseName_
//	const ObjectID::Value& iTableID_
//	const Object::Name& cTableName_
//	const ModVector<ModUnicodeString>& vecDatabasePath_
//	const ModVector<Object::ID::Value>& vecAreaID_
//	int iLogIndexCreatedFiles_
//	int iLogIndexAreaPaths_
//	int iLogIndexTempSuffix_ /* = -1 */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::UndoUtility::
undoCreatedFiles(Trans::Transaction& cTrans_, const LogData& cLogData_,
				 const Object::Name& cDatabaseName_, const ObjectID::Value& iTableID_,
				 const Object::Name& cTableName_,
				 const ModVector<ModUnicodeString>& vecDatabasePath_,
				 const ModVector<Object::ID::Value>& vecAreaID_,
				 int iLogIndexCreatedFiles_,
				 int iLogIndexAreaPaths_,
				 int iLogIndexTempSuffix_ /* = -1 */)
{
	using namespace RecoveryUtility;

	// 作成したファイルを削除する
	const Common::DataArrayData& cPostFiles = cLogData_.getDataArrayData(iLogIndexCreatedFiles_);
	int n = cPostFiles.getCount();

	if (n) {
		int i = 0;
		do {
			// area category
			int iAreaCategory = LogData::getInteger(cPostFiles.getElement(i++));
			if (i < n) {
				// file id
				Object::ID iFileID = LogData::getID(cPostFiles.getElement(i++));
				if (i < n) {
					// file name
					Object::Name cFileName = LogData::getString(cPostFiles.getElement(i++));

					if (iAreaCategory >= 0 && static_cast<ModSize>(iAreaCategory) < vecAreaID_.getSize()) {
						// area id
						Object::ID iAreaID = vecAreaID_[iAreaCategory];
						// replace file name by last name
						(void)Name::getUndoFileName(cDatabaseName_, iFileID, cFileName);
						// get area path
						ModVector<ModUnicodeString> vecAreaPath;
						if (!Path::getUndoAreaPath(cDatabaseName_, iAreaID, vecAreaPath)) {
							// if not registered in the undo process, use log data
							Table::getEffectiveAreaPath(cLogData_, iLogIndexAreaPaths_,
														iAreaCategory, vecAreaID_, vecAreaPath);
						}
						if (iLogIndexTempSuffix_ < 0)
							// delete the file
							Utility::File::rmAll(Table::getPath(vecDatabasePath_, vecAreaPath, cDatabaseName_, cTableName_)
												 .addPart(cFileName));
						else {
							// return saved original files if exists
							ModUnicodeString cSuffix = cLogData_.getString(iLogIndexTempSuffix_);
							Object::Name cTempName(cFileName);
							cTempName.append(cSuffix);
							Os::Path cBasePath = Table::getPath(vecDatabasePath_, vecAreaPath, cDatabaseName_, cTableName_);
							if (Utility::File::isFound(Os::Path(cBasePath).addPart(cTempName))) {
								Object::Name cDeleteName(cTempName);
								cDeleteName.append("RECOVER");
								Utility::File::move(cTrans_,
													Os::Path(cBasePath).addPart(cFileName),
													Os::Path(cBasePath).addPart(cDeleteName),
													true /* force */, true /* recovery */);
								Utility::File::move(cTrans_,
													Os::Path(cBasePath).addPart(cTempName),
													Os::Path(cBasePath).addPart(cFileName),
													true /* force */, true /* recovery */);
								Utility::File::rmAll(Os::Path(cBasePath).addPart(cDeleteName));
							}
						}
						continue;
					}
				}
			}
			// error
			_SYDNEY_THROW0(Exception::LogItemCorrupted);
		} while (i < n);
	}
}

//
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
