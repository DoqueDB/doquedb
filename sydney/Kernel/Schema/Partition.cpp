// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Partition.cpp -- ルール関連のルール定義
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#include "Schema/Partition.h"
#include "Schema/AutoRWLock.h"
#include "Schema/Database.h"
#include "Schema/FakeError.h"
#include "Schema/Function.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/ObjectID.h"
#include "Schema/Parameter.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_Partition.h"
#include "Schema/Table.h"
#include "Schema/Utility.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"

#include "DSchema/Table.h"

#include "Exception/ColumnNotFound.h"
#include "Exception/InvalidPartition.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/PartitionAlreadyDefined.h"
#include "Exception/StoredFunctionNotFound.h"
#include "Exception/SystemTable.h"
#include "Exception/TableNotEmpty.h"
#include "Exception/TableNotFound.h"
#include "Exception/TemporaryTable.h"
#include "Exception/Unexpected.h"

#include "Statement/AlterPartitionStatement.h"
#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/DropPartitionStatement.h"
#include "Statement/Identifier.h"
#include "Statement/Literal.h"
#include "Statement/PartitionDefinition.h"
#include "Statement/Token.h"
#include "Statement/ValueExpression.h"

#include "ModAutoPointer.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace _Name
	{
		// ルール名の重複を調べる
		bool _checkExistence(Trans::Transaction& cTrans_, const Database& cDatabase_, const Partition* pPartition_);
	} // namespace _Name

	// ValueExpressionから文字列を得る
	ModUnicodeString
	_convertValueExpressionToString(const Statement::ValueExpression* pValueExpression_)
	{
		if (pValueExpression_->getOperator() != Statement::ValueExpression::op_Literal) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		const Statement::Literal* pLiteral =
			_SYDNEY_DYNAMIC_CAST(const Statement::Literal*,
								 pValueExpression_->getPrimary());
		const Statement::Token& cToken = pLiteral->getToken();
		if (cToken.getHead() == cToken.getTail()) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		return ModUnicodeString(cToken.getHead(), cToken.getLength());
	}

} // namespace

//////////////////////////
// _Name
//////////////////////////

//	FUNCTION local
//	_Name::_checkExistence -- ルール名の重複を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			ルールが属するデータベース
//		const Schema::Partition* pPartition_
//			作成しようとしているルール
//
//	RETURN
//		true ... 同じ名前のものが存在している、または作成中である
//		false... 同じ名前のものはない
//
//	EXCEPTIONS
//		Exception::PartitionAlreadyDefined
//			同じ名前のものが存在しており、CanceledWhenDuplicatedがfalseである

bool
_Name::_checkExistence(Trans::Transaction& cTrans_,
					   const Database& cDatabase_,
					   const Partition* pPartition_)
{
	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (Manager::ObjectName::reserve(pPartition_) == false) {

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// trueを返し後の処理は何もしない
			SydInfoMessage
				<< "Partition definition of the same name in progress("
				<< pPartition_->getName()
				<< ") canceled"
				<< ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage
				<< "Partition definition of the same name in progress("
				<< pPartition_->getName()
				<< ")"
				<< ModEndl;
			_SYDNEY_THROW2(Exception::PartitionAlreadyDefined,
						   pPartition_->getName(), cDatabase_.getName());
		}
	}

	// さらに、同じ名前のルールがすでにないか調べ、
	// 同時に現在のルールをマネージャーに読み込んでおく
	// ★注意★
	// doAfterPersistの中でマネージャーに追加されるので
	// ここで読み込んでおかないと追加のときに不完全なPartitionを
	// 読み込んでしまう

	if (cDatabase_.getPartition(pPartition_->getName(), cTrans_)) {

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(pPartition_);

		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated partition definition("
						   << pPartition_->getName()
						   << ") canceled"
						   << ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated partition definition("
						   << pPartition_->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::PartitionAlreadyDefined,
						   pPartition_->getName(), cDatabase_.getName());
		}
	}
	return false;
}

///////////////////////
// Schema::Partition	 //
///////////////////////

//	FUNCTION public
//		Schema::Partition::Partition -- コンストラクター
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
//		???
//

