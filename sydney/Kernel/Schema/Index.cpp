// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Index.cpp -- 索引関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "Schema/Index.h"
#include "Schema/Area.h"
#include "Schema/AreaCategory.h"
#include "Schema/AreaContent.h"
#include "Schema/ArrayIndex.h"
#include "Schema/AutoRWLock.h"
#include "Schema/BitmapIndex.h"
#include "Schema/BtreeIndex.h"
#include "Schema/Column.h"
#include "Schema/Constraint.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FullTextIndex.h"
#include "Schema/FakeError.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/FileMap.h"
#include "Schema/Hint.h"
#include "Schema/KdTreeIndex.h"
#include "Schema/Key.h"
#include "Schema/KeyMap.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Message_VerifyStarted.h"
#include "Schema/Message_VerifyFinished.h"
#include "Schema/Meta.h"
#include "Schema/NameParts.h"
#include "Schema/Object.h"
#include "Schema/ObjectTemplate.h"
#include "Schema/Parameter.h"
#include "Schema/PathParts.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/SystemTable_File.h"
#include "Schema/Table.h"
#include "Schema/Utility.h"
#include "Schema/FileVerify.h"

#include "Statement/AlterIndexAction.h"
#include "Statement/AlterIndexStatement.h"
#include "Statement/AreaOption.h"
#include "Statement/ColumnNameList.h"
#include "Statement/ColumnName.h"
#include "Statement/DropIndexStatement.h"
#include "Statement/Hint.h"
#include "Statement/Identifier.h"
#include "Statement/IndexDefinition.h"
#include "Statement/Type.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/NullData.h"

#include "Exception/AreaNotFound.h"
#include "Exception/BadArgument.h"
#include "Exception/FakeError.h"
#include "Exception/IndexAlreadyDefined.h"
#include "Exception/IndexNotFound.h"
#include "Exception/InvalidAreaSpecification.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/SystemTable.h"
#include "Exception/TableNotFound.h"
#include "Exception/TemporaryTable.h"
#include "Exception/TooLongObjectName.h"
#include "Exception/Unexpected.h"

#include "Os/Path.h"

#include "FileCommon/FileOption.h"

#include "Trans/Transaction.h"

#include "ModVector.h"
#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

	namespace _Name
	{
		// 名称の重複を調べる
		bool _checkExistence(Trans::Transaction& cTrans_, Database& cDatabase_, const Index* pIndex_, bool bNoCancel_ = false);

	} // namespace _Name

	//	CONST local
	//	Schema::Index::_iAreaNumber -- 索引が関係するエリアの数
	//
	//	NOTES

	const int _iAreaNumber = 1;

} // namespace

/////////////////////
// _Name
/////////////////////

//	FUNCTION local
//	_Name::_checkExistence -- 表名の重複を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			表が属するデータベース
//		const Schema::Index* pIndex_
//			作成しようとしている表
//		bool bNoCancel_ = false
//			trueなら重複していた場合に常に例外が飛ぶ
//
//	RETURN
//		true ... 同じ名前のものが存在している、または作成中である
//		false... 同じ名前のものはない
//
//	EXCEPTIONS
//		Exception::IndexAlreadyDefined
//			同じ名前のものが存在しており、CanceledWhenDuplicatedがfalseである

bool
_Name::_checkExistence(Trans::Transaction& cTrans_,
					   Database& cDatabase_,
					   const Index* pIndex_,
					   bool bNoCancel_ /* = false */)
{
	if (pIndex_->getName().getLength() > Manager::ObjectName::getMaxLength()) {
		// 名称が制限長を超えていたらエラー
		_SYDNEY_THROW2(Exception::TooLongObjectName,
					   pIndex_->getName().getLength(), Manager::ObjectName::getMaxLength());
	}

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (Manager::ObjectName::reserve(pIndex_) == false) {

		if (!bNoCancel_ && Manager::Configuration::isCanceledWhenDuplicated()) {
			// trueを返し後の処理は何もしない
			SydInfoMessage
				<< "Index definition of the same name in progress("
				<< pIndex_->getName()
				<< ") canceled"
				<< ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage
				<< "Index definition of the same name in progress("
				<< pIndex_->getName()
				<< ")"
				<< ModEndl;
			_SYDNEY_THROW2(Exception::IndexAlreadyDefined, pIndex_->getName(), cDatabase_.getName());
		}
	}

	// さらに、同じ名前のエリアがすでにないか調べ、
	// 同時に現在のエリアをマネージャーに読み込んでおく
	// ★注意★
	// doAfterPersistの中でマネージャーに追加されるので
	// ここで読み込んでおかないと追加のときに不完全なIndexを
	// 読み込んでしまう

	bool bFound = false;
	try {
		bFound = (Index::get(pIndex_->getName(), &cDatabase_, cTrans_) != 0);
	} catch (...) {
		Manager::ObjectName::withdraw(pIndex_);
		_SYDNEY_RETHROW;
	}
	if (bFound) {

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(pIndex_);

		if (!bNoCancel_ && Manager::Configuration::isCanceledWhenDuplicated()) {
			// trueを返し後の処理は何もしない
			SydInfoMessage << "Duplicated index definition("
						   << pIndex_->getName()
						   << ") canceled"
						   << ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated index definition("
						   << pIndex_->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::IndexAlreadyDefined, pIndex_->getName(), cDatabase_.getName());
		}
	}
	return false;
}

//	FUNCTION public
//	Schema::Index::Index -- 索引を表すクラスのデフォルトコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Index::Category::Value eCategory_ = Schema::Index::Category::Unknown
//			索引のカテゴリー
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Index::
Index(Category::Value eCategory_ /* = Category::Unknown */)
	: Object(Object::Category::Index),
	  _category(eCategory_),
	  _unique(false),
	  _clustered(false),
	  _hasAllTuples(false),
	  m_bOffline(false),
	  _table(0),
	  _keys(0),
	  m_pFile(),
	  m_iFileID(ID::Invalid),
	  m_iAreaID(ID::Invalid),
	  m_pArea(0),
	  m_iLogAreaID(ID::Invalid),
	  m_pLogArea(0),
	  m_pHint(0),
	  m_pAreaHint(0),
	  m_pPath(0)
{ }

//	FUNCTION public
//	Schema::Index::Index -- 索引定義からの索引を表すクラスのコンストラクター
//
//	NOTES
//		索引を表すクラスを生成するだけで、「索引」表は更新されない
//		索引の親オブジェクトはデータベースから表に変更された
//
//	ARGUMENTS
//		Schema::Index::Category::Value eCategory_ = Schema::Index::Category::Unknown
//			索引のカテゴリー
//		Schema::Database&	database
//			索引が存在するデータベースを表すクラス
//		Schema::Table&	table
//			索引がつく表を表すクラス
//		const Schema::Object::Name& cName_
//			索引名
//		Schema::Hint* pHint_
//			索引のヒント

Index::
Index(Category::Value eCategory_,
	  const Database& database, Table& table,
	  const Name& cName_, Hint* pHint_)
	: Object(Object::Category::Index, table.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, table.getID(), database.getID(),
			 cName_),
	  _category(eCategory_),
	  _unique(pHint_ && (pHint_->getCategory() & Hint::Category::Unique)),
	  _clustered(false),
	  _hasAllTuples(false),
	  m_bOffline(false),
	  _table(&table),
	  _keys(0),
	  m_pFile(),
	  m_iFileID(ID::Invalid),
	  m_iAreaID(ID::Invalid),
	  m_pArea(0),
	  m_iLogAreaID(ID::Invalid),
	  m_pLogArea(0),
	  m_pHint(pHint_),
	  m_pAreaHint(0),
	  m_pPath(0)
{ }

