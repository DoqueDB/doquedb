// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemTable.h -- システム表関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMTABLE_H
#define	__SYDNEY_SCHEMA_SYSTEMTABLE_H

#include "Schema/Module.h"
#include "Schema/Database.h"
#include "Schema/FileID.h"
#include "Schema/Object.h"
#include "Schema/SystemFile.h"

#include "Common/Object.h"
#include "Os/Path.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

template <class _Object_, class _Pointer_>
class ObjectMap;

namespace SystemTable
{
	class IndexFile;

	namespace Status
	{
		//	ENUM
		//	Schema::SystemTable::Status::Value --
		//		システム表の永続化に関する状態を表す列挙型
		//
		//	NOTES

		enum Value
		{
			Clean =		0x00,					// 永続化の必要なし
			Dirty =		0x01,					// 永続化の必要あり
			Deleted =	0x02,					// 削除されたオブジェクトがある
			DirtyDeleted =	Dirty | Deleted,

			ValueNum
		};
	}

	void					initialize();		// システム表関連の初期化
	void					terminate();		// システム表関連の後処理

	void					setStatus(Schema::Object::ID::Value iID_,
									  Schema::Object::Category::Value eCategory_,
									  Status::Value	eStatus_);
												// システム表の永続化に
												// 関する状態を設定する
	void					unsetStatus(Schema::Object::ID::Value iID_,
										Schema::Object::Category::Value eCategory_);
												// システム表の永続化に
												// 関する状態を解除する
	Status::Value			getStatus(Schema::Object::ID::Value iID_,
									  Schema::Object::Category::Value eCategory_);
												// システム表の永続化に
												// 関する状態を得る
	void					eraseStatus(Schema::Object::ID::Value iID_);
												// システム表の永続化に関する
												// 状態を保持する構造を破棄する

	SystemFile*				getSystemFile(Schema::Object::Category::Value eCategory_,
										  Schema::Database* pDatabase_);
												// カテゴリーを指定して
												// システム表のオブジェクトを得る

	SYD_SCHEMA_FUNCTION
	bool					setAvailability(bool bAvailable_);
												// メタデータベースが利用可能かを設定する

	SYD_SCHEMA_FUNCTION
	bool					isAvailable();		// メタデータベースが利用可能かを取得する

	//	TEMPLATE CLASS
	//	Schema::SystemTable::Base -- SystemTableに共通したコードをまとめる
	//
	//	NOTES

	template <class _Object_, class _Pointer_, class _Parent_>
	class Base
		: public SystemFile
	{
	public:
		// コンストラクター
		Base(Schema::Object::Category::Value eCategory_,
			 const Os::Path& cPathBase_,
			 const Os::Path& cPathPart_,
			 SystemTable::Attribute::Value eAttr_,
			 Schema::Database* pDatabase_);

		// 読み込みの共通定義
		void load(Trans::Transaction& cTrans_, _Parent_& cParent_, IndexFile* pIndexFile_ = 0);
		// 書き込みの共通定義
		void store(Trans::Transaction& cTrans_, const _Parent_& cParent_,
				   const ObjectMap<_Object_, _Pointer_>* pMap_,
				   bool bContinuously_ = false);
		void store(Trans::Transaction& cTrans_, const _Pointer_& pObject_,
				   bool bContinuously_ = false,
				   bool bNeedToErase = true);
		void store(Trans::Transaction& cTrans_, Schema::Object::ID::Value iID_,
				   const ObjectMap<_Object_, _Pointer_>* pMap_,
				   bool bContinuously_ = false,
				   bool bNeedToErase = true);

		// 対応するデータベースIDを得る
		virtual Schema::Object::ID::Value getDatabaseID() const;

	protected:
		// システムファイルに格納するフィールドの情報を設定する
		virtual void		setFieldInfo(LogicalFile::FileID& cFileID_) const;

		Schema::Database* m_pDatabase;
	private:
	};
}

//	TEMPLATE FUNCTION public
//	Schema::SystemTable::Base::getDatabaseID --
//		システム表が属するデータベースID
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		データベースID
//
//	EXCEPTIONS
//		なし

template <class _Object_, class _Pointer_, class _Parent_>
Schema::Object::ID::Value
SystemTable::Base<_Object_, _Pointer_, _Parent_>::
getDatabaseID() const
{
	return m_pDatabase ?
		m_pDatabase->getID() : Schema::Object::ID::SystemTable;
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMTABLE_H

//
 // Copyright (c) 2000, 2001, 2002, 2004, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
