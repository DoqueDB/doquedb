// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Database.h -- データベースオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_DATABASE_H
#define	__SYDNEY_SCHEMA_DATABASE_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Object.h"

#include "Admin/Verification.h"
#include "Common/Hasher.h"
#include "Common/Privilege.h"
#include "Lock/Mode.h"
#include "Lock/Name.h"
#include "Os/Path.h"
#include "Os/CriticalSection.h"
#include "Server/SessionID.h"
#include "Trans/AutoLogFile.h"
#include "Trans/TimeStamp.h"

#include "ModHasher.h"
#include "ModHashMap.h"
#include "ModUnicodeString.h"
#include "ModVector.h"
#include "ModMap.h"


_SYDNEY_BEGIN

namespace Common
{
	class StringArrayData;
}
namespace Statement
{
	class AlterDatabaseStatement;
	class DatabaseCreateOptionList;
	class DatabaseDefinition;
	class DatabasePathElementList;
	class DropDatabaseStatement;
	class MoveDatabaseStatement;
}
namespace Trans
{
	class Transaction;
	namespace Log
	{
		class File;
	}
}

_SYDNEY_SCHEMA_BEGIN

class Area;
class AreaMap;
class Table;
class TableMap;
class Privilege;
class PrivilegeMap;
class Cascade;
class CascadeMap;
class Column;
class Constraint;
class Index;
class Key;
class File;
class Field;
class Function;
class FunctionMap;
class Partition;
class PartitionMap;
class Sequence;
class LogData;
class ObjectSnapshot;
namespace SystemTable
{
	class Cascade;
	class Column;
	class Constraint;
	class Database;
	class Field;
	class File;
	class Function;
	class Index;
	class Key;
	class Partition;
	class Table;
}
namespace Utility
{
	class BinaryData;
}

//	CLASS
//	Schema::Database -- データベースオブジェクトを表すクラス
//
//	NOTES

