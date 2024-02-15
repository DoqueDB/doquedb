// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Reorganize.cpp -- 再構成関連の関数定義(Manager::SystemTable)
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
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

#include "Schema/Database.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/NameParts.h"
#include "Schema/ObjectSnapshot.h"
#include "Schema/Operation.h"
#include "Schema/Parameter.h"
#include "Schema/PathParts.h"
#include "Schema/Reorganize.h"
#include "Schema/ReorganizeArea.h"
#include "Schema/ReorganizeCascade.h"
#include "Schema/ReorganizeDatabase.h"
#include "Schema/ReorganizeFunction.h"
#include "Schema/ReorganizeIndex.h"
#include "Schema/ReorganizePartition.h"
#include "Schema/ReorganizePrivilege.h"
#include "Schema/ReorganizeTable.h"
#include "Schema/SystemDatabase.h"
#include "Schema/SystemTable_Database.h"
#include "Schema/Utility.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#ifndef SYD_COVERAGE
#include "Admin/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/Message.h"
#include "Common/Parameter.h"

#include "DSchema/ReorganizeIndex.h"
#include "DSchema/ReorganizeTable.h"

#include "Exception/DatabaseChanged.h"
#include "Exception/DatabaseNotFound.h"
#include "Exception/NotSupported.h"
#include "Exception/ReadOnlyTransaction.h"
#include "Exception/TemporaryTable.h"

#include "Os/AutoCriticalSection.h"

#include "Server/InstanceManager.h"
#include "Server/Manager.h"
#include "Server/Session.h"

#include "Statement/AlterAreaStatement.h"
#include "Statement/AlterCascadeStatement.h"
#include "Statement/AlterDatabaseAttribute.h"
#include "Statement/AlterDatabaseAttributeList.h"
#include "Statement/AlterDatabaseStatement.h"
#include "Statement/AlterIndexStatement.h"
#include "Statement/AlterPartitionStatement.h"
#include "Statement/AlterTableStatement.h"
#include "Statement/AreaDefinition.h"
#include "Statement/CascadeDefinition.h"
#include "Statement/DatabaseDefinition.h"
#include "Statement/DropAreaStatement.h"
#include "Statement/DropCascadeStatement.h"
#include "Statement/DropDatabaseStatement.h"
#include "Statement/DropFunctionStatement.h"
#include "Statement/DropIndexStatement.h"
#include "Statement/DropPartitionStatement.h"
#include "Statement/DropTableStatement.h"
#include "Statement/FunctionDefinition.h"
#include "Statement/GrantStatement.h"
#include "Statement/IndexDefinition.h"
#include "Statement/MoveDatabaseStatement.h"
#include "Statement/Object.h"
#include "Statement/PartitionDefinition.h"
#include "Statement/RevokeStatement.h"
#include "Statement/TableDefinition.h"
#include "Statement/Type.h"

#include "Trans/Transaction.h"
#include "Trans/LogFile.h"

#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

// このファイルで定義される変数対する操作を
// スレッド間排他制御するためのクリティカルセクション

Os::CriticalSection& _criticalSection = Manager::getCriticalSection();

//	VARIABLE local
//	_bInitialized --
//
//	NOTES

bool _bInitialized = false;

class _AutoEnterSuperUserMode
{
public:
	_AutoEnterSuperUserMode(const Object::Name& cName_, const ObjectID::Value iID_)
		:m_cName(cName_),
		 m_iID(iID_)
	{
		Database::enterSuperUserModeTransitionalState(cName_, iID_);
	}
	~_AutoEnterSuperUserMode() {
		Database::exitSuperUserModeTransitionalState(m_cName, m_iID);
	}
private:
	Object::Name m_cName;

	Object::ID m_iID;
	
};

	
namespace _Timestamp
{
	// この名前空間で定義される変数に対する操作を
	// スレッド間排他制御するためのクリティカルセクション

	Os::CriticalSection _latch;

	//	VARIABLE local
	//	_Timestamp::_currentValue -- 現在のタイムスタンプ
	//
	//	NOTES

	Manager::SystemTable::Timestamp _currentValue = 0;

	// タイムスタンプをひとつ進める
	void _addValue();
	// タイムスタンプの現在値を得る
	Manager::SystemTable::Timestamp _getCurrentValue();

} // namespace _Timestamp

namespace _Log
{
	// この名前空間で定義される変数に対する操作を
	// スレッド間排他制御するためのクリティカルセクション

	Os::CriticalSection _latch;

	//	VARIABLE local
	//	_Log::_pFilePath
	//
	//	NOTES

	ModAutoPointer<Os::Path> _pFilePath;

	// スキーマ操作を記録する論理ログファイルのファイル名を得る
	const Os::Path&	_getFilePath();

	// この名前空間の関数について後処理をする
	void _terminate();

} // namespace _Log

namespace _Lock
{
	// ロック名を取得する
	Lock::Name getLockName(Trans::Transaction& cTrans_,
						   Hold::Target::Value eTarget_,
						   Object::ID::Value iID_,
						   Trans::Log::File::Category::Value eLogCategory_,
						   Database* pDatabase_);

} // namespace _Lock

} // end of non namespace for local

/////////////////////////////////////
// _Timestamp
/////////////////////////////////////

//	FUNCTION local
//	_Timestamp::_addValue -- タイムスタンプをひとつ進める
//
//	NOTES
//		再構成によりスキーマ情報が変化したら
//		タイムスタンプを追加することにより
//		ObjectSnapshotを捨てるべきか否かの判断をする

inline
void
_Timestamp::_addValue()
{
	Os::AutoCriticalSection m(_latch);
	++_currentValue;
}

//	FUNCTION local
//	_Timestamp::_getCurrentValue -- タイムスタンプの現在値を得る
//
//	NOTES

