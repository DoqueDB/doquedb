// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// BtreeFile.cpp -- B+ 木ファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2008, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/BtreeFile.h"
#include "Schema/Field.h"
#include "Schema/FieldMap.h"
#include "Schema/FileID.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/Key.h"
#include "Schema/Manager.h"
#include "Schema/NameParts.h"
#include "Schema/Parameter.h"
#include "Schema/SystemFile.h"
#include "Schema/Table.h"

#include "LogicalFile/FileID.h"
#include "LogicalFile/TreeNodeInterface.h"

#ifndef SYD_CPU_SPARC
#include "Btree/FileOption.h"
#endif
#include "Btree2/FileID.h"

#include "FileCommon/FileOption.h"

#include "Common/Assert.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::BtreeFile::BtreeFile --
//		表を構成する B+ 木ファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		B+ 木ファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			B+ 木ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			B+ 木ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		bool hasAllTuples_ = false
//			ファイルにすべてのタプルが入るならtrue
//		Schema::BtreeFile::Uniqueness::Value uniqueness
//			B+ 木ファイルが格納するオブジェクトの一意性の種類
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

BtreeFile::
BtreeFile(const Database& database, Table& table, const Hint* pHint_, const Hint* pAreaHint_,
		  bool hasAllTuples_, Uniqueness::Value uniqueness)
	: File(database, table, Category::Btree, pHint_, pAreaHint_),
	  _uniqueness(uniqueness), _hasAllTuples(hasAllTuples_)
{ }

//	FUNCTION public
//	Schema::BtreeFile::BtreeFile --
//		索引を構成する B+ 木ファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		B+ 木ファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			B+ 木ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			B+ 木ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			B+ 木ファイルを持つ索引を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		bool hasAllTuples_ = false
//			ファイルにすべてのタプルが入るならtrue
//		Schema::BtreeFile::Uniqueness::Value uniqueness
//			B+ 木ファイルが格納するオブジェクトの一意性の種類
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

BtreeFile::
BtreeFile(const Database& database, Table& table, Index& index, const Hint* pHint_, const Hint* pAreaHint_,
		  bool hasAllTuples_, Uniqueness::Value uniqueness)
	: File(database, table, index, Category::Btree, pHint_, pAreaHint_),
	  _uniqueness(uniqueness), _hasAllTuples(hasAllTuples_)
{ }

#ifdef OBSOLETE // 表にBtreeを使う機能はまだ使用されない

//	FUNCTION public
//	Schema::BtreeFile::create --
//		表を構成する B+ 木ファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			B+ 木ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			B+ 木ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		bool hasAllTuples_ = false
//			ファイルにすべてのタプルが入るならtrue
//		Schema::BtreeFile::Uniqueness::Value uniqueness
//			B+ 木ファイルが格納するオブジェクトの一意性の種類
//
//	RETURN
//		生成されたファイルのスキーマ情報を表すクラス
//
//	EXCEPTIONS
//		なし

File::Pointer
BtreeFile::
create(Trans::Transaction& cTrans_,
	   const Database& database, Table& table,
	   const Hint* pHint_, const Hint* pAreaHint_,
	   bool hasAllTuples_, Uniqueness::Value uniqueness)
{
	ModAutoPointer<BtreeFile>
		pFile = new BtreeFile(database, table, pHint_, pAreaHint_, hasAllTuples_, uniqueness);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, table.getName()));

	return File::Pointer(pFile.release());
}
#endif

//	FUNCTION public
//	Schema::BtreeFile::create --
//		索引を構成する B+ 木ファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			B+ 木ファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			B+ 木ファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			B+ 木ファイルを持つ索引を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		bool hasAllTuples_ = false
//			ファイルにすべてのタプルが入るならtrue
//		Schema::BtreeFile::Uniqueness::Value uniqueness
//			B+ 木ファイルが格納するオブジェクトの一意性の種類
//
//	RETURN
//		生成されたファイルのスキーマ情報を表すクラス
//
//	EXCEPTINS
//		なし