Partition::
Partition()
	: Object(Object::Category::Partition),
	  m_eCategory(Category::Normal),
	  m_iTableID(ID::Invalid),
	  m_pTable(0),
	  m_cTarget()
{
}

//
//	FUNCTION public
//		Schema::Partition::Partition -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			ルールを定義するデータベース
//		const Table& cTable_
//			ルールの対象となる表
//		const Statement::PartitionDefinition& cStatement_
//			ルール定義のステートメント
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Partition::
Partition(const Database& cDatabase_, const Table& cTable_,
		  const Statement::PartitionDefinition& cStatement_)
	: Object(Object::Category::Partition, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_eCategory(Category::Normal),
	  m_iTableID(cTable_.getID()),
	  m_pTable(const_cast<Table*>(&cTable_)),
	  m_cTarget()
{
	// ルール定義のSQL文からルールオブジェクトを作る
	// create partition on <table名> by <function name> [ (<column name>, ...) ]

	// 渡されたStatement::Objectの中身は正しいはずである

	setName(cTable_.getName());

	// 残りは呼び出し側でセットする
}

//
//	FUNCTION public
//		Schema::Partition::Partition -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Database& cDatabase_
//			ルールを定義するデータベース
//		const Schema::LogData& cLogData_
//			ルール定義のログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Partition::
Partition(const Database& cDatabase_, const LogData& cLogData_)
	: Object(Object::Category::Partition, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_eCategory(Category::Normal),
	  m_iTableID(ID::Invalid),
	  m_pTable(0),
	  m_cTarget()
{
	// ログの内容を反映する
	//	ログの内容:
	//		1．名前
	//		2. ID(使用しない)
	//		3．TableID
	//		4．Category
	//		5. FunctionName
	//		6. ColumnIDs
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える

	if (!unpackMetaField(cLogData_[Log::Name].get(), Meta::Partition::Name)
		|| !unpackMetaField(cLogData_[Log::TableID].get(), Meta::Partition::TableID)
		|| !unpackMetaField(cLogData_[Log::Create::Category].get(), Meta::Partition::Category)
		|| !unpackMetaField(cLogData_[Log::Create::FunctionName].get(), Meta::Partition::FunctionName)
		|| !unpackMetaField(cLogData_[Log::Create::ColumnIDs].get(), Meta::Partition::ColumnIDs)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

//
//	FUNCTION public
//		Schema::Partition::~Partition -- デストラクター
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
//		???
//

Partition::
~Partition()
{
	destruct();
}

//	FUNCTION public
//		Schema::Partition::getNewInstance -- オブジェクトを新たに取得する
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
Partition*
Partition::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Partition> pObject = new Partition;
	pObject->unpack(cData_);
	return pObject.release();
}

//
//	FUNCTION public
//		Schema::Partition::create -- ルールを生成する
//
//	NOTES
//		システム表の永続化は呼び出し側で行う
//
//	ARGUMENTS
//		Database& cDatabase_
//			ルールを定義するデータベース
//		const Statement::PartitionDefinition& cStatement_
//			ルール定義のステートメント
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			ルール抹消を行うトランザクション記述子
//
//	RETURN
//		Schema::Partition*
//			生成したルールオブジェクト
//
//	EXCEPTIONS
//		Exception::PartitionAlreadyDefined
//			存在する名前でcreateしようとした

// static
Partition*
Partition::
create(Database& cDatabase_, const Statement::PartitionDefinition& cStatement_,
	   LogData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 表名
	; _SYDNEY_ASSERT(cStatement_.getTableName());
	; _SYDNEY_ASSERT(cStatement_.getTableName()->getIdentifier());
	const Name& cTableName = *(cStatement_.getTableName()->getIdentifier());

	// 対象となる表を得る
	Table* pTable =
		cDatabase_.getTable(cTableName, cTrans_, true /* internal */);
	if (!pTable) {
		// 指定された名称をもつ表がないので例外送出
		_SYDNEY_THROW2(Exception::TableNotFound,
					   cTableName, cDatabase_.getName());
	}
	; _SYDNEY_ASSERT(pTable);
	if (pTable->isSystem()) {
		// システム表に索引は作成できない
		_SYDNEY_THROW1(Exception::SystemTable, cTableName);
	}
	if (pTable->isTemporary()) {
		// 一時表には定義できない
		_SYDNEY_THROW0(Exception::TemporaryTable);
	}
	if (cStatement_.getCategory() != Statement::PartitionDefinition::Category::ReadOnly
		&& DSchema::Table::isEmpty(cTrans_, *pTable) == false) {
		// Table is not empty
		_SYDNEY_THROW1(Exception::TableNotEmpty, cTableName);
	}

	// 定義からルールオブジェクトを生成する
	ModAutoPointer<Partition> pPartition = new Partition(cDatabase_, *pTable, cStatement_);
	; _SYDNEY_ASSERT(pPartition.get());

	// categoryをセットする
	pPartition->setCategory(static_cast<Category::Value>(cStatement_.getCategory()));

	// Targetをセットする
	Target cTarget;
	pPartition->setTarget(cTrans_, cStatement_, cTarget);

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, cDatabase_, pPartition.get())) {
		return 0;
	}
	// Targetをメンバーにセットする
	pPartition->setTarget(cTarget);

	// IDをふり、状態を変える
	pPartition->Object::create(cTrans_);

	// ログデータを作る
	pPartition->makeLogData(cLogData_);

	// 生成されたルールオブジェクトを返す
    return pPartition.release();
}

//
//	FUNCTION public
//		Schema::Partition::create -- ルールを生成する
//
//	NOTES
//		リカバリー中のredoで呼ばれる
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			ルール定義を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			ルールを定義するデータベース
//		const Schema::LogData& cLogData_
//			ルール定義のログデータ
//
//	RETURN
//		Schema::Partition*
//			生成したルールオブジェクト
//
//	EXCEPTIONS
//		Exception::PartitionAlreadyDefined
//			存在する名前でcreateしようとした

// static
Partition*
Partition::
create(Trans::Transaction& cTrans_, const Database& cDatabase_,
	   const LogData& cLogData_)
{
	// ログデータからルールオブジェクトを生成する

	ModAutoPointer<Partition> pPartition = new Partition(cDatabase_, cLogData_);
	; _SYDNEY_ASSERT(pPartition.get());

	// IDはログのものを使用する
	Object::ID::Value id = getObjectID(cLogData_);

	// IDの整合性を取り、状態を変える
	pPartition->Object::create(cTrans_, id);

	// 生成されたルールオブジェクトを返す
    return pPartition.release();
}

//
//	FUNCTION public
//		Schema::Partition::getName -- ルール抹消のSQL文からルール名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::DropPartitionStatement& cStatement_
//			ルール抹消のステートメント
//
//	RETURN
//		対象のルール名
//
//	EXCEPTIONS

// static
Object::Name
Partition::
getName(const Statement::DropPartitionStatement& cStatement_)
{
	const Statement::Identifier* pIdentifier = cStatement_.getPartitionName();
	; _SYDNEY_ASSERT(pIdentifier);
	const ModUnicodeString* pName = pIdentifier->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

//
//	FUNCTION public
//		Schema::Partition::getName -- ルール変更のSQL文からルール名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Statement::AlterPartitionStatement& cStatement_
//			ルール変更のステートメント
//
//	RETURN
//		対象のルール名
//
//	EXCEPTIONS

// static
Object::Name
Partition::
getName(const Statement::AlterPartitionStatement& cStatement_)
{
	const Statement::Identifier* pIdentifier = cStatement_.getPartitionName();
	; _SYDNEY_ASSERT(pIdentifier);
	const ModUnicodeString* pName = pIdentifier->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

// FUNCTION public
//	Schema::Partition::getFunctionName -- SQL文からルールの関数名を得る
//
// NOTES
//
// ARGUMENTS
//	const Statement::AlterPartitionStatement& cStatement_
//	
// RETURN
//	Object::Name
//
// EXCEPTIONS

//static
Object::Name
Partition::
getFunctionName(const Statement::AlterPartitionStatement& cStatement_)
{
	const Statement::Identifier* pIdentifier = cStatement_.getFunction();
	; _SYDNEY_ASSERT(pIdentifier);
	const ModUnicodeString* pName = pIdentifier->getIdentifier();
	; _SYDNEY_ASSERT(pName);

	return *pName;
}

// FUNCTION public
//	Schema::Partition::getColumnIDs -- SQL文からルールに渡す列IDリストを得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Table* pTable_
//	const Statement::ColumnNameList& cColumnList_
//	
// RETURN
//	ModVector<ID::Value>
//
// EXCEPTIONS

//static
ModVector<Object::ID::Value>
Partition::
getColumnIDs(Trans::Transaction& cTrans_,
			 const Table* pTable_,
			 const Statement::ColumnNameList& cColumnList_)
{
	ModVector<ID::Value> vecResult;
	int n = cColumnList_.getCount();
	vecResult.reserve(n);
	for (int i = 0; i < n; ++i) {
		Statement::ColumnName* pColumnName = cColumnList_.getColumnNameAt(i);
		; _SYDNEY_ASSERT(pColumnName);
		; _SYDNEY_ASSERT(pColumnName->getIdentifierString());
		Column* pColumn = pTable_->getColumn(*pColumnName->getIdentifierString(), cTrans_);
		if (pColumn == 0) {
			_SYDNEY_THROW1(Exception::ColumnNotFound, *pColumnName->getIdentifierString());
		}
		vecResult.pushBack(pColumn->getID());
	}
	return vecResult;
}

//
//	FUNCTION public
//		Schema::Partition::getName -- 
//
//	NOTES
//		getNameをオーバーライドしたので
//		Objectのものを再度定義する
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ルール名
//
//	EXCEPTIONS

const Object::Name&
Partition::
getName() const
{
	return Object::getName();
}

//	FUNCTION public
//		Schema::Partition::drop -- ルールの破棄に関する処理をする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Partition& cPartition_
//			破棄対象のルール
//		Scheam::LogData& cLogData_
//			ログに出力するデータを格納する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
Partition::
drop(Trans::Transaction& cTrans_,
	 Partition& cPartition_, LogData& cLogData_)
{
	// 破棄マークをつける
	cPartition_.drop(cTrans_);
	// ログデータを作る
	cPartition_.makeLogData(cLogData_);
}

//
//	FUNCTION public
//		Schema::Partition::drop -- ルールを実際に抹消する
//
//	NOTES
//		システム表のXロックは呼び出し側で行う必要がある
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
//		???
//

void
Partition::
drop(Trans::Transaction& cTrans_, bool bRecovery_/* = false */)
{
	// 状態を変化させる
	Object::drop(bRecovery_);
}

// FUNCTION public
//	Schema::Partition::alter -- ルール変更の準備をする
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Partition& cPartition_
//	const Statement::AlterPartitionStatement& cStatement_
//	Target& cPrevTarget_
//	Target& cPostTarget_
//	LogData& cLogData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Partition::
alter(Trans::Transaction& cTrans_,
	  Partition& cPartition_,
	  const Statement::AlterPartitionStatement& cStatement_,
	  Target& cPrevTarget_,
	  Target& cPostTarget_,
	  LogData& cLogData_)
{
	// get target specification from alter statement
	cPartition_.setTarget(cTrans_,
						  cStatement_,
						  cPostTarget_);
	// get previous specification from object
	cPrevTarget_ = cPartition_.getTarget();
	
	bool bNoChange = (cPrevTarget_.m_cFunctionName == cPostTarget_.m_cFunctionName);
	if (bNoChange) {
		// check columnIDs
		int n = cPostTarget_.m_vecColumn.getSize();
		int m = cPrevTarget_.m_vecColumn.getSize();
		if (n != m) {
			bNoChange = false;
		} else {
			for (int i = 0; i < n; ++i) {
				if (cPostTarget_.m_vecColumn[i] != cPrevTarget_.m_vecColumn[i]) {
					bNoChange = false;
					break;
				}
			}
		}
	}
	if (bNoChange) {
		// nothing to do
		return false;
	}

	// create log data
	cPartition_.makeLogData(cLogData_);
	// prev target
	cLogData_.addString(cPrevTarget_.m_cFunctionName);
	cLogData_.addIDs(cPrevTarget_.m_vecColumn);
	// post target
	cLogData_.addString(cPostTarget_.m_cFunctionName);
	cLogData_.addIDs(cPostTarget_.m_vecColumn);

	return true;
}

// FUNCTION public
//	Schema::Partition::alter -- ルール変更の準備をする
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const LogData& cLogData_
//	Target& cPrevTarget_
//	Target& cPostTarget_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Partition::
alter(Trans::Transaction& cTrans_,
	  const LogData& cLogData_,
	  Target& cPrevTarget_,
	  Target& cPostTarget_)
{
	// set post target
	cPostTarget_.m_cFunctionName = cLogData_.getString(Log::Alter::PostFunctionName);
	cPostTarget_.m_vecColumn = cLogData_.getIDs(Log::Alter::PostColumnIDs);

	return true;
}

//
//	FUNCTION public
//		Schema::Partition::get -- スキーマオブジェクトIDを指定してルールを得る
//
//	NOTES
//		マウント中のデータベースに対してこのルールを使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			取得するルールのスキーマオブジェクトID
//		Schema::Database* pDatabase_
//			取得するルールが属するデータベース
//		Trans::Transaction& cTrans_
//			ルールを取得しようとしているトランザクション記述子
//
//	RETURN
//		取得したルールのオブジェクト
//
//	EXCEPTIONS
//		???
//

// static
Partition*
Partition::
get(ID::Value iID_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pDatabase_);

	if (iID_ == Object::ID::Invalid)
		return 0;

	return (pDatabase_) ? pDatabase_->getPartition(iID_, cTrans_) : 0;
}

// FUNCTION public
//	Schema::Partition::get -- 
//
// NOTES
//
// ARGUMENTS
//	const Name& cName_
//	Database* pDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Partition*
//
// EXCEPTIONS

//static
Partition*
Partition::
get(const Name& cName_, Database* pDatabase_,
	Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pDatabase_);
	return pDatabase_->getPartition(cName_, cTrans_);
}

// PARTITION public
//	Schema::Partition::getTarget -- 対象を指定する情報を得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Partition::Target&
//
// EXCEPTIONS

const Partition::Target&
Partition::
getTarget() const
{
	return m_cTarget;
}

// FUNCTION public
//	Schema::Partition::setTarget -- 対象を指定する情報を設定する
//
// NOTES
//
// ARGUMENTS
//	const Target& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Partition::
setTarget(const Target& cTarget_)
{
	m_cTarget = cTarget_;
}

//	FUNCTION public
//	Schema::Partition::doBeforePersist -- 永続化する前に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::PartitionPointer& pPartition_
//			永続化するルールのオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前のルールの状態
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
Partition::
doBeforePersist(const Pointer& pPartition_, Status::Value eStatus_,
				bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pPartition_.get());

	switch (eStatus_) {
	case Status::Created:
	case Status::Changed:
	case Status::Mounted:
	case Status::DeleteCanceled:
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		; // 何もしない
		break;
	}
	default:
		// 何もしない
		break;
	}
}