//	FUNCTION public
//	Schema::Index::Index -- 制約からの索引を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& database
//			索引が属するデータベース
//		Schema::Table&	table
//			索引が存在する表を表すクラス
//		const Schema::Constraint&	constraint
//			索引に対応する制約を表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Index::
Index(Category::Value eCategory_,
	  const Database& database, Table& table, const Constraint& constraint)
	: Object(Object::Category::Index, table.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, table.getID(), database.getID(),
			 constraint.getName()),
	  _category(eCategory_),
	  _unique(false),
	  _clustered(false),
	  _hasAllTuples(false),
	  m_bOffline(false),
	  _table(&table),
	  _keys(0),
	  m_pFile(),
	  m_iFileID(ID::Invalid),
	  m_iAreaID(ID::Invalid),
	  m_pArea(0),
	  m_iLogAreaID(ID::Invalid),
	  m_pLogArea(0),
	  m_pHint(0),
	  m_pAreaHint(0),
	  m_pPath(0)
{
	// Cluster属性は制約に指定されたものにする
	_clustered = constraint.isClustered();

	// 制約の種別を得て処理する

	switch (constraint.getCategory()) {
	case Constraint::Category::OldPrimaryKey:
	case Constraint::Category::PrimaryKey:
	{
		// 主キーに対応する索引ならすべてのタプルが入り、常にuniqueである
		_hasAllTuples = true;
		_unique = true;
		break;
	}
	case Constraint::Category::OldUnique:
	case Constraint::Category::Unique:
	{
		// 常にuniqueである
		_unique = true;
		break;
	}
	case Constraint::Category::ForeignKey:
	case Constraint::Category::ReferedKey:
	{
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
	}
}

//	FUNCTION public
//	Schema::Index::Index -- データベースからの索引を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			索引が存在するデータベースを表すクラス
//	RETURN
//		なし
//
//	EXCEPTIONS

Index::
Index(Category::Value eCategory_,
	  const Database& database, const LogData& cLogData_)
	: Object(Object::Category::Index, database.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, ID::Invalid, database.getID()),
	  _category(eCategory_),
	  _unique(false),
	  _clustered(false),
	  _hasAllTuples(false),
	  m_bOffline(false),
	  _table(0),
	  _keys(0),
	  m_pFile(),
	  m_iFileID(ID::Invalid),
	  m_iAreaID(ID::Invalid),
	  m_pArea(0),
	  m_iLogAreaID(ID::Invalid),
	  m_pLogArea(0),
	  m_pHint(0),
	  m_pAreaHint(0),
	  m_pPath(0)
{
	// ログデータの内容を反映する
	// ★注意★
	// makeLogDataの実装が変わったらここも変える
	// IDは ここでは設定しない

	setParentID(cLogData_.getID(Log::TableID));
	setName(cLogData_.getString(Log::Name));

	// 後はpackMetaFieldにより作成されたログデータなので単純にunpackMetaFieldする
	if (!unpackMetaField(cLogData_[Log::Category].get(), Meta::Index::Category)
		|| !unpackMetaField(cLogData_[Log::Create::Flag].get(), Meta::Index::Flag)
		|| !unpackMetaField(cLogData_[Log::Create::Hint].get(), Meta::Index::Hint)
		|| !unpackMetaField(cLogData_[Log::Create::AreaHint].get(), Meta::Index::AreaHint)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	// エリアID割り当ては後でUNDO情報により上書きされる可能性がある
	const ModVector<ID::Value>& vecAreaID = cLogData_.getIDs(Log::Create::AreaIDs);
	setAreaID(vecAreaID);
}

//	FUNCTION public
//	Schema::Index::~Index -- 索引を表すクラスのデストラクター
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

Index::
~Index()
{
	destruct();
}

//	FUNCTION private
//	Schema::Index::destruct -- 索引を表すクラスのデストラクター下位関数
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

void
Index::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteするところでresetを呼ぶ
	// ここでは領域を開放するのみ

	clearKey();
	clearHint();
	clearAreaHint();
	clearPath();
}

//	FUNCTION public
//		Schema::Index::getNewInstance -- オブジェクトを新たに取得する
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			元になるデータ
//
//	RETURN
//		新規に作成されたオブジェクト
//
//	EXCEPTIONS

// static
Index*
Index::
getNewInstance(const Common::DataArrayData& cData_)
{
	if (cData_.getCount() <= Meta::Index::Category) {
		SydErrorMessage
			<< "Index get new instance failed. Insufficient data array data."
			<< ModEndl;
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	int iCategory;
	if (!unpack(cData_.getElement(Meta::Index::Category).get(), iCategory)) {
		SydErrorMessage
			<< "Can't get index category."
			<< ModEndl;
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	ModAutoPointer<Index> pObject;

	switch (iCategory) {
	case Category::Normal:
		pObject = new BtreeIndex();			break;
	case Category::FullText:
		pObject = new FullTextIndex();		break;
	case Category::Bitmap:
		pObject = new BitmapIndex();		break;
	case Category::Array:
		pObject = new ArrayIndex();			break;
	case Category::KdTree:
		pObject = new KdTreeIndex();		break;
	default:
		// 例外送出
		SydErrorMessage
			<< "Index get new instance failed. Illegal index category."
			<< ModEndl;
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		break;
	}
	; _SYDNEY_ASSERT(pObject.get());

	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION public
//	Schema::Index::getNewInstance -- 新たなIndexオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database&	database
//			索引が存在するデータベースを表すクラス
//		Schema::Table&	table
//			索引がつく表を表すクラス
//		const Schema::Object::Name& cName_
//			索引名
//		int iIndexType_
//			索引の種別
//		Schema::Hint* pHint_
//			索引のヒント

//static
Index*
Index::
getNewInstance(const Database& database, Table& table,
			   const Name& cName_, int iIndexType_, Hint* pHint_)
{
	// 索引の種別に対応するオブジェクトを作成する
	switch (iIndexType_) {
	case Statement::IndexDefinition::FullText:
		{
			return new FullTextIndex(database, table, cName_, pHint_);
		}
	case Statement::IndexDefinition::Bitmap:
		{
			return new BitmapIndex(database, table, cName_, pHint_);
		}
	case Statement::IndexDefinition::Array:
		{
			return new ArrayIndex(database, table, cName_, pHint_);
		}
	case Statement::IndexDefinition::Clustered:
		{
			Index* pIndex = new BtreeIndex(database, table, cName_, pHint_);
			pIndex->_clustered = true;
			return pIndex;
		}
	case Statement::IndexDefinition::Unique:
		{
			Index* pIndex = new BtreeIndex(database, table, cName_, pHint_);
			pIndex->_unique = true;
			return pIndex;
		}
	case Statement::IndexDefinition::AllRows:
		{
			Index* pIndex = new BtreeIndex(database, table, cName_, pHint_);
			pIndex->_hasAllTuples = true;
			return pIndex;
		}
	case Statement::IndexDefinition::None:
	case Statement::IndexDefinition::NonClustered:
		{
			return new BtreeIndex(database, table, cName_, pHint_);
		}
	case Statement::IndexDefinition::KdTree:
		{
			return new KdTreeIndex(database, table, cName_, pHint_);
		}
	default:
		; _SYDNEY_ASSERT(false);
	}
	return 0;
}

//	FUNCTION public
//	Schema::Index::getNewInstance -- 新たなIndexオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database&	database
//			索引が存在するデータベースを表すクラス
//		Schema::Table&	table
//			索引がつく表を表すクラス
//		const Schema::Constraint& cConstraint_
//			作成する索引によって実現される制約

//static
Index*
Index::
getNewInstance(const Database& database, Table& table, const Constraint& cConstraint_)
{
	// 制約に対応する索引は常にB+木索引である
	return new BtreeIndex(database, table, cConstraint_);
}

//	FUNCTION public
//	Schema::Index::getNewInstance -- 新たなIndexオブジェクトを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database&	database
//			索引が存在するデータベースを表すクラス
//		const Schema::LogData& cLogData_
//			索引作成を表すログデータ

//static
Index*
Index::
getNewInstance(const Database& database, const LogData& cLogData_)
{
	// 索引の種別に対応するオブジェクトを作成する
	switch (getCategory(cLogData_)) {
	case Category::Normal:
		{
			return new BtreeIndex(database, cLogData_);
		}
	case Category::FullText:
		{
			return new FullTextIndex(database, cLogData_);
		}
	case Category::Bitmap:
		{
			return new BitmapIndex(database, cLogData_);
		}
	case Category::Array:
		{
			return new ArrayIndex(database, cLogData_);
		}
	case Category::KdTree:
		{
			return new KdTreeIndex(database, cLogData_);
		}
	default:
		; _SYDNEY_ASSERT(false);
	}
	return 0;
}

//	FUNCTION public
//	Schema::Index::create -- SQL の索引定義から索引を実際に定義する
//
//	NOTES
//		システム表への永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database&	database
//			索引を定義するデータベースを表すクラス
//		Statement::IndexDefinition&	statement
//			解析済の SQL の索引定義
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//
//	RETURN
//		定義された索引オブジェクト
//
//	EXCEPTIONS
//		Exception::TableNotFound
//			指定された名前の表はない

// static
Index::Pointer
Index::
create(Trans::Transaction& cTrans_,
	   Database& database, const Statement::IndexDefinition& statement,
	   LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 索引定義から必要な情報を取得する

	// 表名
	; _SYDNEY_ASSERT(statement.getTableName());
	; _SYDNEY_ASSERT(statement.getTableName()->getIdentifier());
	const Name& cTableName = *(statement.getTableName()->getIdentifier());

	// 索引名
	; _SYDNEY_ASSERT(statement.getName());
	; _SYDNEY_ASSERT(statement.getName()->getIdentifier());
	const Name& cName = *(statement.getName()->getIdentifier());

	// 索引タイプ
	int iIndexType = statement.getIndexType();

	// ヒント
	ModAutoPointer<Hint> pHint;
	if (Statement::Hint* hint = statement.getHint())
		pHint = new Hint(*hint);

	// 索引のつく表を得る
	Table* table =
		database.getTable(cTableName, cTrans_, true /* internal */);
	if (!table) {
		// 指定された名称をもつ表がないので例外送出
		_SYDNEY_THROW2(Exception::TableNotFound,
					   cTableName, database.getName());
	}
	; _SYDNEY_ASSERT(table);
	if (table->isSystem()) {
		// システム表に索引は作成できない
		_SYDNEY_THROW1(Exception::SystemTable, cTableName);
	}

	if (table->isTemporary()
		&& !Table::isToBeTemporary(cName)) {
		// 一時表につける索引は#で始まる名前でなければいけない
		SydInfoMessage << "Can't create ordinal index onto temporary table "
					   << cTableName << "." << ModEndl;
		_SYDNEY_THROW0(Exception::TemporaryTable);
	}

#ifdef OBSOLETE // partial import has been disabled
	if (pHint.get() && (pHint->getCategory() & Hint::Category::PartialImport)
		&& pHint->getLowerBound() != TupleID::Invalid) {
		
		// Partial Importのときは下限が設定されているので既存の index を取得する

		Index* index = Index::get(cName, &database, cTrans_);
		if (index) {
			index->clearHint();
			index->setHint(pHint.release());
		}
		// ObjectPointerにしても解放されないようにconstにして返す
		return static_cast<const Index*>(index);
	}
#endif

	// 与えられた索引定義から、定義しようとしている索引を表すクラスを生成する
	Pointer index = getNewInstance(database, *table, cName, iIndexType, pHint.get());
	pHint.release(); // newが失敗すると解放できないので成功してからreleaseする

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, database, index.get())) {
		return Pointer();
	}

	try {
		// IDをふり、状態を変える
		index->Object::create(cTrans_);
		SCHEMA_FAKE_ERROR("Schema::Index", "Create", "Created");

		ModVector<ID::Value> vecAreaID(_iAreaNumber, ID::Invalid);
		if (!index->isTemporary()) {
			// エリアに指定されている名前があったらエリアのIDを取得して設定する
			if (Statement::AreaOption* pAreaOption = statement.getAreaOption()) {
				if (database.isSlaveStarted()) {
					// area is not allowed for replicated database
					_SYDNEY_THROW1(Exception::InvalidAreaSpecification, database.getName());
				}

				// エリア変更と同じ関数でエリアIDを取得する
				ModVector<ID::Value> vecDummy(_iAreaNumber, ID::Invalid);
				index->prepareSetArea(cTrans_, *pAreaOption, vecDummy, vecAreaID);

				// 配列の内容をメンバーにセットする
				index->setAreaID(vecAreaID);

				// エリア格納関係を作る
				index->createAreaContent(cTrans_);
			}
			SCHEMA_FAKE_ERROR("Schema::Index", "Create", "AreaCreated");
		}

		// エリアIDの指定から実質的に割り当てられるエリアIDを取得する
	    ModVector<ID::Value> vecEffectiveAreaID;
		Index::getEffectiveAreaID(vecAreaID, table->getAreaID(), index->getAreaCategory(), vecEffectiveAreaID);

		// キー定義のログデータを入れる配列を定義しておく
		ModAutoPointer<Common::DataArrayData> pKeyLogData = new Common::DataArrayData();

		// キーオブジェクトを作成する
		index->createKey(cTrans_, *table, statement, *pKeyLogData);

		// 索引を構成するファイルを表すクラスを生成する
		File::Pointer pFile = File::create(cTrans_, index->createFile(cTrans_), *table, *index);
		SCHEMA_FAKE_ERROR("Schema::Index", "Create", "FileCreated");

		// ファイルを表すクラスにフィールドを設定する
		index->createField(cTrans_, *pFile);

		// 生成するファイルのファイル ID を設定する
		pFile->setFileID(cTrans_);

		// 索引にファイルをセットする
		index->setFile(pFile);
		; _SYDNEY_ASSERT(index->getFile(cTrans_));

		if (!index->isTemporary()) {
			// 索引自身のログデータを作る
			// ★注意★
			// ファイルのパスをログに出力する必要があるので
			// setFileしてからでないと作ることができない
			index->makeLogData(cTrans_, cLogData_);

			// キー定義、エリアID、エリアパスを追加する
			cLogData_.addData(pKeyLogData.release());
			cLogData_.addIDs(vecAreaID);
			cLogData_.addIDs(table->getAreaID());
			cLogData_.addData(Area::getPathArray(cTrans_, database, vecEffectiveAreaID));

			// ファイル定義のログデータを作る
			ModAutoPointer<Common::DataArrayData> pFileLogData = new Common::DataArrayData();
			pFile->makeLogData(cTrans_, *pFileLogData);
			cLogData_.addData(pFileLogData.release());

			// obtain the last objectID for log data
			ID::Value iLastID = ObjectID::getLastValue(cTrans_, &database);
			cLogData_.addID(iLastID);

			; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Create::Num);
		}

	} catch (...) {

		// 索引以下のオブジェクトに対して作成の取り消しを行う
		index->drop(cTrans_);

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(index.get());

		_SYDNEY_RETHROW;
	}

	// 生成された索引のスキーマオブジェクトを返す
	return index;
}

//	FUNCTION public
//	Schema::Index::create -- 制約定義に対応する索引を定義する
//
//	NOTES
//		システム表への永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&	table
//			索引を定義する表を表すクラス
//		Schema::Constraint& constraint
//			対象の制約
//
//	RETURN
//		定義された索引オブジェクト
//
//	EXCEPTIONS

// static
Index::Pointer
Index::
create(Trans::Transaction& cTrans_,
	   Table& table, const Constraint& constraint)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// ログの記録は要らない

	// テーブルのチェック
	if (table.isSystem()) {
		// システム表に索引は作成できない
		_SYDNEY_THROW1(Exception::SystemTable, table.getName());
	}
#ifdef OBSOLETE // 制約で作られる索引なら作っても問題ない
	if (table.isTemporary()) {
		// 一時表に索引は作成できない
		SydInfoMessage << "Can't create index onto temporary table " << table.getName() << "." << ModEndl;
		_SYDNEY_THROW0(Exception::TemporaryTable);
	}
#endif

	// 与えられた制約、定義しようとしている索引を表すクラスを生成する
	// ★注意★
	// 制約の時点で重複チェックが行われるので名前重複のチェックは不要だが
	// 表に索引をloadしておく必要があるので重複していないことの確認をする

	Pointer index = getNewInstance(*table.getDatabase(cTrans_), table, constraint);
	; _SYDNEY_ASSERT(index.get());

	if (table.getIndex(index->getName(), cTrans_)) {
		_SYDNEY_THROW2(Exception::IndexAlreadyDefined, index->getName(), table.getDatabase(cTrans_)->getName());
	}

	try {
		// IDをふり、状態を変える
		// Constraintに索引IDが設定されていたらそれを使う
		index->Object::create(cTrans_, constraint.getIndexID());
		SCHEMA_FAKE_ERROR("Schema::Index", "CreateConstraintIndex", "Created");

		// キーオブジェクトを作成する
		index->createKey(cTrans_, table, constraint);

		// 索引を構成するファイルおよびそのフィールドを表すクラスを生成する
		File::Pointer pFile = File::create(cTrans_, index->createFile(cTrans_), table, *index);
		index->createField(cTrans_, *pFile);
		SCHEMA_FAKE_ERROR("Schema::Index", "CreateConstraintIndex", "FileCreated");

		// 生成するファイルのファイル ID を設定する
		pFile->setFileID(cTrans_);

		// 索引にファイルをセットする
		index->setFile(pFile);
		; _SYDNEY_ASSERT(index->getFile(cTrans_));

	} catch (...) {
		// 索引以下のオブジェクトに対して作成の取り消しを行う
		index->drop(cTrans_);
		_SYDNEY_RETHROW;
	}

	// 生成された索引のスキーマオブジェクトを返す
	return index;
}

//	FUNCTION public
//	Schema::Index::create -- 索引定義のログデータから索引を定義する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database&	database
//			索引を定義するデータベースを表すクラス
//		const Schema::LogData& cLogData_
//			索引定義のログデータ
//
//	RETURN
//		定義された索引オブジェクト
//
//	EXCEPTIONS
//		Exception::TableNotFound
//			指定された名前の表はない

