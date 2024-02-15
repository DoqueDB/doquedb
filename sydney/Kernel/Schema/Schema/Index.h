// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Index.h -- 索引オブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_INDEX_H
#define	__SYDNEY_SCHEMA_INDEX_H

#include "Schema/Module.h"
#include "Schema/Object.h"
#include "Schema/AreaCategory.h"
#include "Schema/File.h"
#include "Schema/Key.h"
#include "Schema/Hint.h"

#include "Admin/Verification.h"
#include "Common/Hasher.h"
#include "Os/Path.h"
#include "Statement/AreaOption.h"
#include "Statement/IndexDefinition.h"

#include "ModHashMap.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class IndexDefinition;
	class AlterIndexAction;
	class AlterIndexStatement;
	class DropIndexStatement;
}

namespace Trans
{
	class Transaction;
	class TimeStamp;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class Table;
class Column;
class Constraint;
class Field;
class LogData;
class KeyMap;
namespace SystemTable
{
	class Index;
	class Key;
}
namespace Utility
{
	class BinaryData;
}

//	CLASS
//	Schema::Index -- インデックスオブジェクトを表すクラス
//
//	NOTES
//		実際の索引を現すクラスの親クラスだが、
//		システム表に関するメソッドはこのクラスのオブジェクトを通して
//		操作されるので仮想クラスにしてはいけない

class Index
	: public	Object
{
public:
	friend class SystemTable::Index;
	friend class SystemTable::Key;

	using Object::getName;
	using Object::getDatabaseID;

	//	CLASS
	//	Schema::Index::Category -- 索引の種別を表すクラス
	//
	//	NOTES
	//		このクラスを直接、使用することはない
	//		Value のためのスコープを用意するために定義している

	struct Category
	{
		//	ENUM
		//	Schema::Index::Category::Value -- 索引の種別の値を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =		0,					// 不明
			Normal,								// 通常、すなわち B+ 木索引
			FullText,							// 全文索引
			Bitmap,								// ビットマップ索引
			Array,								// 配列索引
			KdTree,								// KdTree索引
			ValueNum							// 種別数
		};
	};

	struct Log {

		//	ENUM
		//		Schema::Log::Value -- ログの共通要素位置
		//
		//	NOTES
		enum Value {
			TableID = 0,				// テーブルID
			ID,							// ID
			Name,						// インデックス名
			Category,					// 種別
			DatabaseID,					// データベースID(Undo用)
			DatabasePaths,				// データベースのパス指定(Undo用)
			TableName,					// 表名(Undo用)
			FileName,					// ファイル名(Undo用)
			ValueNum
		};

		struct Create {
			//	ENUM
			//		Schema::Log::Create::Value -- Create ログの要素位置
			//
			//	NOTES
			enum Value {
				Flag = ValueNum,			// フラグ
				Hint,						// ヒント
				AreaHint,					// エリアヒント
				KeyDefinitions,				// キー定義配列
				AreaIDs,					// エリアID配列
				TableAreaIDs,				// 表のエリアID割り当て配列
				AreaPaths,					// エリアパス配列(Undo用)
				Num0,
				FileDefinition = Num0,		// ファイル定義
				LastID,						// ID最大値
				Num1,
				Num = Num1
			};
		};

		struct Drop {
			//	ENUM
			//		Schema::Log::Drop::Value -- Drop ログの要素位置
			//
			//	NOTES
			enum Value {
				AreaPaths = ValueNum,		// エリアパス配列(Undo用)
				Num
			};
		};

		struct Alter {
			//	ENUM
			//		Schema::Log::Alter::Value -- Alter ログの要素位置
			//
			//	NOTES
			enum Value {
				PrevAreaID = ValueNum,		// 変更前エリアID配列(指定)
				PostAreaID,					// 変更後エリアID配列(指定)
				PrevEffectiveAreaID,		// 表のエリアID割り当て配列も加味したエリアID
				PostEffectiveAreaID,		// 表のエリアID割り当て配列も加味したエリアID
				PrevAreaPath,				// 変更前エリアパス配列
				PostAreaPath,				// 変更後エリアパス配列
				FileID,						// ファイルのスキーマオブジェクトID
				Num
			};
		};

