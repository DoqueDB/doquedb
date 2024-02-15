// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ArrayFile.cpp -- arrayファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2007, 2010, 2012, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/ArrayFile.h"
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
//	Schema::ArrayFile::ArrayFile --
//		索引を構成する arrayファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		arrayファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			arrayファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			arrayファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			arrayファイルを持つ索引を表すクラス
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

ArrayFile::
ArrayFile(const Database& database, Table& table, Index& index, const Hint* pHint_, const Hint* pAreaHint_)
	: File(database, table, index, Category::Array, pHint_, pAreaHint_)
{ }

//	FUNCTION public
//	Schema::ArrayFile::create --
//		索引を構成する arrayファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			arrayファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			arrayファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			arrayファイルを持つ索引を表すクラス
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
ArrayFile::
create(Trans::Transaction& cTrans_,
	   const Database& database, Table& table, Index& index,
	   const Hint* pHint_, const Hint* pAreaHint_)
{
	ModAutoPointer<ArrayFile>
		pFile = new ArrayFile(database, table, index, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, index.getName()));

	return File::Pointer(pFile.release());
}

//	FUNCTION public
//	Schema::ArrayFile::setFileID -- ファイル ID を設定する
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
ArrayFile::setFileID(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID	fileID;

	if (getScope() != Scope::Permanent
		|| Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Temporary::Key),
						  true);

	// arrayファイルを格納するエリアをセットする

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
//		Array::FileOption::FileSizeMax::Key), /**/);

	// ページサイズ (KB 単位)

//	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
//		Array::FileOption::PhysicalPageSize::Key), /**/);

	// 1 ノードページあたりのオブジェクト数

//	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
//		Array::FileOption::KeyObjectPerNode::Key), /**/);

	// 生成したファイル ID を設定する

	(void) File::setFileID(fileID);
}

//	FUNCTION public
//	Schema::ArrayFile::hasAllTuples --
//		arrayファイルにすべてのタプルが格納されるかを得る
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
Schema::ArrayFile::hasAllTuples() const
{
	return true;
}

// FUNCTION public
//	Schema::ArrayFile::isAbleToFetch -- キーを指定したFetchによる取得が可能か
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
ArrayFile::
isAbleToFetch() const
{
	// can fetch only when arbitrary elemnt is specified
	return true;
}

//	FUNCTION public
//	Schema::ArrayFile::isAbleToSearch --
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
ArrayFile::
isAbleToSearch(const LogicalFile::TreeNodeInterface& pCond_) const
{
	switch (pCond_.getType()) {
	//case LogicalFile::TreeNodeInterface::And:		// ArrayFile can't evaluate and-predicate
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
//	Schema::ArrayFile::isAbleToGetByBitSet --
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
ArrayFile::
isAbleToGetByBitSet() const
{
	return true;
}

// FUNCTION public
//	Schema::ArrayFile::isAbleToSearchByBitSet -- BitSetによる検索が可能か
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
ArrayFile::
isAbleToSearchByBitSet() const
{
	return true;
}

//	FUNCTION private
//	Schema::ArrayFile::createName -- arrayファイルの名前を生成する
//
//	NOTES
//		現状では、ファイルが属する索引の名前の
//		"ARY_%s" 表現である
//
//	ARGUMENTS
//		const Schema::Object::Name& cParentName_
//			親オブジェクトの名前
//
//	RETURN
//		生成された arrayファイルの名前
//
//	EXCEPTIONS

//virtual
Object::Name
ArrayFile::createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	return Name(NameParts::File::Array).append(cParentName_);
}

//	FUNCTION public
//	Schema::ArrayFile::serialize --
//		arrayファイルを表すクラスのシリアライザー
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
ArrayFile::
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
// Copyright (c) 2007, 2010, 2012, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
