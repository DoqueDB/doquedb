// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemFile.h -- システム表を構成するファイル関連のクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2008, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMFILE_H
#define	__SYDNEY_SCHEMA_SYSTEMFILE_H

#include "Schema/Module.h"
#include "Schema/Database.h"
#include "Schema/FileID.h"
#include "Schema/SystemFileSub.h"

#include "Admin/Verification.h"

#include "Common/Object.h"

#include "Os/Path.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
	class TimeStamp;
}

_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	namespace OpenMode
	{
		//	ENUM
		//	Schema::SystemTable::OpenMode::Value --
		//		システム表の保管場所であるファイルを
		//		何のためにオープンするかを表す列挙型
		//
		//	NOTES

		enum Value
		{
			Unknown =	0,						// 不明
			Read,								// 読み出しのため
			Search,								// 索引を使った検索結果の
												// 読み出しのため
			Update,								// 更新操作のため
			ValueNum							// 種別数
		};
	}

	//	CLASS
	//	Schema::SystemTable::SystemFile -- システム表の保管場所を表すクラス
	//
	//	NOTES

	class SystemFile
		: public	Common::Object
	{
	public:
		friend class SystemTable::RecordFile;
		friend class SystemTable::IndexFile;

		// コンストラクター
		SystemFile(Schema::Object::Category::Value eCategory_,
				   const Os::Path& cPathBase_,
				   const Os::Path& cPathPart_,
				   SystemTable::Attribute::Value eAttr_);
		// デストラクター
		SYD_SCHEMA_FUNCTION
		virtual ~SystemFile();

		// システムファイルの初期化
		void				initialize(bool bMounted_ = true);
		// システムファイルの後処理
		void				terminate();

		// システム表の保管場所を生成する
		void				create(Trans::Transaction& cTrans_,
								   bool bAllowExistence_ = false);
		// システム表の保管場所を抹消する
		void				drop(Trans::Transaction& cTrans_);
		// システム表の保管場所を移動する
		void				move(Trans::Transaction& cTrans_,
								 const Os::Path& cPrevPath_,
								 const Os::Path& cNewPath_,
								 bool bUndo_ = false,
								 bool bRecovery_ = false);

		////////////////
		// 運用管理機能のための関数
		SYD_SCHEMA_FUNCTION
		void				mount(Trans::Transaction& cTrans_, bool bUndo_ = false);
		SYD_SCHEMA_FUNCTION
		void				unmount(Trans::Transaction& cTrans_, bool bUndo_ = false);
		SYD_SCHEMA_FUNCTION
		void				flush(Trans::Transaction& cTrans_);
		SYD_SCHEMA_FUNCTION
		void				sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
		SYD_SCHEMA_FUNCTION
		void				startBackup(Trans::Transaction& cTransaction_,
										bool bRestorable_ = true,
										bool bUndo_ = false);
		SYD_SCHEMA_FUNCTION
		void				endBackup(Trans::Transaction& cTransaction_);
		SYD_SCHEMA_FUNCTION
		void				recover(Trans::Transaction& cTransaction_,
									const Trans::TimeStamp& cPoint_);
		SYD_SCHEMA_FUNCTION
		void				restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);

		// システム表の保管場所をオープンする
		void				open(Trans::Transaction& cTrans_,
								 OpenMode::Value mode);
		void				open(Trans::Transaction& cTrans_,
								 IndexFile* pFile_,
								 Schema::Object::ID::Value iID_);
		// システム表の保管場所をクローズする
		void				close(Trans::Transaction& trans);

		// システム表からデータを取得する
		bool				getData(Trans::Transaction& trans, Common::DataArrayData& cData_);

		// システム表からIDを取得する
		Schema::Object::ID::Value
							loadID(Trans::Transaction& trans);
		// システム表に変更を反映する
		void				insert(Trans::Transaction& trans,
								   Common::DataArrayData* pTuple_);
		void				update(Trans::Transaction& trans,
								   const Common::DataArrayData* pKey_,
								   Common::DataArrayData* pTuple_);
		void				expunge(Trans::Transaction& trans,
									const Common::DataArrayData* pKey_,
									Common::DataArrayData* pTuple_);
		// システム表を構成するファイルの格納場所を得る
		const Os::Path&		getAreaPath() const;

		// 整合性検査
		SYD_SCHEMA_FUNCTION
		void				verify(Admin::Verification::Progress& cResult_,
								   Trans::Transaction& cTrans_,
								   Admin::Verification::Treatment::Value eTreatment_,
								   Schema::Object::ID::Value& iMaxID_);

		// システム表のレコードファイルに対応するFileIDを得る
		const LogicalFile::FileID&
							getFileID();
		// システム表種別を得る
		Schema::Object::Category::Value
							getCategory() const;

		// データベースの属性変更による処理を行う
		void				propagateDatabaseAttribute(Trans::Transaction& cTrans_,
													   const Schema::Database::Attribute& cAttribute_);

		// システム表に対応するデータベースIDを得る
		virtual Schema::Object::ID::Value getDatabaseID() const = 0;
		// システム表に対応する表IDを得る
		Schema::Object::ID::Value getTableID() const;
		// システム表に対応するファイルオブジェクトIDを得る
		Schema::Object::ID::Value getID() const;

		// 指定した名前の索引ファイルを得る
		IndexFile*			getIndex(const char* pszIndexName_);
	protected:
		// スキーマオブジェクトをシステムファイルに永続化する(storeの下請け)
		void				storeObject(const Schema::Object& cObject_,
										Schema::Object::Status::Value eStatus_,
										Trans::Transaction& cTrans_);

		// システムファイルに格納するフィールドの情報を設定する
		virtual void		setFieldInfo(LogicalFile::FileID& cFileID_) const = 0;

		// システム表のロック名情報を設定する
		void				setLockName(LogicalFile::FileID& cFileID_) const;

		// レコードファイルを得る
		RecordFile*			getRecord();
		// 索引ファイルを追加する
		void				addIndex(IndexFile* pIndex_);
		// システム表のファイルを初期化する
		void				initializeRecord(const Os::Path& cPath_,
											 bool bMounted_ = true);
		void				initializeIndex(const Os::Path& cPath_,
											IndexFile* pIndex_,
											bool bMounted_ = true);

#ifdef DEBUG
		Os::Path& 	getPath() { return m_cPath; }
#endif

	private:
		void				destruct();			// デストラクターの下位関数

		// 索引ファイルへの変更処理を行う
		void				insertIndex(Trans::Transaction& trans,
										Common::DataArrayData* pTuple_,
										IndexFile* pIndex_,
										bool bUndo_ = false);
		void				expungeIndex(Trans::Transaction& trans,
										 Common::DataArrayData* pTuple_,
										 IndexFile* pIndex_,
										 bool bUndo_ = false);

		Schema::Object::Category::Value m_eCategory;
												// システム表に格納される
												// オブジェクトの種別
		Os::Path			m_cPathPart;		// システムファイルの格納場所のうち固有部分
		Os::Path			m_cPath;			// システムファイルの格納場所
		OpenMode::Value		m_eOpenedMode;		// ファイルがオープンされている
												// モード
		SystemTable::Attribute::Value
							m_eAttr;			// システム表のファイル属性
		IndexFile*			m_pOpenedIndex;		// オープンに使った索引ファイル

		// データを保持するレコードファイルと
		// 検索に使うB+木ファイル

		RecordFile*			m_pRecord;			// システム表を格納する
												// レコードファイル
		ModVector<IndexFile*>*
							m_pIndex;			// 検索するための索引となる
												// B+木ファイル
	};
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMFILE_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2008, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