//	FUNCTION public
//	Schema::Partition::doAfterPersist -- 永続化した後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::PartitionPointer& pPartition_
//			永続化したルールのオブジェクト
//		Schema::Object::Status::Value eStatus_
//			永続化する前のルールの状態
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
Partition::
doAfterPersist(const Pointer& pPartition_, Status::Value eStatus_,
			   bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pPartition_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	ID::Value dbID = pPartition_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		Database* pDatabase = pPartition_->getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);
		// ルールはキャッシュに入れない
		(void) pDatabase->addPartition(pPartition_, cTrans_);
		break;
	}
	case Status::Changed:
	{
		break;
	}
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらマネージャーの登録からの削除も行う

		// 状態を「実際に削除された」にする

		pPartition_->setStatus(Schema::Object::Status::ReallyDeleted);

		// 下位オブジェクトがあればそれを抹消してからdeleteする
		pPartition_->reset();

		Table* pTable = pPartition_->getTable(cTrans_);
		if (pTable) {
			pTable->clearPartition();
		}
		Database* pDatabase = pPartition_->getDatabase(cTrans_);
		if (pDatabase)
			pDatabase->erasePartition(pPartition_->getID());
		break;
	}
	case Status::CreateCanceled:
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::Partition);
}

//	FUNCTION public
//	Schema::Partition::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::PartitionPointer& pPartition_
//			永続化したルールのオブジェクト
//		Schema::Database& cDatabase_
//			ルールが属するデータベース
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
Partition::
doAfterLoad(const Pointer& pPartition_, Database& cDatabase_, Trans::Transaction& cTrans_)
{
	// システムへ読み出したルールを表すクラスを追加する
	// また、マネージャーにこのルールを表すクラスを
	// スキーマオブジェクトとして管理させる
	// ルールはキャッシュに入れない

	cDatabase_.addPartition(pPartition_, cTrans_);
}