		struct Rename {
			//	ENUM
			//		Schema::Log::Rename::Value -- Alter ログの要素位置
			//
			//	NOTES
			enum Value {
				PrevName = Name,			// 変更前名称
				PrevFileName = FileName,	// 変更前ファイル名称
				PostName = ValueNum,		// 変更後名称
				PostFileName,				// 変更後ファイル名称
				EffectiveAreaID,			// 操作時のエリア指定ID配列(Undo用)
				AreaPaths,					// 操作時のエリアパス配列(Undo用)
				FileID,						// ファイルのスキーマオブジェクトID
				Num
			};
		};

		struct Constraint {
			//	ENUM
			//		Schema::Log::Constraint::Value -- Constraint索引ログの要素位置
			//
			//	NOTES
			enum Value {
				ID = 0,						// 索引ID
				KeyIDs,						// キーID配列
				FileDefinition,				// ファイル定義
				Num
			};
		};
	};

	//	TYPEDEF public
	//	Schema::Index::Pointer -- Indexを保持するObjectPointer
	//
	//	NOTES

	typedef IndexPointer Pointer;

	// コンストラクター
	Index(Category::Value eCategory_ = Category::Unknown);
	// デストラクター
	virtual ~Index();

	// メンバーをすべて初期化する
	void					clear();

	// 新たなIndexオブジェクトを得る
	static Index*			getNewInstance(const Common::DataArrayData& cData_);
	static Index*			getNewInstance(const Database& database, Table& table,
										   const Name& cName_,
										   int iIndexType_,
										   Hint* pHint_);
	static Index*			getNewInstance(const Database& database, Table& table,
										   const Constraint& cConstraint_);
	static Index*			getNewInstance(const Database& database, const LogData& cLogData_);

	// データベースに索引を生成する
	static Pointer			create(Trans::Transaction& cTrans_,
								   Database& database,
								   const Statement::IndexDefinition& cStatement_,
								   LogData& cLogData_);
	static Pointer			create(Trans::Transaction& cTrans_,
								   Table& table, const Constraint& constraint);
	static Pointer			create(Trans::Transaction& cTrans_,
								   const Database& database, const LogData& clogData_);
	static Pointer			create(Trans::Transaction& cTrans_,
								   Table& table, const Constraint& constraint,
								   const Common::DataArrayData& cLogData_);

	// 索引を抹消する
	static void				drop(Trans::Transaction& cTrans_, Index& cIndex_, LogData& cLogData_);
	void					drop(Trans::Transaction& cTrans_,
								 bool bRecovery_ = false, bool bNoUnset_ = false);
	// 索引の破棄マークをクリアする
	void					undoDrop(Trans::Transaction& cTrans_);
	// 索引を構成するファイルやそれを格納するディレクトリーを破棄する
	void					destroy(Trans::Transaction& cTrans_,
									ModHashMap<ID::Value, Area*, ModHasher<ID::Value> >*
																pAreaMap_ = 0,
									bool bForce_ = true);

	// 索引定義を変更する
	static bool				alterArea(Trans::Transaction& cTrans_,
									  Index& cIndex_,
									  const Statement::AlterIndexAction& cStmt_,
									  ModVector<ID::Value>& vecPrevArea_,
									  ModVector<ID::Value>& vecPostArea_,
									  LogData& cLogData_);
	static bool				alterName(Trans::Transaction& cTrans_,
									  Index& cIndex_,
									  const Statement::AlterIndexAction& statement,
									  Name& cPostName_,
									  LogData& cLogData_);

