// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapFile.h -- Bitmapファイルオブジェクト関連のクラス定義、関数宣言
// 
// Copyright (c) 2005, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_BITMAPFILE_H
#define	__SYDNEY_SCHEMA_BITMAPFILE_H

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
class Hint;

//	CLASS
//	Schema::BitmapFile -- bitmapファイルオブジェクトを表すクラス
//
//	NOTES
//		bitmapファイルの親オブジェクトは、データベースである

class BitmapFile
	: public	File
{
public:
	BitmapFile();
	BitmapFile(const Database& database, Table& table, Index& index, const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0);
												// コンストラクター

	static File::Pointer	create(Trans::Transaction& cTrans_,
								   const Database& database, Table& table,
								   Index& index,
								   const Hint* pHint_ = 0, const Hint* pAreaHint_ = 0);
												// ファイルのスキーマ情報を
												// 生成する

	void					clear();			// メンバーをすべて初期化する

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

//	virtual void			checkFieldType(Trans::Transaction& cTrans_);
//												// Check the validity of FileID contents and modify if needed
//
//	virtual ModSize			createVirtualField(Trans::Transaction& cTrans_);
//												// 読み込み専用の仮想列を追加する
//
//	virtual bool			isKeyUnique() const;
//												// ファイルに格納されるデータが
//												// キーについて一意かどうか
	virtual bool			hasAllTuples() const;
												// ファイルに格納されるデータが
												// すべてのタプルに対応しているか
//	virtual bool			isKeyGenerated() const;
//												// ファイルのキーは挿入時に生成されるか
//	virtual bool			isAbleToScan(bool bAllTuples_) const;
//												// 順次取得が可能か
	
	virtual bool			isAbleToBitSetSort() const;
//												// Groupingが可能か
	
	virtual bool			isGettable(Trans::Transaction& cTrans_,
									   const Field* pField_,
									   const LogicalFile::TreeNodeInterface* pScalarField_) const;
//												// 値を取得可能か
	
//	virtual bool			isAbleToFetch() const;
//												// キーを指定したFetchによる取得が可能か
	virtual bool			isAbleToSearch(const LogicalFile::TreeNodeInterface& pCond_) const;
												// 条件を指定した検索結果の取得が可能か
	virtual bool			isAbleToGetByBitSet() const;
												// 取得がRowIDのみのときBitSetによる取得が可能か
	virtual bool			isAbleToSearchByBitSet() const;
												// BitSetによる検索が可能か
//	virtual bool			isAbleToUndo() const;
//												// 削除や挿入のUndoはドライバーが行うか
//	virtual bool			isHasFunctionField(Schema::Field::Function::Value eFunction_) const;
//												// 関数フィールドがあるか
//	virtual SkipInsertType::Value
//							getSkipInsertType() const;
//												// 挿入しないデータの種別を得る

	virtual Name			createName(Trans::Transaction& cTrans_, const Name& cParentName_);
												// bitmapファイルの
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
//	virtual Common::Data::Pointer
//							packOption() const;	// サブクラスの付加情報を
//												// Common::Dataにする
//	virtual void			unpackOption(const Common::Data& cData_);
//												// サブクラスの付加情報に
//												// Common:::Dataを反映する
private:
	bool isArbitraryField(const Field* pKey, const LogicalFile::TreeNodeInterface* pField_) const;
};

_SYDNEY_SCHEMA_END

//	FUNCTION public
//	Schema::BitmapFile::BitmapFile --
//		bitmapファイルを表すクラスのデフォルトコンストラクター
//
//	NOTES
//		この bitmapファイルの名前は新たに生成されずに、
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
Schema::BitmapFile::BitmapFile()
{ }

//	FUNCTION public
//	Schema::BitmapFile::getClassID -- このクラスのクラス ID を得る
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
Schema::BitmapFile::getClassID() const
{
	return Externalizable::Category::BitmapFile +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::BitmapFile::clear --
//		bitmapファイルを表すクラスのメンバーをすべて初期化する
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
Schema::BitmapFile::clear()
{
}

//
//	FUNCTION public
//	Schema::BitmapFile::getAreaCategory --
//		bitmapファイルを格納するエリアの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::AreaCategory::Value
//			bitmapファイルを格納するエリアの種別を表す値
//
//	EXCEPTIONS
//

inline
Schema::AreaCategory::Value
Schema::BitmapFile::
getAreaCategory() const
{
	return AreaCategory::Index;
}

_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_BITMAPFILE_H

//
// Copyright (c) 2005, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
