// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Mount.h -- データベースのマウント関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2004, 2007, 2009, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_ADMIN_MOUNT_H
#define __SYDNEY_ADMIN_MOUNT_H

#include "Admin/Module.h"
#include "Admin/Utility.h"

#include "Common/Data.h"
#include "Schema/ObjectID.h"
#include "Trans/TimeStamp.h"

#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Area;
	class Database;
	class Index;
	class LogData;
	class Table;
}
namespace Server
{
	class Session;
}
namespace Statement
{
	class AlterAreaStatement;
	class MountDatabaseStatement;
	class Object;
	class OptionalAreaParameter;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_ADMIN_BEGIN

namespace Recovery
{
	class Database;
}

//
//	TYPEDEF
//	Admin::Mount::StatementList
//		-- Statement::Object* のコンテナクラス
//
//	NOTES
typedef ModVector< const Statement::Object* >	StatementList;

//	CLASS
//	Admin::Mount -- データベースのマウントに関するクラス
//
//	NOTES
//		このクラスは new することはないはずなので、
//		Common::Object の子クラスにしない

class Mount
{
public:
	struct Option
	{
		//	ENUM
		//	Admin::Mount::Option::Value -- マウントオプションを表す値の列挙型
		//
		//	NOTES

		typedef unsigned int	Value;
		enum
		{
			None =				0x00,
			UsingSnapshot =		0x01,
			WithRecovery =		0x02,
			DiscardLogicalLog = 0x04,
			Mask =				0x07
		};
	};

	//
	//	STRUCT
	//		Admin::Mount::AreaModifyInfo -- Area 変更情報
	//
	//	NOTES
	struct AreaModifyInfo
	{
		// 変更の種類
		struct ModifyType {
			enum Value {
				Alter,
				Drop,
				ValueNum
			};
		};
		// コンストラクター
		AreaModifyInfo() : m_NeedToDelete(false), m_Persist(true) {}
		AreaModifyInfo(Statement::Object& cStatement_);
		AreaModifyInfo(Schema::Area& cArea_);

		// 変更後のパスをセットする
		void setStatementData(Statement::Object& cStatement_);
		// IDや変更前後のパスをセットする
		void setAreaData(Trans::Transaction& cTrans_, const Schema::Database& cDatabase_);

		// 構造体の内容をDataArrayDataにする
		Common::DataArrayData* pack() const;
		// DataArrayDataから構造体の内容をセットする
		void unpack(const Common::DataArrayData& cData_);

		ModifyType::Value	m_eType;			// 変更種別
		ModUnicodeString	m_Name;				// Area 名
		Schema::ObjectID::Value
							m_ID;				// Area ID
		ModVector<ModUnicodeString>
							m_vecPrevPath;		// 変更前のパス
		ModVector<ModUnicodeString>
							m_vecPostPath;		// 変更後のパス
		enum {MemberNum=5};

		// packしないメンバー
		mutable bool		m_Persist;
		mutable bool		m_NeedToDelete;
		mutable Schema::Area* m_Object;			// Areaオブジェクト
	};

	//
	//	STRUCT
	//		Admin::Mount::TableModifyInfo -- Table 変更情報
	//
	//	NOTES
	struct TableModifyInfo
	{
		// コンストラクター
		TableModifyInfo() : m_Persist(true) {}
		TableModifyInfo(Schema::Table& cTable_);
		TableModifyInfo(Schema::Index& cIndex_);

		// 構造体の内容をDataArrayDataにする
		Common::DataArrayData* pack() const;
		// DataArrayDataから構造体の内容をセットする
		void unpack(const Common::DataArrayData& cData_);

		// エリア変更内容が適用されるか調べる
		bool setAreaModifyInfo(const ModVector<AreaModifyInfo>& vecAreaModifyInfo_);

		ModUnicodeString	m_Name;				// TableまたはIndex名
		Schema::ObjectID::Value
							m_ID;				// Table IDまたはIndex ID
		ModVector<Schema::ObjectID::Value>
							m_vecPrevID;		// 変更前のエリアID割り当て
		ModVector<Schema::ObjectID::Value>
							m_vecPostID;		// 変更後のエリアID割り当て

		enum {MemberNum=4};

		// packしないメンバー
		mutable bool			m_Persist;
		mutable Schema::Object* m_Object;			// TableまたはIndexオブジェクト
	};

	//
	//	STRUCT
	//		Admin::Mount::IndexModifyInfo -- Index 変更情報
	//
	//	NOTES
	//		定義自体はTableModifyInfoと同じでよい
	typedef TableModifyInfo IndexModifyInfo;

	// コンストラクター
	Mount(Trans::Transaction& cTrans_, Server::Session* pSession_, const ModUnicodeString& strDbName_);
	SYD_ADMIN_FUNCTION
	Mount(Trans::Transaction& trans, const Schema::LogData& logData);
	SYD_ADMIN_FUNCTION
	Mount(Trans::Transaction& trans,
		  const Schema::Database::Pointer& database, const Schema::LogData& logData);

	// デストラクター
	SYD_ADMIN_FUNCTION
	~Mount();