// static
Index::Pointer
Index::
create(Trans::Transaction& cTrans_, const Database& database,
	   const LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 空の索引クラスを生成する
	Pointer index = getNewInstance(database, cLogData_);
	; _SYDNEY_ASSERT(index.get());

	// ALTER INDEXがUNDOされている場合、最終的なエリアの割り当てが登録されているので調べる
	// 名前についても同様の処理を行う
	ID::Value id = getObjectID(cLogData_);
	index->checkUndo(database, id);

	// 表を表すクラスを得る
	Table* pTable = index->getTable(cTrans_);
	if (pTable == 0) {
		// 表データが正しく戻されていない
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// 名前が重複することはありえないが
	// 表に索引をloadしておく必要があるので確認だけする

	if (pTable->getIndex(index->getName(), cTrans_)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// IDをふり、状態を変える
	index->Object::create(cTrans_, id);

	// AreaContentを作成する
	index->createAreaContent(cTrans_);

	// キーを表すデータを取得し、処理する
	// ★注意★
	// makeLogDataの実装を変えたらここも変える
	const Common::DataArrayData& cLogKeys =
		cLogData_.getDataArrayData(Log::Create::KeyDefinitions);

	// 作成しようとしているディレクトリーがあったら破棄しておく
	// -> UNDO処理で破棄されているので不要

	ModSize nKey = cLogKeys.getCount();
	for (ModSize iKey = 0; iKey < nKey; ++iKey) {
		const Common::DataArrayData& cLogKey =
			LogData::getDataArrayData(cLogKeys.getElement(iKey));

		Key::Pointer key = Key::create(*index, iKey, *pTable, cLogKey, cTrans_);

		// 索引に追加する
		// ★注意★
		// 永続化は索引単位で行うので索引にキーが登録されている必要がある
		// キーの新規作成は索引の新規作成と常にセットなので
		// ここで追加しても他のセッションに影響はない
		// キャッシュへの登録は永続化後に行う
		(void) index->addKey(key, cTrans_);
	}

	// 索引を構成するファイルおよびそのフィールドを表すクラスを生成する
	const Common::DataArrayData* pFileLogData = 0;
	if (cLogData_.getCount() >= Log::Create::Num1) {
		pFileLogData =
			&(cLogData_.getDataArrayData(Log::Create::FileDefinition));
	}
	File::Pointer pFile =
		File::create(cTrans_, index->createFile(cTrans_), *pTable, *index, pFileLogData);
	index->createField(cTrans_, *pFile, pFileLogData);

	// 生成するファイルのファイル ID を設定する
	pFile->setFileID(cTrans_);

	// 索引にファイルをセットする
	index->setFile(pFile);
	; _SYDNEY_ASSERT(index->getFile(cTrans_));

	// 生成された索引のスキーマオブジェクトを返す
	return index;
}

//	FUNCTION public
//	Schema::Index::create -- 制約定義のログに対応する索引を定義する
//
//	NOTES
//		システム表への永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&	table
//			索引を定義する表を表すクラス
//		Schema::Constraint& constraint
//			対象の制約
//
//	RETURN
//		定義された索引オブジェクト
//
//	EXCEPTIONS

// static
Index::Pointer
Index::
create(Trans::Transaction& cTrans_,
	   Table& table, const Constraint& constraint,
	   const Common::DataArrayData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 制約に対応する索引オブジェクトを作る
	Pointer index = getNewInstance(*table.getDatabase(cTrans_), table, constraint);
	; _SYDNEY_ASSERT(index.get());

	if (table.getIndex(index->getName(), cTrans_)) {
		_SYDNEY_THROW2(Exception::IndexAlreadyDefined, index->getName(), table.getDatabase(cTrans_)->getName());
	}

	try {
		// ログに記録されているIDをふり、状態を変える
		index->Object::create(cTrans_, LogData::getID(cLogData_.getElement(Log::Constraint::ID)));

		// キーオブジェクトを作成する
		index->createKey(cTrans_, table, constraint, cLogData_);

		// 索引を構成するファイルおよびそのフィールドを表すクラスを生成する
		const Common::DataArrayData* pFileLogData =
			&(LogData::getDataArrayData(cLogData_.getElement(Log::Constraint::FileDefinition)));
		File::Pointer pFile = File::create(cTrans_, index->createFile(cTrans_), table, *index,
										   pFileLogData);
		index->createField(cTrans_, *pFile, pFileLogData);
		SCHEMA_FAKE_ERROR("Schema::Index", "CreateConstraintIndex", "FileCreated");

		// 生成するファイルのファイル ID を設定する
		pFile->setFileID(cTrans_);

		// 索引にファイルをセットする
		index->setFile(pFile);
		; _SYDNEY_ASSERT(index->getFile(cTrans_));

	} catch (...) {
		// 索引以下のオブジェクトに対して作成の取り消しを行う
		index->drop(cTrans_);
		_SYDNEY_RETHROW;
	}

	// 生成された索引のスキーマオブジェクトを返す
	return index;
}

//	FUNCTION protected
//	Schema::Index::createFile --
//		索引を構成するファイルのクラスを生成する
//
//	NOTES
//		オブジェクトを作成するだけで実際のファイルはまだ作成されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		作成したファイルオブジェクト
//
//	EXCEPTIONS

File::Pointer
Index::
createFile(Trans::Transaction& cTrans_)
{
	// サブクラスでオーバーライドする
	; _SYDNEY_ASSERT(false);
	return File::Pointer();
}

//	FUNCTION protected
//	Schema::Index::createField --
//		索引を構成するファイルのフィールドクラスを生成する
//
//	NOTES
//		オブジェクトを作成するだけで実際のファイルはまだ作成されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		File& cFile_
//			索引を構成するファイルオブジェクト
//		const Common::DataArrayData* pLogData_ /* = 0 */
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//virtual
void
Index::
createField(Trans::Transaction& cTrans_,
			File& cFile_,
			const Common::DataArrayData* pLogData_ /* = 0 */)
{
	// サブクラスでオーバーライドする
	; _SYDNEY_ASSERT(false);
}

//	FUNCTION public
//	Schema::Index::createKey -- 索引定義にしたがってキーオブジェクトを作成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&	table
//			索引を定義する表を表すクラス
//		const Statement::IndexDefinition& cStatement_
//			索引の定義を表すオブジェクト
//		Common::DataArrayData& cLogData_
//			ログデータに格納するデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
createKey(Trans::Transaction& cTrans_,
		  Table& cTable_,
		  const Statement::IndexDefinition& cStatement_,
		  Common::DataArrayData& cLogData_)
{
	// 索引定義からキーにする列名の指定を得ながら、処理する
	if (Statement::ColumnNameList* names = cStatement_.getColumnNameList()) {

		// キーにする列名の総数を得る
		int n = names->getCount();

		cLogData_.clear();
		cLogData_.reserve(n);

		for (int i = 0; i < n; ++i) {
			Statement::ColumnName* name = names->getColumnNameAt(i);
			; _SYDNEY_ASSERT(name);

			Key::Pointer key = Key::create(*this, i, cTable_, *name, cTrans_);
			; _SYDNEY_ASSERT(key.get());

			// キーの状態は「生成」である
			; _SYDNEY_ASSERT(key->getStatus() == Status::Created);

			// 索引に追加する
			// ★注意★
			// 永続化は索引単位で行うので索引にキーが登録されている必要がある
			// キーの新規作成は索引の新規作成と常にセットなので
			// ここで追加しても他のセッションに影響はない
			// キャッシュへの登録は永続化後に行う
			(void) addKey(key, cTrans_);

			// 第一キーがついている列がNOT NULL制約つきならすべてのタプルが格納される
			if (i == 0 && !(key->isNullable())) {
				_hasAllTuples = true;
			}

			if (!isTemporary()) {
				// ログデータに追加する
				ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData();
				key->makeLogData(*pData);
				cLogData_.pushBack(pData.release());
			}
		}
	}
}

//	FUNCTION public
//	Schema::Index::createKey -- 制約に対応するキーオブジェクトを作成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Table&	table
//			索引を定義する表を表すクラス
//		const Schema::Constraint& cConstraint_
//			索引の定義を表すオブジェクト
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
createKey(Trans::Transaction& cTrans_,
		  Table& table, const Constraint& constraint)
{
	// 制約からキーにする列のIDを得て処理する
	const ModVector<ID::Value>& columnID = constraint.getColumnID();

	ModSize n = columnID.getSize();
	for (ModSize i = 0; i < n; i++) {
		Key::Pointer key = Key::create(*this, i, table, columnID[i], cTrans_);
		; _SYDNEY_ASSERT(key.get());

		// キーの状態は「生成」である
		; _SYDNEY_ASSERT(key->getStatus() == Status::Created);

		// Primary Keyならキーになっている列にNotNull制約がつく
		if (constraint.isPrimaryKey()) {
			key->setNullable(false);
		}

		// 索引に追加する
		// ★注意★
		// 永続化は索引単位で行うので索引にキーが登録されている必要がある
		// キーの新規作成は索引の新規作成と常にセットなので
		// ここで追加しても他のセッションに影響はない
		// キャッシュへの登録は永続化後に行う
		(void) addKey(key, cTrans_);
	}
}

// FUNCTION public
//	Schema::Index::createKey -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& table
//	const Constraint& constraint
//	const Common::DataArrayData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Index::
createKey(Trans::Transaction& cTrans_,
		  Table& table, const Constraint& constraint,
		  const Common::DataArrayData& cLogData_)
{
	const ModVector<ID::Value>& vecKeyID = LogData::getIDs(cLogData_.getElement(Log::Constraint::KeyIDs));

	// 制約からキーにする列のIDを得て処理する
	const ModVector<ID::Value>& columnID = constraint.getColumnID();

	ModSize n = columnID.getSize();
	if (n != vecKeyID.getSize()) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	for (ModSize i = 0; i < n; i++) {
		Key::Pointer key = Key::create(*this, i, table, columnID[i], cTrans_, vecKeyID[i]);
		; _SYDNEY_ASSERT(key.get());

		// キーの状態は「生成」である
		; _SYDNEY_ASSERT(key->getStatus() == Status::Created);

		// Primary Keyならキーになっている列にNotNull制約がつく
		if (constraint.isPrimaryKey()) {
			key->setNullable(false);
		}

		// 索引に追加する
		// ★注意★
		// 永続化は索引単位で行うので索引にキーが登録されている必要がある
		// キーの新規作成は索引の新規作成と常にセットなので
		// ここで追加しても他のセッションに影響はない
		// キャッシュへの登録は永続化後に行う
		(void) addKey(key, cTrans_);
	}
}

//	FUNCTION public
//	Schema::Index::drop -- 索引のスキーマ定義を破棄する
//
//	NOTES
//		索引に属するキーと実現するファイルの定義も抹消される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Index& cIndex_
//			破棄の対象となる索引
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
Index::
drop(Trans::Transaction& cTrans_, Index& cIndex_, LogData& cLogData_)
{
	cIndex_.drop(cTrans_);
	if (!cIndex_.isTemporary()) {
		cIndex_.makeLogData(cTrans_, cLogData_);
	}
}

//	FUNCTION public
//	Schema::Index::drop -- 索引のスキーマ定義を破棄する
//
//	NOTES
//		索引に属するキーと実現するファイルの定義も抹消される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
drop(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */, bool bNoUnset_ /* = false */)
{
	// 索引に属するキーをすべて破棄する
	// 破棄されたキーに関係する列のtouchは
	// キーの永続化で実行される

	loadKey(cTrans_).apply(ApplyFunction2<Key, bool, bool>(&Key::drop, bRecovery_, bNoUnset_));

	if (!isTemporary()) {
		// 索引と関係するエリア格納関係をすべて破棄する
		AreaCategory::Value eCheckedCategories[_iAreaNumber];
		eCheckedCategories[0] = getAreaCategory();

		for (int i = 0; i < _iAreaNumber; i++) {
			if (Area*
				pArea = getArea(eCheckedCategories[i], true, cTrans_))
				AreaContent::drop(*pArea, *this, eCheckedCategories[i], cTrans_, bRecovery_);
		}
	}

	// 索引を実現するファイルを破棄する
	if (File* pFile = getFile(cTrans_))
		pFile->drop(cTrans_, bRecovery_, bNoUnset_);

	// 状態を変更する
	Object::drop(bRecovery_, bNoUnset_);
}

//	FUNCTION public
//	Schema::Index::undoDrop -- 索引のスキーマ定義の破棄マークをクリアする
//
//	NOTES
//		破棄は論理ログに出力する前なら破棄マークをクリアすることで
//		復旧可能である
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
undoDrop(Trans::Transaction& cTrans_)
{
	// 索引に属するキーのすべての破棄マークをクリアする
	loadKey(cTrans_).apply(ApplyFunction0<Key>(&Key::undoDrop));

	if (!isTemporary()) {
		// 索引と関係するエリア格納関係のすべての破棄マークをクリアする
		AreaCategory::Value eCheckedCategories[_iAreaNumber];
		eCheckedCategories[0] = getAreaCategory();

		for (int i = 0; i < _iAreaNumber; i++) {
			if (Area* pArea = getArea(eCheckedCategories[i], true, cTrans_))
				AreaContent::undoDrop(*pArea, *this, eCheckedCategories[i], cTrans_);
		}
	}

	// 索引を実現するファイルの破棄マークをクリアする
	if (File* pFile = getFile(cTrans_))
		pFile->undoDrop(cTrans_);

	// 状態変化を取り消す
	Object::undoDrop();
}

//	FUNCTION public
//	Schema::Index::destroy --
//		索引を構成するファイルやそれを格納するディレクトリーを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		ModHashMap<ID::Value, Area*, ModHasher<ID::Value> >* pAreaMap_ = 0
//			ディレクトリーを破棄すべきエリアを集めるハッシュマップ
//		bool bForce = true
//			trueの場合チェックポイントを待たずにすぐに削除する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
destroy(Trans::Transaction& cTrans_,
		ModHashMap<ID::Value, Area*, ModHasher<ID::Value> >* pAreaMap_, bool bForce_)
{
	if (File* pFile = getFile(cTrans_)) {
		// ファイルを抹消する
		pFile->destroy(cTrans_, true, bForce_);
	}

	if (pAreaMap_) {
		// 引数のマップにエリアを追加して呼び出し側で一括して破棄する
		// Table::destroyから呼ばれるので使用エリアの重複を調べる必要はない

		Area* pArea = getArea(getAreaCategory(), false, cTrans_);
		if (pArea)
			pAreaMap_->insert(pArea->getID(), pArea);

	} else {

		// 使用エリアが一致するファイルがなければディレクトリーを破棄する
		getTable(cTrans_)->sweepUnusedArea(cTrans_, getAreaID(getAreaCategory(), false, cTrans_), bForce_);
	}
}

//	FUNCTION public
//	Schema::Index::alterArea -- SQL の索引定義変更文から索引を変更する準備をする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::AlterIndexStatement&	statement
//			解析済の SQL の索引定義変更文
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		true ... 変更の必要がある
//		false... 変更の必要がない
//
//	EXCEPTIONS

// static
bool
Index::
alterArea(Trans::Transaction& cTrans_,
		  Index& cIndex_,
		  const Statement::AlterIndexAction& statement,
		  ModVector<ID::Value>& vecPrevAreaID_, ModVector<ID::Value>& vecPostAreaID_,
		  LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	bool bResult = false;

    // 移動の為の準備を行う

	// まず修正前後に現在の値を入れる
	cIndex_.getAreaID(vecPrevAreaID_);
	cIndex_.getAreaID(vecPostAreaID_);

    switch (statement.getActionType()) {
	case Statement::AlterIndexAction::SetArea:
        bResult = cIndex_.prepareSetArea(cTrans_, *(_SYDNEY_DYNAMIC_CAST(Statement::AreaOption*, statement.getAction())), vecPrevAreaID_, vecPostAreaID_);
        break;
   	case Statement::AlterIndexAction::DropArea:
        bResult = cIndex_.prepareDropArea(cTrans_, *(_SYDNEY_DYNAMIC_CAST(Statement::AreaOption*, statement.getAction())),  vecPrevAreaID_, vecPostAreaID_);
        break;
   	default:
        ; _SYDNEY_ASSERT(false);

    }

	if (bResult) {
		Database* pDatabase = cIndex_.getDatabase(cTrans_);
		Table* pTable = cIndex_.getTable(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);
		; _SYDNEY_ASSERT(pTable);

		// ログデータを作る
		cIndex_.makeLogData(cTrans_, cLogData_);

		// 変更前後のエリア ID の追加
		cLogData_.addIDs(vecPrevAreaID_);
		cLogData_.addIDs(vecPostAreaID_);
		// 変更前後のエリア ID に対してInvalidなら表の指定で置き換える
		ModVector<ID::Value> vecEffectivePrevID;
		ModVector<ID::Value> vecEffectivePostID;
		Index::getEffectiveAreaID(vecPrevAreaID_, pTable->getAreaID(), cIndex_.getAreaCategory(), vecEffectivePrevID);
		Index::getEffectiveAreaID(vecPostAreaID_, pTable->getAreaID(), cIndex_.getAreaCategory(), vecEffectivePostID);

		// 実質的なエリアIDもログに格納する
		cLogData_.addIDs(vecEffectivePrevID);
		cLogData_.addIDs(vecEffectivePostID);
		// 変更前後のエリアパスの追加
		// エリアパスには実質的なエリアIDのほうを使う
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, vecEffectivePrevID));
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, vecEffectivePostID));

		// ファイルのスキーマオブジェクトIDを記録する
		cLogData_.addID(cIndex_.getFileID());

		; _SYDNEY_ASSERT(cLogData_.getCount() == Index::Log::Alter::Num);
	}

	return bResult;
}

//	FUNCTION public
//	Schema::Index::alterName -- SQL の索引定義変更文から索引の名前を変更する準備をする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::AlterIndexAction& statement
//			解析済の SQL の索引定義変更文
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		true ... 変更の必要がある
//		false... 変更の必要がない
//
//	EXCEPTIONS

// static
bool
Index::
alterName(Trans::Transaction& cTrans_,
		  Index& cIndex_,
		  const Statement::AlterIndexAction& statement,
		  Name& cPostName_,
		  LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);
	; _SYDNEY_ASSERT(statement.getActionType() == Statement::AlterIndexAction::Rename);

	bool bResult = false;

	Database* pDatabase = cIndex_.getDatabase(cTrans_);
	Table* pTable = cIndex_.getTable(cTrans_);
	; _SYDNEY_ASSERT(pDatabase);
	; _SYDNEY_ASSERT(pTable);

	; _SYDNEY_ASSERT(_SYDNEY_DYNAMIC_CAST(Statement::Identifier*, statement.getAction())->getIdentifier());
	const ModUnicodeString& cName = *(_SYDNEY_DYNAMIC_CAST(Statement::Identifier*, statement.getAction())->getIdentifier());

	if (cIndex_.getName() != cName) {
		// 変更後の名前が使用されていないかをチェックするために一時的にIndexオブジェクトを作る
		Index cAlterredIndex;  // 名前の検査だけなのでカテゴリーは不要
		cAlterredIndex.setID(cIndex_.getID());
		cAlterredIndex.setParentID(cIndex_.getParentID());
		cAlterredIndex.setDatabaseID(cIndex_.getDatabaseID());
		cAlterredIndex.setName(cName);

		// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
		// ★注意★
		// 最後の引数がtrueなので重複していたら常に例外が飛ぶ
		(void) _Name::_checkExistence(cTrans_, *pDatabase, &cAlterredIndex, true /* no cancel */);

		{ // AutoWithdrawのデストラクトがcAlterredIndexより前であることを保証するためのスコープ

			// スコープから抜けたら自動的にwithdrawする
			Manager::ObjectName::AutoWithdraw w(&cAlterredIndex);

			cPostName_ = cName;
			bResult = true;

			// ログデータを作る
			cIndex_.makeLogData(cTrans_, cLogData_);

			// 変更後の名前をログに追加する
			// ★注意★
			// 変更前の名前はmakeLogDataで格納されている
			//   - 変更後名称(Undo用)
			//   - 変更後ファイル名称(Undo用)
			//   - 変更時の表の指定も加味したエリアID指定(Undo用)
			//   - 変更時のエリアパス(Undo用)
			//   - 変更時のファイルID(Undo用)
			File* pIndexFile = cIndex_.getFile(cTrans_);
			ModVector<Object::ID::Value> vecIndexAreaID;
			ModVector<Object::ID::Value> vecEffectiveAreaID;
			cIndex_.getAreaID(vecIndexAreaID);
			Index::getEffectiveAreaID(vecIndexAreaID, pTable->getAreaID(), cIndex_.getAreaCategory(), vecEffectiveAreaID);

			cLogData_.addString(cPostName_);
			cLogData_.addString(pIndexFile->createName(cTrans_, cPostName_));
			cLogData_.addIDs(vecEffectiveAreaID);
			cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, vecEffectiveAreaID));
			cLogData_.addID(cIndex_.getFileID());

			; _SYDNEY_ASSERT(cLogData_.getCount() == Index::Log::Rename::Num);
		}
	}

	return bResult;
}

//	FUNCTION public
//	Schema::Index::undoAlter -- alter の変更箇所を無効化する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//
//	EXCEPTIONS
void
Index::
undoAlter(Trans::Transaction& cTrans_)
{
	// 特に処理する必要は無い
}

//	FUNCTION public
//	Schema::Index::moveArea -- SQL の索引定義変更文から索引を実際に変更する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//      const ModVector<ID::Value>& vecpPrevArea_
//			変更前のエリアオブジェクトID
//		const ModVector<ID::Value>& vecpPostArea_
//			変更後のエリアオブジェクトID
//		bool bUndo_ = false
//			エラー処理中であることを示す
//		bool bRecovery_ = false
//			REDO中であることを示す
//		bool bMount = false
//			MOUNTに伴う処理であることを示す
//
//	RETURN
//		定義された索引オブジェクト
//
//	EXCEPTIONS

