// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// RecordFile.cpp -- レコードファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2007, 2011, 2015, 2017, 2023 Ricoh Company, Ltd.
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

#include "Schema/RecordFile.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/Manager.h"
#include "Schema/NameParts.h"
#include "Schema/Parameter.h"
#include "Schema/Table.h"
#include "Schema/SystemTable.h"

#include "LogicalFile/FileDriverManager.h"
#include "LogicalFile/FileDriver.h"
#include "LogicalFile/File.h"
#include "LogicalFile/FileID.h"
#include "LogicalFile/ObjectID.h"

#include "Common/Assert.h"
#include "Common/Configuration.h"

#include "FileCommon/FileOption.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	// レコードファイルに固定長として扱うように指示する
	// フィールドサイズの閾値
	Common::Configuration::ParameterInteger _cFixedSizeMax("Schema_FixedSizeMax", 0, false);
}

//	FUNCTION public
//	Schema::RecordFile::RecordFile --
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

RecordFile::
RecordFile(const Database& database, Table& table, const Hint* pHint_, const Hint* pAreaHint_, ID::Value iColumnID_)
	: File(database, table, Category::Record, pHint_, pAreaHint_),
	  m_iColumnID(iColumnID_)
{ }

#ifdef OBSOLETE // IndexのファイルとしてRecordを使うことはない

//	FUNCTION public
//	Schema::RecordFile::RecordFile --
//		索引を構成するレコードファイルを表すクラスのコンストラクター
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
//		Schema::Index&		index
//			レコードファイルを持つ索引を表すクラス
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

RecordFile::
RecordFile(const Database& database, Table& table, Index& index, const Hint* pHint_, const Hint* pAreaHint_)
	: File(database, table, index, Category::Record, pHint_, pAreaHint_),
	  m_iColumnID(ID::Invalid)
{ }
#endif

//	FUNCTION public
//	Schema::RecordFile::create --
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
RecordFile::
create(Trans::Transaction& cTrans_, const Database& database, Table& table,
	   const Hint* pHint_, const Hint* pAreaHint_,
	   Column* pColumn_)
{
	ModAutoPointer<RecordFile>
		pFile =
		pColumn_ ? new RecordFile(database, table, pHint_, pAreaHint_, pColumn_->getID())
		: new RecordFile(database, table, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createMyName(cTrans_, table.getName(), pColumn_));

	return File::Pointer(pFile.release());
}

#ifdef OBSOLETE // IndexのファイルとしてRecordを使うことはない

//	FUNCTION public
//	Schema::RecordFile::create --
//		索引を構成するレコードファイルのスキーマ情報を表すクラスを生成する
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
//		Schema::Index&		index
//			レコードファイルを持つ索引を表すクラス
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
RecordFile::
create(Trans::Transaction& cTrans_, const Database& database, Table& table, Index& index,
	   const Hint* pHint_, const Hint* pAreaHint_)
{
	ModAutoPointer<RecordFile>
		pFile = new RecordFile(database, table, index, pHint_, pAreaHint_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createMyName(cTrans_, index.getName()));

	return File::Pointer(pFile.release());
}
#endif

//	FUNCTION public
//	Schema::RecordFile::createSystem --
//		システム表を構成するレコードファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		Schema::Database&	database
//			メタデータベースを表すクラス
//		Schema::Table&		table
//			システム表の表オブジェクトを表すクラス
//		SystemTable::SystemFile& cSystemFile_
//			システム表を表すクラス
//		Schema::Object::ID::Value iObjectID_
//			オブジェクトIDの値
//
//	RETURN
//		生成されたファイルのスキーマ情報を表すクラス
//
//	EXCEPTIONS
//		なし

File::Pointer
RecordFile::
createSystem(Trans::Transaction& cTrans_,
			 const Database& database, Table& table,
			 SystemTable::SystemFile& cSystemFile_,
			 ID::Value iObjectID_)
{
	ModAutoPointer<RecordFile>
		pFile = new RecordFile(database, table);

	; _SYDNEY_ASSERT(pFile.get());

	// IDを設定する
	pFile->setID(iObjectID_);

	// オブジェクトの名前を設定する
	pFile->setName(pFile->createName(cTrans_, table.getName()));

	// FileIDをセットする
	pFile->File::setFileID(cSystemFile_.getFileID());

	// 状態を「永続」にする
	pFile->setStatus(Status::Persistent);

	return File::Pointer(pFile.release());
}

