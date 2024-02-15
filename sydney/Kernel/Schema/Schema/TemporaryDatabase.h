// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// TemporaryDatabase.h -- データベースオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2005, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_TEMPORARYDATABASE_H
#define	__SYDNEY_SCHEMA_TEMPORARYDATABASE_H

#include "Schema/Database.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

//	ENUM
//	Schema::TemporaryDatabase -- 一時データベースオブジェクトを表すクラス
//
//	NOTES

class TemporaryDatabase
	: public	Database
{
public:
	TemporaryDatabase(Trans::Transaction& cTrans_);
												// コンストラクター
	~TemporaryDatabase();						// デストラクター

//	Database::
//	void					create(Trans::Transaction& cTrans_);
//												// データベースを生成する
//	void					drop(Trans::Transaction& cTrans_);
//												// データベースを抹消する
//	void					destroy(Trans::Transaction& cTrans_,
//									bool bForce_ = true);
//												// データベースを構成する
//												// ファイルとそれを格納する
//												// ディレクトリーを破棄する
//	const Trans::Log::File&	getLogFile(Trans::Transaction& cTrans_) const;
//												// 論理ログファイルの
//												// 記述子を得る

	static void				initialize();		// 一時データベースモジュールを初期化する
	static void				terminate();		// 一時データベースモジュールの後処理をする

	static Database*		create(Trans::Transaction& cTrans_);
												// 一時データベースを生成する
	static Database*		get(Trans::Transaction& cTrans_);
												// 一時データベースを得る

	static void				drop(Server::SessionID iSessionID_);
												// 一時データベースを抹消する

//	Object::
//	ID::Value				getID() const;		// オブジェクト ID を得る
//	ID::Value				getParentID() const;
//												// 親オブジェクトの
//												// オブジェクト ID を得る
//	const Name&				getName() const;	// オブジェクトの名前を得る
//	Category::Value			getCategory() const;
//												// オブジェクトの種別を得る

//	Database::
//	// データベースが保持するオブジェクトに関する操作
//
//	void					reset(Trans::Transaction& cTrans_);
//												// 下位オブジェクトを抹消する
//
//	// データベースに登録される表を表すクラスに関する操作
//
//	ModVector<Table*>		getTable(Trans::Transaction& cTrans_) const;
//												// すべての登録の取得
//	Table*					getTable(ID::Value tableID,
//									 Trans::Transaction& cTrans_) const;
//	Table*					getTable(const Name& tableName,
//									 Trans::Transaction& cTrans_) const;
//												// ある登録の取得
//	Table&					addTable(Table& table,
//									 Trans::Transaction& cTrans_);
//												// 登録の追加
//	void					eraseTable(ID::Value tableID);
//												// 登録の抹消
//	void					resetTable();
//	void					resetTable(Trans::Transaction& cTrans_);
//												// 全登録の抹消
//	void					clearTable(Trans::Transaction& cTrans_);
//												// 全登録を抹消し、
//												// 管理用のベクターを破棄する
//
//	// 表以下のオブジェクトをキャッシュするマップに関する操作
//
//	Object*					getCache(ID::Value id,
//									 Category::Value category =
//														  Category::Unknown);
//												// 指定されたオブジェクトIDの
//												// オブジェクトを得る
//	Object&					addCache(Object& object);
//												// 指定されたオブジェクトを
//												// キャッシュに追加する
//	void					eraseCache(ID::Value id);
//												// 指定されたオブジェクトIDの
//												// オブジェクトをキャッシュから
//												// 除く
//	void					resetCache();		// 全登録の抹消
//	void					clearCache();		// 全登録を抹消し、
//												// 管理用のベクターを破棄する
//#ifdef DEBUG
//	void					checkCache() const;	// キャッシュの内容がすべて
//												// Persistかチェックする
//#endif
//
//	// このオブジェクトを親オブジェクトとするオブジェクトのロード
//
//	const TableMap&			loadTable(Trans::Transaction& cTrans_);
//												// 表を読み出す
//
//	virtual void			serialize(ModArchive& archiver);
//												// このクラスをシリアル化する
//	virtual int				getClassID() const;	// このクラスのクラス ID を得る

protected:
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

//	Database::
//	void					setLogFilePath(Trans::Transaction& cTrans_,
//										   bool bMakeDirectory_ = false);
//												// 論理ログファイルのパス名を設定する

private:
	void					destruct();			// デストラクター下位関数

	// 以下のメンバーは、コンストラクト時にセットされる
//	Object::

	// 以下のメンバーは、「データベース」表を検索して得られる

//	Object::
//	ID::Value				_id;				// オブジェクト ID
//	ID::Value				_parent;			// 親オブジェクトの
//												// オブジェクト ID
//	Name					_name;				// オブジェクトの名前
//	Category::Value			_category;			// オブジェクトの種別
//
//	Database::
//	mutable TableMap*		_tables;			// データベースを構成する
//												// 表を表すクラスを
//												// 管理するハッシュマップ
//	mutable CacheMap*		_cache;				// データベースを構成する
//												// オブジェクトのキャッシュを
//												// 管理するハッシュマップ
//	Common::PathName*		m_pLogFilePath;		// データベースに対応するログファイルのパス名
//	mutable Trans::Log::File*
//							m_pLogFile;			// データベースに対応するログファイル記述子
};

//	FUNCTION public
//	Schema::TemporaryDatabase::~TemporaryDatabase -- データベースを表すクラスのデストラクター
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

inline
TemporaryDatabase::
~TemporaryDatabase()
{
	destruct();
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_TEMPORARYDATABASE_H

//
// Copyright (c) 2000, 2001, 2005, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
