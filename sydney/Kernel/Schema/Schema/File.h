// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// File.h -- ファイルオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_FILE_H
#define	__SYDNEY_SCHEMA_FILE_H

#include "Schema/AreaCategory.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/FileID.h"
#include "Schema/Module.h"
#include "Schema/Object.h"

#include "LogicalFile/FileDriverTable.h"
#include "LogicalFile/TreeNodeInterface.h"
#include "Os/Path.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
}
namespace Lock
{
	class FileName;
}
namespace LogicalFile
{
	class TreeNodeInterface;
}
namespace Trans
{
	class Transaction;
	class TimeStamp;

	namespace Log
	{
		class Data;
	}
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class Table;
class Index;
class Column;
class Hint;
class AccessFile;
class FileReflect;
class FieldMap;

namespace SystemTable
{
	class Field;
	class File;
}
namespace Utility
{
	class BinaryData;
}

//	CLASS
//	Schema::File -- データベースファイルオブジェクトを表すクラス
//
//	NOTES
//		ファイルの親オブジェクトは、表である

class File
	: public	Object
{
public:
	friend class SystemTable::Field;			// storeのため
	friend class SystemTable::File;				// unpackOptionのため

	//	CLASS
	//	Schema::File::Category -- ファイルの種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Category
	{
		//	ENUM
		//	Schema::File::Category::Value -- ファイルの種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			Record,								// レコードファイル
			Btree,								// B+ 木ファイル
			FullText,							// 全文ファイル
			Vector,								// ベクターファイル
			Lob,								// LOBファイル
			Bitmap,								// Bitmapファイル
			Array,								// Arrayファイル
			KdTree,								// KdTreeファイル
			ValueNum							// 種類数
		};
	};

