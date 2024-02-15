// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// LobFile.cpp -- 長大データファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2003, 2004, 2005, 2010, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/AccessLobFile.h"
#include "Schema/LobFile.h"
#include "Schema/Field.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/Manager.h"
#include "Schema/NameParts.h"
#include "Schema/Parameter.h"
#include "Schema/Table.h"
#include "Schema/SystemTable.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/ObjectID.h"

#include "FileCommon/FileOption.h"

#include "Common/Assert.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::LobFile::LobFile --
//		表を構成するレコードファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		レコードファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			レコードファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			レコードファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		Schema::Object::ID::Value	columnID = Schema::Object::ID::Invalid
//			レコードファイルには通常の表データを格納する使い方と、
//			おもに無制限可変長文字列に対応してヒープファイルとして
//			使う使い方がある
//			columnIDにInvalid以外が指定される場合はヒープとして使うことを表す
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

LobFile::
LobFile(const Database& database, Table& table, const Hint* pHint_, const Hint* pAreaHint_, ID::Value iColumnID_)
	: File(database, table, Category::Lob, pHint_, pAreaHint_),
	  m_iColumnID(iColumnID_)
{ }

//	FUNCTION public
//	Schema::LobFile::create --
//		表を構成するレコードファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			レコードファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			レコードファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		Schema::Column* pColumn_ = 0
//			レコードファイルには通常の表データを格納する使い方と、
//			おもに無制限可変長文字列に対応してヒープファイルとして
//			使う使い方がある
//			pColumn_が指定される場合はヒープとして使うことを表す
//
//	RETURN
//		生成されたファイルのスキーマ情報を表すクラス
//
//	EXCEPTIONS
//		なし

File::Pointer
LobFile::
create(Trans::Transaction& cTrans_, const Database& database, Table& table,
	   const Hint* pHint_, const Hint* pAreaHint_,
	   Column* pColumn_)
{
	ModAutoPointer<LobFile>
		pFile =
		pColumn_ ? new LobFile(database, table, pHint_, pAreaHint_, pColumn_->getID())
		: new LobFile(database, table, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createMyName(cTrans_, pColumn_));

	return File::Pointer(pFile.release());
}

//	FUNCTION public
//	Schema::LobFile::setFileID -- ファイル ID を設定する
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
LobFile::setFileID(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID	fileID;

	if (getScope() != Scope::Permanent
		|| Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Temporary::Key),
						  true);

	// レコードファイルを格納するエリアを設定する

	setAreaPath(fileID, cTrans_);

	const ModVector<Field*>& fields = getField(cTrans_);

	// レコードファイルの0番は常にOIDである

	; _SYDNEY_ASSERT(fields[0]->getType()
					 == LogicalFile::ObjectID().getType());

	ModSize	n = fields.getSize();
	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(fields[i]);

		// フィールドデータ型

		// ダンプ可能な型であるはず
		; _SYDNEY_ASSERT(fields[i]->getType() == Common::DataType::Array
						 || Common::Data::isAbleToDump(fields[i]->getType()));

		fileID.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
			FileCommon::FileOption::FieldType::Key, i),
						  fields[i]->getType());

		// フィールド最大長

		if (fields[i]->getLength())
			fileID.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
				FileCommon::FileOption::FieldLength::Key, i),
							  fields[i]->getLength());

		//...
		// LOBはFixedではありえないのでFieldFixedの設定はしない
		//...

		if (fields[i]->getType() == Common::DataType::Array) {
			// 配列要素の型と最大長

			// ダンプ可能な型であるはず
			; _SYDNEY_ASSERT(Common::Data::isAbleToDump(fields[i]->getElementType()));

			fileID.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
									FileCommon::FileOption::ElementType::Key, i),
							  fields[i]->getElementType());
			if (fields[i]->getElementLength())
				fileID.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY(
									FileCommon::FileOption::ElementLength::Key, i),
								  fields[i]->getElementLength());
		}

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
	}

	// オブジェクトを構成するフィールド数

	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::FieldNumber::Key),
					  n);

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

//	fileID.setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(
//						FileCommon::FileOption::FileSizeMax::Key), /**/);
//
//	// ページサイズ (KB 単位)
//
//	fileID.setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(
//						FileCommon::FileOption::PhysicalPageSize::Key), /**/);
//
//	// 1 データページあたりの領域使用率
//
//	fileID.setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(
//						FileCommon::FileOption::UseRatePerPage::Key), /**/);
//
//	// コンパクション実行領域使用率
//
//	fileID.setInteger(
//		_SYDNEY_SCHEMA_PARAMETER_KEY(
//						FileCommon::FileOption::CompactionRate::Key), /**/);

	// 生成したファイル ID を設定する

	(void) File::setFileID(fileID);
}