// static
File::Pointer
BtreeFile::
create(Trans::Transaction& cTrans_,
	   const Database& database, Table& table, Index& index,
	   const Hint* pHint_, const Hint* pAreaHint_,
	   bool hasAllTuples_, Uniqueness::Value uniqueness)
{
	ModAutoPointer<BtreeFile>
		pFile = new BtreeFile(database, table, index, pHint_, pAreaHint_, hasAllTuples_, uniqueness);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, index.getName()));

	return File::Pointer(pFile.release());
}

// FUNCTION public
//	Schema::BtreeFile::createSystem -- create system index object
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Database& database
//	Table& table
//	SystemTable::SystemFile& cSystemFile_
//	const char* pszName_
//	ID::Value iObjectID_
//	
// RETURN
//	File::Pointer
//
// EXCEPTIONS

//static
File::Pointer
BtreeFile::
createSystem(Trans::Transaction& cTrans_,
			 const Database& database, Table& table,
			 SystemTable::SystemFile& cSystemFile_,
			 const char* pszName_,
			 ID::Value iObjectID_)
{
	SystemTable::IndexFile* pSystemIndexFile = cSystemFile_.getIndex(pszName_);
	; _SYDNEY_ASSERT(pSystemIndexFile->getCategory() == SystemTable::IndexFile::Category::Btree);

	ModAutoPointer<BtreeFile> pFile = new BtreeFile(database, table);

	// IDを設定する
	pFile->setID(iObjectID_);

	// オブジェクトの名前を設定する
	Name cName(table.getName());
	cName.append(Name(pszName_));
	pFile->setName(pFile->createName(cTrans_, cName));

	// FileIDをセットする
	pFile->File::setFileID(pSystemIndexFile->getFileID());

	// 状態を「永続」にする
	pFile->setStatus(Status::Persistent);

	return File::Pointer(pFile.release());
}

//	FUNCTION public
//	Schema::BtreeFile::setFileID -- ファイル ID を設定する
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
BtreeFile::setFileID(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID	fileID;

	if (getScope() != Scope::Permanent
		|| Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Temporary::Key),
						  true);

	// B+木ファイルを格納するエリアをセットする

	setAreaPath(fileID, cTrans_);
            
	// 仮想列を追加する
	ModSize v = createVirtualField(cTrans_);

	// ユニーク指定

	if (_uniqueness != Uniqueness::None)

		// ★注意★
		// キーがユニークでない場合はバリューも含めてキーとして登録するので
		// 常にキーがユニークになる

		FileID::setMode(fileID,
						_SYDNEY_SCHEMA_PARAMETER_KEY(
										FileCommon::FileOption::Unique::Key),
						_SYDNEY_SCHEMA_PARAMETER_VALUE(
										FileCommon::FileOption::Unique::KeyField));

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
			|| (!hasAllTuples() && k == 0 && fields[i]->isKey())
												// all rows以外では第一キーがNULLのタプルは挿入されない
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

#ifndef SYD_CPU_SPARC
			if (fields[i]->getKey(cTrans_) &&
				fields[i]->getKey(cTrans_)->getOrder()
				== Key::Order::Descending)

				// キーフィールドのソート順(ただし、降順のみ)

				fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									Btree::FileOption::FieldSortOrder::Key),
								  true);
#endif
		} else if (fields[i]->isData()) {

			// キーだけでユニークでない場合はバリューもすべてキーにする

			if (_uniqueness != Uniqueness::OnlyKey)

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
//		Btree::FileOption::FileSizeMax::Key), /**/);

	// ページサイズ (KB 単位)

//	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
//		Btree::FileOption::PhysicalPageSize::Key), /**/);

	// 1 ノードページあたりのオブジェクト数

//	fileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
//		Btree::FileOption::KeyObjectPerNode::Key), /**/);

	// 生成したファイル ID を設定する

	(void) File::setFileID(fileID);
}

