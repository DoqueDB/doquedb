// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Area.h -- エリア関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_AREA_H
#define	__SYDNEY_SCHEMA_AREA_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "Common/Hasher.h"
#include "Common/UnicodeString.h"

#include "Statement/AreaOption.h"

#include "ModCharString.h"
#include "ModHashMap.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class AlterAreaAction;
	class AlterAreaStatement;
	class AreaDefinition;
	class AreaElementList;
	class DropAreaStatement;
}
namespace Trans
{
	class Transaction;
	class TimeStamp;
}
namespace Os
{
	class Path;
}

_SYDNEY_SCHEMA_BEGIN

class AreaContent;
class AreaContentMap;
class Database;
class LogData;
namespace SystemTable
{
	class Area;
	class AreaContent;
}
namespace Utility
{
	class BinaryData;
}

//	CLASS
//	Schema::Area -- エリアオブジェクトを表すクラス
//
//	NOTES
//		エリアの親オブジェクトは存在しない

class Area
	: public Schema::Object
{
public:
	friend class SystemTable::Area;
	friend class SystemTable::AreaContent;

	//	TYPEDEF public
	//	Schema::Area::Pointer -- Areaを保持するObjectPointer
	//
	//	NOTES
	typedef AreaPointer Pointer;

	struct Log {


		//	STRUCT
		//		Schema::Log::Value -- ログの共通要素位置
		//
		//	NOTES
		enum Value {
			Name = 0,						// エリア名
			ID,								// ID
			Path,							// パス(または変更前パス)
			ValueNum
		};

		//	STRUCT
		//		Schema::Log::Create -- Create ログの要素位置
		//
		//	NOTES
		struct Create {
			enum Value {
				Num = ValueNum
			};
		};

		//	STRUCT
		//		Schema::Log::Drop -- Drop ログの要素位置
		//
		//	NOTES
		struct Drop {
			enum Value {
				Num = ValueNum
			};
		};

		//	STRUCT
		//		Schema::Log::Alter -- Alter ログの要素位置
		//
		//	NOTES
		struct Alter {
			enum Value {
				PrevPath = Path,			// 変更前パス
				PostPath,					// 変更後パス
				Num
			};
		};
	};


	////////////////////
	// Areaのメソッド //
	////////////////////

	// コンストラクター
	Area();
	// デストラクター
	SYD_SCHEMA_FUNCTION
	~Area();

	static Area*			getNewInstance(const Common::DataArrayData& cData_);
												// DataArrayDataを元にインスタンスを生成する

	void					clear();			// メンバーをすべて初期化する

	static Area*			create(Database& cDatabase_,
								   const Statement::AreaDefinition& cStatement_,
								   LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	static Area*			create(Trans::Transaction& cTrans_,
								   const Database& cDatabase_,
								   const LogData& cLogData_);
	SYD_SCHEMA_FUNCTION
	static Area*			createForMount(Trans::Transaction& cTrans_,
										   Database& cDatabase_,
										   ID::Value iID_,
										   const Name& name_,
										   const ModVector<ModUnicodeString>& vecPath_);
#ifdef OBSOLETE
	void					create(Trans::Transaction& cTrans_);
#endif
												// エリアを生成する

	static Name				getName(const Statement::DropAreaStatement& cStatement_);
	static Name				getName(const Statement::AlterAreaStatement& cStatement_);
												// SQL文から対象のエリア名を得る

	static void				drop(Area& cArea_, LogData& cLogData_,
								 Trans::Transaction& cTrans_);
												// エリアの破棄処理をする
	void					drop(Trans::Transaction& cTrans_,
								 bool bRecovery_ = false,
								 bool bCheck_ = true);
	SYD_SCHEMA_FUNCTION
	void					dropForMount(Trans::Transaction& cTrans_);
												// エリアに抹消マークをつける

	static bool				alter(Trans::Transaction& cTrans_,
								  Area& cArea_,
								  const Statement::AlterAreaStatement& cStatement_,
  								  ModVector<ModUnicodeString>& vecPrevPath_,
  								  ModVector<ModUnicodeString>& vecPostPath_,
								  LogData& cLogData_);
												// エリア変更の準備をする

	void					move(Trans::Transaction& cTrans_,
								 const ModVector<ModUnicodeString>& vecPrevPath_,
								 const ModVector<ModUnicodeString>& vecPostPath_,
                                 bool bUndo_ = false,
								 bool bRecovery_ = false,
								 bool bMount_ = false);
												// エリアを変更する
	SYD_SCHEMA_FUNCTION
	void					moveForMount(
								  Trans::Transaction& cTrans_,
								  const ModVector<ModUnicodeString>& vecPrevPath_,
								  const ModVector<ModUnicodeString>& vecPostPath_,
								  bool bUndo_ = false,
								  bool bRecovery_ = false);
												// mountのエリア変更文を適用する

	SYD_SCHEMA_FUNCTION
	static Area*			get(ID::Value id, Database* pDatabase_,
								Trans::Transaction& cTrans_);
												// エリアを表すクラスを得る

	static void				doBeforePersist(const Pointer& pArea_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);
												// 永続化前に行う処理

	static void				doAfterPersist(const Pointer& pArea_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);
												// 永続化後に行う処理

	// システム表からの読み込み前に行う処理
	static void				doAfterLoad(const Pointer& pArea_,
										Database& cDatabase_,
										Trans::Transaction& cTrans_);

//	Object::
//	Timestamp				getTimestamp() const; // タイムスタンプを得る
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
	SYD_SCHEMA_FUNCTION
	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

	void					reset(Database& cDatabase_);
	void					reset();
												// 下位オブジェクトを抹消する

	// エリアが対応するパスを得る
	SYD_SCHEMA_FUNCTION
	const ModVector<ModUnicodeString>&
							getPath() const;
	// パス名配列をセットする
	void					setPath(const ModVector<ModUnicodeString>& cPathArray_);
	SYD_SCHEMA_FUNCTION

	const ModUnicodeString& getPath(int iPosition_) const;
												// パス配列の指定位置にある
												// パスを得る

	const ModUnicodeString& addPath(const ModUnicodeString& cstrPath_);
												// エリアが対応するパスを
												// 追加する

	void					resetPath();		// すべてのパスを抹消する
	void					clearPath();		// すべてのパスを抹消し、
												// 管理用のベクターも破棄する

	SYD_SCHEMA_FUNCTION
	bool					setMovePrepare(const int iType_,
										   const Statement::AreaElementList& cStatement_,
										   ModVector<ModUnicodeString>& vecPrevPath_,
										   ModVector<ModUnicodeString>& vecPostPath_);
												// move の為の準備をする

	SYD_SCHEMA_FUNCTION
	ModSize					getSize() const;	// パス配列の要素数を得る

	void					destroy(Trans::Transaction& cTrans_,
									const ModUnicodeString& cPathPart_,
									bool bForce_ = true);
	static void				destroy(Trans::Transaction& cTrans_,
									const ModVector<ModUnicodeString>& vecPath_,
									const ModUnicodeString& cPathPart_,
									bool bForce_ = true);
												// エリアに格納されている
												// あるディレクトリーを破棄する
	void					undoDestroy(Trans::Transaction& cTrans_,
										const ModUnicodeString& cPathPart_);
												// ディレクトリーの破棄を取り消す

	// 格納関係に関する操作
	SYD_SCHEMA_FUNCTION
	AreaContent*			getContent(ID::Value iObjectID_,
									   AreaCategory::Value eCategory_,
									   Trans::Transaction& cTrns_) const;
												// 指定したオブジェクトと
												// 種別を持つ登録の取得
	const AreaContentPointer&
							addContent(const AreaContentPointer& content,
									   Trans::Transaction& cTrans_);
												// 登録の追加
	void					eraseContent(ID::Value iID_);
												// 登録の抹消
	void					resetContent();
												// 全登録の抹消
	void					clearContent();
												// 全登録を抹消し、
												// 管理用のベクターを破棄する

	// 格納関係オブジェクトのロード

	SYD_SCHEMA_FUNCTION
	const AreaContentMap&	loadContent(Trans::Transaction& cTrans_,
										bool bRecovery_ = false);
												// 格納関係を読み出す

	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	SYD_SCHEMA_FUNCTION
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	// 再構成用のメソッド
	SYD_SCHEMA_FUNCTION
	bool					checkPath(Trans::Transaction& cTrans_,
									  const ModVector<ModUnicodeString>* vecPath_ = 0,
									  bool bEraseExistence_ = false,
									  bool bNeedExistence_ = false);
												// パスが他のデータベースや
												// エリアに使われていないか調べる

	// 論理ログ出力用のメソッド
	void					makeLogData(LogData& cLogData_) const;
												// ログデータを作る
	static Common::Data::Pointer
							getPathArray(Trans::Transaction& cTrans_,
										 const Database& cDatabase_,
										 const ModVector<ID::Value>& vecID_);
												// ID配列からログに出すためのパスデータを得る
	static bool				getPathArray(const Common::DataArrayData& cData_,
										 int iIndex_,
										 ModVector<ModUnicodeString>& vecPath_);
												// ログから指定した位置のパスデータを得る

	// ログデータからさまざまな情報を取得する
	static ID::Value		getObjectID(const LogData& log_);
												// ログデータより AreaID を取得する
	static Name				getName(const LogData& cLogData_);
												// ログデータから対象のエリア名を得る
	static void 			getPath(const LogData& cLogData_,
									int iIndex_,
									ModVector<ModUnicodeString>& vecPath_);
												// ログデータからパス指定を得る

	// 論理ログ出力、REDOのためのメソッド
	// pack、unpackの下請けとしても使用される

	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);
protected:
private:
	// コンストラクター
	Area(const Database& cDatabase_, const Statement::AreaDefinition& cStatement_);
	Area(const Database& cDatabase_, const Object::Name& cAreaName_,
		 const ModVector<ModUnicodeString>& cPathArray_);
	Area(const Database& cDatabase_, const LogData& cLogData_);

//	Object::
//	void					addTimestamp();		// タイムスタンプを進める

	void					prepareArray(ModSize iSize_);
												// パス名配列の要素数を確保
	void					setPath(const Statement::AreaElementList& cStmt_);
												// SQL構文要素からパス名配列を
												// セットする
	void					moveFile(Trans::Transaction& cTrans_,
									 const ModVector<ModUnicodeString>& vecPrevPath_,
									 const ModVector<ModUnicodeString>& vecPostPath_,
                                     bool bUndo_=false,
									 bool bRecovery=false,
									 bool bMount_ = false);
												// エリアの定義変更にともない
												// 関係するファイルを移動する
#ifdef OBSOLETE
	static void				prepareMove(Trans::Transaction& cTrans_,
										const ModVector<ModUnicodeString>& vecPrevPath_,
										const ModVector<ModUnicodeString>& vecPostPath_,
										bool bUndo_=false);
												// エリアの移動の準備
#endif
	static void				sweepMove(Trans::Transaction& cTrans_,
									  const ModVector<ModUnicodeString>& vecPrevPath_,
									  const ModVector<ModUnicodeString>& vecPostPath_,
									  bool bUndo_=false);
												// エリアの移動の後始末

	void					destruct();			// デストラクター下位関数

	void					checkUndo(const Database& database, ID::Value iID_);
												// Undo情報を検査して反映する

	// 以下のメンバーは、「エリア」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別

	mutable ModVector<ModUnicodeString>*
							m_pPathArray;		// 対応するパス配列

	// 以下のメンバーは、「格納関係」表を検索して得られる

	mutable AreaContentMap*	_contents;			// エリアに関係する
												// 格納関係を表すクラスを
												// 管理するハッシュマップ
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_AREA_H

//
// Copyright (c) 2000, 2001, 2002, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
