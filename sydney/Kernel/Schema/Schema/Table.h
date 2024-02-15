// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Table.h -- 表オブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_TABLE_H
#define	__SYDNEY_SCHEMA_TABLE_H

#include "Schema/Area.h"
#include "Schema/Column.h"
#include "Schema/Constraint.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Hint.h"
#include "Schema/Hold.h"
#include "Schema/Identity.h"
#include "Schema/Module.h"
#include "Schema/Object.h"
#include "Schema/TupleID.h"

#include "Admin/Verification.h"

#include "Common/Hasher.h"

#include "Lock/Mode.h"
#include "Lock/Name.h"

#include "Os/Path.h"

#include "ModMap.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class TableDefinition;
	class TableElementList;
	class ColumnDefinition;
	class TableConstraintDefinition;
	class DropTableStatement;
	class AlterTableAction;
	class AlterTableStatement;
	class AreaOption;
}
namespace Trans
{
	class Transaction;
	class TimeStamp;
}

_SYDNEY_SCHEMA_BEGIN

class Index;
class Sequence;
class File;
class Field;
class LogData;
class ColumnMap;
class ConstraintMap;
class IndexMap;
class FileMap;

namespace SystemTable
{
	class Column;
	class Constraint;
	class File;
	class Field;
	class Index;
	class Key;
	class Table;
}
namespace Utility
{
	class BinaryData;
}

//	ENUM
//	Schema::Table -- 表オブジェクトを表すクラス
//
//	NOTES
//		表の親オブジェクトは、データベースである

