// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Redo.cpp -- Redo関連の関数定義(Manager::SystemTable)
// 
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Schema/Area.h"
#include "Schema/AreaMap.h"
#include "Schema/Cascade.h"
#include "Schema/Database.h"
#include "Schema/DatabaseMap.h"
#include "Schema/File.h"
#include "Schema/Function.h"
#include "Schema/Hold.h"
#include "Schema/Index.h"
#include "Schema/IndexMap.h"
#include "Schema/LogData.h"
#include "Schema/Message.h"
#include "Schema/NameParts.h"
#include "Schema/ObjectID.h"
#include "Schema/ObjectSnapshot.h"
#include "Schema/Partition.h"
#include "Schema/Recovery.h"
#include "Schema/Redo.h"
#include "Schema/ReorganizeArea.h"
#include "Schema/ReorganizeCascade.h"
#include "Schema/ReorganizeColumn.h"
#include "Schema/ReorganizeDatabase.h"
#include "Schema/ReorganizeFunction.h"
#include "Schema/ReorganizeIndex.h"
#include "Schema/ReorganizePartition.h"
#include "Schema/ReorganizePrivilege.h"
#include "Schema/ReorganizeTable.h"
#include "Schema/SystemTable_Database.h"
#include "Schema/SystemTable_Table.h"
#include "Schema/SystemTable_Area.h"
#include "Schema/SystemTable_AreaContent.h"
#include "Schema/SystemTable_Cascade.h"
#include "Schema/SystemTable_Column.h"
#include "Schema/SystemTable_Constraint.h"
#include "Schema/SystemTable_Partition.h"
#include "Schema/SystemTable_Function.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/Table.h"
#include "Schema/TableMap.h"

#include "Admin/Mount.h"
#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/ReadOnlyTransaction.h"
#include "Lock/Name.h"
#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

	namespace _Utility {

		//
		//	FUNCTION
		//		_Utility::convertPointer
		//			-- ModVector<ModUnicodeString> より ModVector<ModUnicodeString*> の配列を作成する
		//	NOTES
		
		void
		convertPointer(ModVector<ModUnicodeString>& src_, ModVector<ModUnicodeString*>dst_)
		{
			dst_.clear();
			dst_.reserve(src_.getSize());

			ModVector<ModUnicodeString>::Iterator it = src_.begin();
			const ModVector<ModUnicodeString>::Iterator& fin = src_.end();
			for ( ; it != fin; ++it ) dst_.pushBack(&*it);

		}

	} // namespace _Utility

} // namespace $$

//////////////////////////////////////
//	 Schema::Manager::SystemTable	//
//////////////////////////////////////

//	FUNCTION
//	Schema::Manager::SystemTable::redo --
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
//		bool bRollforward_
//			ロールフォワードのためのREDOか
//
//	RETURN
//		0 以外の値
//			回復が新たに必要になったデータベースを表す
//			クラスを格納する領域の先頭アドレス
//		0
//			回復が新たに必要になったデータベースはない
//
//	EXCEPTIONS

