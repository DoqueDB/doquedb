// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VectorFile.cpp -- ベクターファイルオブジェクト関連の関数定義
// 
// Copyright (c) 2000, 2001, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/VectorFile.h"
#include "Schema/Field.h"
#include "Schema/FieldMap.h"
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

//#include "Vector/FileOption.h"
#include "FileCommon/FileOption.h"

#include "Common/Assert.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::VectorFile::VectorFile --
//		表を構成する ベクターファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		ベクターファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			ベクターファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			ベクターファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		bool hasAllTuples_ = false
//			ファイルにすべてのタプルが入るならtrue
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

VectorFile::
VectorFile(const Database& database, Table& table, const Hint* pHint_, const Hint* pAreaHint_, bool hasAllTuples_)
	: File(database, table, Category::Vector, pHint_, pAreaHint_), m_bHasAllTuples(hasAllTuples_)
{ }

#ifdef OBSOLETE // IndexのファイルとしてVectorが使われることはない

//	FUNCTION public
//	Schema::VectorFile::VectorFile --
//		索引を構成する ベクターファイルを表すクラスのコンストラクター
//
//	NOTES
//		新たに採番されたスキーマオブジェクト ID を使って、
//		ベクターファイルの名前が生成される
//
//	ARGUMENTS
//		Schema::Database&	database
//			ベクターファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			ベクターファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			ベクターファイルを持つ索引を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		const Schema::Hint* pHint_ = 0
//			ファイルヒント
//		const Schema::Hint* pAreaHint_ = 0
//			エリアヒント
//		bool hasAllTuples_ = false
//			ファイルにすべてのタプルが入るならtrue
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

VectorFile::
VectorFile(const Database& database, Table& table, Index& index, const Hint* pHint_, const Hint* pAreaHint_, bool hasAllTuples_)
	: File(database, table, index, Category::Vector, pHint_, pAreaHint_), m_bHasAllTuples(hasAllTuples_)
{ }
#endif

//	FUNCTION public
//	Schema::VectorFile::create --
//		表を構成する ベクターファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			ベクターファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			ベクターファイルを持つ表を表すクラス
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
//
//	RETURN
//		生成されたファイルのスキーマ情報を表すクラス
//
//	EXCEPTIONS
//		なし

File::Pointer
VectorFile::
create(Trans::Transaction& cTrans_, const Database& database, Table& table,
	   const Hint* pHint_, const Hint* pAreaHint_, bool hasAllTuples_)
{
	ModAutoPointer<VectorFile> pFile = new VectorFile(database, table, pHint_, pAreaHint_, hasAllTuples_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, table.getName()));

	return File::Pointer(pFile.release());
}

#ifdef OBSOLETE // IndexのファイルとしてVectorが使われることはない

//	FUNCTION public
//	Schema::VectorFile::create --
//		索引を構成する ベクターファイルのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			ベクターファイルが存在するデータベースを表すクラス
//		Schema::Table&		table
//			ベクターファイルを持つ表を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Index&		index
//			ベクターファイルを持つ索引を表すクラス
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
//
//	RETURN
//		生成されたファイルのスキーマ情報を表すクラス
//
//	EXCEPTINS
//		なし

// static
File::Pointer
VectorFile::
create(Trans::Transaction& cTrans_, const Database& database, Table& table, Index& index,
	   const Hint* pHint_, const Hint* pAreaHint_, bool hasAllTuples_)
{
	ModAutoPointer<VectorFile> pFile = new VectorFile(database, table, index, pHint_, pAreaHint_, hasAllTuples_);

	; _SYDNEY_ASSERT(pFile.get());

	// オブジェクトの名前を設定する
	(void) pFile->setName(pFile->createName(cTrans_, index.getName()));

	return File::Pointer(pFile.release());
}
#endif

// FUNCTION public
//	Schema::VectorFile::createSystem -- create system index object
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Database& database
//	Table& table
//	SystemTable::SystemFile& cSystemFile_
//	ID::Value iObjectID_
//	
// RETURN
//	File::Pointer
//
// EXCEPTIONS