Manager::SystemTable::Timestamp
_Timestamp::_getCurrentValue()
{
	Os::AutoCriticalSection m(_latch);
	return _currentValue;
}

/////////////////////////////////////
// _Log
/////////////////////////////////////

//	FUNCTION local
//	_Log::_getFilePath
//		-- スキーマ操作を記録する論理ログファイルのファイル名を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		論理ログファイルのファイル名
//
//	EXCEPTIONS
//		なし

const Os::Path&
_Log::_getFilePath()
{
	if (!_pFilePath) {
		Os::AutoCriticalSection m(_latch);
		// クリティカルセクションの中でもう一度調べる

		if (!_pFilePath) {
			ModAutoPointer<Os::Path> pPath =
				new Os::Path(Manager::Configuration::getSystemAreaPath());
//			pPath->addPart(PathParts::File::Log);
			_pFilePath = pPath.release();
		}
	}
	return *_pFilePath;
}

//	FUNCTION local
//	_Log::_terminate -- この名前空間の関数について後処理をする
//
//	NOTES
//
//	ARGUMENTS
//
//	RETURN
//
//	EXCEPTIONS

void
_Log::_terminate()
{
	Os::AutoCriticalSection m(_latch);
	_Log::_pFilePath = 0;
}

/////////////////////////////////////
// _Lock
/////////////////////////////////////

//	FUNCTION local
//	_Lock::getLockName -- ロック名を取得する
//
//	NOTES

Lock::Name
_Lock::getLockName(Trans::Transaction& cTrans_,
				   Hold::Target::Value eTarget_,
				   Object::ID::Value iID_,
				   Trans::Log::File::Category::Value eLogCategory_,
				   Database* pDatabase_)
{
	switch (eTarget_) {
	case Hold::Target::MetaDatabase:
		{
#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "LockName(MetaDatabase)"
				<< ModEndl;
#endif
			return Lock::DatabaseName(static_cast<Lock::Name::Part>(Object::ID::SystemTable));
		}
	case Hold::Target::MetaTable:
		{
#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "LockName(MetaTable)"
				<< ModEndl;
#endif
			return Lock::TableName(static_cast<Lock::Name::Part>(Object::ID::SystemTable),
								   static_cast<Lock::Name::Part>(Object::Category::Database));
		}
	case Hold::Target::MetaTuple:
		{
			; _SYDNEY_ASSERT(iID_ != Object::ID::Invalid);
#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "LockName(MetaTuple): "<< iID_
				<< ModEndl;
#endif
			return Lock::TupleName(static_cast<Lock::Name::Part>(Object::ID::SystemTable),
								   static_cast<Lock::Name::Part>(Object::Category::Database),
								   iID_);
		}
	case Hold::Target::Database:
		{
			; _SYDNEY_ASSERT(iID_ != Object::ID::Invalid);
#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "LockName(Database): "<< iID_
				<< ModEndl;
#endif
			return Lock::DatabaseName(iID_);
		}
	case Hold::Target::LogicalLog:
		{
#ifdef DEBUG
			SydSchemaParameterMessage(Message::ReportSystemTable)
				<< "LockName(LogicalLog): "<< eLogCategory_
				<< ModEndl;
#endif
			; _SYDNEY_ASSERT(eLogCategory_ == Trans::Log::File::Category::System ||
							 pDatabase_);

			return (eLogCategory_ == Trans::Log::File::Category::System)
				? Manager::SystemTable::getLogFile()->getLockName()
				: pDatabase_->getLogFile()->getLockName();
		}
	default:
		; _SYDNEY_ASSERT(false);
	}
	// never reach
	return Lock::Name();
}

//////////////////////////////////////
//	 Schema::Manager::SystemTable	//
//////////////////////////////////////