class Table
	: public	Object
{
public:
	friend class SystemTable::Column;
	friend class SystemTable::Constraint;
	friend class SystemTable::File;
	friend class SystemTable::Field;
	friend class SystemTable::Index;
	friend class SystemTable::Key;
	friend class SystemTable::Table;

	using Object::getName;
	using Object::getDatabaseID;

	//	TYPEDEF public
	//	Schema::Table::Pointer -- Tableを保持するObjectPointer
	//
	//	NOTES

	typedef TablePointer Pointer;

	struct Log {

		//
		//	ENUM
		//		Schema::Table::Log::Value -- ログの共通要素位置
		//
		//	NOTES
		enum Value {
			Name = 0,						// テーブル名
			ID,								// テーブル ID
			DatabaseID,						// データベースID(UNDO用)
			DatabasePaths,					// データベースパス(UNDO用)
			ValueNum
		};

		struct Create {

			//	ENUM
			//		Schema::Table::Log::Create::Value -- Create ログの要素位置
			//
			enum Value {
				Type = ValueNum,			// 種別
				Hint,						// ヒント
				AreaIDs,					// エリアID配列(Redo用)
				AreaPaths,					// エリアパス配列(UNDO用)
				ColumnDefinitions,			// 列定義配列
				ConstraintDefinitions,		// 制約配列
				LastID,						// ID最大値(Undo, Redo用)
				Num0,
				FileDefinitions = Num0,		// File定義配列
				Num1,
				Num = Num1
			};
		};

		struct Drop {

			//	ENUM 
			//		Schema::Table::Log::Drop::Value -- Drop ログの要素位置
			//
			enum Value {
				IndexIDs = ValueNum,		// 索引ID配列(Undo用)
				AreaPaths,					// エリアパス配列(Undo用)
				Num
			};
		};

		struct Alter {

			//	ENUM
			//		Schema::Table::Log::Alter::Value -- Alter ログの要素位置
			//
			enum Value {
				PrevAreaIDs = ValueNum,		// 変更前エリアID配列(Redo用)
				PostAreaIDs,				// 変更後エリアID配列(Redo用)
				PrevAreaPaths,				// 変更前エリアパス配列(Undo用)
				PostAreaPaths,				// 変更後エリアパス配列(Undo用)
				MovedFiles,					// 移動したファイルの親IDとパスの固有部分(Undo用)
				Num
			};
		};

		struct Rename {

			//	ENUM
			//		Schema::Table::Log::Rename::Value -- Rename ログの要素位置
			//
			enum Value {
				PrevName = Name,			// 変更前表名(Undo用)
				PostName = ValueNum,		// 変更後表名(Undo用)
				AreaIDs,					// 操作時のエリア指定ID配列(Undo用)
				AreaPaths,					// 操作時のエリアパス配列(Undo用)
				MovedFiles,					// 移動したファイルの親IDとパスの固有部分(Undo用)
				Num
			};
		};

		struct AlterAddColumn {

			//	ENUM
			//		Schema::Table::Log::AlterAddColumn::Value -- AddColumn ログの要素位置
			//
			enum Value {
				ColumnDefinitions = ValueNum,// 列定義配列
				LastID,						// ID最大値(Undo, Redo用)
				AreaIDs,					// 操作時のエリア指定ID配列(Undo用)
				AreaPaths,					// 操作時のエリアパス配列(Undo用)
				PostFiles,					// 作成されたファイルのエリアIDとパス名固有部分(Undo用)
				PrevFiles,					// 変更前のレコードファイルのエリアIDとパス名固有部分(Undo用)
				TempSuffix,					// 変更前のレコードファイルの退避先名のサフィックス(Undo用)
				Num0,
				FileDefinitions = Num0,		// File定義配列
				Num1,
				Num = Num1
			};
		};

		struct AlterAlterColumn {

			//	ENUM
			//		Schema::Table::Log::AlterAlterColumn::Value -- AlterColumn ログの要素位置
			//
			enum Value {
				TargetColumnName = ValueNum,// 変更する列名
				PostColumnDefinition,		// 変更後列定義
				Num
			};
		};

		struct AlterAddConstraint {

			//	ENUM
			//		Schema::Table::Log::AlterAddConstraint::Value -- AddConstraint ログの要素位置
			//
			enum Value {
				ConstraintDefinitions = ValueNum,// 制約定義配列
				LastID,						// ID最大値(Undo, Redo用)
				AreaIDs,					// 操作時のエリア指定ID配列(Undo用)
				AreaPaths,					// 操作時のエリアパス配列(Undo用)
				CreatedFiles,				// 作成されたファイルのエリアIDとパス名固有部分(Undo用)
				Num0,
				FileDefinitions = Num0,		// File定義配列
				Num1,
				Num = Num1
			};
		};
	};

	// コンストラクター
	Table();
	// デストラクター
	virtual ~Table();

	// DataArrayDataを元にインスタンスを生成する
	static Table*			getNewInstance(const Common::DataArrayData& cData_);

	void					clear();			// メンバーをすべて初期化する
	void					detachFiles();		// 表が使用しているファイルをdetachする

	// 表を生成する
	static Table*			create(Database& database,
								   const Statement::TableDefinition& statement,
								   LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	static Table*			create(Trans::Transaction& cTrans_,
								   const Database& database,
								   const LogData& cLogData_);
	void					create(Trans::Transaction& cTrans_);

	// システム表のオブジェクトを生成する
	static Table*			createSystem(Trans::Transaction& cTrans_,
										 const Database& cDatabase_,
										 Object::Category::Value eCategory_);

	// SQL文から表名を得る
	static Name				getName(const Statement::DropTableStatement& cStatement_);
	static Name				getName(const Statement::AlterTableStatement& cStatement_);

	// 表を抹消する
	static void				drop(Table& cTable_, LogData& cLogData_, Trans::Transaction& cTrans_);
	void					drop(Trans::Transaction& cTrans_, bool bRecovery_ = false);

	// 表の抹消マークをクリアする
	void					undoDrop(Trans::Transaction& cTrans_);

	// 表を構成するファイルとそれを格納するディレクトリーを破棄する
	void					destroy(Trans::Transaction& cTrans_,
									bool bDestroyArea_ = false,
									bool bForce_ = true);

	// 表を構成するファイルを mount する
	void					mount(Trans::Transaction& cTrans_,
								  const Name& cDatabaseName_,
								  bool bUndo_ = false);
	// 表を構成するファイルを unmount する
	void					unmount(Trans::Transaction& cTrans_,
									const Name& cDatabaseName_,
									bool bUndo_ = false);
	// 表を構成するファイルを flush する
	void					flush(Trans::Transaction& cTrans_);
	// 不要な版を破棄する
	void					sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
	// バックアップを開始する
	void					startBackup(Trans::Transaction& cTrans_,
										bool bRestorable_ = true,
										bool bUndo_ = false);
	// バックアップを終了する
	void					endBackup(Trans::Transaction& cTrans_);
	//	障害から回復する
	void					recover(Trans::Transaction& cTrans_,
									const Trans::TimeStamp& cPoint_,
									const Name& cDatabaseName_);
	// ある時点に開始された読取専用トランザクションが参照する版を最新版とする
	void					restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);

	// 表の定義を変更する
	static bool				alterArea(Trans::Transaction& cTrans_,
									  Table& cTable_,
									  const Statement::AlterTableAction& stmt,
									  ModVector<ID::Value>& vecPrevAreaID_,
									  ModVector<ID::Value>& vecPostAreaID_,
									  LogData& cLogData_);
	static bool				alterName(Trans::Transaction& cTrans_,
									  Table& cTable_,
									  const Statement::AlterTableAction& stmt,
									  Name& cPostName_,
									  LogData& cLogData_);
	// Prepare altering a table adding columns
	static bool				alterAddColumn(Trans::Transaction& cTrans_,
										   Table& cTable_,
										   const Statement::AlterTableAction& stmt,
										   ModVector<FilePointer>& vecPrevFiles_,
										   ModVector<FilePointer>& vecPostFiles_,
										   ModVector<Column::Pointer>& vecNewColumns_,
										   ModVector<Field*>& vecSourceField_,
										   ModVector<Field*>& vecTargetField_,
										   LogData& cLogData_);
	// from logdata
	static bool				alterAddColumn(Trans::Transaction& cTrans_,
										   Table& cTable_,
										   const LogData& cLogData_,
										   ModVector<FilePointer>& vecPrevFiles_,
										   ModVector<FilePointer>& vecPostFiles_,
										   ModVector<Column::Pointer>& vecNewColumns_,
										   ModVector<Field*>& vecSourceField_,
										   ModVector<Field*>& vecTargetField_);
	// Prepare altering a table altering a column
	static bool				alterAlterColumn(Trans::Transaction& cTrans_,
											 Table& cTable_,
											 const Statement::AlterTableAction& stmt,
											 ColumnPointer& pPrevColumn_,
											 ColumnPointer& pPostColumn_,
											 LogData& cLogData_);
	// from logdata
	static bool				alterAlterColumn(Trans::Transaction& cTrans_,
											 Table& cTable_,
											 const LogData& cLogData_,
											 ColumnPointer& pPrevColumn_,
											 ColumnPointer& pPostColumn_);
	// Prepare altering a table adding a constraint
	static bool				alterAddConstraint(Trans::Transaction& cTrans_,
											   Table& cTable_,
											   const Statement::AlterTableAction& cStatement_,
											   Constraint::Pointer& pConstraint_,
											   ModVector<Table*>& vecReferencedTable_,
											   LogData& cLogData_);
	// from logdata
	static bool				alterAddConstraint(Trans::Transaction& cTrans_,
											   Table& cTable_,
											   Constraint::Pointer& pConstraint_,
											   ModVector<Table*>& vecReferencedTable_,
											   const LogData& cLogData_);

	// 表のファイルを移動する
	SYD_SCHEMA_FUNCTION
    void					move(Trans::Transaction& cTrans_,
								 const ModVector<ID::Value>& vecPrevAreaID_,
								 const ModVector<ID::Value>& vecPostAreaID_,
								 bool bUndo_ = false, bool bRecovery_ = false,
								 bool bMount_ = false);

	// 移動の結果使用しなくなったエリア以下のディレクトリーを削除する
	void					sweepArea(Trans::Transaction& cTrans_,
									  const ModVector<ID::Value>& areaID_,
									  const Name& cName_);

	// Replace the altered column
	void					alterColumn(Trans::Transaction& cTrans_,
										const ColumnPointer& pPrevColumn_,
										const ColumnPointer& pPostColumn_,
										bool bUndo_ = false);

	// 表を表すクラスを得る
	SYD_SCHEMA_FUNCTION
	static Table*			get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_,
								bool bInternal_ = false);
	SYD_SCHEMA_FUNCTION
	static Table*			get(ID::Value id, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_,
								bool bInternal_ = false);

	// 陳腐化していないか調べる
	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, ID::Value iDatabaseID_,
									Timestamp iTime_,
									Trans::Transaction& cTrans_);

	// 永続化前に行う処理
	static void				doBeforePersist(const Pointer& pTable_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);

	// 永続化後に行う処理
	static void				doAfterPersist(const Pointer& pTable_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pTable_,
										Database& cDatabase_,
										Trans::Transaction& cTrans_);

	// 表が一時表になるか
	SYD_SCHEMA_FUNCTION
	static bool				isToBeTemporary(const Statement::Identifier* pSt_);
	SYD_SCHEMA_FUNCTION
	static bool				isToBeTemporary(const ModUnicodeString& cstrName_);

	// 一時表かどうかの情報を設定する
	void					setTemporary(bool bTemporary_);

	// この表が一時表かどうか
	SYD_SCHEMA_FUNCTION
	bool					isTemporary() const;

	// システム表かどうかの情報を設定する
	void					setSystem(bool bSystem_);

	// この表がシステム表かどうか
	SYD_SCHEMA_FUNCTION
	bool					isSystem() const;

	// get referencing tables (having foreign key referencing to this table)
	bool					hasReferencingTable(Trans::Transaction& cTrans_);
	const ModVector<Table*>& getReferencingTable(Trans::Transaction& cTrans_);
	// get referenced tables (referenced by a foreign key constraint of this table)
	bool					hasReferencedTable(Trans::Transaction& cTrans_);
	const ModVector<Table*>& getReferencedTable(Trans::Transaction& cTrans_);

	// データベースの属性が変化したので対応して変更すべきものを変更する
	void					propagateDatabaseAttribute(Trans::Transaction& cTrans_,
													   const Database::Attribute& cAttribute_);