	// 定義変更を無効化する
	void					undoAlter(Trans::Transaction& cTrans_);
	// 索引定義の変更に応じてファイルを移動する
	SYD_SCHEMA_FUNCTION
    void					moveArea(Trans::Transaction& cTrans_,
									 const ModVector<ID::Value>& vecPrevArea_,
									 const ModVector<ID::Value>& vecPostArea_,
									 bool bUndo_ = false,
									 bool bRecovery_ = false,
									 bool bMount_ = false);
	// 名前を変更する
	void					rename(const Name& cPostName_);
	// 名前の変更に伴いファイルを移動する
    void					moveRename(Trans::Transaction& cTrans_,
									   const Name& cPrevTableName_,
									   const Name& cPostTableName_,
									   const Name& cPrevName_,
									   const Name& cPostName_,
									   bool bUndo_ = false,
									   bool bRecovery_ = false);

	// SQL文から索引名を得る
	static Name				getName(const Statement::DropIndexStatement& cStatement_);
	static Name				getName(const Statement::AlterIndexStatement& cStatement_);

	// 運用管理系の操作
#ifdef OBSOLETE // 索引に対して独立に以下のメソッドが呼ばれることはない
	void					mount(Trans::Transaction& cTrans_);
	void					unmount(Trans::Transaction& cTrans_);
	void					flush(Trans::Transaction& cTrans_);
	void					startBackup(Trans::Transaction& cTrans_,
										bool bRestorable_ = true);
	void					endBackup(Trans::Transaction& cTrans_);
	void					recover(Trans::Transaction& cTrans_,
									const Trans::TimeStamp& cPoint_,
									const Name& cDatabaseName_);
	void					restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
#endif

