// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemFileSub.h --
//		システム表を構成するファイルの下請けのクラス定義、関数宣言
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_SYSTEMFILE_SUB_H
#define	__SYDNEY_SCHEMA_SYSTEMFILE_SUB_H

#include "Schema/Module.h"
#include "Schema/Field.h"
#include "Schema/FileID.h"
#include "Schema/OpenOption.h"

#include "Common/Object.h"

#include "LogicalFile/AutoLogicalFile.h"

#include "Os/Path.h"

_SYDNEY_BEGIN

namespace Common
{
	class Data;
	class DataArrayData;
}

namespace LogicalFile
{
	class File;
}

namespace Trans
{
	class Transaction;
	class TimeStampans;
}

_SYDNEY_SCHEMA_BEGIN

namespace SystemTable
{
	struct Attribute {
		//
		//	ENUM
		//	Schema::SystemTable::Attribute --
		//		システム表を構築するファイルの属性
		//
		//	NOTES
		
		enum Value {
			Normal = 0,							// デフォルトの属性
			ReadOnly = 1 << 1,					// 読み込み専用
			ValueNum
		};
		
	};

	//	CLASS
	//	Schema::SystemTable::ConsistFile --
	//		システム表を格納するファイルを表すクラス
	//
	//	NOTES

	class ConsistFile
		: public Common::Object
	{
	public:
		ConsistFile();							// コンストラクター
		virtual ~ConsistFile();					// デストラクター

		void				destruct();			// 内部のポインターを
												// 後処理する

		void				create(Trans::Transaction& cTrans_);
												// ファイルを作る
		bool				isAccessible();		// ファイルが作成されているか
		void				drop(Trans::Transaction& cTrans_);
												// ファイルを消す
		void				move(Trans::Transaction& cTrans_,
								 const Os::Path& cNewPath_,
								 bool bUndo_ = false,
								 bool Recovery_ = false);
												// ファイルを移動する

		const LogicalFile::FileID&
							mount(Trans::Transaction& cTrans_);
												// ファイルを mount する
		const LogicalFile::FileID&
							unmount(Trans::Transaction& cTrans_);
												// ファイルを unmount する
		void				flush(Trans::Transaction& cTrans_);
												// ファイルを flush する
		void				sync(Trans::Transaction& trans, bool& incomplete, bool& modified);
												// 不要な版を破棄する
		void				startBackup(Trans::Transaction& cTrans_,
										bool bRestorable_ = true);
												// バックアップを開始する
		void				endBackup(Trans::Transaction& cTrans_);
												// バックアップを終了する
		void				recover(Trans::Transaction& cTrans_,
									const Trans::TimeStamp& cPoint_);
												//	障害から回復する
		void				restore(Trans::Transaction&	Transaction_,
									const Trans::TimeStamp&	Point_);
												// ある時点に開始された
												// 読取専用トランザクションが
												// 参照する版を最新版とする

		void				open(Trans::Transaction& cTrans_,
								 const LogicalFile::OpenOption& cOpenOption_);
												// ファイルをオープンする
		void				close();			// ファイルをクローズする

		// ファイルを得る
		const LogicalFile::AutoLogicalFile& getFile() const;
		LogicalFile::AutoLogicalFile&		getFile();

		void				setAreaPath(const Os::Path& cPath_);
												// FileIDのパス名部分をセットする

		void				setAttribute(Attribute::Value eAttr_);
												// File の属性を設定する

		LogicalFile::FileID& getFileID();

		virtual ModCharString getPathPart() const = 0;
												// ファイルのパス名を得る
	protected:
	private:
		LogicalFile::AutoLogicalFile m_pFile;	// 検索や更新の対象になる
												// ファイルへのポインター
		LogicalFile::FileID* m_pFileID;			// ファイルID
		Os::Path			m_cPath;			// パス名(moveで使うため)
	};

	//	CLASS
	//	Schema::SystemTable::RecordFile --
	//		システム表を格納するファイルのうちレコードファイルを表すクラス
	//
	//	NOTES

	class RecordFile
		: public ConsistFile
	{
	public:
		// コンストラクター
		RecordFile() : ConsistFile(), m_pTupleData(0) {}
		// デストラクター
		~RecordFile();

		// レコードファイルを指すポインタを初期化する
		void				initialize(SystemFile& cFile_,
									   const Os::Path& cPath_,
									   SystemTable::Attribute::Value eAttr_,
									   bool bMounted_ = true);
		// ファイルのパス名を得る
		virtual ModCharString getPathPart() const;

		// 索引から値を取得するのに用いるデータを得る
		Common::DataArrayData& getTupleData();

	protected:
	private:
		Common::DataArrayData* m_pTupleData;	// ファイルから取得するときに使うデータ
	};

