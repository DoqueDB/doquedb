// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ObjectSnapshot.h --
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_OBJECTSNAPSHOT_H
#define	__SYDNEY_SCHEMA_OBJECTSNAPSHOT_H

#include "Schema/Module.h"
#include "Schema/Manager.h"
#ifdef DEBUG
#include "Schema/Message.h"
#endif
#include "Schema/Database.h"
#include "Schema/SessionID.h"

#include "Common/SafeExecutableObject.h"

#include "Os/RWLock.h"

#include "ModVector.h"
#include "ModHashMap.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN
class DatabaseMap;
namespace SystemTable
{
	class Database;
}

//	CLASS
//	Schema::ObjectSnapshot -- オブジェクトスナップショットを保持するクラス
//
//	NOTES
//		Common::ObjectPointerを使いたいためにExecutableObjectにしている

class ObjectSnapshot : public Common::SafeExecutableObject
{
public:
	friend class SystemTable::Database;

	typedef unsigned int ID;

	~ObjectSnapshot();							// デストラクタ

	static void				initialize();
	static void				terminate();

	static ObjectSnapshot*	create(bool bIsNoVersion_ = false);
												// スナップショットを新規に作る
	static ObjectSnapshot*	get(ID id_);		// スナップショットを得る
	static void				erase(ID id_);		// スナップショットを削除する

	void					reset();			// スナップショットが保持する
												// すべてのオブジェクトを
												// 破棄する
	void					clear();			// スナップショットが保持する
												// すべてのオブジェクトを
												// 破棄し、管理用のベクターも
												// 破棄する

	// セッションに関係する操作
	static void				reserveDatabase(SessionID iSessionID_);
	static void				eraseReservation(SessionID iSessionID_);
	void					releaseDatabase(SessionID iSessionID_);

	// システムに登録されるデータベースを表すクラスに関する操作

	const DatabaseMap&		loadDatabase(Trans::Transaction& cTrans_);
												// データベース表の内容を読み込む
	const ModVector<Database*>&
							getDatabase(Trans::Transaction& cTrans_);
												// すべての登録の取得

	Database*				getDatabase(Schema::Object::ID::Value databaseID,
										Trans::Transaction& cTrans_);
	ObjectID::Value			getDatabaseID(const Schema::Object::Name& databaseName,
										  Trans::Transaction& cTrans_);
												// ある登録の取得

	Database&				addDatabase(const Database::Pointer& pDatabase_,
										Trans::Transaction& cTrans_);
												// 登録の追加

	void					eraseDatabase(const Database* database);
	void					eraseDatabase(Schema::Object::ID::Value iDatabaseID_, SessionID iSessionID_ = Server::IllegalSessionID);
												// 登録の抹消
	void					resetDatabase();	// 全登録の抹消

	void					clearDatabase();	// 全登録を抹消し、
												// 管理用のベクターを破棄する

	void					clearDatabaseView();// ベクターに保持されているキャッシュをクリアする

	void					eraseTable(Schema::Object::ID::Value iDatabaseID_,
									   Schema::Object::ID::Value iTableID_);
												// 表の削除
	void					eraseIndex(Schema::Object::ID::Value iDatabaseID_,
									   Schema::Object::ID::Value iTableID_,
									   Schema::Object::ID::Value iIndexID_);
												// 索引の削除

	void					detachFiles();		// ファイルをデタッチする

	void					addDelayedClear(Schema::Object::ID::Value iDatabaseID_, bool bVolatile_);
												// キャッシュのクリアを後回しにしたデータベースとして登録する
	void					eraseDelayedClear(Schema::Object::ID::Value iDatabaseID_);
												// キャッシュのクリアを後回しにしたデータベースとしての登録から除く

	ID						getID() const {return m_uID;} 

	static void				clearDatabaseCache();
												// 後回しにされていたデータベースの
												// キャッシュをクリアする
	void					clearDatabaseCache(Schema::Object::ID::Value iDatabaseID_);
												// 指定されたIDを持つデータベースの
												// キャッシュをクリアする
	void					closeDatabase(Schema::Object::ID::Value iDatabaseID_);
												// 指定されたIDのデータベースをcloseする
protected:
private:
	ObjectSnapshot(bool bIsNoVersion_);			// コンストラクタ

	// コピー禁止
	ObjectSnapshot(const ObjectSnapshot& );
	ObjectSnapshot& operator =(const ObjectSnapshot& );

	bool isDatabaseOpened(SessionID iSessionID_) const;

	// データベース表(キー:データベースID、値:データベースオブジェクト)
	DatabaseMap*			m_mapDatabases;		// データベースマップ

	// getDatabaseしたか表(キー:セッションID、値:真偽値)
	typedef ModHashMap<SessionID, bool, ModHasher<SessionID> > OpenMap;
	OpenMap					m_mapOpened;

	bool					m_bIsNoVersion;		// 版管理をしないか
	Os::RWLock				m_cRWLock;			// 排他制御用の読み書きモードつきロック

	ID						m_uID;				// スナップショットの識別子
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_OBJECTSNAPSHOT_H

//
// Copyright (c) 2000, 2001, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