//	FUNCTION
//	Schema::Manager::SystemTable::initialize --
//		マネージャーのうち、システム表関連の初期化を行う
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
Manager::SystemTable::
initialize()
{
	if (!_bInitialized) {
		Os::AutoCriticalSection m(_criticalSection);
		// クリティカルセクションの中でもう一度調べる

		if (!_bInitialized) {
			Schema::SystemTable::initialize();
			_bInitialized = true;
		}
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::terminate --
//		マネージャーのうち、システム表関連の終了処理を行う
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
Manager::SystemTable::
terminate()
{
	Os::AutoCriticalSection m(_criticalSection);

	if (_bInitialized) {
		Schema::SystemTable::terminate();
		_bInitialized = false;
	}

	_Log::_terminate();
}

//	FUNCTION
//	Schema::Manager::SystemTable::getCurrentTimestamp --
//		システム表のタイムスタンプを得る
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

Manager::SystemTable::Timestamp
Manager::SystemTable::
getCurrentTimestamp()
{
	return _Timestamp::_getCurrentValue();
}

//	FUNCTION
//	Schema::Manager::SystemTable::addTimestamp --
//		システム表のタイムスタンプを加える
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::
addTimestamp(Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.isNoVersion());

	// タイムスタンプを増やす
	_Timestamp::_addValue();
}

//	FUNCTION
//	Schema::Manager::SystemTable::reCache --
//			スキーマ情報に変化があったときコミット後にキャッシュの更新をする
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
Manager::SystemTable::
reCache()
{
	// スナップショットのキャッシュ情報を変更する
	ObjectSnapshot::reCache();
}

//	FUNCTION
//	Schema::Manager::SystemTable::install --
//		マネージャーのうち、システム表関連のインストール処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			インストールするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::install(Trans::Transaction& cTrans_)
{
	// 念のために初期化しておく
	initialize();

	Os::AutoCriticalSection m(_criticalSection);

	// スキーマ操作の論理ログファイルを作る
	//	論理ログは全てのファイルを作成する前に作らなければならない
	cTrans_.createLog(Trans::Log::File::Category::System);

	try {
		// 「データベース」表のファイルを格納するディレクトリを作成する
		Utility::File::AutoRmDir dir;
		dir.setDir(Os::Path(Configuration::getSystemAreaPath()).addPart(PathParts::SystemTable::Schema));

		// 「データベース」表を生成する
		Schema::SystemTable::Database().create(cTrans_);

		// 成功したのでエラー処理のためのクラスをdisableする
		dir.disable();

	} catch (...) {
		// 論理ログファイルを破棄する
		cTrans_.destroyLog(Trans::Log::File::Category::System);
		_SYDNEY_RETHROW;
	}
}

#ifdef OBSOLETE // uninstallは使用されない
//	FUNCTION
//	Schema::Manager::SystemTable::uninstall --
//		マネージャーのうち、システム表関連のアンインストール処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			アンインストールするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::uninstall(Trans::Transaction& cTrans_)
{
	// 念のために初期化しておく
	initialize();

	Os::AutoCriticalSection m(_criticalSection);

	// スキーマ操作の論理ログファイルを抹消する
	cTrans_.destroyLog(Trans::Log::File::Category::System);

	// 「データベース」表を抹消する
	Schema::SystemTable::Database().drop(cTrans_);
}
#endif

//	FUNCTION
//	Schema::Manager::SystemTable::recover --
//		マネージャーのうち、システム表関連の障害回復処理を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::
recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
#ifndef SYD_COVERAGE
	_SYDNEY_ADMIN_RECOVERY_MESSAGE
		<< "Recover [" << NameParts::Database::System << "] : " << cPoint_
		<< ModEndl;
#endif
	// 念のために初期化しておく
	initialize();

	// 「データベース」表を回復する
	Schema::SystemTable::Database().recover(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::SystemTable::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			障害を回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::
restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	// 念のために初期化しておく
	initialize();

	// 「データベース」表を回復する
	Schema::SystemTable::Database().restore(cTrans_, cPoint_);
}

//	FUNCTION
//	Schema::Manager::SystemTable::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理でシステム表を持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理でシステム表を持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、システム表を処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理でシステム表を持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理でシステム表を持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、システム表が更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::sync(
	Trans::Transaction& trans, bool& incomplete, bool& modified)
{
	// 念のために初期化しておく

	initialize();

	//「データベース」表の不要な版を破棄する
	Schema::SystemTable::Database().sync(trans, incomplete, modified);
}

//	FUNCTION public
//	Schema::Manager::SystemTable::hold --
//		自動的に適切なモードを判断してロックをかける
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		ロックをかけるトランザクション記述子
//	Schema::Hold::Target::Value eTarget_
//		ロックする対象を表す値
//	Lock::Name::Category::Value eManipulate_
//		操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperation_
//		読み込みのみを行おうとしているならtrue、
//		書き込みを行う可能性があるならfalse
//	Schema::Object::ID::Value iID_ = ID::Invalid
//		タプルをロックするときに指定するID
//	Trans::Log::File::Category::Value eLogCategory_
// 		= Trans::Log::File::Category::System
//		論理ログをロックするときに指定するログファイルの種別
//	Schema::Database* pDatabase_ = 0
//		データベースのログファイルが対象のときに渡すデータベースオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

bool
Manager::SystemTable::
hold(Trans::Transaction& cTrans_,
	 Hold::Target::Value eTarget_,
	 Lock::Name::Category::Value eManipulate_,
	 Hold::Operation::Value eOperation_,
	 Object::ID::Value iID_,
	 Trans::Log::File::Category::Value eLogCategory_,
	 Database* pDatabase_,
	 Lock::Timeout::Value iTimeout_)
{
	return Hold::hold(cTrans_, _Lock::getLockName(
						  cTrans_, eTarget_, iID_, eLogCategory_, pDatabase_),
					  eTarget_, eManipulate_, eOperation_, iTimeout_);
}

//	FUNCTION public
//	Schema::Manager::SystemTable::convert --
//		自動的に適切なモードを判断してロックモードを変更する
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		ロックをかけるトランザクション記述子
//	Schema::Hold::Target::Value eTarget_
//		ロックする対象をビットパターンで表す値
//	Lock::Name::Category::Value eManipulateFrom_
//		変更前の操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperationFrom_
//		変更前に読み込みのみを行おうとしているならtrue、
//		書き込みを行う可能性があるならfalse
//	Lock::Name::Category::Value eManipulateTo_
//		変更後の操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperationTo_
//		変更後に読み込みのみを行おうとしているならtrue、
//		書き込みを行う可能性があるならfalse
//	Schema::Object::ID::Value iID_ = ID::Invalid
//		タプルをロックするときに指定するID
//	Trans::Log::File::Category::Value eLogCategory_
// 		= Trans::Log::File::Category::System
//		論理ログをロックするときに指定するログファイルの種別
//	Schema::Database* pDatabase_ = 0
//		データベースのログファイルが対象のときに渡すデータベースオブジェクト
//	Lock::Timeout::Value iTimeout = Lock::Timeout::Unlimited
//		タイムアウト値
//
//	RETURN
//		bool true  : 成功
//			 false : 失敗
//
//	EXCEPTIONS

bool 
Manager::SystemTable::
convert(Trans::Transaction& cTrans_,
		Hold::Target::Value eTarget_,
		Lock::Name::Category::Value eManipulateFrom_,
		Hold::Operation::Value eOperationFrom_,
		Lock::Name::Category::Value eManipulateTo_,
		Hold::Operation::Value eOperationTo_,
		Object::ID::Value iID_,
		Trans::Log::File::Category::Value eLogCategory_,
		Database* pDatabase_,
		Lock::Timeout::Value iTimeout)
{
	return Hold::convert(cTrans_,
						 _Lock::getLockName(cTrans_, eTarget_, iID_, eLogCategory_, pDatabase_),
						 eTarget_,
						 eManipulateFrom_, eOperationFrom_,
						 eManipulateTo_, eOperationTo_, iTimeout);
}