	//	CLASS
	//	Schema::File::SkipInsertType -- ファイルに挿入しないデータの種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct SkipInsertType
	{
		//	ENUM
		//	Schema::File::SkipInsertType::Value -- ファイルに挿入しないデータの種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			None = 0,							// すべて挿入する
			ValueIsNull,						// すべてのValueフィールドがNullのとき
			FirstKeyIsNull,						// 第一キーがNullのとき
			AllKeyIsNull,						// すべてのキーがNullのとき
			AllStringKeyIsNull,					// すべての文字列型キーがNullのとき
			ValueNum							// 種類数
		};
	};

	struct Log
	{
		enum Value
		{
			ID = 0,								// ID
			FieldIDs,							// FieldID array
			Num
		};
	};

	//	TYPEDEF public
	//	Schema::File::Pointer -- Fileを保持するObjectPointer
	//
	//	NOTES

	typedef FilePointer Pointer;

	// コンストラクター
	File();
	// デストラクター
	virtual ~File();

	static File*			getNewInstance(const Common::DataArrayData& cData_);
												// DataArrayDataから
												// 該当するオブジェクトを得る

	void					clear();			// メンバーをすべて初期化する

	static const Pointer&	create(Trans::Transaction& cTrans_,
								   const Pointer& file, Table& table,
								   const Common::DataArrayData* pLogData_ = 0);
	static const Pointer&	create(Trans::Transaction& cTrans_,
								   const Pointer& file, Table& table, Index& index,
								   const Common::DataArrayData* pLogData_ = 0);
	void					create(Trans::Transaction& cTrans_);
												// ファイルを生成する
	static Pointer			createForAlter(Trans::Transaction& cTrans_,
										   const File& cFile_, Table& cTable_,
										   const Common::DataArrayData* pLogData_ = 0);
												// alterのために新しいファイルを作る

	void					drop(Trans::Transaction& cTrans_, bool bRecovery_ = false, bool bNoUnset_ = false, bool bImmediate_ = true);
												// ファイルの定義を抹消する
	void					undoDrop(Trans::Transaction& cTrans_);
												// ファイルの破棄マークをクリアする

	void					destroy(Trans::Transaction& cTrans_,
									bool bDestroyDirectory_ = false,
									bool bForce_ = true);
												// ファイルを抹消する
	void					undoDestroy(Trans::Transaction& cTrans_);
												// ファイルを抹消を取り消す

	void					mount(Trans::Transaction& cTrans_);
												// ファイルを mount する
	void					unmount(Trans::Transaction& cTrans_,
									bool bRetainFileID_ = false);
												// ファイルを unmount する
	void					flush(Trans::Transaction& cTrans_);
												// ファイルを flush する
	void					sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
												// 不要な版を破棄する
	void					startBackup(Trans::Transaction& cTransaction_,
										bool bRestorable_ = true);
												// バックアップを開始する
	void					endBackup(Trans::Transaction& cTransaction_);
												// バックアップを終了する
	void					recover(Trans::Transaction& cTransaction_,
									const Trans::TimeStamp& cPoint_);
												//	障害から回復する
	void					restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
												// ある時点に開始された
												// 読取専用トランザクションが
												// 参照する版を最新版とする

	void					import(Trans::Transaction& cTrans_,
								   const Hint* pHint_);
												// 新規作成時に
												// 既存のデータを反映する

	SYD_SCHEMA_FUNCTION
	void					undoTuple(Trans::Transaction& cTrans_,
									  const Trans::Log::Data& cLogData_);
												// import中のタプルに対する
												// 更新操作を UNDO する

	SYD_SCHEMA_FUNCTION
	void					redoTuple(Trans::Transaction& cTrans_,
									  const Trans::Log::Data& cLogData_);
												// import中のタプルに対する
												// 更新操作を REDO する

	void					startImport(Trans::Transaction& cTrans_);
												// importおよびUNDO/REDOの開始処理
	void					endImport(Trans::Transaction& cTrans_);
												// importおよびUNDO/REDOの終了処理

	SYD_SCHEMA_FUNCTION
	static File*			get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static File*			get(ID::Value id, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_);
												// ファイルを表すクラスを得る
	SYD_SCHEMA_FUNCTION
	static File*			getSystem(Object::Category::Value eCategory_,
									  ID::Value id,
									  Database* pDatabase_,
									  Trans::Transaction& cTrans_);
												// システム表のレコードファイルを得る

	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, ID::Value iDatabaseID_,
									Timestamp iTime_,
									Trans::Transaction& cTrans_);
												// 陳腐化していないか調べる

	static void				doBeforePersist(const Pointer& pFile_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pFile_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pFile_,
										Table& cTable_,
										Trans::Transaction& cTrans_);

	void					propagateDatabaseAttribute(Trans::Transaction& cTrans_,
													   const Database::Attribute& cAttribute_);
												// データベースの属性が変化したので
												// 対応して変更すべきものを変更する

	void					setReadOnly(bool bReadOnly_);
												// 読み込み属性の設定