// FUNCTION public
//	Schema::Partition::getTable -- ルールが対象とする表を得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

Table*
Partition::
getTable(Trans::Transaction& cTrans_) const
{
	if (!m_pTable) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!m_pTable) ?
			m_pTable = Table::get(getTableID(), getDatabase(cTrans_), cTrans_, true /* internal */)
			: m_pTable;
	}
	return m_pTable;
}

// FUNCTION public
//	Schema::Partition::getTableID -- ルールが対象とする表のオブジェクトIDを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Partition::ID::Value
//
// EXCEPTIONS

Partition::ID::Value
Partition::
getTableID() const
{
	return m_iTableID;
}

// FUNCTION public
//	Schema::Partition::setTableID -- ルールが対象とする表のオブジェクトIDを設定する
//
// NOTES
//
// ARGUMENTS
//	ID::Value id_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Partition::
setTableID(ID::Value id_)
{
	m_iTableID = id_;
}

//	FUNCTION public
//	Schema::Partition::serialize --
//		ルールを表すクラスのシリアライザー
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
Partition::
serialize(ModArchive& archiver)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::Partition::getClassID -- このクラスのクラス ID を得る
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
Partition::
getClassID() const
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

//	FUNCTION public
//	Schema::Partition::clear --
//		ルールを表すクラスのメンバーをすべて初期化する
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
Partition::
clear()
{
	destruct();
}