void
Index::
moveArea(Trans::Transaction& cTrans_,
		 const ModVector<ID::Value>& vecPrevAreaID_,
		 const ModVector<ID::Value>& vecPostAreaID_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	enum {
		None,
		SetArea,
		DropArea,
		ValueNum
	} eStatus = None;
    try {

        // エリアの変更を行う
        // 同時に索引に属するファイルの格納場所も移動する

		// Set方向の変更を適用する
		setArea(cTrans_, vecPrevAreaID_, vecPostAreaID_, bUndo_, bRecovery_, bMount_);
		eStatus = SetArea;
		SCHEMA_FAKE_ERROR("Schema::Index", "Move", "SetArea");

		// Drop方向の変更を適用する
		dropArea(cTrans_, vecPrevAreaID_, vecPostAreaID_, bUndo_, bRecovery_, bMount_);
		eStatus = DropArea;
		SCHEMA_FAKE_ERROR("Schema::Index", "Move", "DropArea");

		// MOUNTに伴う処理ならディレクトリーを削除する必要はない
		// 自動リカバリーの処理ならファイルの移動は行われていないはずなので
		// ディレクトリーを削除する必要もない
		if (!bMount_ && !bRecovery_) {

			// 表を構成するファイルで移動前のエリアIDと一致するものがないとき
			// エリア以下の表名で表されるディレクトリーを破棄する
			ModSize n = vecPrevAreaID_.getSize();
			for (ModSize i = 0; i < n; ++i) {
				getTable(cTrans_)->sweepUnusedArea(cTrans_, vecPrevAreaID_[i]);
			}
		}
		SCHEMA_FAKE_ERROR("Schema::Index", "Move", "Deleted");

		// エリアオブジェクトのキャッシュをクリアしておく
		m_pLogArea = 0;
		m_pArea = 0;
    }
    catch ( ... ) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		switch (eStatus) {
		case SetArea:
			dropArea(cTrans_, vecPostAreaID_, vecPrevAreaID_, true /* undo */, bRecovery_, bMount_);
			// thru.
		case DropArea:
			setArea(cTrans_, vecPostAreaID_, vecPrevAreaID_, true /* undo */, bRecovery_, bMount_);
			// thru.
		case None:
			// setAreaなどで表のディレクトリーができてしまうと下位のエラー処理では消せないので
			// 以下の処理は常に行う
			if (!bMount_ && !bRecovery_) {
				// ファイルを移動してしまっていた場合
				// 移動後のディレクトリーを必要に応じて破棄する
				ModSize n = vecPostAreaID_.getSize();
				for (ModSize i = 0; i < n; ++i) {
					getTable(cTrans_)->sweepUnusedArea(cTrans_, vecPostAreaID_[i]);
				}
			}
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

        _SYDNEY_RETHROW;
    }
}

// FUNCTION public
//	Schema::Index::rename -- 名前を変更する
//
// NOTES
//
// ARGUMENTS
//	const Name& cPostName_
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
Index::
rename(const Name& cPostName_)
{
	setName(cPostName_);
}

// FUNCTION public
//	Schema::Index::moveRename -- 名前の変更に伴いファイルを移動する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Name& cPrevTableName_
//	const Name& cPostTableName_
//	const Name& cPrevName_
//	const Name& cPostName_
//	bool bUndo_ /* = false */
//	bool bRecovery_ /* = false */
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
Index::
moveRename(Trans::Transaction& cTrans_,
		   const Name& cPrevTableName_,
		   const Name& cPostTableName_,
		   const Name& cPrevName_,
		   const Name& cPostName_,
		   bool bUndo_ /* = false */,
		   bool bRecovery_ /* = false */)
{
	Database* pDatabase = getDatabase(cTrans_);
	File* pFile = getFile(cTrans_);

	Name cPrevFileName(pFile->createName(cTrans_, cPrevName_));
	Name cPostFileName(pFile->createName(cTrans_, cPostName_));

	// Fileの移動に関するエラー処理に使う
    bool bFileMoved = false;

    try {
		// ファイルを移動する
		pFile->moveRename(cTrans_, cPrevTableName_, cPostTableName_, cPrevFileName, cPostFileName, bUndo_, bRecovery_);

		// エラー処理の為、移動フラグを保存する
		bFileMoved = true;
		SCHEMA_FAKE_ERROR("Schema::Index", "MoveRename", "FileMoved");

		// 名前を変更する
		rename(cPostName_);
    }
    catch ( ... ) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		if (bFileMoved) {
			// 移動したファイルを元に戻す
			pFile->moveRename(cTrans_, cPostTableName_, cPrevTableName_, cPostFileName, cPrevFileName, true /* undo */, bRecovery_);
			// 変更した名前を戻す
			rename(cPrevName_);
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
    }
}

//	FUNCTION public
//	Schema::Index::getName -- 索引削除文から索引名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::DropIndexStatement& statement
//			索引破棄のSQL文
//
//	RETURN
//		索引名
//
//	EXCEPTIONS

// static
Object::Name
Index::
getName(const Statement::DropIndexStatement& statement)
{
	Statement::Identifier* pIdent = statement.getIndexName();
	; _SYDNEY_ASSERT(pIdent);
	; _SYDNEY_ASSERT(pIdent->getIdentifier());

	return *pIdent->getIdentifier();
}

//	FUNCTION public
//	Schema::Index::getName -- 索引変更文から索引名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::DropIndexStatement& statement
//			索引破棄のSQL文
//
//	RETURN
//		索引名
//
//	EXCEPTIONS

// static
Object::Name
Index::
getName(const Statement::AlterIndexStatement& statement)
{
	Statement::Identifier* pIdent = statement.getIndexName();
	; _SYDNEY_ASSERT(pIdent);
	; _SYDNEY_ASSERT(pIdent->getIdentifier());

	return *pIdent->getIdentifier();
}

#ifdef OBSOLETE // 索引に対して独立に以下のメソッドが呼ばれることはない

//	FUNCTION public
//	Schema::Index::mount
//		-- ファイルを mount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Index::
mount(Trans::Transaction& cTrans_)
{
	if ( File* pFile = getFile(cTrans_) )
	{
		// ファイルをマウントする
		pFile->mount(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Index::unmount
//		-- ファイルを unmount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Index::
unmount(Trans::Transaction& cTrans_)
{
	if ( File* pFile = getFile(cTrans_) )
	{
		// ファイルをアンマウントする
		pFile->unmount(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Index::flush
//		-- ファイルをフラッシュする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Index::
flush(Trans::Transaction& cTrans_)
{
	if ( File* pFile = getFile(cTrans_) )
	{
		// ファイルをフラッシュする
		pFile->flush(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Index::startBackup
//		-- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRestorable_ = true
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Index::
startBackup(Trans::Transaction& cTrans_, bool bRestorable_)
{
	if ( File* pFile = getFile(cTrans_) )
	{
		// ファイル のバックアップを開始する
		pFile->startBackup(cTrans_, bRestorable_);
	}
}

//	FUNCTION public
//	Schema::Index::
//		-- バックアップを終了する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Index::
endBackup(Trans::Transaction& cTrans_)
{
	if ( File* pFile = getFile(cTrans_) )
	{
		// ファイル のバックアップを終了する
		pFile->endBackup(cTrans_);
	}
}

//	FUNCTION public
//	Schema::Index::recover
//		-- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			回復する時点を表すタイムスタンプ
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//			DROPがUNDOされているか調べるために必要
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Index::
recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_,
		const Name& cDatabaseName_)
{
	using namespace Manager::RecoveryUtility;
	// DropまたはCreateがUndoされている索引についてはrecoverしない
	if (!Undo::isEntered(cDatabaseName_, getID(), Undo::Type::DropIndex)
		&& !Undo::isEntered(cDatabaseName_, getID(), Undo::Type::CreateIndex)) {

		if ( File* pFile = getFile(cTrans_) ) {
			// ファイルの障害回復処理を行う
			pFile->recover(cTrans_, cPoint_);
		}
	}
}

//	FUNCTION public
//	Schema::Index::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			回復する時点を表すタイムスタンプ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Index::restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	if ( File* pFile = getFile(cTrans_) )
	{
		// ファイルの障害回復処理を行う
		pFile->restore(cTrans_, cPoint_);
	}
}
#endif

//	FUNCTION public
//	Schema::Index::get -- あるスキーマオブジェクト ID の索引を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			索引のスキーマオブジェクト ID
//		Schema::Database* pDatabase_
//			索引が属するデータベースのオブジェクトID
//			値が0ならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた索引を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の索引は存在しない
//
//	EXCEPTIONS

// static
Index*
Index::get(ID::Value id_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	return ObjectTemplate::get<Index, SystemTable::Index, Object::Category::Index>(id_, pDatabase_, cTrans_);
}

//	FUNCTION public
//	Schema::Index::get -- あるスキーマオブジェクト ID の索引を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			索引のスキーマオブジェクト ID
//		Schema::Object::ID::Value iDatabaseID_
//			索引が属するデータベースのオブジェクトID
//			値がID::Invalidならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた索引を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の索引は存在しない
//
//	EXCEPTIONS

// static
Index*
Index::get(ID::Value id_, ID::Value iDatabaseID_, Trans::Transaction& cTrans_)
{
	if (id_ == ID::Invalid)
		return 0;

	return get(id_, Database::get(iDatabaseID_, cTrans_), cTrans_);
}

//	FUNCTION public
//	Schema::Index::get -- ある名前の索引を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name&	name
//			索引の名前
//		Schema::Database* pDatabase_
//			索引が属するデータベースのオブジェクトID
//			値が0ならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた索引を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の索引は存在しない
//
//	EXCEPTIONS

// static
Index*
Index::
get(const Name& name_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	Index* pIndex = 0;

	; _SYDNEY_ASSERT(pDatabase_);
	// データベースに属するすべての表について名前で検査する
	ModVector<Table*> vecTable = pDatabase_->getTable(cTrans_, true /* internal */);
	ModVector<Table*>::Iterator iterator = vecTable.begin();
	const ModVector<Table*>::Iterator& end = vecTable.end();
	for (; iterator != end; ++iterator) {
		pIndex = (*iterator)->getIndex(name_, cTrans_);
		if (pIndex) break;
	}
	return pIndex;
}

//	FUNCTION public
//	Schema::Index::isValid -- 陳腐化していないか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			陳腐化していないかを調べるスキーマオブジェクトID
//		Schema::Object::ID::Value iDatabaseID_
//			このスキーマオブジェクトが属するデータベースのID
//		Schema::Object::Timestamp iTimestamp_
//			正しいスキーマオブジェクトの値とこの値が異なっていたら
//			陳腐化していると判断する
//		Trans::Transaction& cTrans_
//			陳腐化したかを調べるトランザクション記述子
//
//	RETURN
//		true
//			自分自身の表すスキーマオブジェクトは最新のものである
//		false
//			陳腐化している
//
//	EXCEPTIONS
//		なし

// static
bool
Index::
isValid(ID::Value iID_, ID::Value iDatabaseID_, Timestamp iTimestamp_,
		Trans::Transaction& cTrans_)
{
	Index* pIndex = get(iID_, iDatabaseID_, cTrans_);

	return (pIndex && pIndex->getTimestamp() == iTimestamp_);
}

//	FUNCTION public
//	Schema::Index::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Index::Pointer& pIndex_
//			永続化するオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の状態
//		bool bNeedToErase_
//			永続化が削除操作のとき、キャッシュから消去するかを示す
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Index::
doBeforePersist(const Pointer& pIndex_, Status::Value eStatus_, bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pIndex_.get());

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	case Status::Changed:
	case Status::DeletedInRecovery:
	case Status::CreateCanceled:
	{
		// 何もしない
		break;
	}
	case Status::Deleted:
	{
		if (bNeedToErase_) {
			// 索引を構成するファイルとディレクトリーを破棄する
			// ★注意★
			// エラーが起きてもログの内容から再実行できるように
			// ファイルやディレクトリーを実際に「消す」操作は
			// システム表から消す操作を永続化する前に行う

			pIndex_->destroy(cTrans_);
#if 0
		} else {
			// 実際に消す必要がなくても下位モジュールの管理情報を消すためunmountは必要

			pIndex_->unmount(cTrans_);
#endif
		}

		break;
	}
	default:
		// 何もしない
		break;
	}
}

//	FUNCTION public
//	Schema::Index::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Index::Pointer& pIndex_
//			永続化したオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前の状態
//		bool bNeedToErase_
//			永続化が削除操作のとき、キャッシュから消去するかを示す
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Index::
doAfterPersist(const Pointer& pIndex_, Status::Value eStatus_, bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pIndex_.get());

	// deleteされる可能性があるのでデータベースIDをここで取得しておく
	const ObjectID::Value dbId = pIndex_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// ★注意★
		// これまでフィールドのsource-destination関係の設定をここで行っていたが
		// エラー処理などを考えるとField::doAfterPersistで行うべき処理であるのでそこに移動した
		// Fieldの永続化が完了するまでは索引を正常に使用することができないが
		// ロックの機構により途中の状態のときにほかのクライアントが参照することはない

		// この索引がついた表を表すクラスに、この索引を表すクラスを登録する
		// また、マネージャーに、この索引を表すクラスを
		// スキーマオブジェクトとして管理させる

		Table* pTable = pIndex_->getTable(cTrans_);
		; _SYDNEY_ASSERT(pTable);
		Database* pDatabase = pIndex_->getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);

		(void) pDatabase->addCache(pTable->addIndex(pIndex_, cTrans_));
		break;
	}
	case Status::Changed:

		// 変更があったらパスのキャッシュをクリアしておく

		pIndex_->clearPath();
		break;
	case Status::CreateCanceled:
		{
			// When the index is related to a constraint, index should be erased here
			Table* pTable = pIndex_->getTable(cTrans_);
			; _SYDNEY_ASSERT(pTable);
			pTable->eraseIndex(pIndex_->getID());
			break;
		}
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらキャッシュや表の登録からの削除も行う

		// 状態を「実際に削除された」にする

		pIndex_->setStatus(Status::ReallyDeleted);

		if (bNeedToErase_) {
			Database* pDatabase = pIndex_->getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			// 下位オブジェクトがあればそれを抹消してからdeleteする
			pIndex_->reset(*pDatabase);

			// キャッシュから抹消する
			// NeedToErase==falseのときは親オブジェクトのdeleteの中で
			// キャッシュから抹消される
			pDatabase->eraseCache(pIndex_->getID());

			// 表の登録から抹消する → deleteされる
			Table* pTable = pIndex_->getTable(cTrans_);
			; _SYDNEY_ASSERT(pTable);
			ObjectID::Value	iIndexID = pIndex_->getID();
			pTable->eraseIndex(iIndexID);

			// すべてのスナップショットから登録を抹消する
			Manager::ObjectSnapshot::eraseIndex(dbId, pTable->getID(), iIndexID);
		}
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbId, Object::Category::Index);
}

//	FUNCTION public
//	Schema::Index::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::IndexPointer& pIndex_
//			読み出したオブジェクト
//		Schema::Table& cTable_
//			オブジェクトが属する親オブジェクト
//		Trans::Transaction& cTrans_
//			この操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

// static
void
Index::
doAfterLoad(const Pointer& pIndex_, Table& cTable_, Trans::Transaction& cTrans_)
{
	// UNDO情報に最終的なエリアID割り当てが登録されている場合はそれに置き換える
	// 名前についても同様の処理を行う
	pIndex_->checkUndo(*cTable_.getDatabase(cTrans_), pIndex_->getID());

	// 表へ読み出した索引を表すクラスを追加する
	// また、データベースにこの索引を表すクラスを
	// スキーマオブジェクトとして管理させる
	cTable_.getDatabase(cTrans_)->addCache(cTable_.addIndex(pIndex_, cTrans_));
}

//	FUNCTION public
//	Schema::Index::reset --
//		下位オブジェクトを抹消する
//
//	NOTES
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::reset(Database& cDatabase_)
{
	if (_keys)
		resetKey(cDatabase_);
}

//	FUNCTION public
//	Schema::Index::getCategory -- 索引の種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた索引の種別
//
//	EXCEPTIONS
//		なし

Index::Category::Value
Index::
getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Schema::Index::isUnique -- 索引のキーの値の組はユニークか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			ユニークである
//		false
//			ユニークでない
//
//	EXCEPTIONS
//		なし

bool
Index::
isUnique() const
{
	return _unique;
}

//	FUNCTION public
//	Schema::Index::isClustered -- 索引はクラスター化されているか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			クラスター化されている
//		false
//			クラスター化されていない
//
//	EXCEPTIONS
//		なし

bool
Index::
isClustered() const
{
	return _clustered;
}

//	FUNCTION public
//	Schema::Index::hasAllTuples -- 索引はすべてのタプルを保持するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			すべてのタプルを保持する
//		false
//			キーがnullのみからなる場合は保持しない
//
//	EXCEPTIONS
//		なし

bool
Index::
hasAllTuples() const
{
	return _hasAllTuples;
}

// FUNCTION public
//	Schema::Index::isOffline -- Offlineか
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

bool
Index::
isOffline() const
{
	return m_bOffline;
}

// FUNCTION public
//	Schema::Index::setOffline -- Change online/Offline status
//
// NOTES
//
// ARGUMENTS
//	bool bSet_ = true
//		true ... set to offline
//		false... set to online
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Index::
setOffline(bool bSet_)
{
	m_bOffline = bSet_;
}

//	FUNCTION public
//	Schema::Index::getHint -- 索引ファイルに指定されるヒントを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ヒントを表すオブジェクトへのポインタ
//
//	EXCEPTIONS
//		なし

const Hint*
Index::
getHint() const
{
	return m_pHint;
}

//	FUNCTION public
//	Schema::Index::setHint -- 索引ファイルに指定されるヒントを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Hint* pHint_
//			ヒントを表すオブジェクトへのポインタ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Index::
setHint(Hint* pHint_)
{
	m_pHint = pHint_;
}

//	FUNCTION public
//	Schema::Index::getAreaHint -- 索引ファイルのエリアに指定されるヒントを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		エリアのヒントを表す文字列へのポインタ
//
//	EXCEPTIONS
//		なし

const Hint*
Index::
getAreaHint() const
{
	return m_pAreaHint;
}

//	FUNCTION public
//	Schema::Index::getTableID --
//		索引が属するデータベースのスキーマオブジェクト ID を得る
//
//	NOTES
//		生成前、中の索引や、排他制御がうまく行われていない場合を除けば、
//		索引が属するデータベースは必ずひとつ存在するはずである
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			索引が属するデータベースのスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			この索引が属するデータベースは存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Index::
getTableID() const
{
	return getParentID();
}

//	FUNCTION public
//	Schema::Index::getFileID --
//		索引を実現するファイルのスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			索引を実現するファイルのスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			この索引を実現するファイルは存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Index::
getFileID() const
{
	return m_iFileID;
}

//	FUNCTION public
//	Schema::Index::setFileID --
//		索引を実現するファイルのスキーマオブジェクト ID を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value
//			設定するID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline // 内部でしか使わない
void
Index::
setFileID(ID::Value id_)
{
	m_iFileID = id_;
}

//	FUNCTION public
//	Schema::Index::getTable -- 索引をつけた表を得る
//
//	NOTES
//		生成前、中の索引や、排他制御がうまく行われていない場合を除けば、
//		索引をつけた表は必ずひとつ存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			索引をつけた表が存在しない
//
//	EXCEPTIONS
//		なし

Table*
Index::getTable(Trans::Transaction& cTrans_) const
{
	if (!_table) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_table) ?
			_table = Table::get(getParentID(), getDatabase(cTrans_), cTrans_, true /* internal */)
			: _table;
	}
	return _table;
}

