// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// FullTextFile.cpp -- 全文ファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#include "Schema/Database.h"
#include "Schema/FullTextFile.h"
#include "Schema/Table.h"
#include "Schema/Index.h"
#include "Schema/Key.h"
#include "Schema/Field.h"
#include "Schema/FieldMap.h"
#include "Schema/Manager.h"
#include "Schema/NameParts.h"
#include "Schema/Parameter.h"
#include "Schema/AccessFullText.h"

#include "Common/Assert.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "FileCommon/FileOption.h"
#include "FullText/FileOption.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

#ifdef OBSOLETE // 表にFullTextを使う機能はまだ使用されない

//	FUNCTION public
//	Schema::FullTextFile::FullTextFile --
//		表を構成する 全文ファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		全文ファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			全文ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			全文ファイルを持つ表を表すクラス
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

FullTextFile::
FullTextFile(const Database& database, Table& table, const Hint* pHint_, const Hint* pAreaHint_)
	: File(database, table, Category::FullText, pHint_, pAreaHint_)
{ }
#endif

//	FUNCTION public
//	Schema::FullTextFile::FullTextFile --
//		索引を構成する 全文ファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		全文ファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			全文ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			全文ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			全文ファイルを持つ索引を表すクラス
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

FullTextFile::
FullTextFile(const Database& database, Table& table, Index& index, const Hint* pHint_, const Hint* pAreaHint_)
	: File(database, table, index, Category::FullText, pHint_, pAreaHint_)
{ }

#ifdef OBSOLETE // 表にFullTextを使う機能はまだ使用されない

//	FUNCTION public
//	Schema::FullTextFile::create --
//		表を構成する全文ファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		Schema::Database&	database
//			全文ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			全文ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
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

File::Pointer
FullTextFile::
create(Trans::Transaction& cTrans_, const Database& database, Table& table,
	   const Hint* pHint_, const Hint* pAreaHint_)
{
	ModAutoPointer<FullTextFile> pFile = new FullTextFile(database, table, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, table.getName()));

	return File::Pointer(pFile.release());
}
#endif

//	FUNCTION public
//	Schema::FullTextFile::create --
//		索引を構成する全文ファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			全文ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			全文ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			全文ファイルを持つ索引を表すクラス
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

File::Pointer
FullTextFile::
create(Trans::Transaction& cTrans_, const Database& database, Table& table, Index& index,
	   const Hint* pHint_, const Hint* pAreaHint_)
{
	ModAutoPointer<FullTextFile> pFile =
		new FullTextFile(database, table, index, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, index.getName()));

	return File::Pointer(pFile.release());
}

//	FUNCTION public
//	Schema::FullTextFile::setFileID -- ファイル ID を設定する
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
FullTextFile::setFileID(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID	fileID;

	if (getScope() != Scope::Permanent
		|| Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Temporary::Key),
						  true);

	// 全文ファイルを格納するエリアをセットする

	setAreaPath(fileID, cTrans_);
            
	// 仮想列を追加する
	ModSize v = createVirtualField(cTrans_);

	const ModVector<Field*>& fields = getField(cTrans_);

	ModSize n = fields.getSize();
	ModSize k = 0; // キーフィールドの数

	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(fields[i]);

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

	// マージデーモンのためにスキーマインデックスオブジェクトIDを渡す
	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
						FullText::FileOption::IndexID::Key),
					  	getIndexID());

// 以下はファイルヒントとして渡されるので設定しない
//
//	// セクション検索を行うモード
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//									FullText::FileOption::Sectionized::Key),
//					  /***/);
//	// 遅延更新を行うモード
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//									FullText::FileOption::DelayProc::Key),
//					  /***/);
//	// 異表記正規化を行うモード
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//									FullText::FileOption::Normalizing::Key),
//					  /***/);
//
//	以下は現在設定しない
//	// トークナイザーに渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							FullText::FileOption::TokenizerDescription::Key),
//					 /***/);
//
//	// ID圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							FullText::FileOption::IDCoderDescription::Key),
//					 /***/);
//
//	// 頻度圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							FullText::FileOption::FreqCoderDescription::Key),
//					 /***/);
//
//	// 文書長圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//							FullText::FileOption::LengthCoderDescription::Key),
//					 /***/);
//
//	// 位置圧縮/伸長器に渡すパラメーター
//
//	fileID.setString(_SYDNEY_SCHEMA_PARAMETER_KEY(
//						FullText::FileOption::LocationCoderDescription::Key),
//					 /***/);
//
//	// 転置リストを物理ファイルではなく一時ファイルに書き込むモード
//
//	fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
//						FullText::FileOption::IsLightWeightFile::Key),
//					 /***/);

	// 生成したファイル ID を設定する

	(void) File::setFileID(fileID);
}

//	FUNCTION private
//	Schema::FullTextFile::createName -- 全文ファイルの名前を生成する
//
//	NOTES
//		現状では、ファイルの
//		スキーマオブジェクト ID の "FTS_%s" 表現である
//
//	ARGUMENTS
//		const Schema::Object::Name& cParentName_
//			親オブジェクトの名前
//
//	RETURN
//		生成された 全文ファイルの名前
//
//	EXCEPTIONS

