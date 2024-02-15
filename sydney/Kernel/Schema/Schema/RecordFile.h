// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RecordFile.h -- レコードファイルオブジェクト関連のクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_RECORDFILE_H
#define	__SYDNEY_SCHEMA_RECORDFILE_H

#include "Schema/Module.h"
#include "Schema/File.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class Table;
class Index;
class Column;
class Hint;
namespace SystemTable
{
	class SystemFile;
}

//	ENUM
//	Schema::RecordFile -- レコードファイルオブジェクトを表すクラス
//
//	NOTES
//		レコードファイルの親オブジェクトは、データベースである

class RecordFile
	: public	File
{
public:

	RecordFile();
	RecordFile(const Database& database, Table& table,
			   const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0,
			   ID::Value iColumnID_ = ID::Invalid);
#ifdef OBSOLETE // IndexのファイルとしてRecordを使うことはない
	RecordFile(const Database& database, Table& table, Index& index,
			   const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0);
#endif
												// コンストラクター

	static File::Pointer	create(Trans::Transaction& cTrans_,
								   const Database& database, Table& table,
								   const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0,
								   Column* pColumn_ = 0);
#ifdef OBSOLETE // IndexのファイルとしてRecordを使うことはない
	static File::Pointer	create(Trans::Transaction& cTrans_,
								   const Database& database, Table& table,
								   Index& index,
								   const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0);
#endif
												// ファイルのスキーマ情報を
												// 生成する

	static File::Pointer	createSystem(Trans::Transaction& cTrans_,
										 const Database& database, Table& table,
										 SystemTable::SystemFile& cSystemFile_,
										 ID::Value iObjectID_);

	virtual void			setFileID(Trans::Transaction& cTrans_);
												// ファイル ID を設定する

//	File:
//	const Area*				getArea() const;	// ファイルを格納する
//												// エリアを得る
//	Common::PathName		getAreaPath(const Common::PathName& cPath_) const;
//												// ファイルを格納する
//												// パス名を得る
	virtual AreaCategory::Value
							getAreaCategory() const;
												// ファイルに対応する
												// エリアの種別を得る

//	File::
	virtual void			checkFieldType(Trans::Transaction& cTrans_);
												// Check the validity of FileID contents and modify if needed
//	virtual bool			isKeyUnique() const;
//												// ファイルに格納されるデータが
//												// キーについて一意かどうか
	virtual bool			hasAllTuples() const;
												// ファイルに格納されるデータが
												// すべてのタプルに対応しているか
	virtual bool			isKeyGenerated() const;
												// ファイルのキーは挿入時に生成されるか
	virtual bool			isAbleToScan(bool bAllTuples_) const;
												// 順次取得が可能か
	virtual bool			isAbleToFetch() const;
												// キーを指定したFetchによる取得が可能か
//	virtual bool			isAbleToSearch(const TreeNodeInterface& pCond_) const;
//												// 条件を指定した検索結果の取得が可能か
//	virtual bool			isAbleToGetByBitSet() const;
//												// 取得がRowIDのみのときBitSetによる取得が可能か
	virtual bool			isAbleToUndo() const;
												// 削除や挿入のUndoはドライバーが行うか
	virtual SkipInsertType::Value
							getSkipInsertType() const;
												// 挿入しないデータの種別を得る
	virtual ModVector<Field*> getFetchKey(Trans::Transaction& cTrans_) const;
												// Fetchに使うことができるフィールドを得る
	virtual Name			createName(Trans::Transaction& cTrans_, const Name& cParentName_);
												// レコードファイルの
												// 名前を設定する

	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

protected:
//	File:
//	void					setAreaPath(LogicalFile::FileID& fileID);
//												// ファイルIDのエリア部分を
//												// 作る
//	const LogicalFile::FileID&
//							setFileID(const LogicalFile::FileID& fileID);
//												// ファイル ID を設定する
	virtual Common::Data::Pointer
							packOption() const;	// サブクラスの付加情報を
												// Common::Dataにする
	virtual void			unpackOption(const Common::Data& cData_);
												// サブクラスの付加情報に
												// Common:::Dataを反映する

	void					clear();			// メンバーをすべて初期化する

private:
	Name					createMyName(Trans::Transaction& cTrans_, const Name& cParentName_,
										 Column* pColumn_ = 0);
												// 名前を設定する

	ID::Value				m_iColumnID;		// ヒープとして使われる場合に
												// 列のIDが設定される
};

_SYDNEY_SCHEMA_END

//	FUNCTION public
//	Schema::RecordFile::RecordFile --
//		レコードファイルを表すクラスのデフォルトコンストラクター
//
//	NOTES
//		このレコードファイルの名前は新たに生成されずに、
//		空文字列のままである
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline
Schema::RecordFile::RecordFile()
	: m_iColumnID(ID::Invalid)
{ }

//	FUNCTION public
//	Schema::RecordFile::getClassID -- このクラスのクラス ID を得る
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
//		なし

inline
int
Schema::RecordFile::getClassID() const
{
	return Externalizable::Category::RecordFile +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::RecordFile::clear --
//		レコードファイルを表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

inline
void
Schema::RecordFile::clear()
{
	m_iColumnID = ID::Invalid;
}

//
//	FUNCTION public
//	Schema::RecordFile::getAreaCategory --
//		レコードファイルを格納するエリアの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		AreaCategory::Heap
//			ヒープファイルとして使うとき
//		AreaCategory::Default
//			ヒープファイルではないとき
//
//	EXCEPTIONS
//

inline
Schema::AreaCategory::Value
Schema::RecordFile::
getAreaCategory() const
{
	return (m_iColumnID != ID::Invalid) ? AreaCategory::Heap : AreaCategory::Default;
}

_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_RECORDFILE_H

//
// Copyright (c) 2000, 2001, 2004, 2005, 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