class Database
	: public	Object
{
public:
	friend class SystemTable::Area;
	friend class SystemTable::Cascade;
	friend class SystemTable::Column;
	friend class SystemTable::Constraint;
	friend class SystemTable::Database;
	friend class SystemTable::Field;
	friend class SystemTable::File;
	friend class SystemTable::Function;
	friend class SystemTable::Index;
	friend class SystemTable::Key;
	friend class SystemTable::Partition;
	friend class SystemTable::Privilege;
	friend class SystemTable::Table;
	friend class ObjectSnapshot;

	using Object::getID;

	//	TYPEDEF public
	//	Schema::Database::CacheMap --
	//
	//	NOTES

	typedef ModHashMap<ID::Value, Object::Pointer, ModHasher<ID::Value> >	CacheMap;

	//	TYPEDEF public
	//	Schema::Database::SuperUserModeNameMap --
	//
	//	NOTES

	typedef ModMap< Object::Name, ModThreadId, ModLess<Object::Name> >	SuperUserModeNameMap;

	
	//	TYPEDEF public
	//	Schema::Database::SuperUserModeNameMap --
	//
	//	NOTES	
	typedef ModMap< ID::Value, ModThreadId, ModLess<ID::Value> >	SuperUserModeIdMap;

	//	TYPEDEF public
	//	Schema::Database::Pointer -- Databaseを保持するObjectPointer
	//
	//	NOTES

	typedef DatabasePointer Pointer;

	struct Path {

		//	ENUM public
		//	Schema::Database::Path::Category::Value -- データベースに設定されるパス名の種類
		//
		//	NOTES

		struct Category {
			enum Value {
				Data = 0,						// 無指定ならシステムパラメーター
				LogicalLog,						// 無指定ならDataの値
				System,							// 無指定ならDataの値
				ValueNum,
			};
		};
	};

	struct Log {
		enum Value {
			Name = 0,
			ID,
			ValueNum
		};
		struct Create {
			enum Value {
				Path = ValueNum,
				Flag,
				Num
			};
		};
		struct Drop {
			enum Value {
				Path = ValueNum,
				AreaPath,
				Num
			};
		};
		struct Unmount {
			enum Value {
				Path = ValueNum,
				MostRecent,
				Num
			};
		};
		struct Move {
			enum Value {
				PrevPath = ValueNum,
				PostPath,
				Num
			};
		};
		struct Alter {
			enum Value {
				MostRecent = ValueNum,
				Flag,
				Num,
				MasterURL = Num,
				Num2,
			};
		};
	};

	//	STRUCT 
	//	Database::Attribute -- データベースの属性を保持する型
	//
	//	NOTES

	struct Attribute
	{
		bool m_bOnline;					// オンラインか
		bool m_bReadOnly;				// 読み取り専用か
		bool m_bUnmounted;				// アンマウントされたか
		bool m_bRecoveryFull;			// 障害回復指定が完全か
		bool m_bSuperUserMode;			// スーパーユーザーモードか

		ModUnicodeString m_cstrMasterURL; // MASTERのURL
		bool m_bSlaveStarted;			  // SLAVEとして開始しているか

		// Attributeのデフォルト値は
		// create databaseで無指定時に採用されるもの
		Attribute()
			: m_bOnline(true), m_bReadOnly(false), m_bUnmounted(false),
			  m_bRecoveryFull(false), m_bSuperUserMode(false),
			  m_cstrMasterURL(), m_bSlaveStarted(false)
		{}

		// 属性を数値化する
		unsigned int getFlag() const
		{
			return static_cast<unsigned int>(m_bReadOnly)
				| (static_cast<unsigned int>(m_bOnline << 1))
				| (static_cast<unsigned int>(m_bUnmounted << 2))
				| (static_cast<unsigned int>(m_bRecoveryFull << 3))
				| (static_cast<unsigned int>(m_bSuperUserMode << 4))
				| (static_cast<unsigned int>(m_bSlaveStarted << 5));
		}
		// 数値から属性をセットする
		void setFlag(unsigned int iFlag_)
		{
			m_bReadOnly = (iFlag_ & 1);
			m_bOnline = (iFlag_ & (1 << 1));
			m_bUnmounted = (iFlag_ & (1 << 2));
			m_bRecoveryFull = (iFlag_ & (1 << 3));
			m_bSuperUserMode = (iFlag_ & (1 << 4));
			m_bSlaveStarted = (iFlag_ & (1 << 5));
		}

		void clear()
		{
			m_bOnline = true;
			m_bReadOnly = false;
			m_bUnmounted = false;
			m_bRecoveryFull = false;
			m_bSuperUserMode = false;
			m_bSlaveStarted = false;
			m_cstrMasterURL.clear();
		}
	};

	// コンストラクター
	Database();
	// デストラクター
	SYD_SCHEMA_FUNCTION
	virtual ~Database();

	// DataArrayDataを元にインスタンスを生成する
	static Database*		getNewInstance(const Common::DataArrayData& cData_);

	// メンバーをすべて初期化する
	void					clear();
	// 使用しているファイルをdetachする
	void					detachFiles();

	// -------------------------------------
	// おもに再構成に使用されるメソッド
	// -------------------------------------

	// データベースを生成する
	static Database*		create(const Statement::DatabaseDefinition& stmt,
								   LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	static Database*		create(const Name& cName_,
								   Trans::Transaction& cTrans_);
	static Database*		create(const LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	void					create(Trans::Transaction& cTrans_,
								   bool bAllowExistence_ = false);
	SYD_SCHEMA_FUNCTION
	static Database*		createForMount(
								   const Name& cName_,
								   const Statement::DatabaseCreateOptionList& stmt_,
								   Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Database*		createForMount(const LogData& cLogData_,
										   Trans::Transaction& cTrans_);

	// SQL文からデータベース名を得る
	static Name				getName(const Statement::DropDatabaseStatement& cStatement_);
	static Name				getName(const Statement::MoveDatabaseStatement& cStatement_);
	static Name				getName(const Statement::AlterDatabaseStatement& cStatement_);

	// データベースを抹消する
	static void				drop(Database& cDatabase_, LogData& cLogData_,
								 Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	void					drop(Trans::Transaction& cTrans_, bool bRecovery_ = false);

	// dropマークをクリアする
	void					undoDrop(Trans::Transaction& cTrans_);

	// データベースを構成するすべてのファイルとそれを格納するディレクトリーを破棄する
	void					destroy(Trans::Transaction& cTrans_,
									bool bForce_ = true);
	// データベースを構成するファイルのうち指定したパス以下のディレクトリーを破棄する
	void					destroy(Trans::Transaction& cTrans_,
									Path::Category::Value eCategory_,
									const ModUnicodeString& cPathPart_,
									bool bForce_ = true);
	// 指定したパス以下のディレクトリーの破棄を取り消す
	void					undoDestroy(Trans::Transaction& cTrans_,
										Path::Category::Value eCategory_,
										const ModUnicodeString& cPathPart_);

	// システム表を構成するファイルのみmount する
	SYD_SCHEMA_FUNCTION
	void					mountSystemTable(Trans::Transaction& cTrans_, bool bUndo_ = false);
	// mount する
	SYD_SCHEMA_FUNCTION
	void					mount(Trans::Transaction& cTrans_, bool bUndo_ = false);
	// システム表を構成するファイルのみunmount する
	SYD_SCHEMA_FUNCTION
	void					unmountSystemTable(Trans::Transaction& cTrans_, bool bUndo_ = false);
	// unmount する
	SYD_SCHEMA_FUNCTION
	void					unmount(Trans::Transaction& cTrans_, bool bUndo_ = false);
	// flush する
	SYD_SCHEMA_FUNCTION
	void					flush(Trans::Transaction& cTrans_);
	// 不要な版を破棄する	
	SYD_SCHEMA_FUNCTION
	void					sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
	// バックアップを開始する
	SYD_SCHEMA_FUNCTION
	void					startBackup(Trans::Transaction& cTrans_,
										bool bRestorable_ = true,
										bool bUndo_ = false);
	// バックアップを終了する
	SYD_SCHEMA_FUNCTION
	void					endBackup(Trans::Transaction& cTrans_);

	//	データベースを障害から回復する
	SYD_SCHEMA_FUNCTION
	void					recoverSystemTable(Trans::Transaction& cTransaction_,
											   const Trans::TimeStamp& cPoint_);
	SYD_SCHEMA_FUNCTION
	void 					recover(Trans::Transaction& cTransaction_,
						 			const Trans::TimeStamp& cPoint_,
									bool bMount_ = false);
	// ある時点に開始された読取専用トランザクションが参照する版を最新版とする
	SYD_SCHEMA_FUNCTION
	void					restoreSystemTable(Trans::Transaction& cTransaction_,
											   const Trans::TimeStamp& cPoint_);
	SYD_SCHEMA_FUNCTION
	void					restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);

	// データベースのパス指定変更の準備をする
	static bool				alter(Database& cDatabase_,
								  const Statement::MoveDatabaseStatement& st,
								  ModVector<ModUnicodeString>& vecPrevPath_,
								  ModVector<ModUnicodeString>& vecPostPath_,
								  ModVector<bool>& vecChanged_,
								  LogData& cLogData_,
								  Trans::Transaction& cTrans_);
	// データベースの属性変更の準備をする
	static bool				alter(Database& cDatabase_,
								  const Statement::AlterDatabaseStatement& cStatement_,
								  Attribute& cPrevAttribute_,
								  Attribute& cPostAttribute_,
								  LogData& cLogData_,
								  Trans::Transaction& cTrans_);
	// ログデータからデータベースの属性変更の準備をする
	static bool				alter(const LogData& cLogData_,
								  Attribute& cPrevAttribute_,
								  Attribute& cPostAttribute_,
								  Trans::Transaction& cTrans_);

	// データベースを構成するファイルを移動する
	void					move(Trans::Transaction& cTrans_,
								 ModVector<ModUnicodeString>& vecPrevPath_,
								 ModVector<ModUnicodeString>& vecPostPath_,
								 bool bUndo_ = false, bool bRecovery_ = false);
	// データベースを構成するファイルの移動を取り消す
	void					undoMove(Trans::Transaction& cTrans_,
									 const ModVector<ModUnicodeString>& vecPrevPath_,
									 const ModVector<ModUnicodeString>& vecPostPath_);

	// -------------------------------------
	// おもに通常の操作に使用されるメソッド
	// -------------------------------------

	// データベース表のタプルに適切なロックをかけて
	// データベースを表すクラスを得る
	SYD_SCHEMA_FUNCTION
	static Database*		getLocked(Trans::Transaction& cTrans_, const Name& cName_,
									  Lock::Name::Category::Value eManipulate_ = Lock::Name::Category::Tuple,
									  Hold::Operation::Value eOperation_ = Hold::Operation::ReadWrite,
									  bool bForce_ = false,
									  Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);
	SYD_SCHEMA_FUNCTION
	static Database*		getLocked(Trans::Transaction& cTrans_, ID::Value iID_,
									  Lock::Name::Category::Value eManipulate_ = Lock::Name::Category::Tuple,
									  Hold::Operation::Value eOperation_ = Hold::Operation::ReadWrite,
									  bool bForce_ = false,
									  Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);

	// データベース表からその中のタプルまで適切なロックをかけて
	// データベースを表すクラスを得る
	SYD_SCHEMA_FUNCTION
	static Database*		getLocked(Trans::Transaction& trans, const Name& name,
									  Lock::Name::Category::Value manipulateDBandTable,
									  Hold::Operation::Value operationDBandTable,
									  Lock::Name::Category::Value manipulateTuple,
									  Hold::Operation::Value operationTuple,
									  bool bForce_ = false,
									  Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);
	SYD_SCHEMA_FUNCTION
	static Database*		getLocked(Trans::Transaction& trans, ID::Value iID_,
									  Lock::Name::Category::Value manipulateDBandTable,
									  Hold::Operation::Value operationDBandTable,
									  Lock::Name::Category::Value manipulateTuple,
									  Hold::Operation::Value operationTuple,
									  bool bForce_ = false,
									  Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);
	SYD_SCHEMA_FUNCTION
	static Database*		getLocked(Trans::Transaction& trans, const Name& name,
									  Lock::Name::Category::Value manipulateDB,
									  Hold::Operation::Value operationDB,
									  Lock::Name::Category::Value manipulateTable,
									  Hold::Operation::Value operationTable,
									  Lock::Name::Category::Value manipulateTuple,
									  Hold::Operation::Value operationTuple,
									  bool bForce_ = false,
									  Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);
	SYD_SCHEMA_FUNCTION
	static Database*		getLocked(Trans::Transaction& trans, ID::Value iID_,
									  Lock::Name::Category::Value manipulateDB,
									  Hold::Operation::Value operationDB,
									  Lock::Name::Category::Value manipulateTable,
									  Hold::Operation::Value operationTable,
									  Lock::Name::Category::Value manipulateTuple,
									  Hold::Operation::Value operationTuple,
									  bool bForce_ = false,
									  Lock::Timeout::Value iTimeout_ = Lock::Timeout::Unlimited);

	// データベースを表すクラスを得る
	SYD_SCHEMA_FUNCTION
	static Database*		get(ID::Value id, Trans::Transaction& cTrans_,
								bool bForce_ = false);
	// 指定された名前からデータベースのスキーマオブジェクトIDを得る
	SYD_SCHEMA_FUNCTION
	static ID::Value		getID(const Name& name, Trans::Transaction& cTrans_,
								  bool bForce_ = false);

	// 一時データベースを表すクラスを得る
	static Database*		getTemporary(Trans::Transaction& cTrans_);
	// 一時データベースを生成する
	static Database*		createTemporary(Trans::Transaction& cTrans_);
	// 一時データベースを破棄する
	static void				dropTemporary(Server::SessionID sessionID);

	// スーパーユーザーモード移行中状態に入る
	static void				enterSuperUserModeTransitionalState(const Object::Name& cName_, const ID::Value iID_);

	// スーパーユーザーモード移行中状態からです
	static void				exitSuperUserModeTransitionalState(const Object::Name& cName_, const ID::Value iID_);

	// スーパーユーザーモード移行中かを確認する
	static bool				isSuperUserMode(const Object::Name& cName_);

	// スーパーユーザーモード移行中における実行権限を判断する
	static	void			checkSuperUserModeState(const ID::Value iID_);


	// データベース用の論理ログファイルの論理ログファイル記述子を得る
	SYD_SCHEMA_FUNCTION
	Trans::Log::AutoFile	getLogFile(bool mounted = false) const;

	// get privilege value for a user
	void getPrivilege(Trans::Transaction& cTrans_,
					  int iUserID_,
					  Common::Privilege::Value* pResult_,
					  int iMaxElement_) const;
	// get privilege object for a user
	Privilege* getPrivilege(Trans::Transaction& cTrans_, int iUserID_) const;
	// get privilege object by ID
	Privilege* getPrivilege(Trans::Transaction& cTrans_, ID::Value iID_) const;

	// get privilege value for a role
	void getRolePrivilege(Trans::Transaction& cTrans_,
						  const ModUnicodeString& cstrRoleName_,
						  ModVector<Common::Privilege::Value>& vecResult_);

	static const ModUnicodeString&
						getBuiltInRoleName(Common::Privilege::Category::Value eCategory_);

	// データベースの使用を開始する
	SYD_SCHEMA_FUNCTION
	static void				reserve(Server::SessionID iSessionID_);
	// データベースの使用を終了する
	SYD_SCHEMA_FUNCTION
	static void				release(Server::SessionID iSessionID_);

#ifdef OBSOLETE // データベースオブジェクトを外部がキャッシュすることはない
	// 陳腐化していないか調べる
	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, Timestamp iTime_,
									Trans::Transaction& cTrans_);
#endif

	// -------------------------------------
	// おもにモジュール内部の処理に使用されるメソッド
	// -------------------------------------
	// 永続化前に行う処理
	static void				doBeforePersist(const Pointer& pDatabase_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
	// 永続化後に行う処理
	static void				doAfterPersist(const Pointer& pDatabase_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pDatabase_,
										ObjectSnapshot& cSnapshot_,
										Trans::Transaction& cTrans_);

//	Object::
//	ID::Value				getID() const;		// IDを得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

	// 読み書き属性を得る
	SYD_SCHEMA_FUNCTION
	bool					isReadOnly() const;
	// 読み書き属性を設定する
	void					setReadOnly(bool bReadOnly_);
	// オンライン属性を得る
	SYD_SCHEMA_FUNCTION
	bool					isOnline() const;
	// オンライン属性を設定する
	void					setOnline(bool bOnline_);

	// スーパーユーザーモード属性を得る
	SYD_SCHEMA_FUNCTION
	bool					isSuperUserMode() const;
	
	// スーパーユーザーモード属性を設定する
	void					setSuperUserMode(bool bSupserUserMode_);
	
	// 障害回復指定が完全かを得る
	SYD_SCHEMA_FUNCTION
	bool					isRecoveryFull() const;
	// 障害回復指定が完全かを設定する
	void					setRecoveryFull(bool bRecoveryFull_);

	// SLAVEが開始中かを得る
	SYD_SCHEMA_FUNCTION
	bool					isSlaveStarted() const;
	// SLAVEが開始中かを設定する
	void					setSlaveStarted(bool bSlaveStarted_);

	// SLAVEデータベースか否かを得る
	SYD_SCHEMA_FUNCTION
	bool					isSlave() const;
	// MASTER URLを得る
	SYD_SCHEMA_FUNCTION
	const ModUnicodeString&	getMasterURL() const;
	// MASTER URLを設定する
	void					setMasterURL(const ModUnicodeString& cstrMasterURL_);

	// 属性変更を行う
	void					setAttribute(const Attribute& cAttribute_);
	// 属性変更に伴ってファイルに関する処理を行う
	void					propagateAttribute(Trans::Transaction& cTrans_, const Attribute& cAttribute_);

	// データベースが保持するオブジェクトに関する操作

	// 下位オブジェクトを抹消する
	void					reset();

	// ファイルを格納するパス名を得る
	SYD_SCHEMA_FUNCTION
	void					getPath(ModVector<ModUnicodeString>& vecPath_) const;
	SYD_SCHEMA_FUNCTION
	Os::Path				getPath(Path::Category::Value eCategory_) const;
	SYD_SCHEMA_FUNCTION
	static Os::Path			getPath(Path::Category::Value eCategory_,
									const ModVector<ModUnicodeString>& cPathList_,
									const Name& name);
	static Os::Path			getDefaultPath(Path::Category::Value category,
										   const Name& name);

	// ファイルを格納するパス名を設定する
	void					setPath(const Statement::DatabasePathElementList& stmt);
	void					setPath(Path::Category::Value eCategory_,
									const ModUnicodeString& cPath_);
	void					setPath(const ModVector<ModUnicodeString>& cPathList_);

	// パス名配列を抹消する
	void					resetPath();
	// パス名配列を抹消し、管理用のベクターを破棄する
	void					clearPath();
	// ログファイルのパス名を破棄する
	void					clearLogFilePath();
	// データ格納用のパス名を破棄する
	void					clearDataPath();

	// データベースに属するオブジェクトの新しいIDを得るためのシーケンスファイルを得る
	Sequence&				getSequence();
	// シーケンスファイルを表すオブジェクトを破棄する
	void					clearSequence();
	// エリア無指定時にデータを格納するパスを得る
	const ModUnicodeString&	getDataPath();
	// データを格納するパスを得る
	static Os::Path			getDataPath(const ModVector<ModUnicodeString>& cPathList_,
										const Name& name);

	// パスがデータベース内で使用されていないか調べる
	bool					checkRelatedPath(Trans::Transaction& cTrans_,
											 const Os::Path& cPath_,
											 const Area* pOmitArea_ = 0) const;

	// データベースに属するエリアを表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Area*>		getArea(Trans::Transaction& cTrans_) const;

	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Area*					getArea(Schema::Object::ID::Value iAreaID_,
									Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Area*					getArea(const Name& cAreaName_,
									Trans::Transaction& cTrans_) const;

	// 登録の追加
	Area&					addArea(const AreaPointer& pArea_, Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseArea(ID::Value areaID);

	// データベースに登録される表を表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Table*>		getTable(Trans::Transaction& cTrans_,
									 bool bInternal_ = false) const;
	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Table*					getTable(ID::Value tableID,
									 Trans::Transaction& cTrans_,
									 bool bInternal_ = false) const;
	SYD_SCHEMA_FUNCTION
	Table*					getTable(const Name& tableName,
									 Trans::Transaction& cTrans_,
									 bool bInternal_ = false) const;
	Table*					getSystemTable(Object::Category::Value eCategory_,
										   Trans::Transaction& cTrans_) const;
	// 登録の追加
	Table&					addTable(const TablePointer& pTable_,
									 Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseTable(ID::Value iTableID_);

	// 索引の抹消
	void					eraseIndex(ID::Value iTableID_, ID::Value iIndexID_);

	// add Privilege object
	Privilege&				addPrivilege(const PrivilegePointer& pPrivilege_, Trans::Transaction& cTrans_);
	// erase Privilege object
	void					erasePrivilege(ID::Value iPrivilegeID_);

	// データベースに属する子サーバーを表すクラスに関する操作

	// 子サーバーの登録があるか
	SYD_SCHEMA_FUNCTION
	bool					hasCascade(Trans::Transaction& cTrans_) const;

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Cascade*>		getCascade(Trans::Transaction& cTrans_) const;

	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Cascade*				getCascade(Schema::Object::ID::Value iCascadeID_,
									   Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Cascade*				getCascade(const Name& cCascadeName_,
									   Trans::Transaction& cTrans_) const;

	// 登録の追加
	Cascade&				addCascade(const CascadePointer& pCascade_, Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseCascade(ID::Value cascadeID);

	// データベースに属するルールを表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Partition*>	getPartition(Trans::Transaction& cTrans_) const;

	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Partition*				getPartition(Schema::Object::ID::Value iPartitionID_,
										 Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Partition*				getPartition(const Name& cPartitionName_,
										 Trans::Transaction& cTrans_) const;

	PartitionPointer		getTablePartition(ID::Value tableID, Trans::Transaction& cTrans_) const;

	// 登録の追加
	Partition&				addPartition(const PartitionPointer& pPartition_, Trans::Transaction& cTrans_);
	// 登録の抹消
	void					erasePartition(ID::Value partitionID);

	// データベースに属する関数を表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Function*>	getFunction(Trans::Transaction& cTrans_) const;

	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Function*				getFunction(Schema::Object::ID::Value iFunctionID_,
										Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Function*				getFunction(const Name& cFunctionName_,
										Trans::Transaction& cTrans_) const;

	// 登録の追加
	Function&				addFunction(const FunctionPointer& pFunction_, Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseFunction(ID::Value functionID);

	// 表以下のオブジェクトをキャッシュするマップに関する操作

	// 指定されたオブジェクトIDのオブジェクトを得る
	Object*					getCache(ID::Value id,
									 Category::Value category = Category::Unknown);
	// 指定されたオブジェクトをキャッシュに追加する
	const Object::Pointer&	addCache(const Object::Pointer& object);
	// 指定されたオブジェクトIDのオブジェクトをキャッシュから除く
	void					eraseCache(ID::Value id);

	// このオブジェクトを親オブジェクトとするオブジェクトのロード

	// エリアを読み出す
	SYD_SCHEMA_FUNCTION
	const AreaMap&			loadArea(Trans::Transaction& cTrans_, bool bRecovery_ = false);
	// 表を読み出す
	SYD_SCHEMA_FUNCTION
	const TableMap&			loadTable(Trans::Transaction& cTrans_, bool bRecovery_ = false);
	// 子サーバーを読み出す
	SYD_SCHEMA_FUNCTION
	const CascadeMap&		loadCascade(Trans::Transaction& cTrans_, bool bRecovery_ = false);
	// ルールを読み出す
	SYD_SCHEMA_FUNCTION
	const PartitionMap&		loadPartition(Trans::Transaction& cTrans_, bool bRecovery_ = false);
	// 関数を読み出す
	SYD_SCHEMA_FUNCTION
	const FunctionMap&		loadFunction(Trans::Transaction& cTrans_, bool bRecovery_ = false);
	// Load privilege system table
	SYD_SCHEMA_FUNCTION
	const PrivilegeMap&		loadPrivilege(Trans::Transaction& cTrans_, bool bRecovery_ = false);

	// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;

	// データベースが UNMOUNT されたか問い合わせる
	bool					isUnmounted() const;
	// データベースのファイルが作成されているか調べる
	bool					isAccessible() const;

	// 再構成用のメソッド

	// パスが他のデータベースに使われていないか調べる
	SYD_SCHEMA_FUNCTION
	bool					checkPath(Trans::Transaction& cTrans_,
									  const ModVector<bool>* vecChanged_ = 0,
									  const ModVector<ModUnicodeString>* vecPath_ = 0,
									  bool bALlowExistence_ = false);
	// 整合性検査
	SYD_SCHEMA_FUNCTION
	void					verify(Admin::Verification::Progress& cResult_,
								   Trans::Transaction& cTrans_,
								   Admin::Verification::Treatment::Value eTreatment_);
	// 論理ログデータを生成する
	SYD_SCHEMA_FUNCTION
	void					makeLogData(Trans::Transaction& trans, LogData& cLogData_) const;
	// ログに書くためのパスリストを得る
	void					getAreaPath(Trans::Transaction& cTrans_,
										ModVector<ModUnicodeString>& vecAreaPath_) const;

	// ログデータから情報を取得する

	// ログデータから Schema ID を得る
	static ObjectID::Value	getObjectID(const LogData& cLogData_);
	// ログデータからデータベース名を得る
	SYD_SCHEMA_FUNCTION
	static Name				getName(const LogData& cLogData_);
	// ログデータからパス指定リストを得る
	static void				getPath(const LogData& log_, int iIndex_,
									ModVector<ModUnicodeString>& cPath_);
	// ログデータからエリアパスリストを得る
	static void				getAreaPath(const LogData& log_, ModVector<ModUnicodeString>& cPath_);

	// データベースの利用可能性を設定する
	SYD_SCHEMA_FUNCTION
	static bool				setAvailability(Object::ID::Value id, bool v);
	void					setAvailability(bool v);
	// データベースが利用可能か調べる
	SYD_SCHEMA_FUNCTION
	static bool				isAvailable(Object::ID::Value id);
	bool					isAvailable();

	// 使用を開始する
	SYD_SCHEMA_FUNCTION
	void					open();
	// 使用を終了する
	SYD_SCHEMA_FUNCTION
	void					close(bool bVolatile_ = false);
#ifdef DEBUG
	unsigned int			getSnapshotID() const;
#endif

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

	// キャッシュが圧縮されているか
	bool					isFreezed() const;
	// 圧縮されているキャッシュを解放する
	void					clearFreezed();

protected:
	// コンストラクター
	explicit Database(const Statement::DatabaseDefinition& cStatement_);
	explicit Database(const LogData& cLogData_);
	explicit Database(const Name& cName_, Scope::Value eScope_ = Scope::Permanent);

//	Object::
//	ID::Value				setID(ID::Value id);
//												// オブジェクト ID を設定する
//	ID::Value				setParentID(ID::Value parent);
//												// 親オブジェクトの
//												// オブジェクト ID を設定する
//	const Name&				setName(const Name& name);
//												// オブジェクトの名前を設定する
//	Category::Value			setCategory(Category::Value category);
//												// オブジェクトの種別を設定する

	// 構築の為のオプションを設定する
	void					setCreateOption(const Statement::DatabaseCreateOptionList& cStatement_);

	// ---↓ moveの下請け ---
	// データファイルを移動する
	void					moveData(Trans::Transaction& cTrans_,
									 const Os::Path& cPrevPath_,
									 const Os::Path& cNewPath_,
									 bool bUndo_ = false,
									 bool bRecovery_ = false);
	// 論理ログファイルを移動する
	void					moveLogicalLog(Trans::Transaction& cTrans_,
										   const Os::Path& cPrevPath_,
										   const Os::Path& cNewPath_,
										   bool bUndo_ = false,
										   bool bRecovery_ = false);
	// システム表を移動する
	void					moveSystem(Trans::Transaction& cTrans_,
									   const Os::Path& cPrevPath_,
									   const Os::Path& cNewPath_,
									   bool bUndo_ = false,
									   bool bRecovery_ = false);
	// ---↑ moveの下請け ---

private:
	void					destruct();			// デストラクター下位関数

	// 表オブジェクトの操作
	void					resetTable();
	void					resetTable(Database& cDatabase_);
												// 全登録の抹消
	void					clearTable();		// 全登録を抹消し、
												// 管理用のベクターを破棄する
	// エリアオブジェクトの操作
	void					resetArea();		// 全登録の抹消
	void					clearArea();		// 全登録を抹消し、
												// 管理用のベクターを破棄する
	// 子サーバーオブジェクトの操作
	void					resetCascade();		// 全登録の抹消
	void					clearCascade();		// 全登録を抹消し、
												// 管理用のベクターを破棄する
	// ルールオブジェクトの操作
	void					resetPartition();	// 全登録の抹消
	void					clearPartition();	// 全登録を抹消し、
												// 管理用のベクターを破棄する
	// 関数オブジェクトの操作
	void					resetFunction();	// 全登録の抹消
	void					clearFunction();	// 全登録を抹消し、
												// 管理用のベクターを破棄する

	// Privilege object's operations
	void					resetPrivilege();	// Erase all the privilege objects
	void					clearPrivilege();	// Destroy hash map holding privilege objects

	// 表以下のオブジェクトを格納するキャッシュの操作
	void					resetCache();		// 全登録の抹消
	void					clearCache();		// 全登録を抹消し、
												// 管理用のベクターを破棄する

#ifdef DEBUG
	void					checkCache() const;	// キャッシュの内容がすべて
												// Persistかチェックする
#endif

	// システム表を表すオブジェクトに関する操作
	void					initializeSystemTables() const;// 管理する領域を用意する
	void					clearSystemTables() const;// 管理する領域を開放する

	// operations related to virtual tables
	void					initializeVirtualTables() const;// prepare vector
	void					clearVirtualTables() const;// clear vector
	Table*					getVirtualTable(int iCategory_, // to avoid including VirtualTable.h
											Trans::Transaction& cTrans_) const;

	void					checkUndo(ID::Value iID_);
												// Undo情報を検査して反映する

	const ModUnicodeString&	getLogFilePath() const;
												// 論理ログファイルのパス名を得る
#ifdef OBSOLETE // Redoの仕様変更によりパス指定で論理ログファイル記述子を得ることはなくなった
	// データベース用の論理ログファイルの論理ログファイル記述子を得る
	Trans::Log::AutoFile	getLogFile(const Os::Path& path) const;
#endif

	void					setSnapshot(ObjectSnapshot* pSnapshot_);
												// オブジェクトを格納するスナップショットをセットする
	void					abandonCache();		// キャッシュを捨てる

	void					freeze();			// キャッシュを捨てる代わりにCommon::Dataに固める
	void					melt();				// Common::Dataに固まっているキャッシュを復活させる

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「データベース」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	Attribute				m_cAttribute;		// 属性

	Sequence*				_sequence;			// オブジェクト ID の値を
												// 生成するためのシーケンス

	// 以下のメンバーは、「エリア」表を検索して得られる

	mutable AreaMap*		_areas;				// データベース内で定義される
												// エリアを表すクラスを
												// 管理するハッシュマップ

	// 以下のメンバーは、「表」表を検索して得られる

	mutable TableMap*		_tables;			// データベースを構成する
												// 表を表すクラスを
												// 管理するハッシュマップ

	mutable CascadeMap*		_cascades;			// データベース内で定義される
												// 子サーバーを表すクラスを
												// 管理するハッシュマップ

	mutable PartitionMap*	_partitions;		// データベース内で定義される
												// ルールを表すクラスを
												// 管理するハッシュマップ

	mutable FunctionMap*	_functions;			// データベース内で定義される
												// 関数を表すクラスを
												// 管理するハッシュマップ

	// Following member can be obtained from Privilege system table

	mutable PrivilegeMap*	_privileges;		// Hash map holding privilege list

	// データベース以下のオブジェクトを保持するキャッシュ

	mutable CacheMap*		_cache;				// データベースを構成する
												// オブジェクトのキャッシュを
												// 管理するハッシュマップ

	ModVector<ModUnicodeString>*
							m_pPath;			// データベース定義にあるパス名
	mutable ModUnicodeString* m_pLogFilePath;	// ログファイルの格納に使うパス名
	mutable ModUnicodeString* m_pDataPath;		// データを格納するのに使うパス名

	mutable ModVector<Table*>* m_pSystemTables;	// システム表を表すクラスを管理する配列
	mutable ID::Value		m_iSystemTableObjectID;	// システム表を表すオブジェクトに使うID
	mutable ModVector<TablePointer>* m_pVirtualTables;	// Vector storing virtual tables

	ObjectSnapshot*			m_pSnapshot;		// データベースオブジェクトを格納するスナップショット
	int						m_iReference;		// 参照カウンター
	bool					m_bDelayedClear;	// キャッシュのクリアが後回しされているか

	Common::Data::Pointer	m_pFreezedData;		// クリアが後回しにされたものを固めておくData
	int						m_iFreezedSize;		// freezeされているデータに含まれるオブジェクト数
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_DATABASE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2005, 2006, 2007, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