Object::Name
FullTextFile::createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	return Name(NameParts::File::FullText).append(cParentName_);
}

// 読み込み専用の仮想列を追加する
//virtual
ModSize
FullTextFile::
createVirtualField(Trans::Transaction& cTrans_)
{
	// 索引がついていたらそのキーとなっている列について仮想列を追加する
	if (Index* pIndex = getIndex(cTrans_)) {
		const ModVector<Key*>& vecKey = pIndex->getKey(cTrans_);
		if (vecKey.getSize() < 0) {
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

		// 仮想列の引数は全文データ列(キーの0番)である
		Column* pColumn = (*vecKey.begin())->getColumn(cTrans_);
		; _SYDNEY_ASSERT(pColumn);

		Database* pDatabase = getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);

		// スコア、セクション番号、関連語などを表すフィールドを追加する
		LogicalFile::FileID& cFileID = const_cast<LogicalFile::FileID&>(getFileID());
		ModSize n = loadField(cTrans_).getSize();

		for (int i = Field::Function::FullTextMin; i <= Field::Function::FullTextMax; ++i) {
			const FieldPointer& pField = addField(static_cast<Field::Function::Value>(i), *pColumn, cTrans_);
			if (getStatus() == Status::Persistent)
				// 永続化されたFileへの追加なのでFileIDにはここで追加する
				setFieldTypeToFileID(cFileID, pField.get(), n++, cTrans_);
		}

		ModSize ret = Field::Function::FullTextMax - Field::Function::FullTextMin + 1;
		cFileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
								FileCommon::FileOption::VirtualFieldNumber::Key),
						   ret);
		return ret;
	}
	return 0;
}

//	FUNCTION public
//	Schema::FullTextFile::isAbleToSearch --
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
FullTextFile::
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
	case LogicalFile::TreeNodeInterface::Contains:
	/*case LogicalFile::TreeNodeInterface::Equals:*/ // Equalsは言語指定ができないので全文でやるのは不適切
		{
			return true;
		}
	}
	return false;
}

//	FUNCTION public
//	Schema::FullTextFile::isAbleToGetByBitSet --
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
FullTextFile::
isAbleToGetByBitSet() const
{
	return true;
}

// FUNCTION public
//	Schema::FullTextFile::isAbleToSeachByBitSet -- 取得対象のRowIDを渡して絞込み検索が可能か
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
FullTextFile::
isAbleToSearchByBitSet() const
{
	return true;
}

// FUNCTION public
//	Schema::FullTextFile::isAbleToSort -- キー順の取得が可能か
//
// NOTES
//	FullTextはキー順での取得はできないがScore順で返すことができる
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
FullTextFile::
isAbleToSort() const
{
	return true;
}

// FUNCTION public
//	Schema::FullTextFile::isAbleToVerifyTuple -- タプル単位の整合性検査が可能か
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
FullTextFile::
isAbleToVerifyTuple() const
{
	return false;
}

// FUNCTION public
//	Schema::FullTextFile::isHasFunctionField -- 関数フィールドがあるか
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
FullTextFile::
isHasFunctionField(Schema::Field::Function::Value eFunction_) const
{
	return eFunction_ >= Schema::Field::Function::FullTextMin
		&& eFunction_ <= Schema::Field::Function::FullTextMax;
}

// FUNCTION public
//	Schema::FullTextFile::isHasFunctionField -- 
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
FullTextFile::
isHasFunctionField(LogicalFile::TreeNodeInterface::Type eFunction_) const
{
	switch (eFunction_) {
	case LogicalFile::TreeNodeInterface::Score:
	case LogicalFile::TreeNodeInterface::Section:
	case LogicalFile::TreeNodeInterface::Word:
	case LogicalFile::TreeNodeInterface::WordDf:
	case LogicalFile::TreeNodeInterface::WordScale:
	case LogicalFile::TreeNodeInterface::AverageLength:
	case LogicalFile::TreeNodeInterface::AverageCharLength:
	case LogicalFile::TreeNodeInterface::AverageWordCount:
	case LogicalFile::TreeNodeInterface::Tf:
	case LogicalFile::TreeNodeInterface::Count:
	case LogicalFile::TreeNodeInterface::ClusterID:
	case LogicalFile::TreeNodeInterface::FeatureValue:
	case LogicalFile::TreeNodeInterface::RoughKwicPosition:
	case LogicalFile::TreeNodeInterface::Existence:
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
//	Schema::FullTextFile::getSkipInsertType --
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
FullTextFile::
getSkipInsertType() const
{
	return SkipInsertType::AllStringKeyIsNull;
}

//	FUNCTION public
//	Schema::FullTextFile::serialize --
//		全文ファイルを表すクラスのシリアライザー
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
FullTextFile::
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

//	FUNCTION public
//	Schema::FullTextFile::getAccessFile --
//		ファイルに対応するAccessFileクラスのインスタンスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//	Schema::AccessFile*
//		ファイルに対応するAccessFileクラスのインスタンス
//
//	EXCEPTIONS

// virtual
AccessFile*
FullTextFile::
getAccessFile(Trans::Transaction& cTrans_) const
{
	return new AccessFullText(cTrans_, *this);
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2010, 2011, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