//	Object::
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
//	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

	static ModSize			getAreaPath(Trans::Transaction& cTrans_,
										Database* pDatabase_, ID::Value iAreaID_,
										ModVector<ModUnicodeString>& cPath_);
	Os::Path				getPathPart(Trans::Transaction& cTrans_) const;
												// オブジェクトごとに固有の
												// パス名部分を得る

	void					reset(Database& cDatabase_);
												// 下位オブジェクトを抹消する

	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const;
												// ファイルの種別を得る
	SYD_SCHEMA_FUNCTION
	const LogicalFile::FileID&
							getFileID() const;	// ファイル ID を得る
	virtual void			setFileID(Trans::Transaction& cTrans_);
												// ファイル ID を設定する

	SYD_SCHEMA_FUNCTION
	LogicalFile::FileDriverID::Value
							getDriverID() const;
	SYD_SCHEMA_FUNCTION
	static LogicalFile::FileDriverID::Value
							getDriverID(Category::Value category);
												// ファイルを扱う
												// ファイルドライバーの
												// ファイルドライバー ID を得る

	SYD_SCHEMA_FUNCTION
	Table*					getTable(Trans::Transaction& cTrans_) const;
												// ファイルを使用する表を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getTableID() const;	// ファイルを使用する表の
												// オブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	Index*					getIndex(Trans::Transaction& cTrans_) const;
												// ファイルを使用する索引を得る
	// ファイルを使用する索引のオブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getIndexID() const;
	// ファイルを使用する索引のオブジェクト ID を設定する
	void					setIndexID(ID::Value id_);

	// ファイルを格納するエリアに関する操作

	SYD_SCHEMA_FUNCTION
	Area*					getArea(AreaCategory::Value eArea_,
									Trans::Transaction& cTrans_) const;
												// ファイルまたは
												// 物理ログファイルを
												// 格納するエリアを得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getAreaID(AreaCategory::Value eArea_ = AreaCategory::Default) const;
												// ファイルを格納するエリアの
												// オブジェクト ID を得る
	void					setAreaID(ID::Value iID_,
									  AreaCategory::Value eArea_);
												// ファイルおよび物理ログを
												// 格納するエリアIDを設定する
	SYD_SCHEMA_FUNCTION
	virtual AreaCategory::Value
							getAreaCategory() const;
												// ファイルに対応する
												// エリアの種別を得る

	void					moveArea(Trans::Transaction& cTrans_,
									 ID::Value iPrevAreaID_,
									 ID::Value iPostAreaID_,
									 bool bUndo_ = false,
									 bool bRecovery_ = false,
									 bool bMount_ = false);
												// エリア割り当ての変更により
												// ファイルを移動する
	void					moveArea(Trans::Transaction& cTrans_,
									 const ModVector<ModUnicodeString>& vecPrevPath_,
									 const ModVector<ModUnicodeString>& vecPostPath_,
									 bool bUndo_ = false,
									 bool bRecovery_ = false,
									 bool bMount_ = false);
												// エリア定義の変更により
												// ファイルを移動する

	void					movePath(Trans::Transaction& cTrans_,
									 const Os::Path& cDir_,
									 bool bUndo_ = false,
									 bool bRecovery_ = false);
												// 親ディレクトリーの変更により
												// ファイルを移動する
	void					rename(const Name& cName_);
											// 名前を変更する
	void					moveRename(Trans::Transaction& cTrans_,
									   const Name& cPrevTableName_,
									   const Name& cPostTableName_,
									   const Name& cPrevName_,
									   const Name& cPostName_,
									   bool bUndo_ = false,
									   bool bRecovery_ = false);
												// 親オブジェクトの名称変更により
												// ファイルを移動する

	// modify fileID according to alter column
	void					alterField(Trans::Transaction& cTrans_,
									   Field* pField_);

	// ファイルに登録されるフィールドを表すクラスに関する操作

	SYD_SCHEMA_FUNCTION
	const ModVector<Field*>& getField(Trans::Transaction& cTrans_) const;
												// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Field*>		getField(Field::Category::Value
														fieldCategory,
									 Trans::Transaction& cTrans_) const;
												// ある種別の登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Field*>		getField(const Field& source,
									 Trans::Transaction& cTrans_) const;
												// あるフィールドをsourceとする
												// フィールドの取得
	SYD_SCHEMA_FUNCTION
	Field*					getFieldByID(ID::Value iFieldID_,
										 Trans::Transaction& cTrans_) const;
#ifdef OBSOLETE // フィールド名からフィールドオブジェクトを得ることはない
	SYD_SCHEMA_FUNCTION
	Field*					getField(const Name& fieldName,
									 Trans::Transaction& cTrans_) const;
