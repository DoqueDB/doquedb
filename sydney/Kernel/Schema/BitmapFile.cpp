// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BitmapFile.cpp -- bitmapファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2005, 2006, 2008, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Database.h"
#include "Schema/BitmapFile.h"
#include "Schema/Field.h"
#include "Schema/FieldMap.h"
#include "Schema/FileID.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/Key.h"
#include "Schema/Manager.h"
#include "Schema/NameParts.h"
#include "Schema/Parameter.h"
#include "Schema/Table.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "FileCommon/FileOption.h"

#include "Common/Assert.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::BitmapFile::BitmapFile --
//		索引を構成する bitmapファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		bitmapファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			bitmapファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			bitmapファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			bitmapファイルを持つ索引を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

BitmapFile::
BitmapFile(const Database& database, Table& table, Index& index, const Hint* pHint_, const Hint* pAreaHint_)
	: File(database, table, index, Category::Bitmap, pHint_, pAreaHint_)
{ }

//	FUNCTION public
//	Schema::BitmapFile::create --
//		索引を構成する bitmapファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			bitmapファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			bitmapファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			bitmapファイルを持つ索引を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//
//	RETURN
//		生成されたファイルのスキーマ情報を表すクラス
//
//	EXCEPTINS
//		なし

// static
File::Pointer
BitmapFile::
create(Trans::Transaction& cTrans_,
	   const Database& database, Table& table, Index& index,
	   const Hint* pHint_, const Hint* pAreaHint_)
{
	ModAutoPointer<BitmapFile>
		pFile = new BitmapFile(database, table, index, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, index.getName()));

	return File::Pointer(pFile.release());
}

//	FUNCTION public
//	Schema::BitmapFile::setFileID -- ファイル ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
BitmapFile::setFileID(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID	fileID;

	if (getScope() != Scope::Permanent
		|| Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Temporary::Key),
						  true);

	// bitmapファイルを格納するエリアをセットする

	setAreaPath(fileID, cTrans_);
            
	// 仮想列を追加する
	ModSize v = createVirtualField(cTrans_);

	const ModVector<Field*>& fields = getField(cTrans_);

	ModSize n = fields.getSize();
	ModSize	k = 0;
	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(fields[i]);

		// フィールドに対応した要素を設定する
		setFieldTypeToFileID(fileID, fields[i], i, cTrans_);

		// フィールドヒント

		ModUnicodeString hint;
		if (Column* column = fields[i]->getColumn(cTrans_))
			if (column->getHint()
				&& (column->getHint()->getCategory() & Hint::Category::LogicalFile))
				hint = column->getHint()->getString();

		// NOT NULLであることをファイルドライバーも使うのでHINTにして渡す
		if (!fields[i]->isNullable(cTrans_)		// NOT NULLの列値を格納するならNULLではない
			|| fields[i]->isTupleID(cTrans_)	// ROWIDはNULLではない
		) {
			if (hint.getLength())
				hint.append(',');
			hint.append(Field::getHintNotNull());
		}

		if (hint.getLength())
			fileID.setString(_SYDNEY_SCHEMA_FORMAT_KEY(
								FileCommon::FileOption::FieldHint::Key, i),
							 hint);

		if (fields[i]->isKey()) {

			// キーフィールドの数を数える

			++k;
		}
	}

	// オブジェクトを構成するフィールド数

	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
					  FileCommon::FileOption::FieldNumber::Key),
					  n - v);

	// キーフィールド数

	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
					  FileCommon::FileOption::KeyFieldNumber::Key),
					  k);

	// ファイルヒント

	if (getHint())
		fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
						 FileCommon::FileOption::FileHint::Key),
						 getHint()->getString());

	// エリアヒント

	if (getAreaHint())
		fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
						 FileCommon::FileOption::AreaHint::Key),
						 getAreaHint()->getString());

	// 読み書き属性

	Database* pcDatabase = getDatabase(cTrans_);
	if ( pcDatabase )
	{
		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
		   				  FileCommon::FileOption::ReadOnly::Key),
						  pcDatabase->isReadOnly());
	}

	// マウントフラグ
	//	create 前は必ず false
	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
					  FileCommon::FileOption::Mounted::Key),
					  false);

	// ファイルのロック名取得に関するオプション
	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
						FileCommon::FileOption::DatabaseID::Key),
						getDatabaseID());
	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
						FileCommon::FileOption::TableID::Key),
						getTableID());
	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
						FileCommon::FileOption::FileObjectID::Key),
					    getID());

	// 【注意】	以下はデフォルトのまま

	// ファイルの最大サイズ (KB 単位)

//	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
//		Bitmap::FileOption::FileSizeMax::Key), /**/);

	// ページサイズ (KB 単位)

//	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
//		Bitmap::FileOption::PhysicalPageSize::Key), /**/);

	// 1 ノードページあたりのオブジェクト数

//	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
//		Bitmap::FileOption::KeyObjectPerNode::Key), /**/);

	// 生成したファイル ID を設定する

	(void) File::setFileID(fileID);
}