	//	CLASS
	//	Schema::SystemTable::IndexFile --
	//		システム表を格納するファイルのうち索引ファイルを表すクラス
	//
	//	NOTES

	class IndexFile
		: public ConsistFile
	{
	public:
		//	ENUM
		//	Schema::SystemTable::IndexFile::Category::Value --
		//		システム表の索引ファイルの種別を表す値
		//
		//	NOTES

		struct Category {
			enum Value {
				Btree = 0,
				Vector,
				ValueNum
			};
		};

		// コンストラクター
		IndexFile(const char* pszName_, Category::Value eCategory_,
				  Schema::Field::Position iKeyPosition_, Schema::Field::Type eKeyType_,
				  Schema::Field::Position iValuePosition_, Schema::Field::Type eValueType_)
			: ConsistFile(), m_cstrName(pszName_), m_eCategory(eCategory_),
			  m_iKeyPosition(iKeyPosition_), m_eKeyType(eKeyType_),
			  m_iValuePosition(iValuePosition_), m_eValueType(eValueType_),
			  m_pTupleData(0)
		{ }
		// デストラクター
		~IndexFile();

		// B+木ファイルを指すポインタを初期化する
		void				initialize(SystemFile& cFile_,
									   const Os::Path& cPath_,
									   SystemTable::Attribute::Value eAttr_,
									   bool bMounted_ = true);

		// 索引を識別する名前を得る
		const ModCharString& getName() const
		{return m_cstrName;}

		// 条件にするフィールドのレコードファイルでの位置を得る
		Schema::Field::Position getRecordKeyPosition() const
		{return m_iKeyPosition;}
		// 条件にするフィールドのレコードファイルでの位置を得る
		Schema::Field::Position getRecordValuePosition() const
		{return m_iValuePosition;}
		// 索引のキーとなるフィールドの索引ファイル上の位置を得る
		Schema::Field::Position getKeyPosition() const
		{return m_eCategory == Category::Btree ? 1 : 0;}
		// 索引のバリューとなるフィールドの索引ファイル上の位置を得る
		Schema::Field::Position getValuePosition() const
		{return m_eCategory == Category::Btree ? 2 : 1;}
		// 索引ファイルの種別を得る
		Category::Value		getCategory() const
		{return m_eCategory;}
		// 索引のキーになるフィールドのデータ型を得る
		Schema::Field::Type	getKeyType() const
		{return m_eKeyType;}
		// 索引のバリューになるフィールドのデータ型を得る
		Schema::Field::Type	getValueType() const
		{return m_eValueType;}

		// 索引ファイルのパス名を得る
		virtual ModCharString getPathPart() const;
		// レコードのタプルからこの索引に入力するタプルを作る
		Common::DataArrayData* makeInsertTuple(Common::DataArrayData* pTuple_);
		// 索引から削除するためのキーを作る
		Common::DataArrayData* makeExpungeKey(Common::DataArrayData* pTulpe_);

		// 索引から値を取得するのに用いるデータを得る
		Common::DataArrayData& getTupleData(bool bIncludeKey_ = false);

	protected:
	private:
		ModCharString		m_cstrName;			// B+木ファイルを識別する名前
		Category::Value		m_eCategory;		// 索引ファイルの種別
		Schema::Field::Position m_iKeyPosition;	// 検索対象のフィールドの
												// レコードファイル上の位置
		Schema::Field::Type	m_eKeyType;			// 索引のキーになるフィールドの
												// データ型
		Schema::Field::Position m_iValuePosition;// 取得対象のフィールドの
												// レコードファイル上の位置
		Schema::Field::Type	m_eValueType;		// 索引のバリューになるフィールドの
												// データ型
		Common::DataArrayData* m_pTupleData;	// ファイルから取得するときに使うデータ
	};
}

//////////////////////////////////////////
//	Schema::SystemTable::ConsistFile	//
//////////////////////////////////////////

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::ConsistFile -- コンストラクター
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
Schema::SystemTable::ConsistFile::
ConsistFile()
	: m_pFileID(0)
{ }

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::~ConsistFile -- デストラクター
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
Schema::SystemTable::ConsistFile::
~ConsistFile()
{
	destruct();
}

//	FUNCTION public
//	Schema::SystemTable::ConsistFile::getFile --
//		システム表を格納するファイルを構成するファイルのうち
//		指定した種類の内部ファイルを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		システム表を格納するファイルを構成するファイル
//
//	EXCEPTIONS

inline
const LogicalFile::AutoLogicalFile&
Schema::SystemTable::ConsistFile::
getFile() const
{
	return m_pFile;
}

inline
LogicalFile::AutoLogicalFile&
Schema::SystemTable::ConsistFile::
getFile()
{
	return m_pFile;
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_SYSTEMFILE_SUB_H

//
// Copyright (c) 2000, 2001, 2002, 2003, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