//	FUNCTION public
//	Schema::Index::getAreaID --
//		索引を実現するファイルを格納するエリアのスキーマオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eArea_
//			取得するのがファイルのエリアか物理ログのエリアかを示す
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			指定した種類のファイルを格納するエリアのスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			指定した種類のファイルを格納するエリアは設定されていない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Index::
getAreaID(AreaCategory::Value eArea_) const
{
	return (eArea_ == getAreaCategory()) ? m_iAreaID
		: ((eArea_ == AreaCategory::PhysicalLog) ? m_iLogAreaID
		   : ID::Invalid);
}

//	FUNCTION public
//	Schema::Index::getAreaID --
//		索引を実現するファイルを格納するエリアのスキーマオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eArea_
//			取得するのがファイルのエリアか物理ログのエリアかを示す
//		bool bParent_
//			trueの場合親オブジェクトの指定までさかのぼる
//			falseの場合自身の指定のみを返す
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			指定した種類のファイルを格納するエリアのスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			指定した種類のファイルを格納するエリアは設定されていない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Index::
getAreaID(AreaCategory::Value eArea_, bool bParent_,
		  Trans::Transaction& cTrans_) const
{
	ID::Value iID = getAreaID(eArea_);

	return (bParent_ && iID == ID::Invalid)
		? getTable(cTrans_)->getAreaID(eArea_, true /* effective */) : iID;
}

//	FUNCTION public
//	Schema::Index::getAreaID --
//		索引を実現するファイルを格納するエリアのスキーマオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<Schema::Object::ID::Value>& vecAreaID_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
getAreaID(ModVector<ID::Value>& vecAreaID_) const
{
	vecAreaID_.reserve(_iAreaNumber);
	vecAreaID_.pushBack(m_iAreaID);
}

//	FUNCTION public
//	Schema::Index::convertAreaCategory -- CategoryをStatementのAreaタイプに変換する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eCategory_
//			エリアの種別
//
//	RETURN
//		Statementで定義されるエリア種別
//
//	EXCEPTIONS
//		なし

// static
Statement::AreaOption::AreaType
Index::
convertAreaCategory(AreaCategory::Value eCategory_)
{
	switch (eCategory_) {
	case AreaCategory::Index:
	case AreaCategory::FullText:
		// 索引に対するalter文ではDefaultのカテゴリーで指定される
		// 表に対するalter文でのカテゴリー変換は
		// Table::convertAreaCategoryで行われる
		return Statement::AreaOption::Default;
	case AreaCategory::PhysicalLog:
		return Statement::AreaOption::PhysicalLog;
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
	// never reach
	return Statement::AreaOption::Default;
}

//	FUNCTION public
//	Schema::Index::getAreaCategory --
//		索引が置かれるエリアの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		索引を格納するエリア種別
//
//	EXCEPTIONS

//virtual
AreaCategory::Value
Index::
getAreaCategory() const
{
	// サブクラスでオーバーライドする
	; _SYDNEY_ASSERT(false);
	return AreaCategory::Default;
}

//
//	FUNCTION public
//	Schema::Index::getArea --
//		ファイルまたは物理ログファイルを格納するエリアを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value
//			索引を実現するファイルを格納するエリアの種別を表す値
//		bool bParent_
//			trueの場合親オブジェクトの指定までさかのぼる
//			falseの場合自身の指定のみを返す
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0以外
//			指定した種類のファイルを格納するエリアを表すオブジェクトの先頭
//		0
//			指定した種類のファイルを格納するエリアは設定されていない
//
//	EXCEPTIONS
//

Area*
Index::
getArea(AreaCategory::Value eArea_, bool bParent_,
		Trans::Transaction& cTrans_) const
{
	; _SYDNEY_ASSERT(eArea_ == AreaCategory::PhysicalLog
					 || eArea_ == getAreaCategory());

	Area* pTmpArea = 0;

	// 親オブジェクトまでさかのぼらない場合、メンバーに記録しない
	Area** pAreaRef =
		(!bParent_) ? &pTmpArea
		: ((eArea_ == AreaCategory::PhysicalLog) ? &m_pLogArea : &m_pArea);

	if (!*pAreaRef) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!*pAreaRef) ?
			*pAreaRef = Area::get(getAreaID(eArea_, bParent_, cTrans_),
								  getDatabase(cTrans_),
								  cTrans_)
			: *pAreaRef;
	}
	return *pAreaRef;
}

//	FUNCTION public
//	Schema::Index::prepareSetArea --
//		SQL構文要素から索引ファイルを格納するエリアを設定する準備をする
//
//	NOTES
//		この中では特にエラー処理は行われない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Statement::AreaOption&			cStatement_
//			エリア指定を表すSQL構文要素
//      ModVector<ID::Value>& vecPrevAreaID_
//			変更前のエリアオブジェクトID
//		ModVector<ID::Value>& vecPostAreaID_
//			変更後のエリアオブジェクトID
//
//	RETURN
//		true ... 変更の必要あり
//		false... 変更の必要なし
//
//	EXCEPTIONS

bool
Index::
prepareSetArea(Trans::Transaction& cTrans_,
               const Statement::AreaOption& cStatement_,
               ModVector<ID::Value>& vecPrevAreaID_,
               ModVector<ID::Value>& vecPostAreaID_)
{
	bool bResult = false;
	AreaCategory::Value eCheckedCategories[_iAreaNumber];
	eCheckedCategories[0] = getAreaCategory();

	// Undo/Redoの仕様変更によりログひとつでAlter後の
	// Area指定すべてを取得できる必要ができた
	// したがってsetの指定の有無に関係なく修正前後のIDを入れる
	// 修正前後に現在の値を入れるのは呼び出し側で行うようにした
	// // まず修正前後に現在の値を入れる
	// getAreaID(vecPrevAreaID_);
	// getAreaID(vecPostAreaID_);

	for (int i = 0; i < _iAreaNumber; i++) {

		if (Statement::Identifier* pAreaName =
				cStatement_.getAreaName(Index::convertAreaCategory(
					eCheckedCategories[i]))) {

            // ★注意★
            // エリア名が省略されているときはgetIdentifierが0を返す
            // setにおいてはエリア名の省略はエリア指定の省略と同義

			if (pAreaName->getIdentifier()) {
				Area* pArea = getDatabase(cTrans_)->getArea(*pAreaName->getIdentifier(), cTrans_);

                if (!pArea) {
                    // 指定された名称のエリアが存在しないので例外送出
                    _SYDNEY_THROW2(Exception::AreaNotFound,
                                   *pAreaName->getIdentifier(),
								   getDatabase(cTrans_)->getName());
                }

                if (eCheckedCategories[i] == getAreaCategory()) {

					// 修正後のIDを設定しなおす
                    vecPostAreaID_[i] = pArea->getID();

					if (!bResult) {
						bResult = (vecPrevAreaID_[i] != vecPostAreaID_[i]);
					}
                }
            }
		}
	}
	return bResult;
}

//	FUNCTION public
//	Schema::Index::setArea --
//		SQL構文要素から索引ファイルを格納するエリアを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//      const ModVector<ID::Value>& vecPrevAreaID_
//			変更前のエリアオブジェクトID
//		const ModVector<ID::Value>& vecPostAreaID_
//			変更後のエリアオブジェクトID
//		bool bUndo_ = false
//			エラー処理中を示すフラグ
//		bool bRecovery_ = false
//			REDO中であることを示す
//		bool bMount = false
//			MOUNTに伴う処理であることを示す
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::AreaNotFound
//			指定された名前のエリアはない

void
Index::
setArea(Trans::Transaction& cTrans_,
        const ModVector<ID::Value>& vecPrevAreaID_,
        const ModVector<ID::Value>& vecPostAreaID_,
        bool bUndo_, bool bRecovery_, bool bMount_)
{
	AreaCategory::Value eCheckedCategories[_iAreaNumber];
	eCheckedCategories[0] = getAreaCategory();

    ModVector<AreaContent*>		vecpContent(_iAreaNumber, 0);
	// Fileの移動に関するエラー処理に使う
    bool bFileMoved = false;
	ID::Value iPrevFileAreaID = ID::Invalid;
	ID::Value iPostFileAreaID = ID::Invalid;
    
	Database* pDatabase = getDatabase(cTrans_);

    try {

        for (int i = 0; i < _iAreaNumber; i++) {

            // 変更するエリアがあるときのみ処理する
            if ( vecPostAreaID_[i] != ID::Invalid ) {

				// 変更前後のエリアを得る

                Area* pPostArea = pDatabase->getArea(vecPostAreaID_[i], cTrans_);
                Area* pPrevArea = pDatabase->getArea(vecPrevAreaID_[i], cTrans_);

				// エリアの移動に対応する格納関係の変更をする
				vecpContent[i] =
					AreaContent::moveArea(cTrans_, pPrevArea, pPostArea, this,
										  eCheckedCategories[i],
										  bUndo_, bRecovery_, bMount_);
				SCHEMA_FAKE_ERROR("Schema::Index", "SetArea", "MoveArea");

				// 新たなエリア ID を設定する
				{
	                ID::Value* pAreaID = (i == 0) ? &m_iAreaID : &m_iLogAreaID;
					*pAreaID = vecPostAreaID_[i];
				}

                if (eCheckedCategories[i] == getAreaCategory()) {

					; _SYDNEY_ASSERT(!bFileMoved);

                    // ファイルを新しいエリアに移動する
					// ファイルのエリア指定には表の指定までさかのぼったIDを使う
					iPrevFileAreaID =
						(vecPrevAreaID_[i] == ID::Invalid)
						? getTable(cTrans_)->getAreaID(getAreaCategory(), true /* effective */)
						: vecPrevAreaID_[i];
					iPostFileAreaID =
						(vecPostAreaID_[i] == ID::Invalid)
						? getTable(cTrans_)->getAreaID(getAreaCategory(), true /* effective */)
						: vecPostAreaID_[i];

                    moveArea(cTrans_, iPrevFileAreaID, iPostFileAreaID,
							 bUndo_, bRecovery_, bMount_);

                    // エラー処理の為、移動フラグを保存する
                    bFileMoved = true;
					SCHEMA_FAKE_ERROR("Schema::Index", "SetArea", "FileMoved");
                }
            }
        }
    }
    catch ( ... ) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		for (int i = 0; i < _iAreaNumber; i++ ) {

			// 対応関係を元に戻す
			if ( AreaContent* pContent = vecpContent[i] ) {
				Area* pPostArea = pDatabase->getArea(vecPostAreaID_[i], cTrans_);
				Area* pPrevArea = pDatabase->getArea(vecPrevAreaID_[i], cTrans_);
				AreaContent::undoMoveArea(cTrans_, pPrevArea, pPostArea, this,
										  eCheckedCategories[i], pContent);
			}

			// ID の割り当てを元に戻す
			ID::Value* pAreaID = (i == 0) ? &m_iAreaID : &m_iLogAreaID;
			*pAreaID = vecPrevAreaID_[i];
		}
                
		// ファイルを移動していたら元に戻す
		if (bFileMoved) {
			// 移動前の Area に戻す
			moveArea(cTrans_, iPostFileAreaID, iPrevFileAreaID, true /* undo */, bRecovery_, bMount_);
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
    }
	// alter index drop areaでもエラーにならないとテストスクリプトが複雑になるので
	// ここでもFakeErrorを入れておく
	SCHEMA_FAKE_ERROR("Schema::Index", "SetArea", "MoveArea");
	SCHEMA_FAKE_ERROR("Schema::Index", "SetArea", "FileMoved");
}

//	FUNCTION public
//	Schema::Index::createAreaContent --
//		索引と関係するエリア格納関係を作成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		Exception::AreaNotFound
//			指定された名前のエリアはない

void
Index::
createAreaContent(Trans::Transaction& cTrans_)
{
	AreaCategory::Value eCheckedCategories[_iAreaNumber];
	eCheckedCategories[0] = getAreaCategory();
    
	Database* pDatabase = getDatabase(cTrans_);

	// createから作成されている場合、エラー処理のdropで格納関係も破棄されるので
	// ここでのエラー処理は不要

	for (int i = 0; i < _iAreaNumber; i++) {

		// 指定されているエリアがあるときのみ処理する
		ID::Value iAreaID = (i == 0) ? m_iAreaID : m_iLogAreaID;

		if ( iAreaID != ID::Invalid ) {

			// エリアを得る
			Area* pArea = pDatabase->getArea(iAreaID, cTrans_);

			if (pArea == 0) {

				// ロールフォワードリカバリの場合エリアが
				// 存在しないことがありうる

				SydMessage << "Area 'ID=" << iAreaID
						   << "' not found in database '"
						   << pDatabase->getName() << "'"
						   << ModEndl;

				if (i == 0)
					m_iAreaID = ID::Invalid;
				else
					m_iLogAreaID = ID::Invalid;

				continue;
			}

			// 作成による格納関係の変更をする
			(void) AreaContent::moveArea(cTrans_, 0/* no prev */, pArea, this,
										 eCheckedCategories[i]);

			// ファイルの格納関係はファイルの作成時に作られるのでここでは何もしない
		}
    }
}

