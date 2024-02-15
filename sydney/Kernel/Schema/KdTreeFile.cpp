// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// KdTreeFile.cpp -- KdTreeファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "Schema/AccessFile.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/FieldMap.h"
#include "Schema/FileID.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/KdTreeFile.h"
#include "Schema/Key.h"
#include "Schema/Manager.h"
#include "Schema/NameParts.h"
#include "Schema/Parameter.h"
#include "Schema/Table.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "FileCommon/FileOption.h"

#include "Common/Assert.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "FileCommon/FileOption.h"
//#include "KdTree/FileOption.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::KdTreeFile::KdTreeFile --
//		索引を構成する KdTreeファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		KdTreeファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			KdTreeファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			KdTreeファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			KdTreeファイルを持つ索引を表すクラス
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

KdTreeFile::
KdTreeFile(const Database& database, Table& table, Index& index, const Hint* pHint_, const Hint* pAreaHint_)
	: File(database, table, index, Category::KdTree, pHint_, pAreaHint_)
{ }

//	FUNCTION public
//	Schema::KdTreeFile::create --
//		索引を構成するKdTreeファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			KdTreeファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			KdTreeファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			KdTreeファイルを持つ索引を表すクラス
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
//	EXCEPTIONS
//		なし

//static
File::Pointer
KdTreeFile::
create(Trans::Transaction& cTrans_,
	   const Database& database, Table& table, Index& index,
	   const Hint* pHint_, const Hint* pAreaHint_)
{
	ModAutoPointer<KdTreeFile> pFile =
		new KdTreeFile(database, table, index, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, index.getName()));

	return File::Pointer(pFile.release());
}

//	FUNCTION public
//	Schema::KdTreeFile::setFileID -- ファイル ID を設定する
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
KdTreeFile::setFileID(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID	fileID;

	if (getScope() != Scope::Permanent
		|| Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Temporary::Key),
						  true);

	// KdTreeファイルを格納するエリアをセットする

	setAreaPath(fileID, cTrans_);
            
	// 仮想列を追加する
	ModSize v = createVirtualField(cTrans_);

	const ModVector<Field*>& fields = getField(cTrans_);

	ModSize n = fields.getSize();
	ModSize k = 0; // キーフィールドの数

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
			|| (k == 0 && fields[i]->isKey())	// 第一キーがNULLのタプルは挿入されない
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

// 以下はファイルヒントとして渡されるので設定しない
//
//	// セクション検索を行うモード
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//									KdTree::FileOption::Sectionized::Key),
//					  /***/);
//	// 遅延更新を行うモード
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//									KdTree::FileOption::DelayProc::Key),
//					  /***/);
//	// 異表記正規化を行うモード
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//									KdTree::FileOption::Normalizing::Key),
//					  /***/);
//
//	以下は現在設定しない
//	// トークナイザーに渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							KdTree::FileOption::TokenizerDescription::Key),
//					 /***/);
//
//	// ID圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							KdTree::FileOption::IDCoderDescription::Key),
//					 /***/);
//
//	// 頻度圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							KdTree::FileOption::FreqCoderDescription::Key),
//					 /***/);
//
//	// 文書長圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							KdTree::FileOption::LengthCoderDescription::Key),
//					 /***/);
//
//	// 位置圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//						KdTree::FileOption::LocationCoderDescription::Key),
//					 /***/);
//
//	// 転置リストを物理ファイルではなく一時ファイルに書き込むモード
//
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//						KdTree::FileOption::IsLightWeightFile::Key),
//					 /***/);

	// 生成したファイル ID を設定する

	(void) File::setFileID(fileID);
}

// FUNCTION public
//	Schema::KdTreeFile::createVirtualField -- 読み込み専用の仮想列を追加する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	ModSize
//
// EXCEPTIONS