//	FUNCTION public
//	Schema::LobFile::isKeyGenerated --
//		ファイルのキーは挿入時に生成されるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		キーが挿入時に生成されるならtrueを返す
//
//	EXCEPTIONS

bool
LobFile::
isKeyGenerated() const
{
	// LOBファイルのキーは挿入時に生成される
	return true;
}

//	FUNCTION public
//	Schema::LobFile::isAbleToFetch --
//		キーを指定したFetchによる取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Fetchが可能ならtrue
//
//	EXCEPTIONS
//		なし

bool
LobFile::
isAbleToFetch() const
{
	return true;
}

// FUNCTION public
//	Schema::LobFile::isAbleToUndo -- 削除や挿入のUndoはドライバーが行うか
//
// NOTES
//		このメソッドがtrueを返すドライバーに対してはUndoのときに
//		ValueフィールドにOIDを入れる
//
// ARGUMENTS
//	なし
//
// RETURN
//		削除や挿入のUndoはドライバーが行うならtrue
//
// EXCEPTIONS

bool
LobFile::
isAbleToUndo() const
{
	return true;
}

//	FUNCTION public
//	Schema::LobFile::getSkipInsertType --
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
LobFile::
getSkipInsertType() const
{
	return SkipInsertType::ValueIsNull;
}

// FUNCTION public
//	Schema::LobFile::getFetchKey -- Fetchに使うことができるフィールドを得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	ModVector<Field*>
//
// EXCEPTIONS

//virtual
ModVector<Field*>
LobFile::
getFetchKey(Trans::Transaction& cTrans_) const
{
	// ObjectIDのフィールドを返す
	ModVector<Field*> vecResult;
	vecResult.reserve(1);
	vecResult.pushBack(getObjectID(cTrans_));
	return vecResult;
}

//	FUNCTION protected
//	Schema::LobFile::packOption --
//		サブクラス固有の付加情報の内容をレコードファイルに格納するために
//		Dataにする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//		Common::Data::Pointer
//			変換されたデータ。呼び出し側でdeleteをする必要がある
//
//	EXCEPTIONS

Common::Data::Pointer
LobFile::
packOption() const
{
	return pack(m_iColumnID);
}

//	FUNCTION protected
//	Schema::LobFile::unpackOption --
//		Dataをサブクラス固有の付加情報に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data& cData_
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
LobFile::
unpackOption(const Common::Data& cData_)
{
	unpack(&cData_, m_iColumnID);
}

// FUNCTION public
//	Schema::LobFile::createName -- LOBファイルの名前を設定する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Name& cParentName_
//	
// RETURN
//	LobFile::Name
//
// EXCEPTIONS

//virtual
LobFile::Name
LobFile::
createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	return createMyName(cTrans_, Column::get(m_iColumnID, getDatabase(cTrans_), cTrans_));
}

//	FUNCTION public
//	Schema::LobFile::serialize --
//		レコードファイルを表すクラスのシリアライザー
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
LobFile::
serialize(ModArchive& archiver)
{
	// まず、ファイル固有の情報をシリアル化する

	File::serialize(archiver);

	if (archiver.isStore()) {

		archiver << m_iColumnID;

	} else {

		// メンバーをすべて初期化しておく

		clear();

		archiver >> m_iColumnID;
	}
}

//	FUNCTION private
//	Schema::LobFile::createMyName -- レコードファイルの名前を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Column* pColumn_ = 0
//			ヒープファイルの場合、対応する列オブジェクト
//
//	RETURN
//		生成されたレコードファイルの名前
//
//	EXCEPTIONS

LobFile::Name
LobFile::createMyName(Trans::Transaction& cTrans_, Column* pColumn_)
{
	; _SYDNEY_ASSERT(pColumn_);
	return Name(NameParts::File::Lob).append(pColumn_->getName());
}

//	FUNCTION public
//	Schema::LobFile::getAccessFile --
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
LobFile::
getAccessFile(Trans::Transaction& cTrans_) const
{
	return new AccessLobFile(cTrans_, *this);
}

//
// Copyright (c) 2003, 2004, 2005, 2010, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