//	FUNCTION public
//	Schema::Index::prepareDropArea --
//		SQL構文要素から索引ファイルを格納するエリアの設定を解除する準備をする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Statement::AreaOption&			cStatement_
//			エリア指定を表すSQL構文要素
//      ModVector<ID::Value>& vecPrevAreaID_
//			変更前のエリアオブジェクトID
//		ModVector<ID::Value>& vecPostAreaID_
//			変更後のエリアオブジェクトID
//		bool bUndo_ = false
//			エラー処理中を表すフラグ
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

bool
Index::
prepareDropArea(Trans::Transaction& cTrans_,
        		const Statement::AreaOption& cStatement_,
         		ModVector<ID::Value>& vecPrevAreaID_,
         		ModVector<ID::Value>& vecPostAreaID_)
{
	bool bResult = false;
	AreaCategory::Value eCheckedCategories[_iAreaNumber];
	eCheckedCategories[0] = getAreaCategory();

	// Undo/Redoの仕様変更によりログひとつでAlter後の
	// Area指定すべてを取得できる必要ができた
	// したがってsetの指定の有無に関係なく修正前後のIDを入れる
	// 修正前後に現在の値を入れるのは呼び出し側で行うようにした
	// // まず修正前後に現在の値を入れる
	// getAreaID(vecPrevAreaID_);
	// getAreaID(vecPostAreaID_);

    for (int i = 0; i < _iAreaNumber; i++) {

        if ( Statement::Identifier* pAreaName = cStatement_.getAreaName(
        			Index::convertAreaCategory(eCheckedCategories[i]))) {

            // ★注意★
            // dropにおいてはエリア名の指定は無効であるが、
            // ここではログを出して無視する

            if (pAreaName->getIdentifier()) {
                SydInfoMessage
					<< "Specified area name in drop area: "
                    << *pAreaName->getIdentifier()
					<< " ignored." << ModEndl;
            }

            // 現在の指定が有効なものであるときにのみ処理すればよい
            if (vecPrevAreaID_[i] != ID::Invalid) {

                if (eCheckedCategories[i] == getAreaCategory()) {

                    // 変更後のエリアIDを設定する
					// ★注意★
					// 以前の実装ではここで表のエリア指定を参照していたが
					// それでは表と同じエリアを指定されたのとDROP AREAされたのとを
					// 区別できないのでここではInvalidを入れる
                    vecPostAreaID_[i] = ID::Invalid;

					if (!bResult) {
						bResult = (vecPrevAreaID_[i] != vecPostAreaID_[i]);
					}
                }
			}
		}
	}
	return bResult;
}

//	FUNCTION public
//	Schema::Index::dropArea --
//		SQL構文要素から索引ファイルを格納するエリアの設定を解除する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//      const ModVector<ID::Value>& vecPrevAreaID_
//			変更前のエリアオブジェクトID
//		const ModVector<ID::Value>& vecPostAreaID_
//			変更後のエリアオブジェクトID
//		bool bUndo_ = false
//			エラー処理中を表すフラグ
//		bool bRecovery_ = false
//			REDO処理中であることを示す
//		bool bMount = false
//			MOUNTに伴う処理であることを示す
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Index::
dropArea(Trans::Transaction& cTrans_,
         const ModVector<ID::Value>& vecPrevAreaID_,
         const ModVector<ID::Value>& vecPostAreaID_,
         bool bUndo_, bool bRecovery_, bool bMount_)
{
	AreaCategory::Value eCheckedCategories[_iAreaNumber];
	eCheckedCategories[0] = getAreaCategory();

	ModVector<bool> vecContentDropped(_iAreaNumber, false);
	// Fileの移動に関するエラー処理に使う
	bool bFileMoved = false;
	ID::Value iPrevFileAreaID = ID::Invalid;
	ID::Value iPostFileAreaID = ID::Invalid;

	Database* pDatabase = getDatabase(cTrans_);

    try {

        for (int i = 0; i < _iAreaNumber; i++) {

            // エリア ID が有効なものについて処理する
            if ( vecPrevAreaID_[i] != ID::Invalid && vecPostAreaID_[i] == ID::Invalid ) {

                Area* pArea = pDatabase->getArea(vecPrevAreaID_[i], cTrans_);

                // リカバリー中でなければエリアオブジェクトは必ずあるはず
                ; _SYDNEY_ASSERT(pArea || bRecovery_);

				if (pArea) {
					// 対応関係を表すクラスを破棄する
					// 永続化は呼び出し側で行われる

					AreaContent::drop(*pArea, *this, eCheckedCategories[i], cTrans_, bRecovery_);
					vecContentDropped[i] = true;
				}
				SCHEMA_FAKE_ERROR("Schema::Index", "DropArea", "AreaDropped");

                // エリアの割り当てを消去する
				ID::Value* pAreaID = (i == 0) ? &m_iAreaID : &m_iLogAreaID;
				*pAreaID = ID::Invalid;
				
				if (eCheckedCategories[i] == getAreaCategory()) {

					; _SYDNEY_ASSERT(!bFileMoved);

                    // ファイルを新しいエリアに移動する
                    // 消去した結果新しいエリアは表での指定が使用される
					// ★注意★
					// PrevがInvalidであることはありえない
					iPrevFileAreaID = vecPrevAreaID_[i];
					; _SYDNEY_ASSERT(iPrevFileAreaID != ID::Invalid);
					iPostFileAreaID =
						(vecPostAreaID_[i] == ID::Invalid)
						? getTable(cTrans_)->getAreaID(getAreaCategory(), true /* effective */)
						: vecPostAreaID_[i];

                    moveArea(cTrans_, iPrevFileAreaID, iPostFileAreaID,
							 bUndo_, bRecovery_, bMount_);
					bFileMoved = true;
					SCHEMA_FAKE_ERROR("Schema::Index", "DropArea", "FileMoved");
                }
			}
		}
	}
    catch ( ... ) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		for ( int i = 0; i < _iAreaNumber; i++ ) {

			// エリア指定を元に戻す
			ID::Value* pAreaID = (i == 0) ? &m_iAreaID : &m_iLogAreaID;
			*pAreaID = vecPrevAreaID_[i];

			if (vecContentDropped[i]) {
				// エリアの取得
				Area* pOldArea = pDatabase->getArea(vecPrevAreaID_[i], cTrans_);
				// AreaContentのdropマークをクリアする
				AreaContent::undoDrop(*pOldArea, *this, eCheckedCategories[i], cTrans_);
			}
		}
		// ファイルが移動している場合ファイルを元に戻す
		if (bFileMoved) {
			// 元に戻す
			moveArea(cTrans_, iPostFileAreaID, iPrevFileAreaID, true /* undo */, bRecovery_, bMount_);
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
    }
	// alter index set areaでもエラーにならないとテストスクリプトが複雑になるので
	// ここでもFakeErrorを入れておく
	SCHEMA_FAKE_ERROR("Schema::Index", "DropArea", "AreaDropped");
	SCHEMA_FAKE_ERROR("Schema::Index", "DropArea", "FileMoved");
}

//	FUNCTION public
//	Schema::Index::moveArea --
//	NOTES
//		索引自体に対するエリア割り当て変更はIndex::alterで処理する
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Object::ID::Value iPrevAreaID_
//		Schema::Object::ID::Value iPostAreaID_
//			移動前後のエリアID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Index::
moveArea(Trans::Transaction& cTrans_,
		 ID::Value iPrevAreaID_, ID::Value iPostAreaID_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	// ファイルを移動する  
	if (File* pFile = getFile(cTrans_)) {
		pFile->moveArea(cTrans_, iPrevAreaID_, iPostAreaID_, bUndo_, bRecovery_, bMount_);
    }
}

//	FUNCTION public
//	Schema::Index::getPathPart --
//			ファイルの格納場所の索引固有部分を作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		ファイルを格納するパス名のうち索引固有部分
//
//	EXCEPTIONS

Os::Path
Index::getPathPart(Trans::Transaction& cTrans_) const
{
	// 表固有部分を使う
	// 索引ひとつに対してファイルもひとつなので
	// 索引に対応した部分は要らない

	return getTable(cTrans_)->getPathPart(cTrans_);
}

#ifdef OBSOLETE // Indexのパスを取得する機能は使用されない
//	FUNCTION public
//		Schema::Index::getPath --
//			索引を構成するファイルを格納するディレクトリー
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		索引を構成するファイルを格納するディレクトリーのパス名
//
//	EXCEPTIONS

const Os::Path&
Index::
getPath(Trans::Transaction& cTrans_) const
{
	if (!m_pPath) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		if (!m_pPath) {

			ModAutoPointer<Os::Path> pPath;

			// Indexのエリアを得る
			Area* pArea = getArea(getAreaCategory(), true, cTrans_);

			if (pArea) {
				//エリアがあれば0番のパスを使う
				pPath = new Os::Path(pArea->getPath(0));

			} else {
				// エリアがなければデータベースのDataパス指定を使う
				Database* pDatabase = getDatabase(cTrans_);
				; _SYDNEY_ASSERT(pDatabase);
				pPath = new Os::Path(pDatabase->getPath(Database::Path::Category::Data));
			}

			pPath->addPart(getPathPart(cTrans_));
			m_pPath = pPath.release();
		}
#ifdef DEBUG
		else {
			ModAutoPointer<Os::Path> pPath;
			// Indexのエリアを得る
			Area* pArea = getArea(getAreaCategory(), true, cTrans_);

			if (pArea) {
				//エリアがあれば0番のパスを使う
				pPath = new Os::Path(pArea->getPath(0));

			} else {
				// エリアがなければデータベースのDataパス指定を使う
				Database* pDatabase = getDatabase(cTrans_);
				; _SYDNEY_ASSERT(pDatabase);
				pPath = new Os::Path(pDatabase->getPath(Database::Path::Category::Data));
			}

			pPath->addPart(getPathPart(cTrans_));

			if (pPath->compare(*m_pPath) != Os::Path::CompareResult::Identical) {
				SydErrorMessage << "Illegal path cache of Index. " << *pPath << " != " << *m_pPath << ModEndl;
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
#endif
	}
	return *m_pPath;
}
#endif

//	FUNCTION public
//		Schema::Index::getPath --
//			索引を構成するファイルの格納場所のトップディレクトリーパス名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<ModUnicodeString>& vecDatabasePath_
//			データベースのパス指定
//		const ModVector<ModUnicodeString>& vecAreaPath_
//			エリアのパス指定
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//		const Schema::Object::Name& cTableName_
//			表名
//		const Schema::Object::Name& cFileName_
//			ファイル名
//
//	RETURN
//		表のデータを格納するディレクトリーのパス名
//
//	EXCEPTIONS

// static
Os::Path
Index::
getPath(const ModVector<ModUnicodeString>& vecDatabasePath_,
		const ModVector<ModUnicodeString>& vecAreaPath_,
		const Schema::Object::Name& cDatabaseName_,
		const Schema::Object::Name& cTableName_,
		const Schema::Object::Name& cFileName_)
{
	return Table::getPath(vecDatabasePath_, vecAreaPath_, cDatabaseName_, cTableName_).addPart(cFileName_);
}

//	FUNCTION public
//	Schema::Index::loadKey --
//		索引に属するすべてのキーを表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		索引に属するキーをひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const KeyMap&
Index::loadKey(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_keys) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!_keys) {
			// 「キー」表のうち、この索引に関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが一時オブジェクトかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			if ((getScope() != Scope::Permanent
				 || getStatus() == Status::Created)
				&& !bRecovery_)
				resetKey(*pDatabase);
			else {
				SystemTable::Key(*pDatabase).load(cTrans_, *this, bRecovery_);
			}
			; _SYDNEY_ASSERT(_keys);
		}
	}
	return *_keys;
}

//	FUNCTION public
//	Schema::Index::getKey --
//		索引に属するすべてのキーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		索引に属するキーを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

const ModVector<Key*>&
Index::getKey(Trans::Transaction& cTrans_) const
{
	return const_cast<Index*>(this)->loadKey(cTrans_).getView(getRWLock());
}

//	FUNCTION public
//	Schema::Index::getKey --
//		索引に属するキーのうち、ある列についたものを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Column&		column
//			この列についたキーを表すクラスを得る
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		指定された列についたキーを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Key*>
Index::
getKey(const Column& column, Trans::Transaction& cTrans_) const
{
	ModVector<Key*> v;

	const KeyMap& cMap = const_cast<Index*>(this)->loadKey(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction1<Key, ID::Value>
				 (KeyMap::findByColumnID, column.getID()));
	return v;
}

//	FUNCTION public
//	Schema::Index::getKey --
//		索引に属するキーのうち、あるオブジェクトIDを持つキーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iKeyID_
//			このオブジェクトIDを持ったキーを表すクラスを得る
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		指定されたオブジェクトIDを持つキー
//
//	EXCEPTIONS

Key*
Index::getKey(ID::Value iKeyID_, Trans::Transaction& cTrans_) const
{
	if (iKeyID_ == ID::Invalid)
		return 0;

	const KeyMap& cMap = const_cast<Index*>(this)->loadKey(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(iKeyID_).get();
}

#ifdef OBSOLETE // 名前や位置でキーを得る機能は使用されない

//	FUNCTION public
//	Schema::Index::getKey --
//		索引に属するキーのうち、ある名前を持つキーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name keyName
//			この名前を持ったキーを表すクラスを得る
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		指定された名前を持つキー
//
//	EXCEPTIONS

Key*
Index::getKey(const Name& keyName, Trans::Transaction& cTrans_) const
{
	const KeyMap& cMap = const_cast<Index*>(this)->loadKey(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Key, const Name&>(_Bool::_findByName<Key>, keyName));
}

//	FUNCTION public
//	Schema::Index::getKey --
//		索引に属するキーのうち、
//		指定した位置にあるキーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Key::Position keyPosition
//			キーの位置
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたキーを格納する領域の先頭アドレス
//		0
//			索引には指定された位置のキーは存在しない
//
//	EXCEPTIONS

Key*
Index::getKey(Key::Position keyPosition, Trans::Transaction& cTrans_) const
{
	const KeyMap& cMap = const_cast<Index*>(this)->loadKey(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Key, Key::Position>(KeyMap::findByPosition, keyPosition));
}
#endif

//	FUNCTION public
//	Schema::Index::addKey --
//		索引を表すクラスのキーとして、
//		指定されたキーを表すクラスを追加する
//
//	NOTES
//		「キー」表は更新されない
//
//	ARGUMENTS
//		const Schema::Key::Pointer&		key
//			追加するキーを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		追加したキーを表すクラス
//
//	EXCEPTIONS

const Key::Pointer&
Index::addKey(const Key::Pointer& key, Trans::Transaction& cTrans_)
{
	// 「キー」表のうち、この索引に関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたキーを追加する

	(void) loadKey(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	(void) _keys->insert(key);

	return key;
}

#ifdef OBSOLETE // KeyのdoAfterPersistで使用しているがそれがOBSOLETEなのでこれもOBSOLETEにする
//	FUNCTION public
//	Schema::Index::eraseKey --
//		データベースを表すクラスからあるキーを表すクラスの登録を抹消する
//
//	NOTES
//		「キー」表は更新されない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	keyID
//			登録を抹消するキーのオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
eraseKey(ID::Value keyID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	if (_keys)
		(void) _keys->erase(keyID);
}
#endif

//	FUNCTION public
//	Schema::File::resetKey --
//		索引にはキーを表すクラスが登録されていないことにする
//
//	NOTES
//		「キー」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::resetKey()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_keys) {

		if (getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_keys->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_keys->reset();

	} else {
		// キーを表すクラスを登録するハッシュマップを生成する

		_keys = new KeyMap;
		; _SYDNEY_ASSERT(_keys);
	}
}

//	FUNCTION public
//	Schema::Index::resetKey --
//		索引にはキーを表すクラスが登録されていないことにする
//
//	NOTES
//		「キー」表は更新されない
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::resetKey(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_keys) {

		_keys->reset(cDatabase_);

	} else {

		// キーを表すクラスを登録するハッシュマップを生成する

		_keys = new KeyMap;
		; _SYDNEY_ASSERT(_keys);
	}
}