	// マウントを実際に行う
	SYD_ADMIN_FUNCTION
	Schema::Database*
	execute(const Statement::MountDatabaseStatement* pcStatement_);

	// マウント処理の REDO を行う
	SYD_ADMIN_FUNCTION
	Recovery::Database*		redo(const Schema::LogData& logData);
#ifdef OBSOLETE // データベース単位のUNDOは呼ばれない
	// マウント処理の UNDO を行う
	SYD_ADMIN_FUNCTION
	void					undo(bool redone_);
#endif

private:
	void					checkAreaPath();	// SQL 文の Area 変更リストに
												// 使用されているパスが使われているか
												// チェックする

	// 回復処理を開始する時点のタイムスタンプを得る
	void
	findStartLSN();
	// 回復処理を行う
	void
	recover();

	// ログデータを作成する
	void
	makeLogData(Schema::LogData& logData);
	// ログデータのある種別の情報を作成する
	Common::Data::Pointer
	packMetaField(Utility::Meta::Mount::Type type) const;
	// ログデータのある種別の情報の値を得る
	void
	unpackMetaField(const Common::Data* data, Utility::Meta::Mount::Type type);
	// ログデータのある種別の情報の型を得る
	static Common::DataType::Type
	getMetaFieldType(Utility::Meta::Mount::Type type);

	void					setAreaModifyInfo(const Statement::MountDatabaseStatement* pcStatement_);
												// Areaの定義変更内容をメンバーにセットする

	void					applyAreaModifyInfo(bool bUndo_ = false);
												// Areaの定義変更内容をAreaオブジェクトに反映する
	void					persistAreaModifyInfo(bool bUndo_ = false);
												// Areaを永続化する
#ifdef OBSOLETE
	void					redoAreaModifyInfo();
												// Areaの変更内容についてREDO処理する
#endif
#ifdef OBSOLETE // データベース単位のUNDOは呼ばれない
	void					undoAreaModifyInfo(bool redone_);
												// Areaの変更内容についてUNDO処理する
#endif
	void					destructAreaModifyInfo();
												// Areaの変更内容を表す構造体の解放処理をする
	void					preloadAreaObject();// Areaに関係するスキーマオブジェクトを読み込んでおく

	void					setTableModifyInfo();
												// Table以下の定義変更内容をメンバーにセットする
	void					applyTableModifyInfo(bool bUndo_ = false);
												// Table以下の定義変更内容をSchemaオブジェクトに反映する
	void					persistTableModifyInfo(bool bUndo_ = false);
												// Table以下のオブジェクトを永続化する
#ifdef OBSOLETE
	void					redoTableModifyInfo();
												// Table以下の変更内容についてUNDO処理する
#endif
#ifdef OBSOLETE // データベース単位のUNDOは呼ばれない
	void					undoTableModifyInfo(bool redone_);
												// Table以下の変更内容についてUNDO処理する
#endif
	void					preloadTableObject();// Table以下のスキーマオブジェクトを読み込んでおく

	Trans::Transaction&		m_cTrans;			// MOUNT 処理中に使用する Transaction

	Server::Session*		m_pSession;			// MOUNT 処理を実行したセッション記述子
	ModUnicodeString		m_strDbName;		// 操作対象のデータベース名

	Schema::Database::Pointer
							m_pDatabase;		// 処理対象のデータベースクラス

	bool					m_bReadOnly;		// READ ONLY 指定フラグ
	bool					m_bOnline;			// ONLINE 指定フラグ
	// マウントオプション
	Option::Value			_option;
	// アンマウントされたデータベースをマウントするか
	Boolean::Value			_unmounted;
	// マウントするデータベースの回復処理を行うためのクラス
	Recovery::Database*		_dbRecovery;
	// 回復処理を開始する時点のタイムスタンプ
	Trans::TimeStamp		_starting;
	// 回復処理中のデータベースであることを通知済みか
	bool					_notified;

	ModVector<AreaModifyInfo>
							m_vecAreaModifyInfo;
												// Area 変更情報
	ModVector<TableModifyInfo>
							m_vecTableModifyInfo;
												// Table 変更情報
	ModVector<IndexModifyInfo>
							m_vecIndexModifyInfo;
												// Index 変更情報
};

//	FUNCTION public
//	Admin::Mount::Mount -- マウントを表すクラスのコンストラクタ
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			マウントを行うトランザクションのトランザクション記述子
//		Server::Session* pSession_
//			マウントを行うセッションのセッション記述子
//		ModUnicodeString&	strDbName_
//			マウントするデータベースの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
Mount::Mount(Trans::Transaction& cTrans_, Server::Session* pSession_, const ModUnicodeString& strDbName_)
	: m_cTrans(cTrans_),
	  m_pSession(pSession_),
	  m_strDbName(strDbName_),
	  _option(Option::None),
	  _unmounted(Boolean::Unknown),
	  _dbRecovery(0),
	  _notified(false)
{}

_SYDNEY_ADMIN_END
_SYDNEY_END

#endif //__SYDNEY_ADMIN_MOUNT_H

//
//	Copyright (c) 2001, 2002, 2004, 2007, 2009, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
