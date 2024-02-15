// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LobFile.h -- 長大データファイルオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2003, 2004, 2005, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_LOBFILE_H
#define	__SYDNEY_SCHEMA_LOBFILE_H

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
//	Schema::LobFile -- LOBファイルオブジェクトを表すクラス
//
//	NOTES
//		LOBファイルの親オブジェクトは、データベースである

class LobFile
	: public	File
{
public:

	// コンストラクター
	LobFile();
	LobFile(const Database& database, Table& table,
			const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0,
			ID::Value iColumnID_ = ID::Invalid);

	// ファイルのスキーマ情報を生成する
	static File::Pointer	create(Trans::Transaction& cTrans_,
								   const Database& database, Table& table,
								   const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0,
								   Column* pColumn_ = 0);

	// ファイル ID を設定する
	virtual void			setFileID(Trans::Transaction& cTrans_);

//	File:
//	// ファイルを格納するエリアを得る
//	const Area*				getArea() const;
//	// ファイルを格納するパス名を得る
//	Common::PathName		getAreaPath(const Common::PathName& cPath_) const;
	// ファイルに対応するエリアの種別を得る
	virtual AreaCategory::Value
							getAreaCategory() const;

//	File::
//	virtual bool			isKeyUnique() const;
//												// ファイルに格納されるデータが
//												// キーについて一意かどうか
//	virtual bool			hasAllTuples() const;
//												// ファイルに格納されるデータが
//												// すべてのタプルに対応しているか
	virtual bool			isKeyGenerated() const;
												// ファイルのキーは挿入時に生成されるか
//	virtual bool			isAbleToScan(bool bAllTuples_) const;
//												// 順次取得が可能か
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
												// LOBファイルの
												// 名前を設定する

	virtual void			serialize(ModArchive& archiver);
												// このクラスをシリアル化する
	virtual int				getClassID() const;	// このクラスのクラス ID を得る

	virtual AccessFile*		getAccessFile(Trans::Transaction& cTrans_) const;
												// ファイルに対応するAccessFileを得る
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
	// LOBファイルの名前を設定する
	Name					createMyName(Trans::Transaction& cTrans_, Column* pColumn_);

	ID::Value				m_iColumnID;		// LOB列のIDが設定される
};

_SYDNEY_SCHEMA_END

//	FUNCTION public
//	Schema::LobFile::LobFile --
//		LOBファイルを表すクラスのデフォルトコンストラクター
//
//	NOTES
//		このLOBファイルの名前は新たに生成されずに、
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
Schema::LobFile::LobFile()
	: m_iColumnID(ID::Invalid)
{ }

//	FUNCTION public
//	Schema::LobFile::getClassID -- このクラスのクラス ID を得る
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
Schema::LobFile::getClassID() const
{
	return Externalizable::Category::LobFile +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::LobFile::clear --
//		LOBファイルを表すクラスのメンバーをすべて初期化する
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
Schema::LobFile::clear()
{
	m_iColumnID = ID::Invalid;
}

//
//	FUNCTION public
//	Schema::LobFile::getAreaCategory --
//		LOBファイルを格納するエリアの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		AreaCategory::Heap
//
//	EXCEPTIONS
//

inline
Schema::AreaCategory::Value
Schema::LobFile::
getAreaCategory() const
{
	return AreaCategory::Heap;
}

_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_LOBFILE_H

//
// Copyright (c) 2003, 2004, 2005, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