// FUNCTION public
//	Schema::BtreeFile::checkFieldType -- Check the validity of FileID contents and modify if needed
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
BtreeFile::
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

//virtual
ModSize
BtreeFile::
createVirtualField(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID& cFileID = const_cast<LogicalFile::FileID&>(getFileID());
	int iVersion;
	if (!cFileID.getInteger(
				_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key),
				iVersion)
		|| iVersion > 0) {
		// 新しいBtreeモジュールなら第一キーの最大値、最小値を得るための仮想列を追加する
		ModSize n = loadField(cTrans_).getSize();

		// 仮想列の引数は第一キーフィールドに対応する列である
		const ModVector<Field*>& vecKey = getField(Field::Category::Key, cTrans_);

		if (vecKey.isEmpty()) {
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

		Field* pSourceField = vecKey[0];
		Column* pColumn = pSourceField->getColumn(cTrans_);
		while (!pColumn) {
			pSourceField = pSourceField->getSource(cTrans_);
			pColumn = pSourceField->getColumn(cTrans_);
		}
		if (pColumn == 0) {
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

		for (int i = Field::Function::BtreeMin; i <= Field::Function::BtreeMax; ++i) {
			const FieldPointer& pField = addField(static_cast<Field::Function::Value>(i), *pColumn, cTrans_);
			if (getStatus() == Status::Persistent)
				// 永続化されたFileへの追加なのでFileIDにはここで追加する
				setFieldTypeToFileID(cFileID, pField.get(), n++, cTrans_);
		}

		ModSize ret = Field::Function::BtreeMax - Field::Function::BtreeMin + 1;
		cFileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
								FileCommon::FileOption::VirtualFieldNumber::Key),
						   ret);
		return ret;
	}
	return 0;
}

//	FUNCTION public
//	Schema::BtreeFile::isKeyUnique --
//		ファイルに格納されるデータがキーについて一意かどうか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		キーについて一意ならtrueを返す
//
//	EXCEPTIONS

bool
BtreeFile::
isKeyUnique() const
{
	return _uniqueness == Uniqueness::OnlyKey
		|| getIndexID() == ID::Invalid;			// isKeyUnique導入前に作られたデータベースでも使うための暫定的な措置
}

//	FUNCTION public
//	Schema::BtreeFile::hasAllTuples --
//		B+ 木ファイルにすべてのタプルが格納されるかを得る
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
Schema::BtreeFile::hasAllTuples() const
{
	return _hasAllTuples
		|| getIndexID() == ID::Invalid;			// hasAllTuples導入前に作られたデータベースでも使うための暫定的な措置
}

//	FUNCTION public
//	Schema::BtreeFile::isAbleToScan --
//		順次取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		bool bAllTuples_
//			trueのとき、hasAllTuplesの返り値に関係なくすべての値を保持するとみなしてよい
//
//	RETURN
//		順次取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
BtreeFile::
isAbleToScan(bool bAllTuples_) const
{
	// すべてのタプルを格納しているなら順次取得可能
	return bAllTuples_ || hasAllTuples();
}

//	FUNCTION public
//	Schema::BtreeFile::isAbleToFetch --
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
BtreeFile::
isAbleToFetch() const
{
	return true;
}

//	FUNCTION public
//	Schema::BtreeFile::isAbleToSearch --
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
BtreeFile::
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
		{
			return true;
		}
	case LogicalFile::TreeNodeInterface::EqualsToNull:
		{
			// IsNullが検索できるのはすべてのタプルを格納しているときのみ
			return hasAllTuples();
		}
	}
	return false;
}

//	FUNCTION public
//	Schema::BtreeFile::isAbleToGetByBitSet --
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
BtreeFile::
isAbleToGetByBitSet() const
{
	return true;
}