//static
File::Pointer
VectorFile::
createSystem(Trans::Transaction& cTrans_,
			 const Database& database, Table& table,
			 SystemTable::SystemFile& cSystemFile_,
			 const char* pszName_,
			 ID::Value iObjectID_)
{
	SystemTable::IndexFile* pSystemIndexFile = cSystemFile_.getIndex(pszName_);
	; _SYDNEY_ASSERT(pSystemIndexFile->getCategory() == SystemTable::IndexFile::Category::Vector);

	ModAutoPointer<VectorFile> pFile = new VectorFile(database, table);

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
//	Schema::VectorFile::setFileID -- ファイル ID を設定する
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
VectorFile::setFileID(Trans::Transaction& cTrans_)
{
	LogicalFile::FileID	fileID;

	if (getScope() != Scope::Permanent
		|| Manager::Configuration::isAlwaysTemporary())

		// 一時ファイルかどうかをセットする

		fileID.setBoolean(_SYDNEY_SCHEMA_PARAMETER_KEY(
									FileCommon::FileOption::Temporary::Key),
						  true);

	// ベクターファイルを格納するエリアをセットする

	setAreaPath(fileID, cTrans_);
            
	// 仮想列を追加する
	ModSize v = createVirtualField(cTrans_);

	const ModVector<Field*>& fields = getField(cTrans_);

	ModSize n = fields.getSize();
	ModSize	k = 0;
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
					  n - v);

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

	// 生成したファイル ID を設定する

	(void) File::setFileID(fileID);
}