//virtual
ModSize
KdTreeFile::
createVirtualField(Trans::Transaction& cTrans_)
{
	// 索引がついていたらそのキーとなっている列について仮想列を追加する
	if (Index* pIndex = getIndex(cTrans_)) {
		const ModVector<Key*>& vecKey = pIndex->getKey(cTrans_);
		if (vecKey.getSize() < 0) {
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

		// 仮想列の引数はキーの0番である
		Column* pColumn = (*vecKey.begin())->getColumn(cTrans_);
		; _SYDNEY_ASSERT(pColumn);

		Database* pDatabase = getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);

		LogicalFile::FileID& cFileID = const_cast<LogicalFile::FileID&>(getFileID());
		ModSize n = loadField(cTrans_).getSize();

		for (int i = Field::Function::KdTreeMin; i <= Field::Function::KdTreeMax; ++i) {
			const FieldPointer& pField = addField(static_cast<Field::Function::Value>(i), *pColumn, cTrans_);
			if (getStatus() == Status::Persistent)
				// 永続化されたFileへの追加なのでFileIDにはここで追加する
				setFieldTypeToFileID(cFileID, pField.get(), n++, cTrans_);
		}

		ModSize ret = Field::Function::KdTreeMax - Field::Function::KdTreeMin + 1;
		cFileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
								FileCommon::FileOption::VirtualFieldNumber::Key),
						   ret);
		return ret;
	}
	return 0;
}

//	FUNCTION public
//	Schema::KdTreeFile::isAbleToSearch --
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
KdTreeFile::
isAbleToSearch(const LogicalFile::TreeNodeInterface& pCond_) const
{
	switch (pCond_.getType()) {
	case LogicalFile::TreeNodeInterface::NeighborIn:
		{
			return true;
		}
	}
	return false;
}

//	FUNCTION public
//	Schema::KdTreeFile::isAbleToGetByBitSet --
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
KdTreeFile::
isAbleToGetByBitSet() const
{
	return true;
}

// FUNCTION public
//	Schema::KdTreeFile::isAbleToSeachByBitSet -- 取得対象のRowIDを渡して絞込み検索が可能か
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
KdTreeFile::
isAbleToSearchByBitSet() const
{
	return true;
}

// FUNCTION public
//	Schema::KdTreeFile::isAbleToSort -- キー順の取得が可能か
//
// NOTES
//	KdTreeはキー順での取得はできないがDistance順で返すことができる
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
KdTreeFile::
isAbleToSort() const
{
	return true;
}

// FUNCTION public
//	Schema::KdTreeFile::isHasFunctionField -- 関数フィールドがあるか
//
// NOTES
//
// ARGUMENTS
//	Schema::Field::Function::Value eFunction_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
KdTreeFile::
isHasFunctionField(Schema::Field::Function::Value eFunction_) const
{
	switch (eFunction_) {
	case Field::Function::NeighborId:
	case Field::Function::NeighborDistance:
		{
			return true;
		}
	default:
		{
			return false;
		}
	}
}

// FUNCTION public
//	Schema::KdTreeFile::isHasFunctionField -- 
//
// NOTES
//
// ARGUMENTS
//	LogicalFile::TreeNodeInterface::Type eFunction_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
KdTreeFile::
isHasFunctionField(LogicalFile::TreeNodeInterface::Type eFunction_) const
{
	switch (eFunction_) {
	case LogicalFile::TreeNodeInterface::NeighborID:
	case LogicalFile::TreeNodeInterface::NeighborDistance:
		{
			return true;
		}
	default:
		{
			return false;
		}
	}
}

//	FUNCTION public
//	Schema::KdTreeFile::getSkipInsertType --
//		挿入しないデータの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		挿入しないデータの種別を返す
//
//	EXCEPTIONS

File::SkipInsertType::Value
KdTreeFile::
getSkipInsertType() const
{
	return SkipInsertType::FirstKeyIsNull;
}

//	FUNCTION private
//	Schema::KdTreeFile::createName -- KdTreeファイルの名前を生成する
//
//	NOTES
//		現状では、ファイルの
//		スキーマオブジェクト ID の "KTR_%s" 表現である
//
//	ARGUMENTS
//		const Schema::Object::Name& cParentName_
//			親オブジェクトの名前
//
//	RETURN
//		生成された KdTreeファイルの名前
//
//	EXCEPTIONS

Object::Name
KdTreeFile::createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	return Name(NameParts::File::KdTree).append(cParentName_);
}

//	FUNCTION public
//	Schema::KdTreeFile::serialize --
//		KdTreeファイルを表すクラスのシリアライザー
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
KdTreeFile::
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
// Copyright (c) 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