//	FUNCTION public
//	Schema::Index::clearKey --
//		索引を表すクラスに登録されているキーを表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「キー」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::clearKey()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_keys) {

		delete _keys, _keys = 0;
	}
}

//	FUNCTION public
//	Schema::Index::getFile -- 索引を実現するファイルを表すクラスを得る
//
//	NOTES
//		索引を実現するファイルは索引一つにつき一つであると決まっている
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたファイルを格納する領域の先頭アドレス
//		0
//			索引を実現するファイルが存在しない
//
//	EXCEPTIONS

File*
Index::
getFile(Trans::Transaction& cTrans_) const
{
	if (m_iFileID != ID::Invalid) {
		if (!m_pFile.get()) {
			AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
			// 書き込みロックの中でもう一度調べる

			if (!m_pFile.get()) {
				// File::Pointerで得るためにFile::getは使わずに
				// 自分でFileMapをたどる
				Table* pTable = getTable(cTrans_);
				; _SYDNEY_ASSERT(pTable);

				File::Pointer pFile = pTable->getIndexFile(getID(), cTrans_);
				if (pFile.get()) {
					return const_cast<Index*>(this)->setFile(pFile).get();
				}
			}
		}
	}
	return m_pFile.get();
}

//	FUNCTION public
//	Schema::Index::setFile -- 索引を実現するファイルを表すクラスを設定する
//
//	NOTES
//		索引を実現するファイルは索引一つにつき一つであると決まっている
//
//	ARGUMENTS
//		Schema::File& cFile_
//			設定するファイルオブジェクト
//
//	RETURN
//		設定したファイルオブジェクト
//
//	EXCEPTIONS

const File::Pointer&
Index::
setFile(const File::Pointer& pFile_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	m_iFileID = pFile_->getID();
	return m_pFile = pFile_;
}

//	FUNCTION public
//	Schema::Index::clearFile -- ファイルの登録を抹消する
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

void
Index::
clearFile()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	m_iFileID = ID::Invalid;
	m_pFile = File::Pointer();
}

//	FUNCTION public
//	Schema::Index::getColumn --
//		索引に属するすべてのキーのそれぞれについて、
//		キーをつけた列を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		索引に属するキーをつけた列をひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Column*>
Index::getColumn(Trans::Transaction& cTrans_) const
{
	ModVector<Column*>	v;

	const KeyMap& cMap = const_cast<Index*>(this)->loadKey(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.apply(ApplyFunction2<Key, ModVector<Column*>&, Trans::Transaction&>
			   (&Key::appendColumn, v, cTrans_));
	return v;
}

#ifdef OBSOLETE // Fieldを得る機能は使用されない

//	FUNCTION public
//	Schema::Index::getSource --
//		索引に属するすべてのキーのそれぞれについて、
//		キーの値を格納するフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		索引に属するキーの値を格納するフィールドを
//		ひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Field*>
Index::getSource(Trans::Transaction& cTrans_) const
{
	ModVector<Field*>	v;

	const KeyMap& cMap = const_cast<Index*>(this)->loadKey(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.apply(ApplyFunction2<Key, ModVector<Field*>&, Trans::Transaction&>
			   (Key::appendField, v, cTrans_));
	return v;
}
#endif

//	FUNCTION public
//	Schema::Index::clearHint --
//		索引を表すクラスに登録されているヒントを表すクラスを破棄する
//
//	NOTES
//		「キー」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
clearHint()
{
	if (m_pHint)
		delete m_pHint, m_pHint = 0;
}

//	FUNCTION public
//	Schema::Index::clearAreaHint --
//		索引を表すクラスに登録されているエリアのヒントを表すクラスを破棄する
//
//	NOTES
//		「キー」表は更新されない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
clearAreaHint()
{
	if (m_pAreaHint)
		delete m_pAreaHint, m_pAreaHint = 0;
}

//	FUNCTION public
//	Schema::Index::serialize --
//		索引を表すクラスのシリアライザー
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
Index::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// 索引の種別
		{
		int tmp = _category;
		archiver << tmp;
		}
		// ユニークか

		archiver << _unique;

		// クラスター化されているか

		archiver << _clustered;

		// すべてのタプルが挿入されるか

		archiver << _hasAllTuples;

		// 索引のヒント
		int hasHint = ((m_pHint) ? 1 : 0) + ((m_pAreaHint) ? 2 : 0);
		archiver << hasHint;
		if (m_pHint) archiver << *m_pHint;
		if (m_pAreaHint) archiver << *m_pAreaHint;

		// ファイルを格納するエリア
		archiver << m_iAreaID;
		archiver << m_iLogAreaID;

		// 対応するファイル
		archiver << m_iFileID;

		// キー
		{
			ModSize n = (_keys) ? _keys->getSize() : 0;
			archiver << n;
			if (n) {
				Utility::OutputArchive& out =
					dynamic_cast<Utility::OutputArchive&>(archiver);

				_keys->writeObject(out);
			}
		}

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// 索引の種別
		{
		int tmp;
		archiver >> tmp;
		_category = static_cast<Category::Value>(tmp);
		}
		// ユニークか

		archiver >> _unique;

		// クラスター化されているか

		archiver >> _clustered;

		// すべてのタプルが挿入されるか

		archiver >> _hasAllTuples;

		// 索引のヒント
		int hasHint;
		archiver >> hasHint;
		if (hasHint % 2) {
			Hint cHint;
			archiver >> cHint;
			m_pHint = new Hint(cHint);
		}
		if (hasHint / 2) {
			Hint cHint;
			archiver >> cHint;
			m_pAreaHint = new Hint(cHint);
		}

		// ファイルを格納するエリア
		archiver >> m_iAreaID;
		archiver >> m_iLogAreaID;

		// 対応するファイル
		archiver >> m_iFileID;

		// キー
		{
			ModSize n;
			archiver >> n;
			if (n) {
				resetKey();
				Utility::InputArchive& in =
					dynamic_cast<Utility::InputArchive&>(archiver);

				_keys->readObject(in, n);
			}
		}
	}
}

//	FUNCTION public
//	Schema::Index::getClassID -- このクラスのクラス ID を得る
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

int
Index::
getClassID() const
{
	return Externalizable::Category::Index +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Index::clear -- 索引を表すクラスのメンバーをすべて初期化する
//
//	NOTES
//		親クラスのメンバーは初期化しない
//		下位オブジェクトのキャッシュからの抹消を行わないので
//		キャッシュに載っているオブジェクトに対してこのメソッドを呼ぶときには
//		あらかじめキャッシュから抹消する処理を行う必要がある
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
clear()
{
	_category = Category::Unknown;
	_unique = false;
	_clustered = false;
	_hasAllTuples = false;
	m_bOffline = false;
	m_pFile = FilePointer();
	m_iFileID = ID::Invalid;
	m_pArea = 0;
	m_iAreaID = ID::Invalid;
	m_pLogArea = 0;
	m_iLogAreaID = ID::Invalid;

	destruct();
}

//////////////////////////////////
// 以下は再構成のためのメソッド //
//////////////////////////////////

//	FUNCTION public
//	Schema::Index::verify --
//		索引の整合性検査を行う
//
//	NOTES
//
//	ARGUMENTS
//		Admin::Verification::Progress& cResult_
//			検査結果を格納する変数
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Admin::Verification::Treatment::Value eTreatment_
//			検査の結果見つかった障害に対する対応を表し、
//			Admin::Verification::Treatment::Value の論理和を指定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Index::
verify(Admin::Verification::Progress& cResult_,
	   Trans::Transaction& cTrans_,
	   Admin::Verification::Treatment::Value eTreatment_)
{
	const ModUnicodeString cstrPath;	// スキーマでProgressに入れるパスは空文字列

	// 呼び出し側で検査の経過が良好であることを保証する必要がある
	; _SYDNEY_ASSERT(cResult_.isGood());

	// 対象の名称を設定する
	Name cName(getDatabase(cTrans_)->getName());
	cName.append(Common::UnicodeChar::usPeriod);
	cName.append(getTable(cTrans_)->getName());
	cName.append(Common::UnicodeChar::usPeriod);
	cName.append(getName());

	cResult_.setSchemaObjectName(cName);

	SydSchemaVerifyMessage
		<< "Verify " << cName
		<< ModEndl;

	// 途中経過を出す
	_SYDNEY_VERIFY_INFO(cResult_, "", Message::VerifyStarted(getName()), eTreatment_);

	ModVector<File*> vecFile;

	if (eTreatment_ & Admin::Verification::Treatment::Data) {
		// RowIDを格納しているファイル、
		// キーフィールドを格納しているファイル、
		// 索引を構成するファイルでverifyを行う

		const ModVector<Key*>& vecKeys = getKey(cTrans_);
		ModSize nKeys = vecKeys.getSize();

		vecFile.reserve(2 + nKeys); // 2はRowIDファイルと索引ファイルの分

		// RowIDを格納しているファイルをセットする
		; _SYDNEY_ASSERT(getTable(cTrans_));
		; _SYDNEY_ASSERT(getTable(cTrans_)->getTupleID(cTrans_));
		; _SYDNEY_ASSERT(getTable(cTrans_)->getTupleID(cTrans_)->getField(cTrans_));
		; _SYDNEY_ASSERT(getTable(cTrans_)->getTupleID(cTrans_)->getField(cTrans_)->getFile(cTrans_));

		File* pRowIDFile = getTable(cTrans_)->getTupleID(cTrans_)->getField(cTrans_)->getFile(cTrans_);
		vecFile.pushBack(pRowIDFile);

		// 索引キーのフィールドを保持するファイルが初出であれば加える
		// ★注意★
		// キーを格納するファイルから高々1回のFetchでRowIDのファイルに到達するとの前提がある
		// キーの数は多くないはずなのでファイルの既出チェックは単純なサーチで行う

		for (ModSize i = 0; i < nKeys; ++i) {
			; _SYDNEY_ASSERT(vecKeys[i]->getColumn(cTrans_));
			; _SYDNEY_ASSERT(vecKeys[i]->getColumn(cTrans_)->getField(cTrans_));
			; _SYDNEY_ASSERT(vecKeys[i]->getColumn(cTrans_)->getField(cTrans_)->getFile(cTrans_));
			File* pKeyFile = vecKeys[i]->getColumn(cTrans_)->getField(cTrans_)->getFile(cTrans_);

			ModSize j = 0;
			for (; j <= i; ++j) {
				if (vecFile[j] == pKeyFile) break;
			}
			if (j > i) {
				// 初出である
				vecFile.pushBack(pKeyFile);
			}
		}
	}

	// 索引を構成するファイルをセットする
	; _SYDNEY_ASSERT(getFile(cTrans_));
	vecFile.pushBack(getFile(cTrans_));

	try {
		// ファイルの内容について調べる
		FileVerify cVerify(*getTable(cTrans_), vecFile);
		cVerify.verify(cResult_, cTrans_, eTreatment_);
		SCHEMA_FAKE_ERROR("Schema::Index", "Verify", "File");
		if (!cResult_.isGood())
			return;

	} catch (Exception::Object& e) {

		_SYDNEY_VERIFY_ABORTED(cResult_, "", e);
		_SYDNEY_RETHROW;

	} catch (...) {

		Exception::Unexpected e(moduleName, srcFile, __LINE__);
		_SYDNEY_VERIFY_ABORTED(cResult_, "", e);
		_SYDNEY_RETHROW;
	}

	// 途中経過を出す
	_SYDNEY_VERIFY_INFO(cResult_, "", Message::VerifyFinished(getName()), eTreatment_);
}

//	FUNCTION public
//	Schema::Index::makeLogData --
//		ログデータを作る
//
//	NOTES
//		引数のログデータには種別が設定されている必要がある
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		LogData& cLogData_
//			値を設定するログデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Index::makeLogData(Trans::Transaction& cTrans_, LogData& cLogData_) const
{
	// 全てに共通のデータ
	//	1. 表ID
	//	2. ID
	//	3. 名前
	//	4．種別

	cLogData_.addID(getTableID());
	cLogData_.addID(getID());
	cLogData_.addString(getName());
	cLogData_.addData(packMetaField(Meta::Index::Category));

	//	5．データベースID(Undo用)
	Database* pDatabase = getDatabase(cTrans_);
	cLogData_.addID(pDatabase->getID());

	//	6．データベースのパス指定(Undo用)
	ModVector<ModUnicodeString> vecDatabasePath;
	pDatabase->getPath(vecDatabasePath);
	cLogData_.addStrings(vecDatabasePath);

	//	7. 表名(UNDO用)
	cLogData_.addString(getTable(cTrans_)->getName());
	//	8. ファイル名(UNDO用)
	cLogData_.addString(getFile(cTrans_)->getName());

	switch (cLogData_.getSubCategory()) {
	case LogData::Category::CreateIndex:
	{
		//	 索引の作成
		//		9．フラグ
		//	   10．ヒント
		//	   12．エリアヒント
		//	   13．キー定義配列(DataArrayDataのDataArrayData)
		//	   11．エリアID配列
		//	   14．エリアパス配列(Undo用)
		cLogData_.addData(packMetaField(Meta::Index::Flag));
		cLogData_.addData(packMetaField(Meta::Index::Hint));
		cLogData_.addData(packMetaField(Meta::Index::AreaHint));

		// キー定義、エリアID、エリアパスはIndex::createでセットされる

		break;
	}
	case LogData::Category::DropIndex:
	{
		//	 索引の破棄
		//		9. エリアパス配列(UNDO用)

		// エリアパス配列(UNDO用)
		// エリアID配列をパス配列に変える
		ModVector<ID::Value> vecAreaID;
		getAreaID(vecAreaID);
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, vecAreaID));

		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Drop::Num);

		break;
	}
	case LogData::Category::AlterIndex:
	case LogData::Category::RenameIndex:
	{
		//	索引のエリア割り当て変更、名称変更
		//		<ここでセットすべきものはない>
		//		<変更前後のエリアID配列、パス配列はReorganize.cppでセットされる>

		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
}

// FUNCTION public
//	Schema::Index::makeLogData -- ログデータを作る(Constraint用)
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Common::DataArrayData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Index::
makeLogData(Trans::Transaction& cTrans_,
			Common::DataArrayData& cLogData_) const
{
	// 1. 索引ID
	// 2. キーID配列
	// 3. ファイル定義
	cLogData_.pushBack(LogData::createID(getID()));
	ModVector<ID::Value> vecKeyIDs;
	const ModVector<Key*>& vecKey = getKey(cTrans_);
	ModSize n = vecKey.getSize();
	vecKeyIDs.reserve(n);
	for (ModSize i = 0; i < n; ++i) {
		vecKeyIDs.pushBack(vecKey[i]->getID());
	}
	cLogData_.pushBack(LogData::createIDs(vecKeyIDs));
	File* pFile = getFile(cTrans_);
	ModAutoPointer<Common::DataArrayData> pFileLogData = new Common::DataArrayData;
	pFile->makeLogData(cTrans_, *pFileLogData);
	cLogData_.pushBack(pFileLogData.release());
}

//	FUNCTION public
//	Schema::Index::getCategory -- ログデータから索引種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			索引作成のログデータ
//
//	RETURN
//		索引種別
//
//	EXCEPTIONS

// static
Index::Category::Value
Index::
getCategory(const LogData& cLogData_)
{
	int value = cLogData_.getInteger(Log::Category);
	if (value <= Category::Unknown || value >= Category::ValueNum) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return static_cast<Category::Value>(value);
}

//	FUNCTION public
//	Schema::Index::getAreaCategory -- ログデータからエリア種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			索引変更または破棄のログデータ
//
//	RETURN
//		索引種別
//
//	EXCEPTIONS