// FUNCTION public
//	Schema::VectorFile::createVirtualField -- 読み込み専用の仮想列を追加する
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
VectorFile::
createVirtualField(Trans::Transaction& cTrans_)
{
	// 件数を得るための仮想列を追加する
	LogicalFile::FileID& cFileID = const_cast<LogicalFile::FileID&>(getFileID());
	ModSize n = loadField(cTrans_).getSize();

	// 仮想列の引数は第一フィールドに対応する列である
	Field* pSourceField = getFieldByPosition(Field::Position(0), cTrans_);
	if (pSourceField == 0) {
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}
	Column* pColumn = pSourceField->getColumn(cTrans_);
	while (!pColumn) {
		pSourceField = pSourceField->getSource(cTrans_);
		pColumn = pSourceField->getColumn(cTrans_);
	}
	if (pColumn == 0) {
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	for (int i = Field::Function::VectorMin; i <= Field::Function::VectorMax; ++i) {
		const FieldPointer& pField = addField(static_cast<Field::Function::Value>(i), *pColumn, cTrans_);
		if (getStatus() == Status::Persistent)
			// 永続化されたFileへの追加なのでFileIDにはここで追加する
			setFieldTypeToFileID(cFileID, pField.get(), n++, cTrans_);
	}

	ModSize iVirtual = Field::Function::VectorMax - Field::Function::VectorMin + 1;

	cFileID.setInteger(_SYDNEY_SCHEMA_PARAMETER_KEY(
								FileCommon::FileOption::VirtualFieldNumber::Key),
					   iVirtual);
	return iVirtual;
}

//	FUNCTION public
//	Schema::VectorFile::isKeyUnique --
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
VectorFile::
isKeyUnique() const
{
	// ベクターファイルはその作り方からキーについて一意であることは決まっている
	return true;
}

//	FUNCTION public
//	Schema::VectorFile::hasAllTuples --
//		ファイルにすべてのタプルが格納されるかどうか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		すべてのタプルが保持されるならtrueを返す
//
//	EXCEPTIONS

bool
VectorFile::
hasAllTuples() const
{
	return m_bHasAllTuples;
}

//	FUNCTION public
//	Schema::VectorFile::isAbleToScan --
//		順次取得が可能か
//
//	NOTES
//
//	ARGUMENTS
//		bool bAllTuples_
//			trueのとき、hasAllTuplesの返り値に関係なくすべてのタプルを保持しているとみなしてよい
//
//	RETURN
//		順次取得が可能ならtrueを返す
//
//	EXCEPTIONS

bool
VectorFile::
isAbleToScan(bool bAllTuples_) const
{
	// VECTORは順次取得しない
	return false;
}

//	FUNCTION public
//	Schema::VectorFile::isAbleToFetch --
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
VectorFile::
isAbleToFetch() const
{
	return true;
}

//	FUNCTION public
//	Schema::VectorFile::isAbleToSearch --
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
VectorFile::
isAbleToSearch(const LogicalFile::TreeNodeInterface& pCond_) const
{
	switch (pCond_.getType()) {
	case LogicalFile::TreeNodeInterface::And:
	case LogicalFile::TreeNodeInterface::Or:
		{
			// And/Or でチェックされるときは個別のチェックが終わったあとだと仮定する
			return true;
		}
	case LogicalFile::TreeNodeInterface::Equals:
	case LogicalFile::TreeNodeInterface::GreaterThan:
	case LogicalFile::TreeNodeInterface::GreaterThanEquals:
	case LogicalFile::TreeNodeInterface::LessThan:
	case LogicalFile::TreeNodeInterface::LessThanEquals:
		{
			// equals検索のみ可能
			return true;
		}
	}
	return false;
}

//	FUNCTION public
//	Schema::VectorFile::isAbleToGetByBitSet --
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
VectorFile::
isAbleToGetByBitSet() const
{
	return true;
}

// FUNCTION public
//	Schema::VectorFile::isAbleToSort -- キー順の取得が可能か
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
VectorFile::
isAbleToSort() const
{
	return true;
}

// FUNCTION public
//	Schema::VectorFile::isHasFunctionField -- 関数フィールドがあるか
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
VectorFile::
isHasFunctionField(Schema::Field::Function::Value eFunction_) const
{
	return eFunction_ >= Schema::Field::Function::VectorMin
		&& eFunction_ <= Schema::Field::Function::VectorMax;
}

// FUNCTION public
//	Schema::VectorFile::isHasFunctionField -- 
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
VectorFile::
isHasFunctionField(LogicalFile::TreeNodeInterface::Type eFunction_) const
{
	switch (eFunction_) {
	case LogicalFile::TreeNodeInterface::Count:
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
//	Schema::VectorFile::getSkipInsertType --
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
VectorFile::
getSkipInsertType() const
{
	return hasAllTuples() ? SkipInsertType::None : SkipInsertType::FirstKeyIsNull;
}

//	FUNCTION protected
//	Schema::VectorFile::packOption --
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
VectorFile::
packOption() const
{
	return pack(m_bHasAllTuples?1:0);
}

//	FUNCTION protected
//	Schema::VectorFile::unpackOption --
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
VectorFile::
unpackOption(const Common::Data& cData_)
{
	int iValue;
	if (unpack(&cData_, iValue))
		m_bHasAllTuples = iValue;
}

//	FUNCTION private
//	Schema::VectorFile::createName -- ベクターファイルの名前を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成された ベクターファイルの名前
//
//	EXCEPTIONS

Object::Name
VectorFile::
createName(Trans::Transaction& cTrans_, const Name& cParentName_)
{
	if (getIndexID() == ID::Invalid) {
		return Name(NameParts::File::Vector).append(cParentName_).append(NameParts::File::Conversion);

	} else {
		return Name(NameParts::File::Vector).append(cParentName_);
	}
}

//	FUNCTION public
//	Schema::VectorFile::serialize --
//		ベクターファイルを表すクラスのシリアライザー
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
VectorFile::
serialize(ModArchive& archiver)
{
	// まず、ファイル固有の情報をシリアル化する

	File::serialize(archiver);

	if (archiver.isStore()) {

		archiver << m_bHasAllTuples;

	} else {

		// メンバーをすべて初期化しておく

		clear();

		archiver >> m_bHasAllTuples;
	}
}

//
// Copyright (c) 2000, 2001, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