//	FUNCTION public
//	Schema::BitmapFile::hasAllTuples --
//		bitmapファイルにすべてのタプルが格納されるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		すべてのタプルが格納されるならtrue
//
//	EXCEPTIONS
//		なし

bool
Schema::BitmapFile::hasAllTuples() const
{
	return true;
}

//	FUNCTION public
//	Schema::BitmapFile::isAbleToSearch --
//		条件を指定した検索結果の取得が可能か
//
//	NOTES
//		Schemaモジュールではオペレーターまでのチェックを行う
//		必ずできないことが分かる場合にのみfalseを返すこと
//
//	ARGUMENTS
//		const LogicalFile::TreeNodeInterface& pCond_
//			条件ノード
//
//	RETURN
//		検索が不可能であることが明らかでないならtrueを返す
//
//	EXCEPTIONS

bool
BitmapFile::
isAbleToSearch(const LogicalFile::TreeNodeInterface& pCond_) const
{
	switch (pCond_.getType()) {
	case LogicalFile::TreeNodeInterface::And:
	case LogicalFile::TreeNodeInterface::Or:
		{
			// And/Or でチェックされるときは個別のチェックが終わったあとだと仮定する
			return true;
		}
	case LogicalFile::TreeNodeInterface::Like:
	case LogicalFile::TreeNodeInterface::Equals:
	case LogicalFile::TreeNodeInterface::NotEquals:
	case LogicalFile::TreeNodeInterface::GreaterThan:
	case LogicalFile::TreeNodeInterface::GreaterThanEquals:
	case LogicalFile::TreeNodeInterface::LessThan:
	case LogicalFile::TreeNodeInterface::LessThanEquals:
	case LogicalFile::TreeNodeInterface::NotNull:
	case LogicalFile::TreeNodeInterface::EqualsToNull:
	case LogicalFile::TreeNodeInterface::Between:
		{
			return true;
		}
	}
	return false;
}

//	FUNCTION public
//	Schema::BitmapFile::isAbleToGetByBitSet --
//		取得がRowIDのみのときBitSetによる取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		BitSetによる取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
BitmapFile::
isAbleToGetByBitSet() const
{
	return true;
}


//	FUNCTION public
//	Schema::BitmapFile::isAbleToSearchByBitSet --
//		BitSetによる検索が可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		BitSetによる取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
BitmapFile::
isAbleToSearchByBitSet() const
{
	return true;
}


//	FUNCTION public
//	Schema::BitmapFile::isAbleToBitSetGrouping --
//		Grouping可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		BitSetによる取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
BitmapFile::
isAbleToBitSetSort() const
{
	return true;
}


// FUNCTION public
//	Schema::File::isGettable -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
BitmapFile::
isGettable(Trans::Transaction& cTrans_,
		   const Field* pField_,
		   const LogicalFile::TreeNodeInterface* pScalarField_) const
		   
{
	Field* pKey = getField(Field::Category::Key, cTrans_)[0];
	if (!pField_->isTupleID(cTrans_)
		&& isArbitraryField(pKey, pScalarField_)) {
		return false;
	} else {
		if (pField_) {
			return pField_->isGetable();
		} else {
			return true;
		}
			
	}
}



//	FUNCTION private
//	Schema::BitmapFile::createName -- bitmapファイルの名前を生成する
//
//	NOTES
//		現状では、ファイルが属する索引の名前の
//		"BMP_%s" 表現である
//
//	ARGUMENTS
//		const Schema::Object::Name& cParentName_
//			親オブジェクトの名前
//
//	RETURN
//		生成された bitmapファイルの名前
//
//	EXCEPTIONS

//virtual
Object::Name
BitmapFile::createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	return Name(NameParts::File::Bitmap).append(cParentName_);
}



//	FUNCTION private
//	Schema::BitmapFile::isArbitraryField -- 
//
//	NOTES
//		
//
//	ARGUMENTS
//	   const Field* pKey_
//		索引ファイルのキー
//
//     const LogicalFile::TreeNodeInterface* pField_
//		フィールド

//	RETURN
//		bool
//
//	EXCEPTIONS

//
bool
BitmapFile::isArbitraryField(const Field* pKey_,
							 const LogicalFile::TreeNodeInterface* pField_) const
{

	return  ((pField_->getOptionSize() != 1
			  || pField_->getOptionAt(0)->getType()  != LogicalFile::TreeNodeInterface::Expand)
			 && pKey_->getType() == Common::DataType::Array);
}



//	FUNCTION public
//	Schema::BitmapFile::serialize --
//		bitmapファイルを表すクラスのシリアライザー
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive&			archiver
//			シリアル化先(または元)のアーカイバー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
BitmapFile::
serialize(ModArchive& archiver)
{
	// まず、ファイル固有の情報をシリアル化する

	File::serialize(archiver);

	if (archiver.isStore()) {
	} else {

		// メンバーをすべて初期化しておく

		clear();
	}
}

//
// Copyright (c) 2005, 2006, 2008, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