//	Object::
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
//	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る
//	ID::Value				getDatabaseID() const;

	// オブジェクトごとに固有のパス名部分を得る
	const ModUnicodeString&	getPathPart(Trans::Transaction& cTrans_) const;
	const Os::Path&			getPath(Trans::Transaction& cTrans_) const;
	// 表を構成するファイルを格納するトップディレクトリーのパス名を得る
	static Os::Path			getPath(const ModVector<ModUnicodeString>& vecDatabasePath_,
									const ModVector<ModUnicodeString>& vecAreaPath_,
									const Name& cDatabaseName_,
									const Name& cTableName_);

	// 表を構成するファイルを格納するトップディレクトリーのパス名を設定する
	void					setPath(Trans::Transaction& cTrans_,
									const ModUnicodeString& cPath_,
									const Name& cName_);
	// getPathで得られるパス名のキャッシュをクリアする
	void					clearPath();
	// 下位オブジェクトを抹消する
	void					reset(Database& cDatabase_);

	// タプル ID の値を生成するためのシーケンスを得る
	Sequence&				getTupleSequence(Trans::Transaction& cTrans_);

	// シーケンスファイルを作成する
	void					createSequence(Trans::Transaction& cTrans_);
	// シーケンスファイルを破棄する
	void					dropSequence(Trans::Transaction& cTrans_, bool bForce_ = true);
	// シーケンスファイルをmountする
	void					mountSequence(Trans::Transaction& cTrans_);
	// シーケンスファイルをunmountする
	void					unmountSequence(Trans::Transaction& cTrans_);
	// シーケンスファイルをflushする
	void					flushSequence(Trans::Transaction& cTrans_);
	// シーケンスファイルをsyncする
	void					syncSequence(Trans::Transaction& cTrans_, bool& incomplete, bool& modified);
	// シーケンスファイルをstartBackupする
	void					startBackupSequence(Trans::Transaction& cTrans_,
												bool bRestorable_ = true);
	// シーケンスファイルをendBackupする
	void					endBackupSequence(Trans::Transaction& cTrans_);
	// シーケンスファイルをrecoverする
	void					recoverSequence(Trans::Transaction& cTrans_,
											const Trans::TimeStamp& cPoint_);
	// シーケンスファイルをrestoreする
	void					restoreSequence(Trans::Transaction&	cTrans_,
											const Trans::TimeStamp&	cPoint_);
	// シーケンスファイルを移動する
	void					moveSequence(Trans::Transaction& cTrans_,
										 const Os::Path& cPrevPath_,
										 const Os::Path& cPostPath_,
										 const Name& cPrevName_,
										 const Name& cPostName_,
										 bool bUndo_ = false,
										 bool bRecovery_ = false);
	// シーケンスの検査を行う
	void verifySequence(Admin::Verification::Progress& cResult_,
						Trans::Transaction& cTrans_,
						Admin::Verification::Treatment::Value eTreatment_,
						TupleID::Value iMaxRowID_,
						Identity::Value iMaxIdentity_);

	// シーケンスを表すオブジェクトを破棄する
	void					clearSequence();

	SYD_SCHEMA_FUNCTION
	const Hint*				getHint() const;	// 表に対するヒントを得る
	void					clearHint();		// 表に対するヒントを消去する

	// 再構成に関する操作

	// この表に属するファイルが再構成中か
	SYD_SCHEMA_FUNCTION
	bool					isReorganizationInProgress() const;
	// 再構成中の更新操作でログ出力が必要なカラムを得る
	SYD_SCHEMA_FUNCTION
	ModVector<Column*>		getLogColumns(Trans::Transaction& cTrans_) const;

	// 再構成中のファイルを追加/削除する
	void					addReorganizedFile(Trans::Transaction& cTrans_,
											   File* pFile_);
	void					eraseReorganizedFile(Trans::Transaction& cTrans_,
												 File* pFile_);

	// 表に属するファイルに対するデフォルトエリアに関する操作

	// 表に関するファイルを格納するエリアを得る
	Area*					getDefaultArea(Trans::Transaction& cTrans_) const;
	Area*					getArea(AreaCategory::Value eArea_,
									Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	ID::Value				getDefaultAreaID() const;
	SYD_SCHEMA_FUNCTION
	ID::Value				getAreaID(AreaCategory::Value eArea_, bool bEffective_ = false) const;

	static ID::Value		getAreaID(const ModVector<ID::Value>& vecAreaID,
									  AreaCategory::Value eArea_, bool bEffective_ = false);

	// 表に関するファイルを格納するエリアのIDを得る
	SYD_SCHEMA_FUNCTION
	const ModVector<ID::Value>&
							getAreaID() const;
	// エリアのIDを指定のものに変える
	void					setAreaID(const ModVector<ID::Value>& cAreaID_);

	void					resetArea();		// 全登録を抹消
	void					clearArea();		// 全登録を抹消し、
												// 管理用のベクターを破棄する

	// エリア指定のSQL構文要素にもとづいてエリアのIDを設定する
	bool					setArea(Trans::Transaction& cTrans_,
									const Statement::AreaOption& cStatement_,
									ModVector<ID::Value>& vecPrevAreaID_,
									ModVector<ID::Value>& vecPostAreaID_);

	// エリア指定のSQL構文要素にもとづいてエリアの設定を解除する
	bool					dropArea(Trans::Transaction& cTrans_,
									 const Statement::AreaOption& cStatement_,
									 ModVector<ID::Value>& vecPrevAreaID_,
									 ModVector<ID::Value>& vecPostAreaID_);

	// エリアの変更を下位オブジェクトに反映する
    void					moveArea(Trans::Transaction& cTrans_,
									 AreaCategory::Value eCat_,
									 ID::Value iPrevAreaID_,
									 ID::Value iPostAreaID_,
									 const ModVector<ID::Value>& vecPrevAreaID_,
									 const ModVector<ID::Value>& vecPostAreaID_,
									 ModVector<ID::Value>* pvecNonEmptyArea_ = 0,
									 bool bUndo_ = false, bool bRecovery_ = false,
									 bool bMount_ = false);

	// パス指定の変更を下位オブジェクトに反映する
	void					movePath(Trans::Transaction& cTrans_,
									 const Os::Path& cPrevPath_,
									 const Os::Path& cPostPath_,
									 bool bUndo_ = false,
									 bool bRecovery = false);
	// 名前を変更する
	void rename(const Name& cPostName_);
	// 名前の変更を下位オブジェクトに反映する
	void moveRename(Trans::Transaction& cTrans_, const Name& cPrevName_, const Name& cPostName_,
					bool bUndo_ = false, bool bRecovery_ = false);

	// 索引などのエリア変更の結果空になったエリアを消す
	void					sweepUnusedArea(Trans::Transaction& cTrans_,
											ID::Value iAreaID_,
											bool bForce_ = true);

	// CategoryをStatementのAreaタイプに変換する
	static
	Statement::AreaOption::AreaType
							convertAreaCategory(AreaCategory::Value eCategory_);

	// 表に登録される列を表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	const ModVector<Column*>& getColumn(Trans::Transaction& cTrans_) const;
	// ある種別の登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Column*>		getColumn(Column::Category::Value columnCategory,
									  Trans::Transaction& cTrans_) const;
	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Column*					getColumnByID(ID::Value columnID,
										  Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Column*					getColumn(const Name& columnName,
									  Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Column*					getColumnByPosition(Column::Position columnPosition,
											Trans::Transaction& cTrans_) const;
	// 登録の追加
	const ColumnPointer&	addColumn(const ColumnPointer& column,
									  Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseColumn(ID::Value columnID);
	// 全登録の抹消
	void					resetColumn();
	void					resetColumn(Database& cDatabase_);
	// 全登録を抹消し、管理用のベクターを破棄する
	void					clearColumn(Trans::Transaction& cTrans_);

	// タプル ID を格納する列を得る
	SYD_SCHEMA_FUNCTION
	Column*					getTupleID(Trans::Transaction& cTrans_) const;

	// Identity Column列を得る
	SYD_SCHEMA_FUNCTION
	Column*					getIdentity(Trans::Transaction& cTrans_) const;

	// 表に登録される制約を表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	const ModVector<Constraint*>&
							getConstraint(Trans::Transaction& cTrans_) const;
// IDと名前での検索は内部で使用している
//	SYD_SCHEMA_FUNCTION
	Constraint*				getConstraint(ID::Value constraintID,
										  Trans::Transaction& cTrans_) const;
//	SYD_SCHEMA_FUNCTION
	Constraint*				getConstraint(const Name& constraintName,
										  Trans::Transaction& cTrans_) const;

	// ある種別の登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Constraint*>	getConstraint(Constraint::Category::Value cnstCat_,
										  Trans::Transaction& cTrans_) const;
#ifdef OBSOLETE // 以下の関数は使用されない
	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Constraint*				getConstraint(
								Constraint::Position constraintPosition,
								Trans::Transaction& cTrans_) const;
#endif

	// Primary Key制約の取得
	Constraint*				getPrimaryKeyConstraint(Trans::Transaction& cTrans_) const;

	// 登録の追加
	const ConstraintPointer&
							addConstraint(const ConstraintPointer& constraint,
										  Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseConstraint(ID::Value constraintID);
	// 全登録の抹消
	void					resetConstraint();
	void					resetConstraint(Database& cDatabase_);
	// 全登録を抹消し、管理用のベクターを破棄する
	void					clearConstraint(Trans::Transaction& cTrans_);

	// 表に登録されているファイルを表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	// あるエリア種別に対応する登録の取得
	const ModVector<File*>&	getFile(Trans::Transaction& cTrans_) const;
	ModVector<File*>		getFile(AreaCategory::Value eCategory_,
									const ModVector<ID::Value>& vecPrevAreaID_,
									const ModVector<ID::Value>& vecPostAreaID_,
									Trans::Transaction& cTrans_) const;
	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	File*					getFile(ID::Value fileID,
									Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	File*					getFile(const Name& fileName,
									Trans::Transaction& cTrans_) const;
	// 索引を実現するファイルを得る
	FilePointer				getIndexFile(ID::Value indexID, Trans::Transaction& cTrans_) const;
	// フィールドを保持するファイルを得る
	FilePointer				getFieldFile(const Field& cField_, Trans::Transaction& cTrans_) const;

	// 登録の追加
	const FilePointer&		addFile(const FilePointer& file,
									Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseFile(ID::Value fileID);
	// 全登録の抹消
	void					resetFile();
	void					resetFile(Database& cDatabase_);
	// 全登録を抹消し、管理用のベクターを破棄する
	void					clearFile(Trans::Transaction& cTrans_);

	// 表についた索引として登録されている索引を表すクラスに関する操作

	// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	const ModVector<Index*>& getIndex(Trans::Transaction& cTrans_) const;
	// あるエリア種別に対応する登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Index*>		getIndex(AreaCategory::Value eCategory_,
									 Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	ModVector<Index*>		getIndexArea(ID::Value areaID_,
										 Trans::Transaction& cTrans_) const;
	// ある登録の取得
	SYD_SCHEMA_FUNCTION
	Index*					getIndex(ID::Value indexID,
									 Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Index*					getIndex(const Name& indexName,
									 Trans::Transaction& cTrans_) const;
	// 登録の追加
	const IndexPointer&		addIndex(const IndexPointer& index,
									 Trans::Transaction& cTrans_);
	// 登録の抹消
	void					eraseIndex(ID::Value indexID);
	// ある索引の抹消
	void					eraseIndex(Database& cDatabase_,
									   ID::Value iIndexID_);
	// 全登録の抹消
	void					resetIndex();
	void					resetIndex(Database& cDatabase_);
	// 全登録を抹消し、管理用のベクターを破棄する
	void					clearIndex(Trans::Transaction& cTrans_);

	// 分散ルールを表すクラス
	Partition*				getPartition(Trans::Transaction& cTrans_) const;
	const PartitionPointer&	setPartition(const PartitionPointer& pPartition_);
	void					clearPartition();
	ID::Value				getPartitionID();
	void					setPartitionID(ID::Value id_);

#ifdef OBSOLETE // Fieldを得る機能は使用されない
	// 表の列の値を格納するフィールドを得る
	SYD_SCHEMA_FUNCTION
	ModVector<Field*>		getSource(Trans::Transaction& cTrans_) const;
#endif

	// このオブジェクトを親オブジェクトとするオブジェクトのロード

	// 列を読み出す
	SYD_SCHEMA_FUNCTION
	const ColumnMap&		loadColumn(Trans::Transaction& cTrans_,
									   bool bRecovery_ = false);
	// 制約を読み出す
	SYD_SCHEMA_FUNCTION
	const ConstraintMap&	loadConstraint(Trans::Transaction& cTrans_,
										   bool bRecovery_ = false);
	// 索引を読み出す
	SYD_SCHEMA_FUNCTION
	const IndexMap&			loadIndex(Trans::Transaction& cTrans_,
									  bool bRecovery_ = false);
	// ファイルを読み出す
	SYD_SCHEMA_FUNCTION
	const FileMap&			loadFile(Trans::Transaction& cTrans_,
									 bool bRecovery_ = false);

	// 表に属するスキーマオブジェクトをあらかじめ読み込む
	void					doPreLoad(Trans::Transaction& cTrans_,
									  Database& cDatabase_);

	// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	// このクラスのクラス ID を得る
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;

	// 整合性検査
	SYD_SCHEMA_FUNCTION
	void					verify(Admin::Verification::Progress& cResult_,
								   Trans::Transaction& cTrans_,
								   Admin::Verification::Treatment::Value eTreatment_);

	// is this table virtual?
	virtual bool isVirtual() const {return false;}

	// 論理ログ出力用のメソッド

	// ログデータを作る
	void					makeLogData(Trans::Transaction& cTrans_,
										LogData& cLogData_) const;
	Common::Data::Pointer	getMovedFiles(Trans::Transaction& cTrans_,
										  const ModVector<ID::Value>& vecPrevAreaID_,
										  const ModVector<ID::Value>& vecPostAreaID_) const;
	Common::Data::Pointer	getMovedFiles(Trans::Transaction& cTrans_,
										  const Name& cPostName_) const;
	Common::Data::Pointer	getCreatedFiles(Trans::Transaction& cTrans_,
											const ModVector<FilePointer>& vecPostFiles_) const;

public:
	// ログデータから情報を取得する

	// ログデータから表名を得る
	static Name				getName(const LogData& cLogData_);

	// ログデータから Schema ID を得る
	static ID::Value		getObjectID(const LogData& cLogData_);
	// ログデータからデータベースIDを得る
	static ID::Value		getDatabaseID(const LogData& cLogData_);
	// ログデータからデータベースパスを得る
	static void				getDatabasePath(const LogData& cLogData_,
											ModVector<ModUnicodeString>& vecPath_);
	// ログデータからエリアIDリストを得る
	static void				getAreaID(const LogData& cLogData_, int iIndex_,
									  ModVector<ID::Value>& vecAreaID_);
	// ログデータからエリアパスリストを得る
	static bool				getAreaPath(const LogData& cLogData_, int iIndex_, int iCategory_,
										ModVector<ModUnicodeString>& vecAreaPath_);
	// ログデータからエリアパスリストを得る
	// (ファイルに使われるパスを得る)
	static bool				getEffectiveAreaPath(const LogData& cLogData_, int iIndex_, int iCategory_,
												 const ModVector<ID::Value>& vecAreaID_,
												 ModVector<ModUnicodeString>& vecAreaPath_);
	// ログデータから索引のIDリストを得る
	static const ModVector<ID::Value>&
							getIndexID(const LogData& cLogData_);
	// ログデータから指定した位置の移動ファイルリストを得る
	static void				getMovedFiles(const Common::DataArrayData& cData_,
										  int iCategory_,
										  ModVector<ID::Value>& vecParentID_,
										  ModVector<ID::Value>& vecFileID_,
										  ModVector<ModUnicodeString>& vecPrevFileName_,
										  ModVector<ModUnicodeString>& vecPostFileName_,
										  bool bRename_ = false);

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

	// システム表に関する操作
	static const Name&		getSystemTableName(Object::Category::Value eCategory_);
	static ID::Value		getSystemTableID(Object::Category::Value eCategory_);
	static Object::Category::Value
							getSystemTableCategory(const Name& cName_);
	static Object::Category::Value
							getSystemTableCategory(ID::Value iID_);

protected:
	// constructor which is overriden by subclasses
	Table(const Database& database, const Name& name);

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

private:
	// コンストラクター
	Table(Trans::Transaction& cTrans_, const Database& database, const LogData& cLogData_);
	Table(const Table& original);

	void					destruct();			// デストラクター下位関数

	// Create column objects according to TableElementList
	void					createColumn(Trans::Transaction& cTrans_,
										 Statement::TableElementList* pElement_,
										 Common::DataArrayData& cLogData_);
	// Create a column object from a ColumnDefinition
	ColumnPointer			createColumn(Trans::Transaction& cTrans_,
										 Statement::ColumnDefinition* pColumnDefinition_,
										 Column::Position iPosition_,
										 Common::DataArrayData& cLogData_);
	// Create a column object from a log data
	ColumnPointer			createColumn(Trans::Transaction& cTrans_,
										 const Common::DataArrayData& cLogData_,
										 Column::Position iPosition_,
										 const Column* pOriginalColumn_ = 0);
	// create constraint objects according to TableElementList
	void					createConstraint(Trans::Transaction& cTrans_,
											 Statement::TableElementList* pElement_,
											 Common::DataArrayData& cLogData_);
	// create a constraint object from TableConstraintDefinition
	Constraint::Pointer		createConstraint(Trans::Transaction& cTrans_,
											 Statement::TableConstraintDefinition* pConstraintDefinition_,
											 Constraint::Position iPosition_,
											 Common::DataArrayData& cLogData_);
	// create a constraint object from log data
	Constraint::Pointer		createConstraint(Trans::Transaction& cTrans_,
											 const Common::DataArrayData& cLogData_,
											 Constraint::Position iPosition_);

	// 表を構成するファイルを表すクラスを生成する
	void					createFile(Trans::Transaction& cTrans_,
									   const Common::DataArrayData* pLogData_ = 0);

	// create Vector file
	void					createVectorFile(Trans::Transaction& cTrans_,
											 Database& cDatabase_, Column& cColumn_,
											 const File::Pointer& pFile_,
											 const Common::DataArrayData* pFileLogData_ = 0,
											 const Common::DataArrayData* pVectorLogData_ = 0);
	// create Heap file
	Field::Pointer			createHeapFile(Trans::Transaction& cTrans_,
										   Database& cDatabase_, Column& cColumn_,
										   const File::Pointer& pFile_,
										   const Common::DataArrayData* pFileLogData_ = 0,
										   const Common::DataArrayData* pHeapLogData_ = 0);
	// create Lob file
	Field::Pointer			createLobFile(Trans::Transaction& cTrans_,
										  Database& cDatabase_, Column& cColumn_,
										  const File::Pointer& pFile_,
										  const Common::DataArrayData* pFileLogData_ = 0,
										  const Common::DataArrayData* pLobLogData_ = 0);

	// create a file for add column
	void					createFileForAddColumn(Trans::Transaction& cTrans_,
												   ModVector<FilePointer>& vecPrevFiles_,
												   ModVector<FilePointer>& vecPostFiles_,
												   ModVector<Field*>& vecSourceField_,
												   ModVector<Field*>& vecTargetField_,
												   const Common::DataArrayData* pFileLogData_ = 0);

	// make log data for files
	template<class E>
	void					makeFileLogData(Trans::Transaction& cTrans_,
											const ModVector<E>& vecFiles_,
											Common::DataArrayData& cLogData_);

	//create a field from a created column
	void					addField(Trans::Transaction& cTrans_, Database& cDatabase_, Column& cColumn_,
									 const FilePointer& pFile_, ModVector<FilePointer>* pvecNewFiles_ = 0,
									 const Common::DataArrayData* pFileLogData_ = 0,
									 const Common::DataArrayData* pTargetFileLogData_ = 0);

	// add reference table id according to foreign key / refered key
	void					addReference(Constraint& cConstraint_);
	// erase reference table id according to foreign key / refered key
	void					eraseReference(Constraint& cConstraint_);

	// エリア格納関係を表すクラスを生成する
	void					createAreaContent(Trans::Transaction& cTrans_);

	// カテゴリーDefaultに対応する移動の処理を行う
	void					moveAreaDefault(Trans::Transaction& cTrans_,
											ID::Value iPrevAreaID_,
											ID::Value iPostAreaID_,
											bool bUndo_ = false, bool bRecovery_ = false,
											bool bMount_ = false);

	// 指定したカテゴリーに対応するファイルの移動の処理を行う
	void					moveAreaFile(Trans::Transaction& cTrans_,
										 AreaCategory::Value eCat_,
										 ID::Value iPrevAreaID_,
										 ID::Value iPostAreaID_,
										 const ModVector<ID::Value>& vecPrevAreaID_,
										 const ModVector<ID::Value>& vecPostAreaID_,
										 ModVector<ID::Value>* pvecNonEmptyArea_ = 0,
										 bool bUndo_ = false, bool bRecovery_ = false,
										 bool bMount_ = false);

	// 指定したカテゴリーに対応する索引の移動の処理を行う
	void					moveAreaIndex(Trans::Transaction& cTrans_,
										  AreaCategory::Value eCat_,
										  ID::Value iPrevAreaID_,
										  ID::Value iPostAreaID_,
										  const ModVector<ID::Value>& vecPrevAreaID_,
										  const ModVector<ID::Value>& vecPostAreaID_,
										  ModVector<ID::Value>* pvecNonEmptyArea_ = 0,
										  bool bUndo_ = false, bool bRecovery_ = false,
										  bool bMount_ = false);

	// 指定したエリアカテゴリーの移動で実際に移動するファイルのファイル名を得る
	void					getMovedFiles(Trans::Transaction& cTrans_,
										  AreaCategory::Value eCategory_,
										  ModVector<ID::Value>& vecParentID_,
										  ModVector<ID::Value>& vecFileID_,
										  ModVector<ModUnicodeString>& vecFileName_) const;
	void					getMovedFiles(Trans::Transaction& cTrans_,
										  AreaCategory::Value eCategory_,
										  const Name& cPostName_,
										  ModVector<ID::Value>& vecParentID_,
										  ModVector<ID::Value>& vecFileID_,
										  ModVector<ModUnicodeString>& vecPrevFileName_,
										  ModVector<ModUnicodeString>& vecPostFileName_) const;

	// Move files corresponding to the specified area category
	void moveRename(Trans::Transaction& cTrans_, AreaCategory::Value eCat_, ID::Value iAreaID_,
					const Name& cPrevName_, const Name& cPostName_,
					bool bUndo_, bool bRecovery_);
	// Move files corresponding to the default area category
	void moveRenameDefault(Trans::Transaction& cTrans_,
						   ID::Value iAreaID_, const Name& cPrevName_, const Name& cPostName_,
						   bool bUndo_, bool bRecovery_);
	// Move files corresponding to the area category other than default
	void moveRenameFile(Trans::Transaction& cTrans_, AreaCategory::Value eCat_, ID::Value iAreaID_,
						const Name& cPrevName_, const Name& cPostName_,
						bool bUndo_, bool bRecovery_);

	// データベースとエリアの指定からデータを格納するパスを得る
	static Os::Path			getDataPath(Trans::Transaction& cTrans_,
										Database* pDatabase_, ID::Value iAreaID_);

	// Undo情報を検査して反映する
	void					checkUndo(const Database& database, ID::Value iID_);

	// ファイルオブジェクト操作用のRWLockを得る
	Os::RWLock&				getRWLockForFile() const;

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「表」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	ColumnPointer			m_pRowID;			// タプル ID の値を保持する列
	ColumnPointer			m_pIdentity;		// IdentityColumn の値を保持する列

	ModVector<ID::Value>	m_veciAreaID;
												// ファイルのデフォルトエリア
	mutable ModVector<Area*>* m_vecpArea;		// 対象ごとに設定される
												// デフォルトエリアの
												// オブジェクト
	mutable Os::Path*		m_pPath;			// 表を構成するファイルを
												// 格納するトップディレクトリー
												// のパス名

	Hint*					m_pHint;			// 表のヒント

	ModVector<File*>*		m_pReorganizedFiles;
												// 再構成中のファイル

	bool					m_bTemporary;		// 一時表か（true:一時表である）
	bool					m_bSystem;			// システム表か（true:システム表である）

	typedef ModMap<ID::Value, int, ModLess<ID::Value> > TableReferenceMap;

	TableReferenceMap		m_mapReferencingTable; // ID-refcount map of referencing tables
	TableReferenceMap		m_mapReferencedTable; // ID-refcount map of referenced tables
	mutable ModVector<Table*>* m_vecReferencingTable; // Object vector of referencing tables
	mutable ModVector<Table*>* m_vecReferencedTable; // Object vector of referenced tables

	// 以下のメンバーは、「列」表を検索して得られる

	//【注意】	垂直分割を導入するときは、列を直接参照せずに、
	//			垂直分割クラスを介して、列を参照することになる ?

	mutable	ColumnMap*		_columns;			// 表に属する
												// 列を表すクラスを
												// 管理するハッシュマップ

	// 以下のメンバーは、「制約」表を検索して得られる

	mutable ConstraintMap*	_constraints;		// 表に属する
												// 制約を表すクラスを
												// 管理するハッシュマップ

	// 以下のメンバーは、「索引」表を検索して得られる

	mutable	IndexMap*		_indices;			// 表に属する
												// 索引を表すクラスを
												// 管理するハッシュマップ

	// 以下のメンバーは、「ファイル」表を検索して得られる

	mutable FileMap*		_files;				// 表に属する
												// ファイルを表すクラスを
												// 管理するハッシュマップ
	Os::RWLock				m_cRWLockForFile;	// メンバー操作の排他制御に使う
												// ファイル操作は時間がかかる場合があるので別途用意

	// 分散ルールを表すクラスと、
	// そのスキーマオブジェクトID

	PartitionPointer		m_pPartition;
	ID::Value				m_iPartitionID;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_TABLE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