// FUNCTION public
//	Schema::BtreeFile::isAbleToSearchByBitSet -- BitSetによる検索が可能か
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
BtreeFile::
isAbleToSearchByBitSet() const
{
	const LogicalFile::FileID& cFileID = getFileID();

	// Btree can search by bitset when version is larger than 1
	return cFileID.getInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Version::Key))
		>= Btree2::FileID::Version2;
}

// FUNCTION public
//	Schema::BtreeFile::isAbleToSort -- キー順の取得が可能か
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
BtreeFile::
isAbleToSort() const
{
	return true;
}

// FUNCTION public
//	Schema::BtreeFile::isHasFunctionField -- 関数フィールドがあるか
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
BtreeFile::
isHasFunctionField(Schema::Field::Function::Value eFunction_) const
{
	return eFunction_ >= Schema::Field::Function::BtreeMin
		&& eFunction_ <= Schema::Field::Function::BtreeMax;
}

// FUNCTION public
//	Schema::BtreeFile::isHasFunctionField -- 
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
BtreeFile::
isHasFunctionField(LogicalFile::TreeNodeInterface::Type eFunction_) const
{
	switch (eFunction_) {
	case LogicalFile::TreeNodeInterface::Min:
	case LogicalFile::TreeNodeInterface::Max:
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
//	Schema::BtreeFile::getSkipInsertType --
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
BtreeFile::
getSkipInsertType() const
{
	return _hasAllTuples ? SkipInsertType::None : SkipInsertType::FirstKeyIsNull;
}

//	FUNCTION protected
//	Schema::BtreeFile::packOption --
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
//			変換されたデータ
//
//	EXCEPTIONS

Common::Data::Pointer
BtreeFile::
packOption() const
{
	int iFlag = _uniqueness;
	if (_hasAllTuples) iFlag += static_cast<int>(Uniqueness::ValueNum);
	return pack(iFlag);
}

//	FUNCTION protected
//	Schema::BtreeFile::unpackOption --
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
BtreeFile::
unpackOption(const Common::Data& cData_)
{
	int iValue;
	if (unpack(&cData_, iValue)) {
		if (iValue >= Uniqueness::ValueNum) {
			_hasAllTuples = true;
			iValue -= static_cast<int>(Uniqueness::ValueNum);
		}
		if (iValue >= Uniqueness::Unknown
			&& iValue < Uniqueness::ValueNum)
			_uniqueness = static_cast<Uniqueness::Value>(iValue);
	}
}

//	FUNCTION private
//	Schema::BtreeFile::createName -- B+ 木ファイルの名前を生成する
//
//	NOTES
//		現状では、ファイルが属する索引の名前(ROWID-OIDなら固定値)の
//		"BTR_%s" 表現である
//
//	ARGUMENTS
//		const Schema::Object::Name& cParentName_
//			親オブジェクトの名前
//
//	RETURN
//		生成された B+ 木ファイルの名前
//
//	EXCEPTIONS

//virtual
Object::Name
BtreeFile::createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	return Name(NameParts::File::Btree).append(cParentName_);
}

//	FUNCTION public
//	Schema::BtreeFile::serialize --
//		B+ 木ファイルを表すクラスのシリアライザー
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
BtreeFile::
serialize(ModArchive& archiver)
{
	// まず、ファイル固有の情報をシリアル化する

	File::serialize(archiver);

	if (archiver.isStore()) {

		// オブジェクトの一意性
		{
		int tmp = _uniqueness;
		archiver << tmp;
		}
		// すべてのタプルが入っているか
		archiver << _hasAllTuples;

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// オブジェクトの一意性
		{
		int tmp;
		archiver >> tmp;
		_uniqueness = static_cast<Uniqueness::Value>(tmp);
		}
		// すべてのタプルが入っているか
		archiver >> _hasAllTuples;
	}
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2007, 2008, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