//	FUNCTION public
//	Schema::Manager::SystemTable::release --
//		必要ならアンロックする
//
//	NOTES
//
//	ARGUMENTS
//	Trans::Transaction& cTrans_
//		ロックをかけるトランザクション記述子
//	Schema::Hold::Target::Value eTarget_
//		ロックする対象を表す値
//	Lock::Name::Category::Value eManipulate_
//		操作するオブジェクトのカテゴリー
//	Hold::Operation::Value eOperation_
//		読み込みのみを行おうとしているならtrue、
//		書き込みを行う可能性があるならfalse
//	Schema::Object::ID::Value iID_ = ID::Invalid
//		タプルをロックするときに指定するID
//	Trans::Log::File::Category::Value eLogCategory_
// 		= Trans::Log::File::Category::System
//		論理ログをロックするときに指定するログファイルの種別
//	Schema::Database* pDatabase_ = 0
//		データベースのログファイルが対象のときに渡すデータベースオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Manager::SystemTable::
release(Trans::Transaction& cTrans_,
		Hold::Target::Value eTarget_,
		Lock::Name::Category::Value eManipulate_,
		Hold::Operation::Value eOperation_,
		Object::ID::Value iID_,
		Trans::Log::File::Category::Value eLogCategory_,
		Database* pDatabase_)
{
	Hold::release(cTrans_,
				  _Lock::getLockName(cTrans_, eTarget_, iID_, eLogCategory_, pDatabase_),
				  eTarget_, eManipulate_, eOperation_);
}

//	FUNCTION public
//	Schema::Manager::SystemTable::getLogFile --
//		システム全体用の論理ログファイルの論理ログファイル記述子を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたオート論理ログファイル記述子
//
//	EXCEPTIONS

Trans::Log::AutoFile
Manager::SystemTable::getLogFile()
{
	Trans::Log::File::StorageStrategy storageStrategy;
	storageStrategy._mounted = true;
	storageStrategy._readOnly = false;
	storageStrategy._path = _Log::_getFilePath();
	storageStrategy._truncatable = true;
	storageStrategy._category = Trans::Log::File::Category::System;

	return Trans::Log::File::attach(
		storageStrategy, Lock::LogicalLogName(
			static_cast<Lock::Name::Part>(Object::ID::SystemTable)),
		Object::Name(NameParts::Database::System));
}

//	FUNCTION
//	Schema::Manager::SystemTable::reorganize --
//		データベースのシステム表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabase_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
//		Exception::NotBeginTransaction
//			更新トランザクションに入らずにこのメソッドを呼んだ