	SYD_SCHEMA_FUNCTION
	static Index*			get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Index*			get(ID::Value id, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Index*			get(const Name& name, Database* pDatabase_,
								Trans::Transaction& cTrans_);
												// 索引を表すクラスを得る

	SYD_SCHEMA_FUNCTION
	static bool				isValid(ID::Value iID_, ID::Value iDatabaseID_,
									Timestamp iTime_,
									Trans::Transaction& cTrans_);
												// 陳腐化していないか調べる

	static void				doBeforePersist(const Pointer& pIndex_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pIndex_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pIndex_,
										Table& cTable_,
										Trans::Transaction& cTrans_);

//	Object::
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
//	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る
//	ID::Value				getDatabaseID() const;

	Os::Path				getPathPart(Trans::Transaction& cTrans_) const;
												// オブジェクトごとに固有の
												// パス名部分を得る
#ifdef OBSOLETE // Indexのパスを取得する機能は使用されない
	const Os::Path&			getPath(Trans::Transaction& cTrans_) const;
												// 索引を構成するファイルを
												// 格納するトップディレクトリーの
												// パス名を得る
#endif
	static Os::Path			getPath(const ModVector<ModUnicodeString>& vecDatabasePath_,
									const ModVector<ModUnicodeString>& vecAreaPath_,
									const Name& cDatabaseName_,
									const Name& cTableName_,
									const Name& cFileName_);
												// 索引を構成するファイルを
												// 格納するディレクトリーのパス名を得る

	void					reset(Database& cDatabase_);
												// 下位オブジェクトを抹消する

	SYD_SCHEMA_FUNCTION
	Category::Value			getCategory() const;
												// 索引の種別を得る

	SYD_SCHEMA_FUNCTION
	Table*					getTable(Trans::Transaction& cTrans_) const;
												// 索引をつけた表を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getTableID() const;	// 索引をつけた表の
												// オブジェクトIDを得る

	// 索引を実現するファイルを格納するエリアに関する操作

	SYD_SCHEMA_FUNCTION
	Area*					getArea(AreaCategory::Value eArea_,
									bool bParent_,
									Trans::Transaction& cTrans_) const;
												// ファイルまたは物理ログを
												// 格納するエリアを得る
	bool					prepareSetArea(Trans::Transaction& cTrans_,
                                    	   const Statement::AreaOption& cStatement_,
                                    	   ModVector<ID::Value>& vecPrevArea_,
                                    	   ModVector<ID::Value>& vecPostArea_);
												// ファイルおよび物理ログを
												// 格納するエリアを設定する準備をする
	void					setArea(Trans::Transaction& cTrans_,
                                    const ModVector<ID::Value>& vecPrevArea_,
                                    const ModVector<ID::Value>& vecPostArea_,
                                    bool bUndo_ = false, bool bRecovery_ = false,
									bool bMount_ = false);
												// ファイルおよび物理ログを
												// 格納するエリアを設定する
	void					createAreaContent(Trans::Transaction& cTrans_);
												// エリア格納関係を作る
    bool					prepareDropArea(Trans::Transaction& cTrans_,
                                    		const Statement::AreaOption& cStatement_,
                                     		ModVector<ID::Value>& vecPrevArea_,
                                     		ModVector<ID::Value>& vecPostArea_);
                                                // SQL構文要素から索引ファイルを
                                                // 格納するエリアの設定を解除する準備をする
    void					dropArea(Trans::Transaction& cTrans_,
                                     const ModVector<ID::Value>& vecPrevArea_,
                                     const ModVector<ID::Value>& vecPostArea_,
                                     bool bUndo_ = false, bool bRecovery_ = false,
									 bool bMount_ = false);
                                                // SQL構文要素から索引ファイルを
                                                // 格納するエリアの設定を解除する
	void					moveArea(Trans::Transaction& cTrans_,
									 ID::Value iPrevAreaID_,
									 ID::Value iPostAreaID_,
									 bool bUndo_ = false, bool bRecovery = false, bool bMount_ = false);
												// エリアの割り当て
												// 変更によるファイルの移動

	ID::Value				getAreaID(AreaCategory::Value eArea_) const;

	SYD_SCHEMA_FUNCTION
	ID::Value				getAreaID(AreaCategory::Value eArea_,
									  bool bParent_,
									  Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	void					getAreaID(ModVector<ID::Value>& vecAreaID_) const;
												// ファイルまたは物理ログを
												// 格納するエリアの
												// オブジェクトIDを得る

	static void				getEffectiveAreaID(const ModVector<ID::Value>& vecAreaID_,
											   const ModVector<ID::Value>& vecTableAreaID_,
											   AreaCategory::Value eAreaCategory_,
											   ModVector<ID::Value>& vecEffectiveAreaID_);
												// ファイルまたは物理ログを
												// 格納する場所として実質的に使われるエリアの
												// オブジェクトIDを取得する

	// CategoryをStatementのAreaタイプに変換する
	static Statement::AreaOption::AreaType
							convertAreaCategory(AreaCategory::Value eCategory_);

	// 索引ファイルを格納する エリアの種別を得る
	SYD_SCHEMA_FUNCTION
	virtual AreaCategory::Value getAreaCategory() const;

	// 索引に登録されるキーを表すクラスに関する操作

	SYD_SCHEMA_FUNCTION
	const ModVector<Key*>&	getKey(Trans::Transaction& cTrans_) const;
												// すべての登録の取得
	SYD_SCHEMA_FUNCTION
	ModVector<Key*>			getKey(const Column& cColumn_,
								   Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Key*					getKey(ID::Value keyID,
								   Trans::Transaction& cTrans_) const;
#ifdef OBSOLETE // 名前や位置でキーを得る機能は使用されない
	SYD_SCHEMA_FUNCTION
	Key*					getKey(const Name& keyName,
								   Trans::Transaction& cTrans_) const;
	SYD_SCHEMA_FUNCTION
	Key*					getKey(Key::Position keyPosition,
								   Trans::Transaction& cTrans_) const;
#endif
												// ある登録の取得
	const KeyPointer&		addKey(const KeyPointer& key,
								   Trans::Transaction& cTrans_);
												// 登録の追加
#ifdef OBSOLETE // KeyのdoAfterPersistで使用しているがそれがOBSOLETEなのでこれもOBSOLETEにする
	void					eraseKey(ID::Value keyID);
												// 登録の抹消
#endif
	void					resetKey();
	void					resetKey(Database& cDatabase_);
												// 全登録の抹消
	void					clearKey();			// 全登録を抹消し、
												// 管理用のベクターを破棄する

	// 索引に登録されるファイルを表すクラスに関する操作

	SYD_SCHEMA_FUNCTION
	File*					getFile(Trans::Transaction& cTrans_) const;
												// 索引を実現するファイルの取得
												// ★注意★
												// 索引を実現する
												// ファイルは索引一つにつき
												// 一つであると決まっている
	const FilePointer&		setFile(const FilePointer& pFile_);
												// 索引を実現するファイルの設定
	void					clearFile();		// ファイルの登録を抹消する
	// 索引を実現するファイルのオブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	ID::Value				getFileID() const;
	// 索引を実現するファイルのオブジェクト ID を設定する
	void					setFileID(ID::Value id_);

	SYD_SCHEMA_FUNCTION
	ModVector<Column*>		getColumn(Trans::Transaction& cTrans_) const;
												// 索引のキーにした
												// すべての列を得る
#ifdef OBSOLETE // Fieldを得る機能は使用されない
	SYD_SCHEMA_FUNCTION
	ModVector<Field*>		getSource(Trans::Transaction& cTrans_) const;
												// 索引のキーの値を
												// 格納するフィールドを得る
#endif

	// ユニークか
	SYD_SCHEMA_FUNCTION
	bool					isUnique() const;
	// クラスター化されているか
	SYD_SCHEMA_FUNCTION
	bool					isClustered() const;
	// すべてのタプルを保持するか
	bool					hasAllTuples() const;
	// Offlineか
	bool					isOffline() const;
	// Change online/Offline status
	void					setOffline(bool bSet_ = true);

	// 索引ファイルのヒントを得る
	SYD_SCHEMA_FUNCTION
	const Hint*				getHint() const;
	// 索引ファイルのヒントを設定する
	void					setHint(Hint* pHint);
	// 索引ファイルのヒントを消去
	void					clearHint();

	SYD_SCHEMA_FUNCTION
	const Hint*				getAreaHint() const;// エリアのヒントを得る
	void					clearAreaHint();	// エリアのヒントを消去

	// パスのキャッシュをクリアする
	void					clearPath();

	// このオブジェクトを親オブジェクトとするオブジェクトのロード

	SYD_SCHEMA_FUNCTION
	const KeyMap&			loadKey(Trans::Transaction& cTrans_,
									bool bRecovery_ = false);
												// キーを読み出す

	// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	virtual int				getClassID() const;

	// 整合性検査
	SYD_SCHEMA_FUNCTION
	void					verify(Admin::Verification::Progress& cResult_,
								   Trans::Transaction& cTrans_,
								   Admin::Verification::Treatment::Value eTreatment_);

	// ログデータを作る
	void					makeLogData(Trans::Transaction& cTrans_,
										LogData& cLogData_) const;
	void					makeLogData(Trans::Transaction& cTrans_,
										Common::DataArrayData& cLogData_) const;
	// ログから情報を取得する

	// 種別を得る
	static Category::Value	getCategory(const LogData& cLogData_);
	// エリア種別を得る
	static AreaCategory::Value getAreaCategory(const LogData& cLogData_);
	// ID を得る
	static ObjectID::Value	getObjectID(const LogData& cLogData_);
	// 索引名を得る
	static Name				getName(const LogData& cLogData_);
	// データベースIDを得る
	static ID::Value		getDatabaseID(const LogData& cLogData_);
	// データベースパス指定を得る
	static void				getDatabasePath(const LogData& cLogData_, ModVector<ModUnicodeString>& vecPath_);
	// 表IDを得る
	static ID::Value		getTableID(const LogData& cLogData_);
	// 表名を得る
	static Name				getTableName(const LogData& cLogData_);
	// ファイル名を得る
	static Name				getFileName(const LogData& cLogData_);
	// エリア指定を得る
	static void				getAreaID(const LogData& log_,
									  const unsigned int pos_,
									  ModVector<ID::Value>& vecAreaID_);
	// エリアパス指定を得る
	static bool				getAreaPath(const LogData& cLogData_, int iIndex_, int iCategory_,
										ModVector<ModUnicodeString>& vecAreaPath_);

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

protected:
	// コンストラクター
	Index(Category::Value eCategory_,
		  const Database& database, Table& table,
		  const Name& cName_, Hint* pHint_);
	Index(Category::Value eCategory_,
		  const Database& database, Table& table, const Constraint& constraint);
	Index(Category::Value eCategory_,
		  const Database& database, const LogData& cLogData_);

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

	// 索引のキーオブジェクトを生成する
	virtual void			createKey(Trans::Transaction& cTrans_,
									  Table& cTable_,
									  const Statement::IndexDefinition& cStatement_,
									  Common::DataArrayData& cLogData_);
	virtual void			createKey(Trans::Transaction& cTrans_,
									  Table& table, const Constraint& constraint);
	virtual void			createKey(Trans::Transaction& cTrans_,
									  Table& table, const Constraint& constraint,
									  const Common::DataArrayData& cLogData_);

	// 索引を実現するファイルを生成し、既存のデータを反映する
	virtual FilePointer		createFile(Trans::Transaction& cTrans_);

	// 索引を実現するファイルにフィールドを設定する
	virtual void			createField(Trans::Transaction& cTrans_,
										File& cFile_,
										const Common::DataArrayData* pLogData_ = 0);

private:
	void					destruct();			// デストラクター下位関数

	bool					isTemporary();

	// ファイルまたは物理ログを格納するエリアのオブジェクトIDを設定する
	void					setAreaID(const ModVector<ID::Value>& vecAreaID_);
	// Undo情報を検査して反映する
	void					checkUndo(const Database& database, ID::Value iID_);

	Common::Data::Pointer	packIntegerMetaField(int iMemberID_) const;
	Common::Data::Pointer	packBinaryMetaField(int iMemberID_) const;

	bool					unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_);
	bool					unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_);

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「索引」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	Category::Value			_category;			// 索引の種別

	bool					_clustered;			// クラスター化されているか
	bool					_unique;			// ユニークか
	bool					_hasAllTuples;		// 索引にすべてのタプルが挿入されるか
	bool					m_bOffline;			// 索引がオフラインか

	Hint*					m_pHint;			// 索引ファイルに対するヒント
	Hint*					m_pAreaHint;		// 索引ファイルに対するヒント

	// ファイルを格納するエリアと、
	// そのスキーマオブジェクトID

	mutable Area*			m_pArea;
	ID::Value				m_iAreaID;

	// 物理ログファイルを格納するエリアと、
	// そのスキーマオブジェクトID

	mutable Area*			m_pLogArea;
	ID::Value				m_iLogAreaID;

	// 索引をつけた表を表すクラス
	// ★注意★
	// Indexの親オブジェクトを表にしたのでIDはParentIDである

	mutable Table*			_table;

	// 索引を実現するファイルを表すクラスと、
	// そのスキーマオブジェクトID

	FilePointer				m_pFile;
	ID::Value				m_iFileID;

	// 以下のメンバーは、「キー」表を検索して得られる

	mutable	KeyMap*			_keys;
												// 索引に属する
												// キーを表すクラスを
												// 管理するハッシュマップ

	mutable Os::Path*		m_pPath;			// 索引を格納するパス名
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_INDEX_H

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