Admin::Recovery::Database*
Manager::SystemTable::redo(
	Trans::Transaction& cTrans_, const LogData& cLogData_,
	const Object::Name& cDatabaseName_, bool bRollforward_)
{
	if (cTrans_.getCategory() == Trans::Transaction::Category::ReadOnly)

		// 読取専用トランザクションでは、
		// スキーマ操作を UNDO できない

		_SYDNEY_THROW0(Exception::ReadOnlyTransaction);

	// 念のために初期化しておく
	initialize();

#ifdef DEBUG
#ifndef SYD_COVERAGE
	SydSchemaDebugMessage << "REDO " << cLogData_.toString() << ModEndl;
#endif
#endif

	// ログデータの種類に応じて再構成処理を行う
	switch ( cLogData_.getSubCategory() )
	{

	//======================================================================
	//	Admin の操作
	//======================================================================
	case LogData::Category::Mount:
		return RedoUtility::mount(cTrans_, cLogData_, cDatabaseName_,
								  bRollforward_);

	case LogData::Category::Unmount:
	{
		RedoUtility::unmount(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	case LogData::Category::StartBackup:
	{
		RedoUtility::startBackup(cTrans_, cLogData_, cDatabaseName_,
								 bRollforward_);
		break;
	}
	case LogData::Category::EndBackup:
	{
		RedoUtility::endBackup(cTrans_, cLogData_, cDatabaseName_,
							   bRollforward_);
		break;
	}
	//=======================================================================
	//	Database の操作
	//=======================================================================

	case LogData::Category::CreateDatabase:
		return RedoUtility::createDatabase(cTrans_, cLogData_, cDatabaseName_,
										   bRollforward_);

	case LogData::Category::DropDatabase:
	{
		RedoUtility::dropDatabase(cTrans_, cLogData_, cDatabaseName_,
								  bRollforward_);
		break;
	}
	case LogData::Category::MoveDatabase:
	{
		RedoUtility::moveDatabase(cTrans_, cLogData_, cDatabaseName_,
								  bRollforward_);
		break;
	}
	case LogData::Category::AlterDatabase:
	case LogData::Category::AlterDatabase_ReadOnly:
	case LogData::Category::AlterDatabase_SetToMaster:
	{
		RedoUtility::alterDatabase(cTrans_, cLogData_, cDatabaseName_,
								   bRollforward_);
		break;
	}

	//=======================================================================
	//	Database 以下の操作
	//=======================================================================

	case LogData::Category::CreateArea:
	{
		RedoUtility::createArea(cTrans_, cLogData_, cDatabaseName_,
								bRollforward_);
		break;
	}
	case LogData::Category::DropArea:
	{
		RedoUtility::dropArea(cTrans_, cLogData_, cDatabaseName_,
							  bRollforward_);
		break;
	}
	case LogData::Category::AlterArea:
	{
		RedoUtility::alterArea(cTrans_, cLogData_, cDatabaseName_,
							   bRollforward_);
		break;
	}
	case LogData::Category::CreateTable:
	{
		RedoUtility::createTable(cTrans_, cLogData_, cDatabaseName_,
								 bRollforward_);
		break;
	}
	case LogData::Category::DropTable:
	{
		RedoUtility::dropTable(cTrans_, cLogData_, cDatabaseName_,
							   bRollforward_);
		break;
	}
	case LogData::Category::AlterTable:
	case LogData::Category::RenameTable:
	case LogData::Category::AddColumn:
	case LogData::Category::AlterColumn:
	case LogData::Category::AddConstraint:
	case LogData::Category::DropConstraint:
	{
		RedoUtility::alterTable(cTrans_, cLogData_,	cDatabaseName_,
								bRollforward_);
		break;
	}
	case LogData::Category::CreateIndex:
	{
		RedoUtility::createIndex(cTrans_, cLogData_, cDatabaseName_,
								 bRollforward_);
		break;
	}
	case LogData::Category::DropIndex:
	{
		RedoUtility::dropIndex(cTrans_, cLogData_, cDatabaseName_,
							   bRollforward_);
		break;
	}
	case LogData::Category::AlterIndex:
	{
		RedoUtility::alterIndex(cTrans_, cLogData_, cDatabaseName_,
								bRollforward_);
		break;
	}
	case LogData::Category::RenameIndex:
	{
		RedoUtility::renameIndex(cTrans_, cLogData_, cDatabaseName_,
								 bRollforward_);
		break;
	}
	//////////////////
	// Privileges
	//////////////////
	case LogData::Category::CreatePrivilege:
	{
		RedoUtility::createPrivilege(cTrans_, cLogData_, cDatabaseName_,
									 bRollforward_);
		break;
	}
	case LogData::Category::DropPrivilege:
	{
		RedoUtility::dropPrivilege(cTrans_, cLogData_, cDatabaseName_,
								   bRollforward_);
		break;
	}
	case LogData::Category::AlterPrivilege:
	{
		RedoUtility::alterPrivilege(cTrans_, cLogData_, cDatabaseName_,
									bRollforward_);
		break;
	}
	///////////////////
	// Cascades
	///////////////////
	case LogData::Category::CreateCascade:
	{
		RedoUtility::createCascade(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	case LogData::Category::DropCascade:
	{
		RedoUtility::dropCascade(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	case LogData::Category::AlterCascade:
	{
		RedoUtility::alterCascade(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	///////////////////
	// Partitions
	///////////////////
	case LogData::Category::CreatePartition:
	{
		RedoUtility::createPartition(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	case LogData::Category::DropPartition:
	{
		RedoUtility::dropPartition(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	case LogData::Category::AlterPartition:
	{
		RedoUtility::alterPartition(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	/////////////////
	// Functions
	/////////////////
	case LogData::Category::CreateFunction:
	{
		RedoUtility::createFunction(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	case LogData::Category::DropFunction:
	{
		RedoUtility::dropFunction(cTrans_, cLogData_, cDatabaseName_, bRollforward_);
		break;
	}
	default:
		// 知らないログデータがきた
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
		break;
	}

	return 0;
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::mount --
//		データベースのマウントの REDO を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			回復処理を行うトランザクションのトランザクション記述子
//		Schema::LogData&	logData
//			REDO するデータベースのマウントを表す論理ログデータ
//		Schema::Object::Name&	dbName
//			メタデータベース名
//
//	RETURN
//		0 以外の値
//			回復処理の結果、新たにマウントされたデータベースの
//			回復処理を行うためのクラスを格納する領域の先頭アドレス
//		0
//			データベースの生成は REDO されなかった
//
//	EXCEPTIONS

Admin::Recovery::Database*
Manager::SystemTable::RedoUtility::mount(
	Trans::Transaction& trans,
	const LogData& logData, const Object::Name& dbName,
	bool bRollforward_)
{
	//【注意】	MOUNT 文の論理ログは
	//			システム専用の論理ログファイルと
	//			マウントするデータベースの論理ログファイルの両方に記録される

	// マウントしようとしているデータベースが
	// 後に DROP または UNMOUNT されているか調べる
	// また、ロールフォワード中なら REDO しない

	const ObjectID::Value dbID = Database::getObjectID(logData);
	; _SYDNEY_ASSERT(dbID != ObjectID::Invalid);

	if (RecoveryUtility::Undo::isEntered(
			dbID, RecoveryUtility::Undo::Type::DropDatabase) ||
		RecoveryUtility::Undo::isEntered(
			dbID, RecoveryUtility::Undo::Type::Unmount) || bRollforward_) {

		// データベースのマウントを REDO しない
		// ただし、オブジェクトIDの整合性をとる必要がある

		Object::ID::assign(trans, 0 /* system */, dbID, Object::ID::SystemTable);
		return 0;
	}

	// マウント処理を実際に REDO する
	//
	//【注意】	MOUNT 文では論理ログがシステム用の論理ログファイルと
	//			マウントされたデータベース用の論理ログファイルの
	//			両方に記録される
	//			が、データベース用の論理ログファイルには MOUNT 文
	//			(とそれを実行するトランザクション)の論理ログが
	//
	//			START TRANSACTION
	//			MOUNT
	//			CHECKPOINT(DATABASE)
	//			COMMIT
	//
	//			のように記録されるため、常にデータベースに対する
	//			チェックポイント処理の終了時から回復処理を行う
	//
	//			そのため、マウントするデータベースのシステム表や
	//			構成するファイルに対するマウント処理は永続化されているので、
	//			この関数ではそれらの永続化処理は行わない

	return Admin::Mount(trans, logData).redo(logData);
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::unmount
//		-- unmount の redo 処理
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
Manager::SystemTable::RedoUtility::
unmount(Trans::Transaction& cTrans_, const LogData& cLogData_,
		const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// ロールフォワードでは何もしない

	if (bRollforward_) return ;

	// Unmount のログはシステムおよびデータベースのログに書かれるように
	// 仕様変更されたのでアサートチェックはしない
	// ; _SYDNEY_ASSERT(cDatabaseName_ == NameParts::Database::System);

	bool bNeedToDrop = true;
	ObjectID::Value id = Database::getObjectID(cLogData_);

	// Unmount の Undo 中の記録を解除
	Undo::remove(id, Undo::Type::Unmount);

	// 対象の database が create or mount されているか
	if ( Undo::isEntered(id, Undo::Type::CreateDatabase) ) {
		// 見つかったものを解除する
		Undo::remove(id, Undo::Type::CreateDatabase);
		bNeedToDrop = false;
	}
	if ( Undo::isEntered(id, Undo::Type::Mount) ) {
		// 見つかったものを解除する
		Undo::remove(id, Undo::Type::Mount);
		bNeedToDrop = false;
	}
	// CreateまたはMountがUndoされている場合は
	// システム表に格納されていないはずなので何もしない
	if (!bNeedToDrop) return;

	// CREATEまたはMOUNTがUNDOされていないとき
	// データベース表からエントリーを削除する
	// ★注意★
	// ディレクトリーはUndoのときに削除されているので
	// ここではシステム表からエントリーを削除すればよい

	Database::Pointer pDatabase =
		Manager::ObjectSnapshot::get(cTrans_)->loadDatabase(cTrans_).get(id);
	if (pDatabase.get()) {
		pDatabase->drop(cTrans_, true /* recovery_ */);
		Schema::SystemTable::Database().store(cTrans_, pDatabase);
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::startBackup
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
Manager::SystemTable::RedoUtility::
startBackup(Trans::Transaction& cTrans_, const LogData& cLogData_,
			const Object::Name& cDatabaseName_, bool bRollforward_)
{
	// startbackup は 何もしない
}
		

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::endBackup
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
Manager::SystemTable::RedoUtility::
endBackup(Trans::Transaction& cTrans_, const LogData& cLogData_,
		  const Object::Name& cDatabaseName_, bool bRollforward_)
{
	// endbackup は 何もしない
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::createDatabase --
//		データベースの作成の REDO を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			回復処理を行うトランザクションのトランザクション記述子
//		Schema::LogData&	logData
//			REDO するデータベースの作成を表す論理ログデータ
//		Schema::Object::Name&	dbName
//			メタデータベース名
//
//	RETURN
//		0 以外の値
//			回復処理の結果、新たに生成されたデータベースの
//			回復処理を行うためのクラスを格納する領域の先頭アドレス
//		0
//			データベースの生成は REDO されなかった
//
//	EXCEPTIONS

Admin::Recovery::Database*
Manager::SystemTable::RedoUtility::createDatabase(
	Trans::Transaction& trans,
	const LogData& logData, const Object::Name& dbName,
	bool bRollforward_)
{
	//【注意】	CREATE DATABASE 文の論理ログは
	//			システム専用の論理ログファイルと
	//			生成するデータベースの論理ログファイルの両方に記録される

	// 生成しようとしているデータベースが
	// 後に DROP または UNMOUNT されているか調べる

	const ObjectID::Value dbID = Database::getObjectID(logData);

	if (RecoveryUtility::Undo::isEntered(
			 dbID, RecoveryUtility::Undo::Type::DropDatabase) ||
		RecoveryUtility::Undo::isEntered(
			 dbID, RecoveryUtility::Undo::Type::Unmount)) {

		// データベースの生成を REDO しない
		// ただし、オブジェクトIDの整合性をとる必要がある

		Object::ID::assign(trans, 0 /* system */, dbID, Object::ID::SystemTable);
		return 0;
	}

	// 生成するデータベースのスキーマオブジェクトを表すクラスを生成する

	Database::Pointer database = Database::create(logData, trans);
	; _SYDNEY_ASSERT(database.get());

	// マウント処理を実際に REDO する
	//
	//【注意】	データベース用の論理ログファイルには CREATE DATABASE 文
	//			(とそれを実行するトランザクション)の論理ログが
	//
	//			START TRANSACTION
	//			CREATE DATABASE
	//			CHECKPOINT(DATABASE)
	//			COMMIT
	//
	//			のように記録されるため、常にデータベースに対する
	//			チェックポイント処理の終了時から回復処理を行う
	//
	//			そのため、生成するデータベースのシステム表や
	//			構成するファイルに対する生成処理は永続化されているので、
	//			この関数ではそれらの永続化処理は行わない
	//
	//			つまり、マウントの REDO とほぼ同じになる

	return Admin::Mount(trans, database, logData).redo(logData);
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Database
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
dropDatabase(Trans::Transaction& cTrans_, const LogData& cLogData_,
			 const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;
	
	// DropDatabaseはシステムのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ == Object::Name(NameParts::Database::System));

	// IDの取得
	Object::ID::Value id = Database::getObjectID(cLogData_);

	// 総合テストNo.619の対応ではDROPのUNDOに際して親子関係のあるオブジェクトにも
	// UNDO DROPを記録するようにしていたが、データベースにUNDO DROPが記録される場合は
	// その子オブジェクトに対してリカバリー処理は行われないので記録する意味がない。
	// また、データベース以下のオブジェクトに関する記録のキーとしてデータベース名も
	// 使うためにここで記録すると誤った記録となる可能性がある

	// 記録を削除する
	Undo::remove(id, Undo::Type::DropDatabase);

	if (Undo::isEntered(id, Undo::Type::CreateDatabase)
		|| Undo::isEntered(id, Undo::Type::Mount)) {

		// create database または mount されたなら記録を削除する

		Undo::remove(id, Undo::Type::CreateDatabase);
		Undo::remove(id, Undo::Type::Mount);

		return;
	}

	// CREATEまたはMOUNTがUNDOされていないとき
	// データベース表からエントリーを削除する
	// ★注意★
	// DROPがREDOされている状況でCREATEがREDOされていることはないので
	// ログに書かれているIDのデータベースが削除対象のデータベースである

	Database::Pointer pDatabase =
		Manager::ObjectSnapshot::get(cTrans_)->loadDatabase(cTrans_).get(id);
	if (pDatabase.get()) {
		pDatabase->drop(cTrans_, true /* recovery */);
		Schema::SystemTable::Database().store(cTrans_, pDatabase);
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Database
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
moveDatabase(Trans::Transaction& cTrans_, const LogData& cLogData_,
			 const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// ★注意★
	// ALTER DATABASE SET/DROP PATH文の論理ログは
	// システム専用の論理ログファイルと
	// データベースの論理ログファイルの両方に記録される
	// システム用の論理ログファイルについてREDOされるときのみREDOを実行すればよい

	if (cDatabaseName_ == Object::Name(NameParts::Database::System)) {

		// ログに記載されているID の取得
		ObjectID::Value id = Database::getObjectID(cLogData_);

		// database のDROPまたはUNMOUNTがUNDOされているか調べる
		if (Undo::isEntered(id, Undo::Type::DropDatabase) ||
			Undo::isEntered(id, Undo::Type::Unmount)) {
			// どうせ消されるので何もしない
			return;
		}

		// ALTERがUNDOされたことを忘れる
		Undo::remove(id, Undo::Type::MoveDatabase);

		// ログのIDからデータベースのスキーマオブジェクトを取得
		// CREATEがREDOされているかもしれないがIDは変化しない

		Database::Pointer pDatabase =
			Manager::ObjectSnapshot::get(cTrans_)->loadDatabase(cTrans_).get(id);

		if (!pDatabase.get())
			_SYDNEY_THROW1(Exception::DatabaseNotFound, Database::getName(cLogData_));

		// オブジェクトのパス指定を最新のものにする
		// UNDO処理で最新のパス指定に対応する場所にファイルがすでにあるので
		// ファイルの移動を行う必要はない
		ModVector<ModUnicodeString> vecPath;
		if (Path::getUndoDatabasePath(id, vecPath)) {
			// 必ず登録されているはずである

			// パス指定を書き換えて変更があったことを指示する
			pDatabase->setPath(vecPath);
			pDatabase->touch();
			// 変更を永続化する
			Schema::SystemTable::Database().store(cTrans_, pDatabase);
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Database
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
alterDatabase(Trans::Transaction& cTrans_, const LogData& cLogData_,
			  const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	if (bRollforward_)
		// ロールフォワードリカバリのときには、
		// データベースに関するログは Redo する必要はない
		return;

	// ★注意★
	// ALTER DATABASE文の論理ログは
	// READ WRITEへの変更のときとONLINEへの変更のときに
	// にシステム専用の論理ログファイルと
	// データベースの論理ログファイルの両方に記録される
	// システムの論理ログファイルについてREDOされるときはデータベース表のみ触る

	// ID の取得
	ObjectID::Value id = Database::getObjectID(cLogData_);

	// drop または unmount されているかチェック
	if ( Undo::isEntered(id, Undo::Type::DropDatabase) ||
		 Undo::isEntered(id, Undo::Type::Unmount) )
	{
		// どうせ消されるので何もしない
		return;
	}

	// IDからデータベースのスキーマオブジェクトを取得
	// ログに記録された属性変更を適用する
	Database::Pointer pDatabase =
		Manager::ObjectSnapshot::get(cTrans_)->loadDatabase(cTrans_).get(id);

	if (!pDatabase.get())
		_SYDNEY_THROW1(Exception::DatabaseNotFound, Database::getName(cLogData_));

	// 下位のオブジェクトも使用するのでキャッシュが破棄されないようにopenする
	pDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Database, bool> autoCloser(pDatabase.get(), &Database::close, true /* volatile */);

	// ログデータから変更前後の属性を得る
	Database::Attribute cPrevAttribute;
	Database::Attribute cPostAttribute;
	Database::alter(cLogData_, cPrevAttribute, cPostAttribute, cTrans_);

	if (cDatabaseName_ == Object::Name(NameParts::Database::System)) {
		// システムのログについてはデータベース表の更新のみを行う

		// 属性を設定する
		pDatabase->setAttribute(cPostAttribute);
		pDatabase->touch();

		// j.データベース表を更新
		Schema::SystemTable::Database().store(cTrans_, pDatabase);

	} else {
		// データベースのログについてはファイルに変更を伝播させる
		// ただしREAD ONLY属性が変化したときのみでよい
		if (cPrevAttribute.m_bReadOnly != cPostAttribute.m_bReadOnly) {
			pDatabase->propagateAttribute(cTrans_, cPostAttribute);
		}
	}
}


//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Area
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
createArea(Trans::Transaction& cTrans_, const LogData& cLogData_,
		   const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	if (bRollforward_)
		// ロールフォワードリカバリの場合、エリア関係のRedoは実施しない
		return;
	
	// CreateAreaはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Area::getObjectID(cLogData_);

	//--- CREATE AREA ---

	// 渡されたデータベース名からデータベースオブジェクトを取得する
	Database* pcDatabase =
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadWrite,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropArea)) {
		// どうせ Drop されているから何もしない
		// ただし、オブジェクトIDの整合性をとる必要がある

		Object::ID::assign(cTrans_, pcDatabase, id, Object::ID::SystemTable);
		return;
	}

	Manager::SystemTable::ReorganizeArea::Create(cTrans_, pcDatabase, &cLogData_).execute();
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Area
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
dropArea(Trans::Transaction& cTrans_, const LogData& cLogData_,
		 const Object::Name& cDatabaseName_, bool bRollforward_)
{
	if (bRollforward_)
		// ロールフォワードリカバリの場合、エリア関係のRedoは実施しない
		return;
	
	// DropAreaはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// ログデータからIDを取得する
	ObjectID::Value id = Area::getObjectID(cLogData_);

	// MOUNTでも使用する部分を分離する
	dropArea(cTrans_, id, cDatabaseName_);
}

// MOUNTでも使用する部分を分離する
void
Manager::SystemTable::RedoUtility::
dropArea(Trans::Transaction& cTrans_, Object::ID::Value iID_,
		 const Object::Name& cDatabaseName_,
		 Database* pDatabase_ /* = 0 */, bool bMount_ /* = false */)
{
	using namespace RecoveryUtility;

	// DROPがUNDOされたことを忘れる
	// -> DROPのREDO以降もAREAがDROPされたことを使うのでここでは忘れない
	// Undo::remove(cDatabaseName_, iID_, Undo::Type::DropArea);
	if (Undo::isEntered(cDatabaseName_, iID_, Undo::Type::CreateArea)) {
		// CREATEがUNDOされているとき、そのことを忘れる
		Undo::remove(cDatabaseName_, iID_, Undo::Type::CreateArea);

	} else {
		// CREATEがUNDOされていないとき、エリア表から登録を抹消する

		Database* pcDatabase = pDatabase_;

		if (!pcDatabase) {
			// データベース名からデータベースのスキーマオブジェクトを取得
			pcDatabase =
				Database::getLocked(cTrans_, cDatabaseName_,
									Lock::Name::Category::Tuple,
									Hold::Operation::ReadForWrite);
			if (!pcDatabase) {
				_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
			}
		}
		; _SYDNEY_ASSERT(pcDatabase);

		// キャッシュが破棄されないようにopenする
		pcDatabase->open();
		// スコープを抜けるときにデータベースのcloseを呼ぶ
		Common::AutoCaller1<Database, bool> autoCloser(pcDatabase, &Database::close, true /* volatile */);

		// IDからエリアオブジェクトを得て登録を抹消する
		Area::Pointer pArea = pcDatabase->loadArea(cTrans_).get(iID_);
		if (pArea.get()) {

			if (bMount_) {
				// MOUNTに伴う処理の場合、削除マークをつけるだけである
				pArea->dropForMount(cTrans_);

			} else {
				// 通常処理の場合エントリーを削除する
				pArea->drop(cTrans_, true/* recovery */, false /* no check */);
				Schema::SystemTable::Area(*pcDatabase).store(cTrans_, pArea);
			}
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Area
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
alterArea(Trans::Transaction& cTrans_, const LogData& cLogData_,
		  const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	if (bRollforward_)
		// ロールフォワードリカバリの場合、エリア関係のRedoは実施しない
		return;
	
	// AlterAreaはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Area::getObjectID(cLogData_);

	if ( Undo::isEntered(cDatabaseName_, id, Undo::Type::DropArea) )
	{
		// どうせ Drop されているから何もしない
		return;
	}

	// エリア表を更新する

	// データベース名からデータベースのスキーマオブジェクトを取得
	Database* pcDatabase = Database::getLocked(cTrans_, cDatabaseName_,
											   Lock::Name::Category::Tuple,
											   Hold::Operation::ReadWrite,
											   Lock::Name::Category::Tuple,
											   Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	// キャッシュが破棄されないようにopenする
	pcDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Database, bool> autoCloser(pcDatabase, &Database::close, true /* volatile */);

	// オブジェクトを得る
	Area::Pointer pArea = pcDatabase->loadArea(cTrans_).get(id);
	; _SYDNEY_ASSERT(pArea.get());

	// ログデータから移動前後のパスを得る
	ModVector<ModUnicodeString> vecPrevPath;
	ModVector<ModUnicodeString> vecPostPath;
	pArea->getPath(cLogData_, Area::Log::Alter::PrevPath, vecPrevPath);
	pArea->getPath(cLogData_, Area::Log::Alter::PostPath, vecPostPath);

	// MOUNTでも使用する部分を分離する
	alterArea(cTrans_, id, vecPrevPath, vecPostPath, cDatabaseName_, *pArea);

	// 永続化する
	Schema::SystemTable::Area(*pcDatabase).store(cTrans_, pArea);
	Schema::SystemTable::File(*pcDatabase).store(cTrans_);
	Schema::SystemTable::AreaContent(*pcDatabase).store(cTrans_);

	// ObjectIDを永続化する
	Schema::ObjectID::persist(cTrans_, pcDatabase);
}

// MOUNTでも使用する部分を分離する
void
Manager::SystemTable::RedoUtility::
alterArea(Trans::Transaction& cTrans_,
		  ObjectID::Value id_,
		  const ModVector<ModUnicodeString>& vecPrevPath_,
		  const ModVector<ModUnicodeString>& vecPostPath_,
		  const Object::Name& cDatabaseName_,
		  Area& cArea_, bool bMount_ /* = false */)
{
	using namespace RecoveryUtility;

	// ファイルの移動が必要であるかを示す
	bool bMoveActually = !bMount_;

	// ALTERがUNDOされたことを忘れる
	Undo::remove(cDatabaseName_, id_, Undo::Type::AlterArea);

	ModVector<ModUnicodeString> vecPrevPath;
	ModVector<ModUnicodeString> vecPostPath;

	// その後のALTER AREAでパスが変更されている場合変更後のパスを最終的なものに変更する
	if (!Path::getUndoAreaPath(cDatabaseName_, id_, vecPrevPath)) {
		// 登録されていない場合引数のパスを使う
		vecPrevPath = vecPrevPath_;
		vecPostPath = vecPostPath_;
	} else {
		// 登録されている場合変更前後ともに登録されているものを使う
		// パスが同一なのでファイルの移動はしない
		vecPostPath = vecPrevPath;
		bMoveActually = false;
	}

	// ファイルを移動する
	if (!bMoveActually) {
		// Areaのメンバーを変更するのみでファイルの移動処理はしない
		cArea_.moveForMount(cTrans_, vecPrevPath_, vecPostPath, false /* not undo */, true /* recovery */);
	} else {
		// 実際にファイルを移動する
		cArea_.move(cTrans_, vecPrevPath_, vecPostPath, false /* not undo */, true /* recovery */);
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Table
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
createTable(Trans::Transaction& cTrans_, const LogData& cLogData_,
			const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// CreateTableはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// table ID を取得
	ObjectID::Value id = Table::getObjectID(cLogData_);

	// データベース名からデータベースのスキーマオブジェクトを取得
	// ★注意★
	//	Redoのときは一時表のcreateはこない

	Database* pcDatabase = Database::getLocked(cTrans_, cDatabaseName_,
											   Lock::Name::Category::Tuple,
											   Hold::Operation::ReadForWrite);
	if (!pcDatabase)
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

	// table が drop されているか調べる
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropTable)) {
		// どうせ消されるので何もしない
		// ただし、オブジェクトIDの整合性をとる必要がある

		Object::ID::assign(cTrans_, pcDatabase, id, Object::ID::SystemTable);
		return;
	}

	Manager::SystemTable::ReorganizeTable::Create(cTrans_, pcDatabase, &cLogData_, false).execute();
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Table
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
dropTable(Trans::Transaction& cTrans_, const LogData& cLogData_,
		  const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropTableはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Table::getObjectID(cLogData_);

	// drop table のフラグをオフにする
	Undo::remove(cDatabaseName_, id, Undo::Type::DropTable);

	// create がUndoされているときは作成自体行われていない状態なので
	// フラグをオフにするだけでよい
	if ( Undo::isEntered(cDatabaseName_, id, Undo::Type::CreateTable) )
	{
		Undo::remove(cDatabaseName_, id, Undo::Type::CreateTable);
		return;
	}

	// データベース名からデータベースのスキーマオブジェクトを取得
	Database* pcDatabase =
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	Manager::SystemTable::ReorganizeTable::Drop(cTrans_, pcDatabase, &cLogData_, false, bRollforward_).execute();
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Table
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
//		bool bRollfoward_
//			ロールフォワードのためかどうか
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
alterTable(Trans::Transaction& cTrans_, const LogData& cLogData_,
		   const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterTableはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Table::getObjectID(cLogData_);

	// drop されているか調べる
	if ( Undo::isEntered(cDatabaseName_, id, Undo::Type::DropTable) )
		// どうせ消されるので何もしない
		return;

	// データベース名からデータベースのスキーマオブジェクトを取得
	Database* pcDatabase =
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	switch (cLogData_.getSubCategory()) {
	case LogData::Category::AlterTable:
		{
			// call special function because the process is shared with Mount
			alterTableArea(cTrans_, id, cLogData_, pcDatabase, bRollforward_);
			break;
		}
	default:
		{
			Manager::SystemTable::ReorganizeTable::Alter(cTrans_, pcDatabase, &cLogData_, bRollforward_).execute();
			break;
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		ObjectID::Value id_
//			対象の表ID
//		const Schema::LogData& cLogData_
//			ログデータ
//		Schema::Database* pDatabase_
//			データベースオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
alterTableArea(Trans::Transaction& cTrans_, ObjectID::Value id_,
			   const LogData& cLogData_, Schema::Database* pDatabase_,
			   bool bRollforward_)
{
	if (bRollforward_)
		// ロールフォワードリカバリの場合、エリア関係のRedoは実施しない
		return;
	
	// キャッシュが破棄されないようにopenする
	pDatabase_->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Database, bool> autoCloser(pDatabase_, &Database::close, true /* volatile */);

	ModVector<Object::ID::Value> vecPrevAreaID;
	ModVector<Object::ID::Value> vecPostAreaID;

	// ログデータから変更前後のエリア割り当てを取得
	Table::getAreaID(cLogData_, Table::Log::Alter::PrevAreaIDs, vecPrevAreaID);
	Table::getAreaID(cLogData_, Table::Log::Alter::PostAreaIDs, vecPostAreaID);

	// オブジェクトを得る
	Table::Pointer pTable = pDatabase_->loadTable(cTrans_).get(id_);
	; _SYDNEY_ASSERT(pTable.get());

	// MOUNTと共通の処理を分離する
	alterTable(cTrans_, id_, vecPrevAreaID, vecPostAreaID, pDatabase_->getName(), *pTable);

	// 関係するシステム表を更新
	//	 表の変更によりオブジェクトの内容が変化した可能性のある
	//	 ものについてシステム表に永続化する
	//	 対象:表、ファイル

	Schema::SystemTable::Table(*pDatabase_).store(cTrans_, pTable);
	Schema::SystemTable::File(*pDatabase_).store(cTrans_, *pTable);

	// 変更したエリア格納関係の情報を「エリア格納関係」表へ登録する
	Schema::SystemTable::AreaContent(*pDatabase_).store(cTrans_);

	// ObjectIDを永続化する
	Schema::ObjectID::persist(cTrans_, pDatabase_);
}

// MOUNTと共通の処理を分離する
void
Manager::SystemTable::RedoUtility::
alterTable(Trans::Transaction& cTrans_,
		   Object::ID::Value iID_,
		   const ModVector<Object::ID::Value>& vecPrevAreaID_,
		   const ModVector<Object::ID::Value>& vecPostAreaID_,
		   const Object::Name& cDatabaseName_,
		   Table& cTable_,
		   bool bMount_ /* = false */)
{
	using namespace RecoveryUtility;

	ModVector<Object::ID::Value> vecPrevAreaID;
	ModVector<Object::ID::Value> vecPostAreaID;

	// 変更後のエリア割り当ては最終的なエリア割り当てがあればそれを使用する
	// ★注意★
	// REDO処理ではファイルの移動はせず、「表」表のエリア割り当てやエリア格納関係を
	// 修正することが処理内容なので変更前までUndo情報の最新のエリア割り当てを使用してはいけない
	if (!ID::getUndoAreaID(cDatabaseName_, iID_, vecPostAreaID)) {
		// 最終的なエリアIDが登録されていなければ引数のエリアIDを使用する
		vecPostAreaID = vecPostAreaID_;
	}

	// i.スキーマオブジェクトのエリア割り当てを変更
	// j.ファイルを移動

	cTable_.move(cTrans_, vecPrevAreaID_, vecPostAreaID, false /* not undo */, true /* recovery */, bMount_);
	cTable_.touch();
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Index
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
createIndex(Trans::Transaction& cTrans_, const LogData& cLogData_,
			const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// CreateIndexはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Index::getObjectID(cLogData_);

	// データベース名からデータベースのスキーマオブジェクトを取得
	Database* pcDatabase =
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	// index または属する表が drop されているか調べる
	ObjectID::Value tableID = Index::getTableID(cLogData_);
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropIndex)
		|| Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable)) {

		// どうせ消されるので何もしない
		// ただし、オブジェクトIDの整合性をとる必要がある

		Object::ID::assign(cTrans_, pcDatabase, id, Object::ID::SystemTable);
		return;
	}

	// execute redo create index
	Manager::SystemTable::ReorganizeIndex::Create(cTrans_, pcDatabase, &cLogData_).execute();
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Index
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
dropIndex(Trans::Transaction& cTrans_, const LogData& cLogData_,
		  const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropIndexはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Index::getObjectID(cLogData_);
   
	// DROPをUNDOしたことを忘れる
	{
		Undo::remove(cDatabaseName_, id, Undo::Type::DropIndex);

		// CREATEがUNDOされているか調べる
		if ( Undo::isEntered(cDatabaseName_, id, Undo::Type::CreateIndex) )
		{
			// CREATEがUNDOされたことを忘れる
			Undo::remove(cDatabaseName_, id, Undo::Type::CreateIndex);
			
			// 最初から何もなかったことになっているので
			// これ以上することはない
			return;
		}
	}

	// データベース名からデータベースのスキーマオブジェクトを取得
	Database* pcDatabase =
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	// ログデータから表名を得てオブジェクトを得る
	// 以前はTABLEのCREATEがREDOされているかも知れないので
	// TABLEのIDはログを書いた時点のものを使うことができないとしていたが
	// CREATEのREDOでもIDが変わらないようにしたのでIDを使うことができる
	Object::ID::Value iTableID = Index::getTableID(cLogData_);
	Table* pTable = pcDatabase->getTable(iTableID, cTrans_, true /* internal */);
	if (!pTable) {
		// TableのDROPがUNDOされているのでなければログの内容がおかしい
		if (Undo::isEntered(cDatabaseName_, iTableID, Undo::Type::DropTable)) {
			return;
		}
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// execute redo drop index
	Manager::SystemTable::ReorganizeIndex::Drop(cTrans_, pcDatabase, pTable, &cLogData_).execute();
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Index
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
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
alterIndex(Trans::Transaction& cTrans_, const LogData& cLogData_,
		   const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropIndexはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Index::getObjectID(cLogData_);

	// index または属する表が drop されているか調べる
	ObjectID::Value tableID = Index::getTableID(cLogData_);
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropIndex)
		|| Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable))

		// どうせ消されるので何もしない
		return;

	if (bRollforward_)
		// ロールフォワードリカバリの場合、エリア関係のRedoは実施しない
		return;
	
	//--- ALTER INDEX SET/DROP AREA ---

	// データベース名からデータベースのスキーマオブジェクトを取得
	Database* pcDatabase =
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	// キャッシュが破棄されないようにopenする
	pcDatabase->open();
	// スコープを抜けるときにデータベースのcloseを呼ぶ
	Common::AutoCaller1<Database, bool> autoCloser(pcDatabase, &Database::close, true /* volatile */);

	// ログデータから表名と索引名を得てオブジェクトを得る
	// CREATEがREDOされているかもしれないので
	// ログを書いた時点のIDを使うことはできない
	Object::Name cTableName = Index::getTableName(cLogData_);
	Object::Name cName = Index::getName(cLogData_);
	Table* pTable = pcDatabase->getTable(cTableName, cTrans_, true /* internal */);
	if (!pTable) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	Index::Pointer pIndex = pTable->loadIndex(cTrans_).get(id);
	if (!pIndex.get()) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// 変更前後のエリアIDを得る
	ModVector<Object::ID::Value> vecPrevAreaID;
	ModVector<Object::ID::Value> vecPostAreaID;
	Index::getAreaID(cLogData_, Index::Log::Alter::PrevAreaID, vecPrevAreaID);
	Index::getAreaID(cLogData_, Index::Log::Alter::PostAreaID, vecPostAreaID);

	// MOUNTと共通の処理を分離する
	alterIndex(cTrans_, id, vecPrevAreaID, vecPostAreaID, cDatabaseName_, *pIndex);

	// 関係するシステム表を更新
	//	 変更によりオブジェクトの内容が変化した可能性のある
	//	 ものについてシステム表に永続化する
	//	 対象:索引、ファイル

	Schema::SystemTable::Index(*pcDatabase).store(cTrans_, pIndex);
	Schema::SystemTable::File(*pcDatabase).store(cTrans_, *pIndex);

	// 変更したエリア格納関係の情報を「エリア格納関係」表へ登録する
	Schema::SystemTable::AreaContent(*pcDatabase).store(cTrans_);

	// ObjectIDを永続化する
	Schema::ObjectID::persist(cTrans_, pcDatabase);
}

// MOUNTと共通の処理を分離する
void
Manager::SystemTable::RedoUtility::
alterIndex(Trans::Transaction& cTrans_,
		   Object::ID::Value iID_,
		   const ModVector<Object::ID::Value>& vecPrevAreaID_,
		   const ModVector<Object::ID::Value>& vecPostAreaID_,
		   const Object::Name& cDatabaseName_,
		   Index& cIndex_,
		   bool bMount_ /* = false */)
{
	using namespace RecoveryUtility;

	ModVector<Object::ID::Value> vecPostAreaID;

	// 変更後のエリア割り当ては最終的なエリア割り当てがあればそれを使用する
	// ★注意★
	// REDO処理ではファイルの移動はせず、「索引」表のエリア割り当てやエリア格納関係を
	// 修正することが処理内容なので変更前までUndo情報の最新のエリア割り当てを使用してはいけない
	if (!ID::getUndoAreaID(cDatabaseName_, iID_, vecPostAreaID)) {
		// 最終的なエリアIDが登録されていなければ引数のエリアIDを使用する
		vecPostAreaID = vecPostAreaID_;
	}

	// i.スキーマオブジェクトのエリア割り当てを変更
	// j.ファイルを移動
	cIndex_.moveArea(cTrans_, vecPrevAreaID_, vecPostAreaID, false /* not undo */, true /* recovery */, bMount_);
	cIndex_.touch();
}

//	FUNCTION
//	Schema::Manager::SystemTable::RedoUtility::Index
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
//		bool bRollforward_
//			ロールフォワードのためかどうか
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Manager::SystemTable::RedoUtility::
renameIndex(Trans::Transaction& cTrans_, const LogData& cLogData_,
			const Object::Name& cDatabaseName_, bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropIndexはデータベースのログに書かれているはずである
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	ObjectID::Value id = Index::getObjectID(cLogData_);

	// index または属する表が drop されているか調べる
	ObjectID::Value tableID = Index::getTableID(cLogData_);
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropIndex)
		|| Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable))

		// どうせ消されるので何もしない
		return;

	// データベース名からデータベースのスキーマオブジェクトを取得
	Database* pcDatabase =
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	// オブジェクトを得る
	Table* pTable = pcDatabase->getTable(tableID, cTrans_);
	if (!pTable) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	Index* pIndex = pTable->getIndex(id, cTrans_);
	if (!pIndex) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// execute redo alter index rename
	Manager::SystemTable::ReorganizeIndex::AlterName(cTrans_, pcDatabase, pTable, pIndex, &cLogData_, bRollforward_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::createPrivilege -- create privilege from log data
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
createPrivilege(Trans::Transaction& cTrans_,
				const Schema::LogData& cLogData_,
				const Schema::ObjectName& cDatabaseName_,
				bool bRollforward_)
{
	using namespace RecoveryUtility;

	// CreatePrivilege should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object ID for the privilege
	ObjectID::Value id = Privilege::getID(cLogData_);

	// get Database object from the database name
	Database* pcDatabase =
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPrivilege)) {
		// This object has been dropped by following log data
		// Only take consistency about objectID

		Object::ID::assign(cTrans_, pcDatabase, id, Object::ID::SystemTable);
		return;
	}

	Manager::SystemTable::ReorganizePrivilege::Create(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::dropPrivilege -- drop privilege from log data
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
dropPrivilege(Trans::Transaction& cTrans_,
			  const Schema::LogData& cLogData_,
			  const Schema::ObjectName& cDatabaseName_,
			  bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropPrivilege should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Privilege::getID(cLogData_);

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::CreatePrivilege)) {
		// If the privilege has been created in the same log sequence, forget it.
		Undo::remove(cDatabaseName_, id, Undo::Type::CreatePrivilege);

	} else {
		// If the privilege has had been created before the log sequence,
		// delete the privilege object here

		// get database object
		Database* pcDatabase = 
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
		if (!pcDatabase) {
			_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
		}
		; _SYDNEY_ASSERT(pcDatabase);

		Manager::SystemTable::ReorganizePrivilege::Drop(cTrans_, pcDatabase, &cLogData_).execute();
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::alterPrivilege -- alter privilege from log data
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
alterPrivilege(Trans::Transaction& cTrans_,
			   const Schema::LogData& cLogData_,
			   const Schema::ObjectName& cDatabaseName_,
			   bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterPrivilege should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Privilege::getID(cLogData_);

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPrivilege)) {
		// the privilege object will be dropped by following log, so do nothing
		return;
	}

	// get database object
	Database* pcDatabase = 
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	; _SYDNEY_ASSERT(pcDatabase);

	Manager::SystemTable::ReorganizePrivilege::Alter(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::createCascade -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
createCascade(Trans::Transaction& cTrans_,
			  const Schema::LogData& cLogData_,
			  const Schema::ObjectName& cDatabaseName_,
			  bool bRollforward_)
{
	using namespace RecoveryUtility;

	// CreateCascade should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object ID for the cascade
	ObjectID::Value id = Cascade::getObjectID(cLogData_);

	// get Database object from the database name
	Database* pcDatabase =
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropCascade)) {
		// This object has been dropped by following log data
		// Only take consistency about objectID

		Object::ID::assign(cTrans_, pcDatabase, id, Object::ID::SystemTable);
		return;
	}

	Manager::SystemTable::ReorganizeCascade::Create(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::dropCascade -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
dropCascade(Trans::Transaction& cTrans_,
			const Schema::LogData& cLogData_,
			const Schema::ObjectName& cDatabaseName_,
			bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropCascade should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Cascade::getObjectID(cLogData_);

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::CreateCascade)) {
		// If the cascade has been created in the same log sequence, forget it.
		Undo::remove(cDatabaseName_, id, Undo::Type::CreateCascade);

	} else {
		// If the cascade has had been created before the log sequence,
		// delete the cascade object here

		// get database object
		Database* pcDatabase = 
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
		if (!pcDatabase) {
			_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
		}
		; _SYDNEY_ASSERT(pcDatabase);

		Manager::SystemTable::ReorganizeCascade::Drop(cTrans_, pcDatabase, &cLogData_).execute();
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::alterCascade -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
alterCascade(Trans::Transaction& cTrans_,
			 const Schema::LogData& cLogData_,
			 const Schema::ObjectName& cDatabaseName_,
			 bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterCascade should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Cascade::getObjectID(cLogData_);

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropCascade)) {
		// the cascade object will be dropped by following log, so do nothing
		return;
	}

	// get database object
	Database* pcDatabase = 
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	; _SYDNEY_ASSERT(pcDatabase);

	Manager::SystemTable::ReorganizeCascade::Alter(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::createPartition -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
createPartition(Trans::Transaction& cTrans_,
				const Schema::LogData& cLogData_,
				const Schema::ObjectName& cDatabaseName_,
				bool bRollforward_)
{
	using namespace RecoveryUtility;

	// CreatePartition should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object ID for the partition
	ObjectID::Value id = Partition::getObjectID(cLogData_);
	ObjectID::Value tableID = Partition::getTableID(cLogData_);

	// get Database object from the database name
	Database* pcDatabase =
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPartition)
		|| Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable)) {
		// just verify objectID
		Object::ID::assign(cTrans_, pcDatabase, id, Object::ID::SystemTable);
		return;
	}

	Manager::SystemTable::ReorganizePartition::Create(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::dropPartition -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
dropPartition(Trans::Transaction& cTrans_,
			   const Schema::LogData& cLogData_,
			   const Schema::ObjectName& cDatabaseName_,
			   bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropPartition should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Partition::getObjectID(cLogData_);

	// remove drop flag
	Undo::remove(cDatabaseName_, id, Undo::Type::DropPartition);

	// Check CREATE flag
	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::CreatePartition)) {
		// remove create flag
		Undo::remove(cDatabaseName_, id, Undo::Type::CreatePartition);
		// nothing more
		return;
	}

	// get database object
	Database* pcDatabase = 
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	; _SYDNEY_ASSERT(pcDatabase);

	Manager::SystemTable::ReorganizePartition::Drop(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::alterPartition -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
alterPartition(Trans::Transaction& cTrans_,
			   const Schema::LogData& cLogData_,
			   const Schema::ObjectName& cDatabaseName_,
			   bool bRollforward_)
{
	using namespace RecoveryUtility;

	// AlterPartition should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Partition::getObjectID(cLogData_);
	ObjectID::Value tableID = Partition::getTableID(cLogData_);

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropPartition)
		|| Undo::isEntered(cDatabaseName_, tableID, Undo::Type::DropTable)) {
		// the partition object will be dropped by following log, so do nothing
		return;
	}

	// get database object
	Database* pcDatabase = 
		Database::getLocked(cTrans_, cDatabaseName_,
							Lock::Name::Category::Tuple,
							Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	; _SYDNEY_ASSERT(pcDatabase);

	Manager::SystemTable::ReorganizePartition::Alter(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::createFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
createFunction(Trans::Transaction& cTrans_,
			   const Schema::LogData& cLogData_,
			   const Schema::ObjectName& cDatabaseName_,
			   bool bRollforward_)
{
	using namespace RecoveryUtility;

	// CreateFunction should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object ID for the function
	ObjectID::Value id = Function::getObjectID(cLogData_);

	// get Database object from the database name
	Database* pcDatabase =
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::DropFunction)) {
		// This object has been dropped by following log data
		// Only take consistency about objectID

		Object::ID::assign(cTrans_, pcDatabase, id, Object::ID::SystemTable);
		return;
	}

	Manager::SystemTable::ReorganizeFunction::Create(cTrans_, pcDatabase, &cLogData_).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::RedoUtility::dropFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Schema::LogData& cLogData_
//	const Schema::ObjectName& cDatabaseName_
//	bool bRollforward_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::RedoUtility::
dropFunction(Trans::Transaction& cTrans_,
			 const Schema::LogData& cLogData_,
			 const Schema::ObjectName& cDatabaseName_,
			 bool bRollforward_)
{
	using namespace RecoveryUtility;

	// DropFunction should be written in database log
	; _SYDNEY_ASSERT(
		cDatabaseName_ != Object::Name(NameParts::Database::System));

	// get schema object id from log data
	ObjectID::Value id = Function::getObjectID(cLogData_);

	if (Undo::isEntered(cDatabaseName_, id, Undo::Type::CreateFunction)) {
		// If the function has been created in the same log sequence, forget it.
		Undo::remove(cDatabaseName_, id, Undo::Type::CreateFunction);

	} else {
		// If the function has had been created before the log sequence,
		// delete the function object here

		// get database object
		Database* pcDatabase = 
			Database::getLocked(cTrans_, cDatabaseName_,
								Lock::Name::Category::Tuple,
								Hold::Operation::ReadForWrite);
		if (!pcDatabase) {
			_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
		}
		; _SYDNEY_ASSERT(pcDatabase);

		Manager::SystemTable::ReorganizeFunction::Drop(cTrans_, pcDatabase, &cLogData_).execute();
	}
}

//
// Copyright (c) 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