// static
AreaCategory::Value
Index::
getAreaCategory(const LogData& cLogData_)
{
	static AreaCategory::Value _CategoryTable[] =
	{
		AreaCategory::Default,	// Unknown
		AreaCategory::Index,	// Normal
		AreaCategory::FullText, // FullText
		AreaCategory::Index,	// Bitmap
		AreaCategory::Index,	// Array
	};

	int value = cLogData_.getInteger(Log::Category);
	if (value <= Category::Unknown || value >= Category::ValueNum) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	return _CategoryTable[value];
}

//	FUNCTION public
//	Schema::Index::getName -- ログデータから索引名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			索引破棄または変更のログデータ
//
//	RETURN
//		索引名
//
//	EXCEPTIONS

// static
Object::Name
Index::
getName(const LogData& cLogData_)
{
	return cLogData_.getString(Log::Name);
}

//	FUNCTION public
//	Schema::Index::getObjectID -- ログデータから Schema ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			索引破棄または変更のログデータ
//
//	RETURN
//		Schema ID
//
//	EXCEPTIONS

// static
ObjectID::Value
Index::
getObjectID(const LogData& cLogData_)
{
	return cLogData_.getID(Log::ID);
}

//	FUNCTION public
//	Schema::Index::getDatabaseID -- ログデータからデータベースIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			索引作成のログデータ
//
//	RETURN
//		ログに記録されているデータベースID
//
//	EXCEPTIONS

// static
Object::ID::Value
Index::
getDatabaseID(const LogData& cLogData_)
{
	return cLogData_.getID(Log::DatabaseID);
}

//	FUNCTION public
//	Schema::Index::getDatabasePath -- ログデータからデータベースのパス指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			索引作成のログデータ
//		ModVector<ModUnicodeString>& vecPath_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Index::
getDatabasePath(const LogData& cLogData_, ModVector<ModUnicodeString>& vecPath_)
{
	vecPath_ = cLogData_.getStrings(Log::DatabasePaths);
}

//	FUNCTION public
//	Schema::Index::getTableID -- ログデータから表IDを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			索引破棄または変更のログデータ
//
//	RETURN
//		表ID
//
//	EXCEPTIONS

// static
Object::ID::Value
Index::
getTableID(const LogData& cLogData_)
{
	return cLogData_.getID(Log::TableID);
}

//	FUNCTION public
//	Schema::Index::getTableName -- ログデータから表名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			ログデータ
//
//	RETURN
//		索引が属する表の名称
//
//	EXCEPTIONS

// static
Object::Name
Index::
getTableName(const LogData& cLogData_)
{
	return cLogData_.getString(Log::TableName);
}

//	FUNCTION public
//	Schema::Index::getFileName -- ログデータからファイル名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			ログデータ
//
//	RETURN
//		索引を構成するファイルの名称
//
//	EXCEPTIONS

// static
Object::Name
Index::
getFileName(const LogData& cLogData_)
{
	return cLogData_.getString(Log::FileName);
}

//	FUNCTION public
//	Schema::Index::getAreaID --
//		索引を実現するファイルを格納するエリアのスキーマオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			ログデータ
//		const unsigned int pos_
//			ログデータでエリア指定が記録されている位置
//		ModVector<Schema::Object::ID::Value>& vecAreaID_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Index::
getAreaID(const LogData& log_, const unsigned int pos_,
		  ModVector<ID::Value>& vecAreaID_)
{
	vecAreaID_ = log_.getIDs(pos_);
}

//	FUNCTION public
//	Schema::Index::getAreaPath -- ログデータからエリアパスリストを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表のログデータ
//		int iIndex_
//			エリアパスが格納されているログの位置
//		int iCategory_
//			取得するエリアのカテゴリー(ログを作ったときに渡した配列上の位置)
//		ModVector<ModUnicodeString>& vecAreaPath_
//			エリアパスを格納するVector
//
//	RETURN
//		true .. 取得した
//		false.. ログデータにiCategory_に対応するデータがなかった
//
//	EXCEPTIONS

//static
bool
Index::
getAreaPath(const LogData& cLogData_, int iIndex_, int iCategory_, ModVector<ModUnicodeString>& vecAreaPath_)
{
	; _SYDNEY_ASSERT(vecAreaPath_.isEmpty());

	// ログデータからエリアパス部分を取り出す
	// ★注意★
	// makeLogDataの実装が変わったらここも変える
	return Area::getPathArray(cLogData_.getDataArrayData(iIndex_),
							  iCategory_, vecAreaPath_);
}

//	FUNCTION public
//	Index::clearPath -- パスのキャッシュをクリアする
//
//	NOTES
//
//	ARGUMENTS
//	なし
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Index::
clearPath()
{
	delete m_pPath, m_pPath = 0;
}

// FUNCTION private
//	Schema::Index::isTemporary -- 
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

bool
Index::
isTemporary()
{
	return getScope() == Scope::SessionTemporary;
}

//	FUNCTION private
//	Schema::Index::setAreaID --
//		エリアID割り当て情報をセットする
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<Schema::Object::ID::Value>& vecAreaID_
//			設定するエリアID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Index::
setAreaID(const ModVector<ID::Value>& vecAreaID_)
{
	if (vecAreaID_.getSize() > 0) {
		m_iAreaID = vecAreaID_[0];
		m_pArea = 0;
	}
}

//	FUNCTION private
//	Schema::Index::getEffectiveAreaID --
//		実質的なエリアID情報をセットする
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<Schema::Object::ID::Value>& vecAreaID_
//			割り当て情報として渡されるエリアID
//		const ModVector<ID::Value>& vecTableAreaID_
//			表の割り当て情報を保持する配列
//		AreaCategory::Value eAreaCategory_
//			索引に対応するエリアカテゴリー
//		ModVector<Schema::Object::ID::Value>& vecEffectiveAreaID_
//			返り値の実質的なエリアID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Index::
getEffectiveAreaID(const ModVector<ID::Value>& vecAreaID_,
				   const ModVector<ID::Value>& vecTableAreaID_,
				   AreaCategory::Value eAreaCategory_,
				   ModVector<ID::Value>& vecEffectiveAreaID_)
{
	AreaCategory::Value eCheckedCategories[_iAreaNumber];
	eCheckedCategories[0] = eAreaCategory_;

	vecEffectiveAreaID_ = vecAreaID_;
	; _SYDNEY_ASSERT(vecEffectiveAreaID_.getSize() >= _iAreaNumber);

	for (int i = 0; i < _iAreaNumber; ++i) {
		if (vecEffectiveAreaID_[i] == ID::Invalid) {
			vecEffectiveAreaID_[i] = Table::getAreaID(vecTableAreaID_, eCheckedCategories[i], true /* effective */);
		}
	}
}

//	FUNCTION private
//	Schema::Index::checkUndo --
//		Undo情報を検査し反映する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& cDatabase_
//			索引が属するデータベースオブジェクト
//		Schema::Object::ID::Value iID_
//			検査に使用するID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Index::
checkUndo(const Database& cDatabase_, ID::Value iID_)
{
	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(cDatabase_.getName(), iID_, Undo::Type::AlterIndex)) {
		ModVector<Object::ID::Value> vecUndoAreaID;
		if (Manager::RecoveryUtility::ID::getUndoAreaID(cDatabase_.getName(), iID_, vecUndoAreaID)) {
			// Alter後のエリアID割り当てが登録されているときはログデータのパスではなく
			// Alter後のエリアID割り当てを使用する
			setAreaID(vecUndoAreaID);
		}
	}
	if (Undo::isEntered(cDatabase_.getName(), iID_, Undo::Type::RenameIndex)
		|| Undo::isEntered(cDatabase_.getName(), getParentID(), Undo::Type::RenameTable)) {
		Name cUndoName;
		if (Manager::RecoveryUtility::Name::getUndoName(cDatabase_.getName(), iID_, cUndoName)) {
			// Rename後の名前が登録されているときはログデータの名前ではなく
			// Rename後の名前を使用する
			setName(cUndoName);
		}
	}
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

// メタデータベースにおける「索引」表の構造は以下のとおり
// create table Index_DBXXXX (
//		ID			id,
//		parent		id,
//		name		nvarchar,
//		category	int,
//		flag		int,
//		areaCategory int,
//		file		id,
//		hint		nvarchar,
//		area		<id array>, -- エリアIDの配列
//		areaHint	nvarchar,
//		time		timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Index>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Index>(Meta::MemberType::_type_, &Index::_get_, &Index::_set_)

	Meta::Definition<Index> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(ParentID),		// ParentID
		_DEFINE0(Name),			// Name
		_DEFINE0(Integer),		// Category,
		_DEFINE0(Integer),		//Flag,
		_DEFINE2(ID, getFileID, setFileID), //FileID,
		_DEFINE0(Binary),		//Hint,
		_DEFINE0(IDArray),		//AreaIDs,
		_DEFINE0(Binary),		//AreaHint,
		_DEFINE0(Timestamp),		// Timestamp
		_DEFINE0(Unknown),		// MemberNum
		_DEFINE0(String),		//HintString,
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Index::getMetaFieldNumber --
//		スキーマオブジェクトを格納するファイルのフィールド数を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

int
Index::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Index::MemberNum);
}

//	FUNCTION public
//	Schema::Index::getMetaFieldDefinition --
//		スキーマオブジェクトを格納するファイルのフィールド定義を得る
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			フィールドのメンバーを識別する番号
//
//	RETURN
//	フィールドの数
//
//	EXCEPTIONS

Meta::MemberType::Value
Index::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Index::packMetaField --
//		スキーマオブジェクトの内容をレコードファイルに格納するため
//		DataArrayDataにする
//
//	NOTES
//
//	ARGUMENTS
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		0以外...正しく変換された
//		0    ...変換に失敗した
//
//	EXCEPTIONS

Common::Data::Pointer
Index::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Index>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::ID:
		{
			return pack((this->*(cDef.m_funcGet._id))());
		}
	case Meta::MemberType::Integer:
		{
			return packIntegerMetaField(iMemberID_);
		}
	case Meta::MemberType::Binary:
		{
			return packBinaryMetaField(iMemberID_);
		}
	case Meta::MemberType::IDArray:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Index::AreaIDs);
			ModVector<ID::Value> vecID;
			vecID.reserve(2);
			if (m_iAreaID != ID::Invalid || m_iLogAreaID != ID::Invalid) {
				vecID.pushBack(m_iAreaID);
				if (m_iLogAreaID != ID::Invalid)
					vecID.pushBack(m_iLogAreaID);
			}
			return pack(vecID);
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Index::unpackMetaField --
//		DataArrayDataをスキーマオブジェクトの内容に反映させる
//
//	NOTES
//
//	ARGUMENTS
//		const Common::Data* pData_
//			内容を反映するData
//		int iMemberID_
//			データベース表のフィールドに対応する数値
//
//	RETURN
//		true...正しく変換された
//		false..変換に失敗した
//
//	EXCEPTIONS

bool
Index::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Index>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::ID:
		{
			ID::Value id;
			if (unpack(pData_, id)) {
				(this->*(cDef.m_funcSet._id))(id);
				return true;
			}
			break;
		}
	case Meta::MemberType::Integer:
		{
			return unpackIntegerMetaField(pData_, iMemberID_);
		}
	case Meta::MemberType::Binary:
		{
			return unpackBinaryMetaField(pData_, iMemberID_);
		}
	case Meta::MemberType::IDArray:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Index::AreaIDs);
			ModVector<ID::Value> vecID;
			if (unpack(pData_, vecID)) {
				if (vecID.getSize() > 0)
					m_iAreaID = vecID[0];
				if (vecID.getSize() > 1)
					m_iLogAreaID = vecID[1];
				return true;
			}
			break;
		}
	default:
		break;
	}
	return false;
}

Common::Data::Pointer
Index::
packIntegerMetaField(int iMemberID_) const
{
	switch (iMemberID_) {
	case Meta::Index::Category:
		{
			return pack(static_cast<int>(getCategory()));
		}
	case Meta::Index::Flag:
		{
			//		0:	非unique, 非clustered, 非hasAllTuples, 非offline
			//		1:	unique, 非clustered, 非hasAllTuples, 非offline
			//		2:	非unique, clustered, 非hasAllTuples, 非offline
			//		3:	unique, clustered, 非hasAllTuples, 非offline
			//		4:	非unique, 非clustered, hasAllTuples, 非offline
			//		5:	unique, 非clustered, hasAllTuples, 非offline
			//		6:	非unique, clustered, hasAllTuples, 非offline
			//		7:	unique, clustered, hasAllTuples, 非offline
			//		8:	非unique, 非clustered, 非hasAllTuples, offline
			//		9:	unique, 非clustered, 非hasAllTuples, offline
			//	   10:	非unique, clustered, 非hasAllTuples, offline
			//	   11:	unique, clustered, 非hasAllTuples, offline
			//	   12:	非unique, 非clustered, hasAllTuples, offline
			//	   13:	unique, 非clustered, hasAllTuples, offline
			//	   14:	非unique, clustered, hasAllTuples, offline
			//	   15:	unique, clustered, hasAllTuples, offline

			int iValue = (_unique ? 1 : 0) + (_clustered ? 2 : 0) + (_hasAllTuples ? 4 : 0) + (m_bOffline ? 8 : 0);
			return pack(iValue);
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

Common::Data::Pointer
Index::
packBinaryMetaField(int iMemberID_) const
{
	Utility::BinaryData& cArchiver = getArchiver();
	switch (iMemberID_) {
	case Meta::Index::Hint:
		{
			return cArchiver.put(getHint());
		}
	case Meta::Index::AreaHint:
		{
			return cArchiver.put(getAreaHint());
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

bool
Index::
unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_)
{
	int value;
	if (unpack(pData_, value)) {
		switch (iMemberID_) {
		case Meta::Index::Category:
			{
				if (value >= 0 && value < Category::ValueNum) {
					_category = static_cast<Category::Value>(value);
					return true;
				}
				break;
			}
		case Meta::Index::Flag:
			{
				//		0:	非unique, 非clustered, 非hasAllTuples, 非offline
				//		1:	unique, 非clustered, 非hasAllTuples, 非offline
				//		2:	非unique, clustered, 非hasAllTuples, 非offline
				//		3:	unique, clustered, 非hasAllTuples, 非offline
				//		4:	非unique, 非clustered, hasAllTuples, 非offline
				//		5:	unique, 非clustered, hasAllTuples, 非offline
				//		6:	非unique, clustered, hasAllTuples, 非offline
				//		7:	unique, clustered, hasAllTuples, 非offline
				//		8:	非unique, 非clustered, 非hasAllTuples, offline
				//		9:	unique, 非clustered, 非hasAllTuples, offline
				//	   10:	非unique, clustered, 非hasAllTuples, offline
				//	   11:	unique, clustered, 非hasAllTuples, offline
				//	   12:	非unique, 非clustered, hasAllTuples, offline
				//	   13:	unique, 非clustered, hasAllTuples, offline
				//	   14:	非unique, clustered, hasAllTuples, offline
				//	   15:	unique, clustered, hasAllTuples, offline
				// ★注意★
				// フラグで表現されるものが増えたら
				// 下の判定に使う上限値を変える必要がある。

				if (value >= 0 && value < 16) {
					_unique = (value % 2);
					_clustered = ((value % 4) / 2);
					_hasAllTuples = ((value % 8) / 4);
					m_bOffline = (value / 8);
					return true;
				}
				break;
			}
		}
	}
	return false;
}

bool
Index::
unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		return true;

	} else if (pData_ && pData_->getType() == Common::DataType::Binary) {
		const Common::BinaryData* pBinary =
			_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);

		Utility::BinaryData& cArchiver = getArchiver();

		switch (iMemberID_) {
		case Meta::Index::Hint:
			{
				ModAutoPointer<Hint> pData =
					dynamic_cast<Hint*>(cArchiver.get(pBinary));
				if (pData.get())
					m_pHint = pData.release();
				return true;
			}
		case Meta::Index::AreaHint:
			{
				ModAutoPointer<Hint> pData =
					dynamic_cast<Hint*>(cArchiver.get(pBinary));
				if (pData.get())
					m_pAreaHint = pData.release();
				return true;
			}
		}
	}
	return false;
}

//
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