#endif
	SYD_SCHEMA_FUNCTION
	Field*					getFieldByPosition(Field::Position fieldPosition,
											 Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Field*					getField(Field::Function::Value function,
									 ID::Value iColumnID_,
									 Trans::Transaction& cTrans_) const;
												// ある登録の取得
	SYD_SCHEMA_FUNCTION
	const ModVector<Field*>& getSkipCheckKey(Trans::Transaction& cTrans_) const;
												// 挿入をスキップするか検査するフィールド
	SYD_SCHEMA_FUNCTION
	const ModVector<Field*>& getPutKey(Trans::Transaction& cTrans_) const;
												// 更新/削除のキーとなるフィールド

	const FieldPointer&		addField(const FieldPointer& field,
									 Trans::Transaction& cTrans_);
												// 登録の追加
	FieldPointer			addField(Column& column,
									 Trans::Transaction& cTrans_,
									 ObjectID::Value iID_ = ObjectID::Invalid);
												// ある列の値を格納する
												// 登録の追加
	FieldPointer			addField(Field::Category::Value category,
									 Field::Permission::Value permission,
									 Field& source,
									 Trans::Transaction& cTrans_,
									 ObjectID::Value iID_ = ObjectID::Invalid);
	FieldPointer			addField(Field::Category::Value category,
									 Field::Permission::Value permission,
									 Field& source, Column& column,
									 Trans::Transaction& cTrans_,
									 ObjectID::Value iID_ = ObjectID::Invalid);
												// あるフィールドを派生元とする
												// 登録の追加
	FieldPointer			addField(Key& cKey_,
									 Field::Permission::Value permission,
									 Trans::Transaction& cTrans_,
									 ObjectID::Value iID_ = ObjectID::Invalid);
												// あるキーに対応する登録の追加
	FieldPointer			addField(Field::Function::Value function,
									 Column& column, Trans::Transaction& cTrans_);
												// ある関数を表すフィールドの
												// 登録の追加
	// add a field copying an original field
	FieldPointer			addField(Field& field,
									 Trans::Transaction& cTrans_,
									 ObjectID::Value iID_ = ObjectID::Invalid);

#ifdef OBSOLETE // FieldのdoAfterPersistで使用しているがそれがOBSOLETEなのでこれもOBSOLETEにする
	void					eraseField(ID::Value fieldID);
												// 登録の抹消
#endif
	void					resetField();
	void					resetField(Database& cDatabase_);
												// 全登録の抹消
	void					clearField(Trans::Transaction& cTrans_);
												// 全登録を抹消し、
												// 管理用のベクターを破棄する

	virtual void			checkFieldType(Trans::Transaction& cTrans_);
												// Check the validity of FileID contents and modify if needed
	virtual ModSize			createVirtualField(Trans::Transaction& cTrans_);
												// 読み込み専用の仮想列を追加する

	SYD_SCHEMA_FUNCTION
	Field*					getObjectID(Trans::Transaction& cTrans_,
										bool bForPut_ = false) const;
												// オブジェクト ID を
												// 格納するフィールドを得る

	SYD_SCHEMA_FUNCTION
	ModVector<Field::Position>
							getPosition(Field::Category::Value fieldCategory,
										Trans::Transaction& cTrans_) const;
												// ファイルを構成する
												// ある種別のフィールドの
												// ファイルの先頭から位置を得る

	SYD_SCHEMA_FUNCTION
	bool					isCompoundIndex(Trans::Transaction& cTrans_) const;
												// 2つ以上のキーを持つか

	SYD_SCHEMA_FUNCTION
	virtual bool			isKeyUnique() const;
												// ファイルに格納されるデータが
												// キーについて一意かどうか
	SYD_SCHEMA_FUNCTION
	virtual bool			isKeyNotNull(Trans::Transaction& cTrans_) const;
												// すべてのキーにNotNull制約がついているか
	SYD_SCHEMA_FUNCTION
	virtual bool			hasAllTuples() const;
												// ファイルに格納されるデータが
												// すべてのタプルに対応しているか
	SYD_SCHEMA_FUNCTION
	virtual bool			isKeyGenerated() const;
												// ファイルのキーは挿入時に生成されるか
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToScan(bool bAllTuples_) const;
												// 順次取得が可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToFetch() const;
												// キーを指定したFetchによる取得が可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToSearch(const LogicalFile::TreeNodeInterface& pCond_) const;
												// 条件を指定した検索結果の取得が可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToGetByBitSet() const;
												// 取得がRowIDのみのときBitSetによる取得が可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToSearchByBitSet() const;
												// 取得対象のRowIDを渡して絞込み検索が可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToUndo() const;
												// 削除や挿入のUndoはドライバーが行うか
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToSort() const;
												// キー順の取得が可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToBitSetSort() const;
												// BitSetによるソートが可能か

	SYD_SCHEMA_FUNCTION
	virtual bool			isGettable(Trans::Transaction& cTrans_,
									   const Field* pField_,
									   const LogicalFile::TreeNodeInterface* pScalarField_) const;
	                                            // 値を取得可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isAbleToVerifyTuple() const;
												// タプル単位の整合性検査が可能か
	SYD_SCHEMA_FUNCTION
	virtual bool			isHasFunctionField(Schema::Field::Function::Value eFunction_) const;
	SYD_SCHEMA_FUNCTION
	virtual bool			isHasFunctionField(LogicalFile::TreeNodeInterface::Type eFunction_) const;
												// 関数フィールドがあるか
	SYD_SCHEMA_FUNCTION
	virtual SkipInsertType::Value
							getSkipInsertType() const;
												// 挿入しないデータの種別を得る
	SYD_SCHEMA_FUNCTION
	virtual ModVector<Field*>
							getFetchKey(Trans::Transaction& cTrans_) const;
												// Fetchに使うフィールドを得る
	SYD_SCHEMA_FUNCTION
	virtual Name			createName(Trans::Transaction& cTrans_, const Name& cParentName_);
												// ファイルの名前を生成する

	SYD_SCHEMA_FUNCTION
	const Hint*				getHint() const;	// ファイルのヒントを得る
	void					clearHint();		// ファイルのヒントを消去

	SYD_SCHEMA_FUNCTION
	const Hint*				getAreaHint() const;// エリアのヒントを得る
	void					clearAreaHint();	// エリアのヒントを消去

	// このオブジェクトを親オブジェクトとするオブジェクトのロード

	SYD_SCHEMA_FUNCTION
	const FieldMap&			loadField(Trans::Transaction& cTrans_,
									  bool bRecovery_ = false);
												// フィールドを読み出す
	// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	virtual int				getClassID() const;

	// 論理ログ出力用のメソッド
	void					makeLogData(Trans::Transaction& cTrans_,
										Common::DataArrayData& cLogData_) const;
												// ログデータを作る

	// get field objectid from logdata
	ID::Value				getNextFieldID(Trans::Transaction& cTrans_,
										   const Common::DataArrayData* pLogData_ = 0) const;

	virtual AccessFile*		getAccessFile(Trans::Transaction& cTrans_) const;
												// ファイルに対応するAccessFileを得る

	SYD_SCHEMA_FUNCTION
	static bool				setAvailability(Object::ID::Value dbID, Object::ID::Value fileID, bool v);
												// 論理ファイルの利用可能性を設定する

	SYD_SCHEMA_FUNCTION
	static bool				setAvailability(const Lock::FileName& lockName, bool v);
												// 論理ファイルの利用可能性を設定する

	SYD_SCHEMA_FUNCTION
	static bool				isAvailable(Object::ID::Value dbID, Object::ID::Value fileID);
												// 論理ファイルが利用可能か調べる

	SYD_SCHEMA_FUNCTION
	static bool				isAvailable(const Lock::FileName& lockName);
												// 論理ファイルが利用可能か調べる

	// importやreflectにおいてキー値がファイルに挿入できるものかを判断する
	bool					isKeyImportable(const Common::Data::Pointer& pKey_) const;

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

protected:
	// コンストラクター
	File(const Database& database, Table& table, Category::Value category,
		 const Hint* pHint_, const Hint* pAreaHint_);
	File(const Database& database, Table& table, Index& index,
		 Category::Value category, const Hint* pHint_, const Hint* pAreaHint_);

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

	void					setAreaPath(LogicalFile::FileID& cFileID_,
										Trans::Transaction& cTrans_);
	void					setAreaPath(LogicalFile::FileID& cFileID_,
										const ModVector<ModUnicodeString>& vecPath_,
										const Os::Path& cPathPart_);
	void					setAreaPath(Trans::Transaction& cTrans_,
										Database* pDatabase_);
												// ファイルIDのエリア部分を
												// 作る

	void					setDatabaseIDInFileID();
												// ファイルIDのデータベースID部分を作る

	void					setFieldTypeToFileID(LogicalFile::FileID& cFileID_, Field* pField_, ModSize index_,
												 Trans::Transaction& cTrans_);
												// FileIDにフィールドの型情報をセットする
	const LogicalFile::FileID&
							setFileID(const LogicalFile::FileID& fileID);
												// ファイル ID を設定する

	// 以下のメソッドはデータベースファイル化に伴い追加された

	virtual Common::Data::Pointer
							packOption() const;	// サブクラスの付加情報を
												// Common::Dataにする
	virtual void			unpackOption(const Common::Data& cData_);
												// サブクラスの付加情報に
												// Common:::Dataを反映する

private:
	void					destruct();			// デストラクター下位関数

	void					setAreaID(const Table& cTable_, Trans::Transaction& cTrans_);
	void					setAreaID(const Index& cIndex_, Trans::Transaction& cTrans_);
												// 親オブジェクトの指定からエリアIDを設定する
	void					checkUndo(const Database& database, const Table& cTable_,
									  Trans::Transaction& cTrans_);
	void					checkUndo(const Database& database, const Index& cIndex_,
									  Trans::Transaction& cTrans_);
												// Undo情報を検査して反映する

	Common::Data::Pointer	packBinaryMetaField(int iMemberID_) const;
	bool					unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_);

	// 以下のメンバーは、「ファイル」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	Category::Value			_category;			// ファイルの種別
	LogicalFile::FileID*	m_pFileID;			// ファイル ID

	// ファイルを使う表を表すクラス
	// (スキーマオブジェクト IDは親オブジェクトIDである)

	mutable Table*			_table;

	// ファイルを使う索引を表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Index*			_index;
	ID::Value				_indexID;

	Hint*					m_pHint;			// ファイルに対するヒント
	Hint*					m_pAreaHint;		// ファイルに対するヒント

	// ファイルを格納するエリアを表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Area*			_area;
	ID::Value				_areaID;

	// 物理ログファイルを格納するエリアを表すクラスと、
	// そのスキーマオブジェクト ID

	mutable Area*			_logArea;
	ID::Value				_logAreaID;

	mutable Os::Path*		m_pPathPart;		// ファイルを格納する
												// ディレクトリパス名の
												// ファイル固有部分

	// 以下のメンバーは、「フィールド」表を検索して得られる

	mutable	FieldMap*		_fields;
												// ファイルに属する
												// フィールドを表すクラスを
												// 管理するハッシュマップ

	mutable ModVector<Field*>* m_pvecSkipCheckKey; // getSkipCheckKeyのキャッシュ
	mutable ModVector<Field*>* m_pvecPutKey;	   // getPutKeyのキャッシュ

	FileReflect*			m_pFileReflect;		// import/tupleUndo/tupleRedoで用いる

	bool					m_bCreated;			// ファイルの実体が作成されているか
	bool					m_bImmediateDestroy; // DROP時に即座にOSファイルを破棄するか
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_FILE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