//
//	FUNCTION private
//		Schema::Partition::destruct -- デストラクター下位ルール
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
//		???
//
void
Partition::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteする前にresetを呼ぶ
	// ここでは領域を開放するのみ
	;
}

//
//	FUNCTION public
//		Schema::Partition::reset --
//			下位オブジェクトを抹消する
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
//		???
//
void
Partition::
reset()
{
	;
}
void
Partition::
reset(Database& cDatabase_)
{
	reset();
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

//	FUNCTION public
//	Schema::Partition::makeLogData --
//		ログデータを作る
//
//	NOTES
//		引数のログデータには種別が設定されている必要がある
//
//	ARGUMENTS
//		LogData& cLogData_
//			値を設定するログデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Partition::
makeLogData(LogData& cLogData_) const
{
	// 全ログに共通のデータ
	//	1. 名前
	//	2. ID
	//	3. TableID
	cLogData_.addData(packMetaField(Meta::Partition::Name));
	cLogData_.addData(packMetaField(Meta::Partition::ID));
	cLogData_.addData(packMetaField(Meta::Partition::TableID));

	switch (cLogData_.getSubCategory()) {
	case LogData::Category::CreatePartition:
	{
		//	ルール作成
		//		4．Category
		//		5. FunctionName
		//		6. ColumnIDs
		cLogData_.addData(packMetaField(Meta::Partition::Category));
		cLogData_.addData(packMetaField(Meta::Partition::FunctionName));
		cLogData_.addData(packMetaField(Meta::Partition::ColumnIDs));
		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Create::Num);
		break;
	}
	case LogData::Category::DropPartition:
	{
		//	ルール削除
		break;
	}
	case LogData::Category::AlterPartition:
	{
		//	ルール変更
		//		-- 以下は呼び出し側で入れる
		//		4. FunctionName(変更前)
		//		5. ColumnIDs(変更前)
		//		6．FunctionName(変更後)
		//		7．ColumnIDs(変更後)
		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
}

//
//	FUNCTION public
//		Schema::Partition::getObjectID -- ログデータより Partition ID を取得する
//
//	NOTES
//
//	ARGUMENTS
//		const LogData& log_
//			ログテータ
//		ObjectID::Value& id_
//			ID 値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

// static 

Object::ID::Value
Partition::
getObjectID(const LogData& log_)
{
	return log_.getID(Log::ID);
}

//
//	FUNCTION public
//		Schema::Partition::getTableID -- ログデータより TableID を取得する
//
//	NOTES
//
//	ARGUMENTS
//		const LogData& log_
//			ログテータ
//		ObjectID::Value& id_
//			ID 値
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//

// static 

Object::ID::Value
Partition::
getTableID(const LogData& log_)
{
	return log_.getID(Log::TableID);
}

//
//	FUNCTION public
//		Schema::Partition::getName -- ログデータからルール名を得る
//
//	NOTES
//		ルールの変更または抹消のログデータから取得する
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			ルール変更または抹消のログデータ
//
//	RETURN
//		対象のルール名
//
//	EXCEPTIONS
//		Exception::LogItemCorrupted
//			ログデータからルール名が得られなかった

// static
Object::Name
Partition::
getName(const LogData& cLogData_)
{
	// ログデータの内容を取得
	// ★注意★
	//	makeLogDataの実装を変えたらここも変える
	return cLogData_.getString(Log::Name);
}

// FUNCTION public
//	Schema::Partition::getFunctionName -- ログデータからルールの関数名を得る
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	
// RETURN
//	Object::Name
//
// EXCEPTIONS

//static
Object::Name
Partition::
getFunctionName(const LogData& cLogData_)
{
	switch ( cLogData_.getSubCategory() ) {
	case LogData::Category::CreatePartition:
		{
			return cLogData_.getString(Log::Create::FunctionName);
		}
	case LogData::Category::AlterPartition:
		{
			return cLogData_.getString(Log::Alter::PostFunctionName);
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

// FUNCTION private
//	Schema::Partition::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Statement::PartitionDefinition& cStatement_
//	Target& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Partition::
setTarget(Trans::Transaction& cTrans_,
		  const Statement::PartitionDefinition& cStatement_,
		  Target& cTarget_)
{
	; _SYDNEY_ASSERT(cStatement_.getFunction());
	; _SYDNEY_ASSERT(cStatement_.getFunction()->getIdentifier());
	cTarget_.m_cFunctionName = *cStatement_.getFunction()->getIdentifier();

	// check function name
	Database* pDatabase = getDatabase(cTrans_);
	; _SYDNEY_ASSERT(pDatabase);
	Function* pFunction = pDatabase->getFunction(cTarget_.m_cFunctionName, cTrans_);
	if (pFunction == 0) {
		_SYDNEY_THROW2(Exception::StoredFunctionNotFound,
					   cTarget_.m_cFunctionName,
					   pDatabase->getName());
	}

	Table* pTable = getTable(cTrans_);
	; _SYDNEY_ASSERT(pTable);

	Statement::ColumnNameList* pList = cStatement_.getColumnList();
	if (pList) {
		cTarget_.m_vecColumn = getColumnIDs(cTrans_, pTable, *pList);
	} else {
		cTarget_.m_vecColumn.clear();
	}
	if (pFunction->getParameterCount() != cTarget_.m_vecColumn.getSize()) {
		_SYDNEY_THROW0(Exception::InvalidPartition);
	}
}

// FUNCTION private
//	Schema::Partition::setTarget -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Statement::AlterPartitionStatement& cStatement_
//	Target& cTarget_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Partition::
setTarget(Trans::Transaction& cTrans_,
		  const Statement::AlterPartitionStatement& cStatement_,
		  Target& cTarget_)
{
	cTarget_.m_cFunctionName = getFunctionName(cStatement_);

	// check function name
	Database* pDatabase = getDatabase(cTrans_);
	; _SYDNEY_ASSERT(pDatabase);
	Function* pFunction = pDatabase->getFunction(cTarget_.m_cFunctionName, cTrans_);
	if (pFunction == 0) {
		_SYDNEY_THROW2(Exception::StoredFunctionNotFound,
					   cTarget_.m_cFunctionName,
					   pDatabase->getName());
	}

	Table* pTable = getTable(cTrans_);
	; _SYDNEY_ASSERT(pTable);

	Statement::ColumnNameList* pList = cStatement_.getColumnList();
	if (pList) {
		cTarget_.m_vecColumn = getColumnIDs(cTrans_, pTable, *pList);
	} else {
		cTarget_.m_vecColumn.clear();
	}

	if (pFunction->getParameterCount() != cTarget_.m_vecColumn.getSize()) {
		_SYDNEY_THROW0(Exception::InvalidPartition);
	}
}

// FUNCTION public
//	Schema::Partition::getCategory -- カテゴリーを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Partition::Category::Value
//
// EXCEPTIONS

Partition::Category::Value
Partition::
getCategory() const
{
	return m_eCategory;
}

// FUNCTION public
//	Schema::Partition::setCategory -- カテゴリーを設定する
//
// NOTES
//
// ARGUMENTS
//	Category::Value eCategory_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Partition::
setCategory(Category::Value eCategory_)
{
	m_eCategory = eCategory_;
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

// メタデータベースにおける「ルール」表の構造は以下のとおり
// create table Partition (
//		ID		id,
//		name	nvarchar,
//		time	timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Partition>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Partition>(Meta::MemberType::_type_, &Partition::_get_, &Partition::_set_)

	Meta::Definition<Partition> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(Name),			// Name
		_DEFINE2(ID, getTableID, setTableID),		// TableID
		_DEFINE0(Integer),		// Category
		_DEFINE0(String),		// FunctionName
		_DEFINE0(IDArray),		// ColumnIDs
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION protected
//	Schema::Partition::getMetaFieldNumber --
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
Partition::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Partition::MemberNum);
}

//	FUNCTION public
//	Schema::Partition::getMetaFieldDefinition --
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
Partition::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Partition::packMetaField --
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
Partition::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Partition>& cDef = _vecDefinition[iMemberID_];

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
			switch (iMemberID_) {
			case Meta::Partition::Category:
				{
					return pack(static_cast<int>(m_eCategory));
				}
			}
			break;
		}
	case Meta::MemberType::String:
		{
			switch (iMemberID_) {
			case Meta::Partition::FunctionName:
				{
					return pack(m_cTarget.m_cFunctionName);
				}
			}
			break;
		}
	case Meta::MemberType::IDArray:
		{
			switch (iMemberID_) {
			case Meta::Partition::ColumnIDs:
				{
					return pack(m_cTarget.m_vecColumn);
				}
			}
			break;
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Partition::unpackMetaField --
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
Partition::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Partition>& cDef = _vecDefinition[iMemberID_];

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
			int value;
			if (unpack(pData_, value)) {
				switch (iMemberID_) {
				case Meta::Partition::Category:
					{
						if (value >= 0 && value < Category::ValueNum) {
							m_eCategory = static_cast<Category::Value>(value);
							return true;
						}
						break;
					}
				}
			}
			break;
		}
	case Meta::MemberType::String:
		{
			ModUnicodeString value;
			if (unpack(pData_, value)) {
				switch (iMemberID_) {
				case Meta::Partition::FunctionName:
					{
						m_cTarget.m_cFunctionName = value;
						return true;
					}
				}
			}
			break;
		}
	case Meta::MemberType::IDArray:
		{
			ModVector<ID::Value> vecID;
			if (unpack(pData_, vecID)) {
				switch (iMemberID_) {
				case Meta::Partition::ColumnIDs:
					{
						m_cTarget.m_vecColumn = vecID;
						return true;
					}
				}
			}
			break;
		}
	default:
		// これ以外の型はないはず
		break;
	}
	return false;
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