//	FUNCTION public
//	Schema::RecordFile::setFileID -- ファイル ID を設定する
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
RecordFile::setFileID(Trans::Transaction& cTrans_)
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

		// ダンプ可能な型であるはず
		; _SYDNEY_ASSERT((fields[i]->getType() == Common::DataType::Array
						  && Common::Data::isAbleToDump(fields[i]->getElementType()))
						 || Common::Data::isAbleToDump(fields[i]->getType()));

		// Fieldの情報からFileIDの内容をセットする
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

		// 閾値を下回る型に対してはFIXEDの指示をHINTにして渡す
		if (fields[i]->getType() != Common::DataType::Array
			&& fields[i]->getLength() > 0
			&& fields[i]->getLength() <= static_cast<ModSize>(_cFixedSizeMax.get())) {
			if (hint.getLength())
				hint.append(',');
			hint.append(Field::getHintFixed());
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

// FUNCTION public
//	Schema::RecordFile::checkFieldType -- Check the validity of FileID contents and modify if needed
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	なし
//
// EXCEPTIONS

//virtual
void
RecordFile::
checkFieldType(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID& cFileID = const_cast<LogicalFile::FileID&>(getFileID());
	if (cFileID.getVersion() < 3) {
		// If FileID's layout version is less than 3,
		// EncodingForm and FieldLength might have an inconsistency.

		ModVector<Field*> vecField;
		int i = 0;
		int iDataType;
		while (cFileID.getInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::FieldType::Key, i), iDataType)) {
			bool bElement = false;
			
			if (iDataType == Common::DataType::Array) {
				// If the field is an array, check the element type.
				iDataType = cFileID.getInteger(_SYDNEY_SCHEMA_FORMAT_KEY(FileCommon::FileOption::ElementType::Key, i));
				bElement = true;
			}

			if (iDataType == Common::DataType::String) {

				if (vecField.isEmpty())
					// Field information is loaded only when it is needed
					vecField = getField(cTrans_);
				; _SYDNEY_ASSERT(vecField.getSize() > static_cast<ModSize>(i));

				// Get the column type.
				; _SYDNEY_ASSERT(vecField[i]);
				; _SYDNEY_ASSERT(vecField[i]->getRelatedColumn(cTrans_));
				const Column::DataType& cColumnType = vecField[i]->getRelatedColumn(cTrans_)->getType();

				ModSize iExpectedLength = 0;

				if (cColumnType.getType() == Column::DataType::Type::UniqueIdentifier)
					cColumnType.setFieldType(0, &iExpectedLength, 0, 0);

				else {
					int iEncodingForm;
					if (!cFileID.getInteger(_SYDNEY_SCHEMA_FORMAT_KEY((bElement
																	   ? FileCommon::FileOption::ElementEncodingForm::Key
																	   : FileCommon::FileOption::FieldEncodingForm::Key),
																	  i),
											iEncodingForm)
						|| iEncodingForm == Common::StringData::EncodingForm::Unknown) {
						// If encoding form is not set or is set to unknown,
						// fieldLength must be doubled to the length of column type.
						iExpectedLength = sizeof(ModUnicodeChar) * cColumnType.getLength();
					}
				}
				if (iExpectedLength > 0) {
					// check the field length
					int iFieldLength;
					if (!cFileID.getInteger(_SYDNEY_SCHEMA_FORMAT_KEY((bElement
																	   ? FileCommon::FileOption::ElementLength::Key
																	   : FileCommon::FileOption::FieldLength::Key),
																	  i),
											iFieldLength)) {
						// never occurred
						; _SYDNEY_ASSERT(false);
					}
					if (cColumnType.getFlag() != Common::SQLData::Flag::Unlimited
						&& static_cast<ModSize>(iFieldLength) != iExpectedLength) {
						// FieldLength must be expected length.
						cFileID.setInteger(_SYDNEY_SCHEMA_FORMAT_KEY((bElement
																	  ? FileCommon::FileOption::ElementLength::Key
																	  : FileCommon::FileOption::FieldLength::Key),
																	 i),
										   iExpectedLength);
					}
				}
			}
			++i;
		}
	}
}

//	FUNCTION public
//	Schema::RecordFile::hasAllTuples --
//		レコードファイルにすべてのタプルが格納されるかを得る
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
Schema::RecordFile::hasAllTuples() const
{
	// ヒープとして使われていなければすべてのタプルが格納されているとみなす
	return m_iColumnID == ID::Invalid;
}