Manager::SystemTable::Result::Value
Manager::SystemTable::
reorganize(Trans::Transaction& cTrans_,
		   Server::Session* pSession_,
		   const Object::Name& cDatabase_,
		   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	using namespace Statement;

	if (cTrans_.getCategory() != Trans::Transaction::Category::ReadWrite) {
		_SYDNEY_THROW0(Exception::ReadOnlyTransaction);
	}

	// 念のために初期化しておく
	initialize();

	Result::Value iResult = Result::None;

	// Statementの種類に応じて再構成処理を行う
	switch (pStatement_->getType()) {

		//=======================================================================
		//	Database の操作
		//=======================================================================

	case ObjectType::DatabaseDefinition:
		{
			SydMessage << pStatement_->toSQLStatement() << ModEndl;
			iResult = ReorganizeUtility::createDatabase(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::DropDatabaseStatement:
		{
			SydMessage << pStatement_->toSQLStatement() << ModEndl;
			iResult = ReorganizeUtility::dropDatabase(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::MoveDatabaseStatement:
		{
			SydMessage << pStatement_->toSQLStatement() << ModEndl;
			iResult = ReorganizeUtility::moveDatabase(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::AlterDatabaseStatement:
		{
			SydMessage << pStatement_->toSQLStatement() << ModEndl;
			iResult = ReorganizeUtility::alterDatabase(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}

		//=======================================================================
		//	Database 以下の操作
		//=======================================================================

	case ObjectType::AreaDefinition:
		{
			iResult = ReorganizeUtility::createArea(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::DropAreaStatement:
		{
			iResult = ReorganizeUtility::dropArea(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::AlterAreaStatement:
		{
			iResult = ReorganizeUtility::alterArea(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::TableDefinition:
	case ObjectType::TemporaryTableDefinition:
		{
			iResult = ReorganizeUtility::createTable(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::DropTableStatement:
	case ObjectType::DropTemporaryTableStatement:
		{
			iResult = ReorganizeUtility::dropTable(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::AlterTableStatement:
		{
			iResult = ReorganizeUtility::alterTable(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::IndexDefinition:
	case ObjectType::TemporaryIndexDefinition:
		{
			iResult = ReorganizeUtility::createIndex(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::DropIndexStatement:
	case ObjectType::DropTemporaryIndexStatement:
		{
			iResult = ReorganizeUtility::dropIndex(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::AlterIndexStatement:
		{
			iResult = ReorganizeUtility::alterIndex(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::GrantStatement:
		{
			iResult = ReorganizeUtility::grant(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::RevokeStatement:
		{
			iResult = ReorganizeUtility::revoke(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::CascadeDefinition:
		{
			iResult = ReorganizeUtility::createCascade(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::AlterCascadeStatement:
		{
			iResult = ReorganizeUtility::alterCascade(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::DropCascadeStatement:
		{
			iResult = ReorganizeUtility::dropCascade(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::PartitionDefinition:
		{
			iResult = ReorganizeUtility::createPartition(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::AlterPartitionStatement:
		{
			iResult = ReorganizeUtility::alterPartition(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::DropPartitionStatement:
		{
			iResult = ReorganizeUtility::dropPartition(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::FunctionDefinition:
		{
			iResult = ReorganizeUtility::createFunction(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	case ObjectType::DropFunctionStatement:
		{
			iResult = ReorganizeUtility::dropFunction(cTrans_, pSession_, cDatabase_, pStatement_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	} // end of switch ( pStatement_->getType() )

	return iResult;
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Database
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabase_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
createDatabase(Trans::Transaction& cTrans_,
			   Server::Session* pSession_,
			   const Object::Name& cDatabase_,
			   const Statement::Object* pStatement_)
{
	const Statement::DatabaseDefinition* pStatement
			= _SYDNEY_DYNAMIC_CAST(const Statement::DatabaseDefinition*,
								   pStatement_);
	; _SYDNEY_ASSERT(pStatement_);

	// メタデータベースとデータベース表を
	// データベース表を操作するためにロックする

	hold(cTrans_, Hold::Target::MetaDatabase,
		 Lock::Name::Category::Tuple, Hold::Operation::ReadWrite);
	hold(cTrans_, Hold::Target::MetaTable,
		 Lock::Name::Category::Tuple, Hold::Operation::ReadWrite);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Database* pSystemDatabase = SystemDatabase::getInstance(cTrans_);
	Operation::isApplicable(cTrans_, *pSystemDatabase, pStatement_, pSession_);

	// execute create database
	return ReorganizeDatabase::Create(cTrans_, pStatement, pSession_).execute();
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Database
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabase_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
dropDatabase(Trans::Transaction& cTrans_,
			 Server::Session* pSession_,
			 const Object::Name& cDatabase_,
			 const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::DropDatabaseStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::DropDatabaseStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// SQL 文から破棄するデータベースの名前を得る

	const Object::Name& dbName = Database::getName(*pStatement);

	//【注意】	版管理トランザクションとはロックで排他されるので、
	//			データベースIDのチェックは行わない

	// メタデータベース、データベース表と
	// 破棄するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Database* pcDatabase = Database::getLocked(
		cTrans_, dbName,
		Lock::Name::Category::Tuple, Hold::Operation::Drop,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
		true /* ignore unavailable */);
	if (!pcDatabase) {

		// 指定されたデータベース名を取得してから、
		// これまでにデータベースが破棄されている

		if (pStatement->isIfExists()) {
			// With 'IF EXISTS', ignore this case
			return Result::None;
		} else {
			_SYDNEY_THROW1(Exception::DatabaseNotFound, dbName);
		}
	}

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute drop database
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizeDatabase::Drop(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Database
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabase_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
moveDatabase(Trans::Transaction& cTrans_,
			 Server::Session* pSession_,
			 const Object::Name& cDatabase_,
			 const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::MoveDatabaseStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::MoveDatabaseStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// SQL 文から移動するデータベースの名前を得る

	const Object::Name& cName = Database::getName(*pStatement);

	//【注意】	版管理トランザクションとはロックで排他されるので、
	//			データベースIDのチェックは行わない

	// メタデータベース、データベース表と
	// 移動するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Database* pcDatabase = Database::getLocked(
		cTrans_, cName,
		Lock::Name::Category::Table, Hold::Operation::MoveDatabase,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase)

		// 指定されたデータベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, cName);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute move database
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizeDatabase::Move(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Database
//		データベースの「データベース」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabase_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
alterDatabase(Trans::Transaction& cTrans_,
			  Server::Session* pSession_,
			  const Object::Name& cDatabase_,
			  const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);
	const Statement::AlterDatabaseStatement* pcAlterDatabase =
		_SYDNEY_DYNAMIC_CAST(const Statement::AlterDatabaseStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pcAlterDatabase);

	// SQL 文から変更する属性の指定を得る
	// 変更する内容によってロックモードが異なるので
	// ここでSQL文を調べてしまう

	const Boolean::Value readWrite = pcAlterDatabase->getAlterDatabaseOption(
		Statement::AlterDatabaseAttribute::ReadWrite);
	const Boolean::Value online = pcAlterDatabase->getAlterDatabaseOption(
		Statement::AlterDatabaseAttribute::Online);
	const Boolean::Value recoveryFull = pcAlterDatabase->getAlterDatabaseOption(
		Statement::AlterDatabaseAttribute::RecoveryFull);
	const Boolean::Value superUser = pcAlterDatabase->getAlterDatabaseOption(
		Statement::AlterDatabaseAttribute::SuperUserMode);
	
	Statement::AlterDatabaseStatement::ReplicationType replicationType =
		pcAlterDatabase->getReplicationType();
	
	// SQL 文から操作するデータベースの名前を得る

	const Object::Name& dbName = Database::getName(*pcAlterDatabase);

	//【注意】	版管理トランザクションとはロックで排他されるので、
	//			データベースIDのチェックは行わない

	// メタデータベース、データベース表と
	// 定義を更新するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	const Hold::Operation::Value operation =
		((readWrite == Boolean::Unknown &&
		  recoveryFull == Boolean::Unknown &&
		  superUser == Boolean::Unknown &&
		  (online == Boolean::True ||
		   replicationType == Statement::AlterDatabaseStatement::StartSlave ||
		   replicationType == Statement::AlterDatabaseStatement::StopSlave ||
		   replicationType == Statement::AlterDatabaseStatement::SetToMaster)) ?
		 Hold::Operation::ReadForWrite : Hold::Operation::Drop);

	Database* pcDatabase = Database::getLocked(
		cTrans_, dbName,
		Lock::Name::Category::Tuple, operation,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase)

		// 指定されたデータベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, dbName);

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	Manager::SystemTable::Result::Value r;

	// execute alter database
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		if((superUser == Boolean::True)&&
		   (pcDatabase->isSuperUserMode() == false)) {
			SydInfoMessage << "enter super usermode" << ModEndl;
		
			_AutoEnterSuperUserMode cEnterSuperUserMode(dbName, pcDatabase->getID());
			Server::InstanceManager::
				terminateOtherSessionsOfAllInstanceManagers(dbName, cTrans_.getSessionID());
			r = ReorganizeDatabase::Alter(cTrans_, pcDatabase, pcAlterDatabase).execute();
		} else {
			r = ReorganizeDatabase::Alter(cTrans_, pcDatabase, pcAlterDatabase).execute();
		}

		if (r != Manager::SystemTable::Result::None &&
			replicationType == Statement::AlterDatabaseStatement::SetToMaster)
		{
			// スレーブからマスターデータベースに変わったので、
			// セッションに設定する

			Server::Session::setDatabaseInfo(pcDatabase->getName(),
											 pcDatabase->getID(), false);
		}
	}

	return r;
}


//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Area
//		データベースの「エリア」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabase_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
createArea(Trans::Transaction& cTrans_,
		   Server::Session* pSession_,
		   const Object::Name& cDatabaseName_,
		   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::AreaDefinition* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::AreaDefinition*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// メタデータベース、データベース表と
	// 生成するエリアが存在するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Database* pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
		Lock::Name::Category::Tuple, Hold::Operation::ReadWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizeArea::Create(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Area
//		データベースの「エリア」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
dropArea(Trans::Transaction& cTrans_,
		 Server::Session* pSession_,
		 const Object::Name& cDatabaseName_,
		 const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::DropAreaStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::DropAreaStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// メタデータベース、データベース表と
	// 破棄するエリアが存在するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Database* pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase)

		// 指定されたエリアが存在するデータベースの
		// データベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizeArea::Drop(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Area
//		データベースの「エリア」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
alterArea(Trans::Transaction& cTrans_,
		  Server::Session* pSession_,
		  const Object::Name& cDatabaseName_,
		  const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::AlterAreaStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::AlterAreaStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// メタデータベース、データベース表と
	// 定義を更新するエリアが存在するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Database* pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Table, Hold::Operation::MoveDatabase,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase)

		// 指定されたエリアが存在するデータベースの
		// データベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizeArea::Alter(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
createTable(Trans::Transaction& cTrans_,
			Server::Session* pSession_,
			const Object::Name& cDatabaseName_,
			const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::TableDefinition* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::TableDefinition*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);
	; _SYDNEY_ASSERT(pStatement->getName());

	// 作成するのが一時表か否かを得る
	bool isTemporary = Table::isToBeTemporary(pStatement->getName());

    Database* pcDatabase = 0;
	if (isTemporary) {
		// 一時表は一時データベースに属する表である
		// 一時表に対してはロックや論理ログ出力は不要である
		pcDatabase = Database::createTemporary(cTrans_);

		if (Server::Manager::getUserList() != 0) {
			// 権限の検査のためにデータベースを取得する
			Schema::Database* pSessionDatabase =
				Schema::Database::getLocked(cTrans_, cDatabaseName_,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport);
			if (!pSessionDatabase) {
				_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
			}
			(void) Operation::isApplicable(cTrans_, *pSessionDatabase, pStatement_, pSession_);
		}
	} else {

		// メタデータベース、データベース表と
		// 生成する表が存在するデータベースの情報を格納する
		// データベース表のタプルをロックしてから、
		// データベースを表すスキーマ情報を取得する

		pcDatabase = Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
		if (!pcDatabase) {
			_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
		}
		
		if (pcDatabase->getID() != pSession_->getDatabaseID())
		{
			// データベースの実体が変更されている
		
			pSession_->setDatabaseInfo(pcDatabase->getID(),
									   pcDatabase->isSlave());
			_SYDNEY_THROW0(Exception::DatabaseChanged);
		}

		// データベースの検査
		(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);
	}

	if (!isTemporary && pcDatabase->hasCascade(cTrans_)) {
		return DSchema::Manager::SystemTable::ReorganizeTable::Create(cTrans_, pcDatabase, pStatement, isTemporary).execute();
	} else {
		return ReorganizeTable::Create(cTrans_, pcDatabase, pStatement, isTemporary).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
dropTable(Trans::Transaction& cTrans_,
		  Server::Session* pSession_,
		  const Object::Name& cDatabaseName_,
		  const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::DropTableStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::DropTableStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// 一時表をdropしようとしているか否かを得る
	bool isTemporary = Table::isToBeTemporary(pStatement->getName());

	Database* pcDatabase;
	if (isTemporary) {

		//【注意】	一時表を操作するときは、
		//			ロックや論理ログ出力は必要ない

		pcDatabase = Database::createTemporary(cTrans_);

		if (Server::Manager::getUserList() != 0) {
			// 権限の検査のためにデータベースを取得する
			Schema::Database* pSessionDatabase =
				Schema::Database::getLocked(cTrans_, cDatabaseName_,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport);
			if (!pSessionDatabase) {
				_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
			}
			(void) Operation::isApplicable(cTrans_, *pSessionDatabase, pStatement_, pSession_);
		}

	} else {

		// メタデータベース、データベース表と
		// 破棄する表が存在するデータベースの情報を格納する
		// データベース表のタプルをロックしてから、
		// データベースを表すスキーマ情報を取得する

		pcDatabase = Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::Drop,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
		if (!pcDatabase)

			// 指定された表が存在するデータベースの
			// データベース名を取得してから、
			// これまでにデータベースが破棄されている

			_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

		if (pcDatabase->getID() != pSession_->getDatabaseID())
		{
			// データベースの実体が変更されている
		
			pSession_->setDatabaseInfo(pcDatabase->getID(),
									   pcDatabase->isSlave());
			_SYDNEY_THROW0(Exception::DatabaseChanged);
		}

		// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
		// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

		Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);
	}

	// execute
	if (!isTemporary && pcDatabase->hasCascade(cTrans_)) {
		return DSchema::Manager::SystemTable::ReorganizeTable::Drop(cTrans_, pcDatabase, pStatement, isTemporary).execute();
	} else {
		return ReorganizeTable::Drop(cTrans_, pcDatabase, pStatement, isTemporary).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Table
//		データベースの「テーブル」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
alterTable(Trans::Transaction& cTrans_,
		   Server::Session* pSession_,
		   const Object::Name& cDatabaseName_,
		   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::AlterTableStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::AlterTableStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	if (Table::isToBeTemporary(pStatement->getTableName())) {
		// alterを一時表に対して呼んだので例外送出
		_SYDNEY_THROW0(Exception::TemporaryTable);
	}

	// メタデータベース、データベース表と
	// 定義を更新する表が存在するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Database* pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Tuple, Hold::Operation::Drop,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase)

		// 指定された表が存在するデータベースの
		// データベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizeTable::Alter(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Index
//		データベースの「インデックス」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
createIndex(Trans::Transaction& cTrans_,
			Server::Session* pSession_,
			const Object::Name& cDatabaseName_,
			const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::IndexDefinition* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::IndexDefinition*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);
	; _SYDNEY_ASSERT(pStatement->getName());

	// 作成するのが一時表の索引か否かを得る
	bool isTemporary = Table::isToBeTemporary(pStatement->getName());

    Database* pcDatabase = 0;
	if (isTemporary) {
		// 一時表は一時データベースに属する表である
		// 一時表に対してはロックや論理ログ出力は不要である
		pcDatabase = Database::createTemporary(cTrans_);

		if (Server::Manager::getUserList() != 0) {
			// 権限の検査のためにデータベースを取得する
			Schema::Database* pSessionDatabase =
				Schema::Database::getLocked(cTrans_, cDatabaseName_,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport);
			if (!pSessionDatabase) {
				_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
			}
			(void) Operation::isApplicable(cTrans_, *pSessionDatabase, pStatement_, pSession_);
		}
	} else {

		// メタデータベース、データベース表と
		// 生成する索引が存在するデータベースの情報を格納する
		// データベース表のタプルをロックしてから、
		// データベースを表すスキーマ情報を取得する

		pcDatabase = Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
		if (!pcDatabase) {
			_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
		}

		if (pcDatabase->getID() != pSession_->getDatabaseID())
		{
			// データベースの実体が変更されている
		
			pSession_->setDatabaseInfo(pcDatabase->getID(),
									   pcDatabase->isSlave());
			_SYDNEY_THROW0(Exception::DatabaseChanged);
		}

		// データベースの検査
		(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);
	}

	if (!isTemporary && pcDatabase->hasCascade(cTrans_)) {
		return DSchema::Manager::SystemTable::ReorganizeIndex::Create(cTrans_, pcDatabase, pStatement).execute();
	} else {
		return ReorganizeIndex::Create(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Index
//		データベースの「インデックス」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
dropIndex(Trans::Transaction& cTrans_,
		  Server::Session* pSession_,
		  const Object::Name& cDatabaseName_,
		  const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::DropIndexStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::DropIndexStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// 一時表をdropしようとしているか否かを得る
	bool isTemporary = Table::isToBeTemporary(pStatement->getIndexName());

	Database* pcDatabase = 0;
	if (isTemporary) {

		//【注意】	一時表を操作するときは、
		//			ロックや論理ログ出力は必要ない

		pcDatabase = Database::createTemporary(cTrans_);

		if (Server::Manager::getUserList() != 0) {
			// 権限の検査のためにデータベースを取得する
			Schema::Database* pSessionDatabase =
				Schema::Database::getLocked(cTrans_, cDatabaseName_,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport,
											Lock::Name::Category::Tuple,
											Schema::Hold::Operation::ReadForImport);
			if (!pSessionDatabase) {
				_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
			}
			(void) Operation::isApplicable(cTrans_, *pSessionDatabase, pStatement_, pSession_);
		}
	} else {

		// メタデータベース、データベース表と
		// 破棄する索引が存在するデータベースの情報を格納する
		// データベース表のタプルをロックしてから、
		// データベースを表すスキーマ情報を取得する

		pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Tuple, Hold::Operation::Drop,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
		if (!pcDatabase)

			// 指定された索引が存在するデータベースの
			// データベース名を取得してから、
			// これまでにデータベースが破棄されている

			_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

		if (pcDatabase->getID() != pSession_->getDatabaseID())
		{
			// データベースの実体が変更されている
		
			pSession_->setDatabaseInfo(pcDatabase->getID(),
									   pcDatabase->isSlave());
			_SYDNEY_THROW0(Exception::DatabaseChanged);
		}

		// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
		// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

		Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);
	}

	// execute
	if (!isTemporary && pcDatabase->hasCascade(cTrans_)) {
		return DSchema::Manager::SystemTable::ReorganizeIndex::Drop(cTrans_, pcDatabase, pStatement).execute();
	} else {
		return ReorganizeIndex::Drop(cTrans_, pcDatabase, pStatement).execute();
	}
}

//	FUNCTION
//	Schema::Manager::SystemTable::ReorganizeUtility::Index
//		データベースの「インデックス」表の再構成を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			再構成を行うトランザクション記述子
//		Server::Session* pSession_
//			セッション記述子
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Statement::Object* pStatement_
//			Statement オブジェクト
//
//	RETURN
//		Result::Value
//
//	EXCEPTIONS
Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
alterIndex(Trans::Transaction& cTrans_,
		   Server::Session* pSession_,
		   const Object::Name& cDatabaseName_,
		   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::AlterIndexStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::AlterIndexStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	if (Table::isToBeTemporary(pStatement->getIndexName())) {
		// alterを一時表に対して呼んだので例外送出
		_SYDNEY_THROW0(Exception::TemporaryTable);
	}

	// メタデータベース、データベース表と
	// 定義を更新する索引が存在するデータベースの情報を格納する
	// データベース表のタプルをロックしてから、
	// データベースを表すスキーマ情報を取得する

	Database* pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Tuple, Hold::Operation::Drop,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase)

		// 指定された索引が存在するデータベースの
		// データベース名を取得してから、
		// これまでにデータベースが破棄されている

		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// 操作を実行するトランザクションにおいて実行可能な SQL 文か調べる
	// また、操作対象であるデータベース対して実行可能な SQL 文か調べる

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizeIndex::Alter(cTrans_, pcDatabase, pStatement).execute();
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::grant -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
grant(Trans::Transaction& cTrans_,
	  Server::Session* pSession_,
	  const Schema::Object::Name& cDatabaseName_,
	  const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::GrantStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::GrantStatement*, pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// Get Database object with locking by totaly exclusive with other operations

	Database* pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Tuple, Hold::Operation::Drop,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);

	if (!pcDatabase)
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// Check whether this statement can be processed by this session/transaction
	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizePrivilege::Alter(cTrans_, pcDatabase, pStatement).execute();
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::revoke -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
revoke(Trans::Transaction& cTrans_,
	   Server::Session* pSession_,
	   const Schema::Object::Name& cDatabaseName_,
	   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::RevokeStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::RevokeStatement*, pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	// Get Database object with locking by totaly exclusive with other operations

	Database* pcDatabase = Database::getLocked(
		cTrans_, cDatabaseName_,
		Lock::Name::Category::Tuple, Hold::Operation::Drop,
		Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);

	if (!pcDatabase)
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// Check whether this statement can be processed by this session/transaction
	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	if (pcDatabase->hasCascade(cTrans_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	} else {
		return ReorganizePrivilege::Alter(cTrans_, pcDatabase, pStatement).execute();
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::createCascade -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
createCascade(Trans::Transaction& cTrans_,
			  Server::Session* pSession_,
			  const Schema::Object::Name& cDatabaseName_,
			  const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::CascadeDefinition* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::CascadeDefinition*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);
	; _SYDNEY_ASSERT(pStatement->getCascadeName());

    Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	
	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	return ReorganizeCascade::Create(cTrans_, pcDatabase, pStatement).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::dropCascade -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
dropCascade(Trans::Transaction& cTrans_,
			Server::Session* pSession_,
			const Schema::Object::Name& cDatabaseName_,
			const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::DropCascadeStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::DropCascadeStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

    Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	
	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	return ReorganizeCascade::Drop(cTrans_, pcDatabase, pStatement).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::alterCascade -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
alterCascade(Trans::Transaction& cTrans_,
			 Server::Session* pSession_,
			 const Schema::Object::Name& cDatabaseName_,
			 const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::AlterCascadeStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::AlterCascadeStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

    Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::MoveDatabase,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	
	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	return ReorganizeCascade::Alter(cTrans_, pcDatabase, pStatement).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::createPartition -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
createPartition(Trans::Transaction& cTrans_,
				Server::Session* pSession_,
				const Schema::Object::Name& cDatabaseName_,
				const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::PartitionDefinition* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::PartitionDefinition*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);
	; _SYDNEY_ASSERT(pStatement->getTableName());

    Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	
	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	return ReorganizePartition::Create(cTrans_, pcDatabase, pStatement).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::alterPartition -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
alterPartition(Trans::Transaction& cTrans_,
			   Server::Session* pSession_,
			   const Schema::Object::Name& cDatabaseName_,
			   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::AlterPartitionStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::AlterPartitionStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

    Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::MoveDatabase,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	
	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	return ReorganizePartition::Alter(cTrans_, pcDatabase, pStatement).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::dropPartition -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
dropPartition(Trans::Transaction& cTrans_,
			   Server::Session* pSession_,
			   const Schema::Object::Name& cDatabaseName_,
			   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::DropPartitionStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::DropPartitionStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

    Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::Drop,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}
	
	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	return ReorganizePartition::Drop(cTrans_, pcDatabase, pStatement).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::createFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
createFunction(Trans::Transaction& cTrans_,
			   Server::Session* pSession_,
			   const Schema::Object::Name& cDatabaseName_,
			   const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::FunctionDefinition* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::FunctionDefinition*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);
	; _SYDNEY_ASSERT(pStatement->getFunctionName());

    Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	// データベースの検査
	(void) Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	return ReorganizeFunction::Create(cTrans_, pcDatabase, pStatement).execute();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeUtility::dropFunction -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Server::Session* pSession_
//	const Schema::Object::Name& cDatabaseName_
//	const Statement::Object* pStatement_
//	
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeUtility::
dropFunction(Trans::Transaction& cTrans_,
			 Server::Session* pSession_,
			 const Schema::Object::Name& cDatabaseName_,
			 const Statement::Object* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);

	const Statement::DropFunctionStatement* pStatement =
		_SYDNEY_DYNAMIC_CAST(const Statement::DropFunctionStatement*,
							 pStatement_);
	; _SYDNEY_ASSERT(pStatement);

	Database* pcDatabase =
		Database::getLocked(
			cTrans_, cDatabaseName_,
			Lock::Name::Category::Tuple, Hold::Operation::Drop,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	if (!pcDatabase) {
		_SYDNEY_THROW1(Exception::DatabaseNotFound, cDatabaseName_);
	}

	if (pcDatabase->getID() != pSession_->getDatabaseID())
	{
		// データベースの実体が変更されている
		
		pSession_->setDatabaseInfo(pcDatabase->getID(),
								   pcDatabase->isSlave());
		_SYDNEY_THROW0(Exception::DatabaseChanged);
	}

	Operation::isApplicable(cTrans_, *pcDatabase, pStatement_, pSession_);

	// execute
	return ReorganizeFunction::Drop(cTrans_, pcDatabase, pStatement).execute();
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