//	FUNCTION public
//	Schema::RecordFile::isKeyGenerated --
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
RecordFile::
isKeyGenerated() const
{
	// レコードファイルのキーは挿入時に生成される
	return true;
}

//	FUNCTION public
//	Schema::RecordFile::isAbleToScan --
//		順次取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		bool bAllTuples_
//			trueのとき、hasAllTuplesの値に関係なくすべてのタプルを保持しているとみなしてよい
//
//	RETURN
//		順次取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
RecordFile::
isAbleToScan(bool bAllTuples_) const
{
	// すべてのタプルを格納しているなら順次取得可能
	// recordの場合は引数のbAllTuplesは無視する
	return hasAllTuples();
}

//	FUNCTION public
//	Schema::RecordFile::isAbleToFetch --
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
RecordFile::
isAbleToFetch() const
{
	return true;
}

//	FUNCTION public
//	Schema::RecordFile::getFetchKey --
//		Fetchに使うことができるフィールドを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		Fetchが可能ならキーとなるフィールドの配列
//
//	EXCEPTIONS

//virtual
ModVector<Field*>
RecordFile::
getFetchKey(Trans::Transaction& cTrans_) const
{
	// ObjectIDのフィールドを返す
	ModVector<Field*> vecResult;
	vecResult.reserve(1);
	vecResult.pushBack(getObjectID(cTrans_));
	return vecResult;
}

// FUNCTION public
//	Schema::RecordFile::isAbleToUndo -- 削除や挿入のUndoはドライバーが行うか
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
RecordFile::
isAbleToUndo() const
{
	// Get logical file driver
	LogicalFile::FileDriver* pDriver =
		LogicalFile::FileDriverManager::getDriver(getDriverID());
	; _SYDNEY_ASSERT(pDriver);

	// Return capability
	return LogicalFile::AutoLogicalFile(*pDriver, getFileID()).isAbleTo(LogicalFile::File::Capability::Undo);
}

//	FUNCTION public
//	Schema::RecordFile::getSkipInsertType --
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
RecordFile::
getSkipInsertType() const
{
	return hasAllTuples() ? SkipInsertType::None : SkipInsertType::ValueIsNull;
}

//	FUNCTION protected
//	Schema::RecordFile::packOption --
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
RecordFile::
packOption() const
{
	return pack(m_iColumnID);
}

//	FUNCTION protected
//	Schema::RecordFile::unpackOption --
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
RecordFile::
unpackOption(const Common::Data& cData_)
{
	unpack(&cData_, m_iColumnID);
}

//	FUNCTION public
//	Schema::RecordFile::serialize --
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
RecordFile::
serialize(ModArchive& archiver)
{
	// まず、ファイル固有の情報をシリアル化する

	File::serialize(archiver);

	if (archiver.isStore()) {

		// ヒープとして使われるか
		archiver << m_iColumnID;

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// ヒープとして使われるか
		archiver >> m_iColumnID;
	}
}

//	FUNCTION private
//	Schema::RecordFile::createName -- レコードファイルの名前を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Object::Name& cParentName_
//			親オブジェクトの名前
//
//	RETURN
//		生成されたレコードファイルの名前
//
//	EXCEPTIONS

Object::Name
RecordFile::
createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	return m_iColumnID == ID::Invalid ?
		createMyName(cTrans_, cParentName_)
		:
		createMyName(cTrans_, cParentName_, Column::get(m_iColumnID, getDatabase(cTrans_), cTrans_));
}

//	FUNCTION private
//	Schema::RecordFile::createMyName -- レコードファイルの名前を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Object::Name& cParentName_
//			親オブジェクトの名前
//		Schema::Column* pColumn_ = 0
//			ヒープファイルの場合、対応する列オブジェクト
//
//	RETURN
//		生成されたレコードファイルの名前
//
//	EXCEPTIONS

Object::Name
RecordFile::createMyName(Trans::Transaction& cTrans_, const Name& cParentName_, Column* pColumn_)
{
	if (!pColumn_) {
		; _SYDNEY_ASSERT(m_iColumnID == ID::Invalid);
		Table* pTable = getTable(cTrans_);
		; _SYDNEY_ASSERT(pTable);
		return Name(NameParts::File::Record).append(cParentName_);
	} else
		return Name(NameParts::File::Heap).append(pColumn_->getName());
}

//
// Copyright (c) 2000, 2001, 2004, 2005, 2007, 2011, 2015, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
