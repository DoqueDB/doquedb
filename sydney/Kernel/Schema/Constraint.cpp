// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Constraint.cpp -- 制約関連の関数定義
// 
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2015, 2023 Ricoh Company, Ltd.
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

#include "Schema/Constraint.h"
#include "Schema/Database.h"
#include "Schema/Index.h"
#include "Schema/Key.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/NameParts.h"
#include "Schema/Object.h"
#include "Schema/ObjectTemplate.h"
#include "Schema/Parameter.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_Constraint.h"
#include "Schema/Table.h"
#include "Schema/Utility.h"

#include "Exception/ColumnNotFound.h"
#include "Exception/ConstraintAlreadyDefined.h"
#include "Exception/InvalidReference.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "FileCommon/FileOption.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/NullData.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Identifier.h"
#include "Statement/TableConstraintDefinition.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModUnicodeOstrStream.h"
#include "ModUnicodeString.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::Constraint::Constraint --
//		制約を表すクラスのデフォルトコンストラクター
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

Constraint::
Constraint()
	: Object(Object::Category::Constraint),
	  _position(0),
	  _category(Category::Unknown),
	  _clustered(false),
	  m_pHint(0),
	  m_pTable(0),
	  m_vecColumnID(0),
	  m_iIndexID(ID::Invalid),
	  m_iReferedTableID(ID::Invalid),
	  m_iReferedIndexID(ID::Invalid),
	  m_iReferedConstraintID(ID::Invalid),
	  m_vecID(0)
{ }

//	FUNCTION public
//	Schema::Constraint::Constraint --
//		制約定義からの制約を表すクラスのコンストラクター
//
//	NOTES
//		制約を表すクラスを生成するだけで、「制約」表は更新されない
//
//	ARGUMENTS
//		Schema::Table&		table
//			制約が存在する表を表すクラス
//		Schema::Constraint::Position	position
//			表の先頭からの制約の位置
//		Statement::TableConstraintDefinition&	statement
//			解析済の SQL の制約定義
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Constraint::
Constraint(const Table& table, Constraint::Position position,
		   const Statement::TableConstraintDefinition& statement)
	: Object(Object::Category::Constraint, table.getScope(), table.getStatus(),
			 ID::Invalid, table.getID(), table.getDatabaseID()),
	  _position(position),
	  _category(Category::Unknown),
	  _clustered(false),
	  m_pHint(0),
	  m_pTable(const_cast<Table*>(&table)),
	  m_vecColumnID(0),
	  m_iIndexID(ID::Invalid),
	  m_iReferedTableID(ID::Invalid),
	  m_iReferedIndexID(ID::Invalid),
	  m_iReferedConstraintID(ID::Invalid),
	  m_vecID(0)
{
	// 制約定義から制約の種類を得て、処理する

	switch (statement.getConstraintType()) {
	case Statement::TableConstraintDefinition::None:
	{
		// 何もしない
		break;
	}
	case Statement::TableConstraintDefinition::PrimaryKey:
	{
		_category = Category::PrimaryKey;
		_clustered = statement.getClustered();

		setName(createName(table.getName()));
		break;
	}
	case Statement::TableConstraintDefinition::Unique:
	{
		_category = Category::Unique;

		// ユニークキーあらわす文字列に列名を追加して制約の名前とする
		Name name(_TRMEISTER_U_STRING(NameParts::Constraint::Unique));

		; _SYDNEY_ASSERT(statement.getColumnNameList());
		; _SYDNEY_ASSERT(statement.getColumnNameList()->getCount());

		int n = statement.getColumnNameList()->getCount();
		for (int i = 0; i < n; i++) {
			Statement::ColumnName* pName =
				statement.getColumnNameList()->getColumnNameAt(i);
			; _SYDNEY_ASSERT(pName);
			; _SYDNEY_ASSERT(pName->getIdentifierString());

			// 名前を追加する
			name.append('_');
			name.append(*pName->getIdentifierString());
		}

		setName(name);

		break;
	}
	case Statement::TableConstraintDefinition::ForeignKey:
	{
		_category = Category::ForeignKey;
		// 外部キーを表す文字列に列名を追加して制約の名前とする
		Name name(_TRMEISTER_U_STRING(NameParts::Constraint::ForeignKey));

		; _SYDNEY_ASSERT(statement.getColumnNameList());
		; _SYDNEY_ASSERT(statement.getColumnNameList()->getCount());

		int n = statement.getColumnNameList()->getCount();
		for (int i = 0; i < n; i++) {
			Statement::ColumnName* pName =
				statement.getColumnNameList()->getColumnNameAt(i);
			; _SYDNEY_ASSERT(pName);
			; _SYDNEY_ASSERT(pName->getIdentifierString());

			// 名前を追加する
			name.append('_');
			name.append(*pName->getIdentifierString());
		}
		setName(name);
		break;
	}
	default:
	{
		; _SYDNEY_ASSERT(false);
		break;
	}
	}
}

// FUNCTION public
//	Schema::Constraint::Constraint -- 外部キーの参照索引を用いたコンストラクター
//
// NOTES
//
// ARGUMENTS
//	const Table& cTable_
//	const Index& cIndex_
//	const Index& cReferedIndex_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Constraint::
Constraint(const Table& cTable_, const Index& cIndex_, const Index& cReferedIndex_)
	: Object(Object::Category::Constraint, cTable_.getScope(), cTable_.getStatus(),
			 ID::Invalid, cTable_.getID(), cTable_.getDatabaseID()),
	  _position(0),
	  _category(Category::ReferedKey),
	  _clustered(false),
	  m_pHint(0),
	  m_pTable(const_cast<Table*>(&cTable_)),
	  m_vecColumnID(0),
	  m_iIndexID(cIndex_.getID()),
	  m_iReferedTableID(cReferedIndex_.getTableID()),
	  m_iReferedIndexID(cReferedIndex_.getID()),
	  m_iReferedConstraintID(ID::Invalid),
	  m_vecID(0)
{
	// 被参照キーを表す文字列に表IDと索引の名前を追加して制約の名前とする
	ModUnicodeOstrStream stream;
	stream << _TRMEISTER_U_STRING(NameParts::Constraint::ReferedKey)
		   << '_' << m_iReferedTableID
		   << '_' << cReferedIndex_.getName();

	setName(stream.getString());
}

//	FUNCTION public
//	Schema::Constraint::Constraint --
//		ログデータからの制約を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table&		table
//			制約が存在する表を表すクラス
//		Schema::Constraint::Position	position
//			表の先頭からの制約の位置
//		const Common::DataArrayData& cLogData_
//			制約定義のログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Constraint::
Constraint(const Table& table, Constraint::Position position,
		   const Common::DataArrayData& cLogData_, Trans::Transaction& cTrans_)
	: Object(Object::Category::Constraint, table.getScope(), table.getStatus(),
			 ID::Invalid, table.getID(), table.getDatabaseID()),
	  _position(position),
	  _category(Category::Unknown),
	  _clustered(false),
	  m_pHint(0),
	  m_pTable(const_cast<Table*>(&table)),
	  m_vecColumnID(0),
	  m_iIndexID(ID::Invalid),
	  m_iReferedTableID(ID::Invalid),
	  m_iReferedIndexID(ID::Invalid),
	  m_iReferedConstraintID(ID::Invalid),
	  m_vecID(0)
{
	// ログの内容を反映する
	// ★注意★
	// makeLogDataの実装が変わったらここも変える
	int i = ((cLogData_.getCount() >= Log::ValueNum0) ? 1 : 0); // IDもログに含んでいたらここではスキップする
	if (!unpackMetaField(cLogData_.getElement(i++).get(), Meta::Constraint::Name)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Constraint::Category)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Constraint::ColumnIDs)
		|| !unpackHint(cLogData_.getElement(i++).get())) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

//	FUNCTION public
//	Schema::Constraint::~Constraint -- 制約を表すクラスのデストラクター
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

Constraint::
~Constraint()
{
	destruct();
}

//	FUNCTION public
//		Schema::Constraint::getNewInstance -- オブジェクトを新たに取得する
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
Constraint*
Constraint::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Constraint> pObject = new Constraint;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION private
//	Schema::Constraint::destruct -- 制約を表すクラスのデストラクター下位関数
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
Constraint::
destruct()
{
	clearHint();
	clearColumnID();
}

//	FUNCTION public
//	Schema::Constraint::create -- 制約のスキーマ定義を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Table& table
//			制約が属する表
//		Position position
//			制約の定義中の順序
//	   const Statement::TableConstraintDefinition& statement
//			制約定義のSQL構文要素
//	   Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成した制約オブジェクト
//
//	EXCEPTIONS

// static
Constraint::Pointer
Constraint::
create(Table& table, Position position,
	   const Statement::TableConstraintDefinition& statement,
	   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 指定されたデータにより新たな制約を表すクラスを生成する
	Pointer pConstraint = new Constraint(table, position, statement);

	// 同じ名前の制約がないか調べる
	if (table.getConstraint(pConstraint->getName(), cTrans_))
		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated constraint definition("
						   << pConstraint->getName()
						   << ")	canceled"
						   << ModEndl;
			return Pointer();
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated constraint definition("
						   << pConstraint->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::ConstraintAlreadyDefined, pConstraint->getName(), table.getName());
		}

	// 列名リストから列のIDを得る
	{
		; _SYDNEY_ASSERT(statement.getColumnNameList());
		; _SYDNEY_ASSERT(statement.getColumnNameList()->getCount());

		int n = statement.getColumnNameList()->getCount();

		pConstraint->resetColumnID();
		; _SYDNEY_ASSERT(pConstraint->m_vecColumnID);
		pConstraint->m_vecColumnID->reserve(n);

		for (int i = 0; i < n; i++) {
			Statement::ColumnName* pName =
				statement.getColumnNameList()->getColumnNameAt(i);
			; _SYDNEY_ASSERT(pName);
			; _SYDNEY_ASSERT(pName->getIdentifierString());

			// 名前で列を探す
			Name columnName(*pName->getIdentifierString());
			Column* column = table.getColumn(columnName, cTrans_);

			if (!column) {
				// 列が見つからなかったので例外送出

				SydInfoMessage
					<< "Constraint argument column not found: "
					<< table.getName() << "." << columnName
					<< ModEndl;
				_SYDNEY_THROW1(Exception::ColumnNotFound, columnName);
			}
			pConstraint->m_vecColumnID->pushBack(column->getID());
		}
	}

	if (pConstraint->getCategory() == Category::ForeignKey) {
		Database* pDatabase = table.getDatabase(cTrans_);
		// get the refered table
		; _SYDNEY_ASSERT(statement.getReferedTableName());
		; _SYDNEY_ASSERT(statement.getReferedTableName()->getIdentifier());

		Name cName(*statement.getReferedTableName()->getIdentifier());

		Table* pReferedTable = pDatabase->getTable(cName, cTrans_);
		if (!pReferedTable) {
			SydInfoMessage << "Constraint definition failed. Refered table \""
						   << cName
						   << "\" not found."
						   << ModEndl;
			_SYDNEY_THROW2(Exception::TableNotFound, cName, pDatabase->getName());
		}
		// Get the index ID corresponding to a foreign key
		ID::Value iReferedIndexID = pConstraint->getReferedIndexID(cTrans_, *pReferedTable, statement);

		// set the referenced objects' ID
		pConstraint->m_iReferedTableID = pReferedTable->getID();
		pConstraint->m_iReferedIndexID = iReferedIndexID;
	}

	// ヒント
	ModAutoPointer<Hint> pHint;
	if (Statement::Hint* hint = statement.getHint())
		pConstraint->setHint(new Hint(*hint));

	// IDをふり、状態を変える
	pConstraint->Object::create(cTrans_);

	// create corresponding index
	pConstraint->createIndex(cTrans_, table);

	return pConstraint;
}

// FUNCTION public
//	Schema::Constraint::create -- 制約のスキーマ定義を生成する(被参照キー)
//
// NOTES
//
// ARGUMENTS
//	Table& cTable_
//	const Index& cIndex_
//	const Index& cReferedIndex_
//	Trans::Transaction& cTrans_
//	ID::Value iID_ = ID::Invalid
//	
// RETURN
//	Constraint::Pointer
//
// EXCEPTIONS

//static
Constraint::Pointer
Constraint::
create(Table& cTable_, const Index& cIndex_, const Index& cReferedIndex_,
	   Trans::Transaction& cTrans_,
	   ID::Value iID_ /* = ID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 指定されたデータにより新たな制約を表すクラスを生成する
	Pointer pConstraint = new Constraint(cTable_, cIndex_, cReferedIndex_);

	// 同じ名前の制約がないか調べる
	if (cTable_.getConstraint(pConstraint->getName(), cTrans_))
		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated constraint definition("
						   << pConstraint->getName()
						   << ")	canceled"
						   << ModEndl;
			return Pointer();
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated constraint definition("
						   << pConstraint->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::ConstraintAlreadyDefined, pConstraint->getName(), cTable_.getName());
		}

	// 索引のキーになっているすべての列を得る
	ModVector<Column*> vecKeyColumns = cIndex_.getColumn(cTrans_);
	int n = vecKeyColumns.getSize();

	pConstraint->resetColumnID();
	; _SYDNEY_ASSERT(pConstraint->m_vecColumnID);
	pConstraint->m_vecColumnID->reserve(n);

	for (int i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(vecKeyColumns[i]->getParentID() == cTable_.getID());
		pConstraint->m_vecColumnID->pushBack(vecKeyColumns[i]->getID());
	}

	// IDをふり、状態を変える
	pConstraint->Object::create(cTrans_, iID_);

	// do not create corresponding index
	//pConstraint->createIndex(cTrans_, table);

	return pConstraint;
}

//	FUNCTION public
//	Schema::Constraint::create -- 制約のスキーマ定義を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Table& table
//			制約が属する表
//		Position position
//			制約の定義中の順序
//		const Statement::TableConstraintDefinition& statement
//			制約定義のSQL構文要素
//	  	Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成した制約オブジェクト
//
//	EXCEPTIONS

// static
Constraint::Pointer
Constraint::
create(Table& table, Position position,
	   const Common::DataArrayData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 新たな制約を表すクラスを生成する
	Pointer pConstraint = new Constraint(table, position, cLogData_, cTrans_);

	// 表のRENAMEがUNDOされている場合、表のIDに対応して登録されている最終的な名前を使う
	Database* pDatabase = table.getDatabase(cTrans_);
	pConstraint->checkUndo(*pDatabase, table);

	// ログからIDを得る
	ID::Value iID = getObjectID(cLogData_);

	// IDをふり、状態を変える
	pConstraint->Object::create(cTrans_, iID);

	// create corresponding index
	const Common::DataArrayData* pLogIndex = 0;
	if (cLogData_.getCount() >= Log::ValueNum1) {
		pLogIndex = &(LogData::getDataArrayData(cLogData_.getElement(Log::IndexDefinition)));
	}
	pConstraint->createIndex(cTrans_, table, pLogIndex);

	return pConstraint;
}

// FUNCTION public
//	Schema::Constraint::create -- create corresponding files
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
create(Trans::Transaction& cTrans_)
{
	Table* pTable = getTable(cTrans_);
	; _SYDNEY_ASSERT(pTable);
	Index* pIndex = pTable->getIndex(getIndexID(), cTrans_);
	; _SYDNEY_ASSERT(pIndex);

	File* pFile = pIndex->getFile(cTrans_);
	; _SYDNEY_ASSERT(pFile);

	// create file
	pFile->create(cTrans_);
}

// FUNCTION public
//	Schema::Constraint::drop -- 制約のスキーマ定義を破棄する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	bool bRecovery_ = false
//	bool bNoUnset_ = false
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
drop(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */, bool bNoUnset_ /* = false */)
{
	// If the constraint is foreign key, drop refered key constraint too
	if (getCategory() == Category::ForeignKey) {
		Table* pReferedTable = Table::get(getReferedTableID(), getDatabase(cTrans_), cTrans_);
		; _SYDNEY_ASSERT(bRecovery_ || pReferedTable);
		if (pReferedTable) {
			// search  for refered key constraint
			Constraint* pReferedConstraint = pReferedTable->getConstraint(getReferedConstraintID(), cTrans_);
			; _SYDNEY_ASSERT(bRecovery_ || pReferedConstraint);
			if (pReferedConstraint) {
				pReferedConstraint->drop(cTrans_, bRecovery_, bNoUnset_);
				pReferedTable->touch();
			}
		}
	}
	// call superclass implementation
	Object::drop(bRecovery_, bNoUnset_);
}

//	FUNCTION public
//	Schema::Constraint::get --
//		あるスキーマオブジェクト ID の制約を表すクラスを得る
//
//	NOTES
//		マウント中のデータベースに対してこの関数を使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			制約のスキーマオブジェクト ID
//		Schema::Database* pDatabase_
//			制約が属するデータベースのオブジェクト
//			値が0ならすべてのデータベースについて調べる
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//	
//	RETURN
//		0 以外の値
//			得られた制約を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の制約は存在しない
//
//	EXCEPTIONS

// static
Constraint*
Constraint::
get(ID::Value id_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	return ObjectTemplate::get<Constraint, SystemTable::Constraint, Object::Category::Constraint>(id_, pDatabase_, cTrans_);
}

//	FUNCTION public
//	Schema::Constraint::get --
//		あるスキーマオブジェクト ID の制約を表すクラスを得る
//
//	NOTES
//		マウント中のデータベースに対してこの関数を使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			制約のスキーマオブジェクト ID
//		Schema::Object::ID::Value	iDatabaseID_
//			制約が属するデータベースのオブジェクトID
//			値がID::Invalidならすべてのデータベースについて調べる
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//	
//	RETURN
//		0 以外の値
//			得られた制約を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の制約は存在しない
//
//	EXCEPTIONS

// static
Constraint*
Constraint::
get(ID::Value id_, ID::Value iDatabaseID_, Trans::Transaction& cTrans_)
{
	if (id_ == Object::ID::Invalid)
		return 0;

	return get(id_, Database::get(iDatabaseID_, cTrans_), cTrans_);
}

// FUNCTION public
//	Schema::Constraint::isPrimaryKey -- primary key constraint?
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
Constraint::
isPrimaryKey() const
{
	return (getCategory() == Category::OldPrimaryKey)
		|| (getCategory() == Category::PrimaryKey);
}

// FUNCTION public
//	Schema::Constraint::isUnique -- unique constraint?
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
Constraint::
isUnique() const
{
	return (getCategory() == Category::OldUnique)
		|| (getCategory() == Category::Unique);
}

// FUNCTION public
//	Schema::Constraint::isForeignKey -- foreign key constraint?
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
Constraint::
isForeignKey() const
{
	return getCategory() == Category::ForeignKey;
}

// FUNCTION public
//	Schema::Constraint::isReferedKey -- implicit constraint for foreign key?
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
Constraint::
isReferedKey() const
{
	return getCategory() == Category::ReferedKey;
}

// FUNCTION public
//	Schema::Constraint::isReferenceConstraint -- foreign key or refered key?
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
Constraint::
isReferenceConstraint() const
{
	return (getCategory() == Category::ForeignKey)
		|| (getCategory() == Category::ReferedKey);
}

//	FUNCTION public
//	Schema::Constraint::isValid -- 陳腐化していないか
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			陳腐化していないかを調べるスキーマオブジェクトID
//		Schema::Object::ID::Value iDatabaseID_
//			このスキーマオブジェクトが属しているデータベースのID
//		Schema::Object::Timestamp iTimestamp_
//			正しいスキーマオブジェクトの値とこの値が異なっていたら
//			陳腐化していると判断する
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
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
Constraint::
isValid(ID::Value iID_, ID::Value iDatabaseID_, Timestamp iTimestamp_,
		Trans::Transaction& cTrans_)
{
	Constraint* pConstraint = get(iID_, iDatabaseID_, cTrans_);

	return (pConstraint && pConstraint->getTimestamp() == iTimestamp_);
}

//	FUNCTION public
//	Schema::Constraint::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Constraint::Pointer& pConstraint_
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
Constraint::
doBeforePersist(const Pointer& pConstraint_, Status::Value eStatus_,
			   bool bNeedToErase_, Trans::Transaction& cTrans_)
{
	// 何もしない
	;
}

//	FUNCTION public
//	Schema::Constraint::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Constraint::Pointer& pConstraint_
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
Constraint::
doAfterPersist(const Pointer& pConstraint_, Status::Value eStatus_,
			   bool bNeedToErase_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pConstraint_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	ID::Value dbID = pConstraint_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::DeleteCanceled:
	{
		// データベースのキャッシュに登録する

		if (Database* pDatabase = pConstraint_->getDatabase(cTrans_))
			pDatabase->addCache(pConstraint_);
		break;
	}
	case Status::CreateCanceled:
	{
		// 表の登録から抹消する
		Table* pTable = pConstraint_->getTable(cTrans_);
		; _SYDNEY_ASSERT(pTable);

		pTable->eraseConstraint(pConstraint_->getID());
		break;
	}
	case Status::Changed:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらキャッシュや表の登録からの削除も行う

		// 状態を「実際に削除された」にする

		pConstraint_->setStatus(Status::ReallyDeleted);

		if (bNeedToErase_) {
			Database* pDatabase = pConstraint_->getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			// 下位オブジェクトがあればそれを抹消してからdeleteする
			pConstraint_->reset(*pDatabase);

			// キャッシュから抹消する
			// NeedToErase==falseのときは親オブジェクトのdeleteの中で
			// キャッシュから抹消される
			pDatabase->eraseCache(pConstraint_->getID());

			// 表の登録から抹消する → deleteされる

			Table* pTable = pConstraint_->getTable(cTrans_);
			; _SYDNEY_ASSERT(pTable);
			pTable->eraseConstraint(pConstraint_->getID());
		}
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::Constraint);
}

//	FUNCTION public
//	Schema::Constraint::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::ConstraintPointer& pConstraint_
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
Constraint::
doAfterLoad(const Pointer& pConstraint_, Table& cTable_, Trans::Transaction& cTrans_)
{
	// UNDO情報に最終的な表の名前が登録されているときはそれに置き換える
	pConstraint_->checkUndo(*cTable_.getDatabase(cTrans_), cTable_);

	// 表へ読み出した制約を表すクラスを追加する
	// また、データベースにこの制約を表すクラスを
	// スキーマオブジェクトとして管理させる

	cTable_.getDatabase(cTrans_)->addCache(cTable_.addConstraint(pConstraint_, cTrans_));
}
// object method version (for apply function)
void
Constraint::
doAfterLoad(Table& cTable_, Trans::Transaction& cTrans_)
{
	doAfterLoad(Pointer(this), cTable_, cTrans_);
}

//	FUNCTION public
//	Schema::Constraint::reset --
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
Constraint::
reset(Database& cDatabase_)
{}

//	FUNCTION public
//	Schema::Constraint::getPosition -- 制約の表の先頭からの位置を得る
//
//	NOTES
//		表の先頭からなん番目にその制約が定義されているかを、制約の位置とする
//		ただし、定義されている列は数に入れない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた制約の位置
//
//	EXCEPTIONS
//		なし

Constraint::Position
Constraint::
getPosition() const
{
	return _position;
}

//	FUNCTION public
//	Schema::Constraint::isClustered -- clustered指定の有無を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた有無
//
//	EXCEPTIONS
//		なし

bool
Constraint::
isClustered() const
{
	return _clustered;
}

//	FUNCTION public
//	Schema::Constraint::getCategory -- 制約の種類を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた制約の種類
//
//	EXCEPTIONS
//		なし

Constraint::Category::Value
Constraint::
getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Schema::Constraint::getTableID --
//		制約が存在する表のオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid以外
//			制約が存在する表のオブジェクトID
//		Schema::Object::ID::Invalid
//			制約が存在する表は存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Constraint::
getTableID() const
{
	return getParentID();
}

// FUNCTION public
//	Schema::Constraint::getIndexID -- 制約に対応して作成された索引のIDを得る
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object::ID::Value
//
// EXCEPTIONS

Object::ID::Value
Constraint::
getIndexID() const
{
	return m_iIndexID;
}

// FUNCTION public
//	Schema::Constraint::getIndex -- 制約に対応して作成された索引を得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Index*
//
// EXCEPTIONS

Index*
Constraint::
getIndex(Trans::Transaction& cTrans_) const
{
	Table* pTable = getTable(cTrans_);
	if (pTable) {
		return pTable->getIndex(getIndexID(), cTrans_);
	}
	return 0;
}

// FUNCTION public
//	Schema::Constraint::setIndexID -- 制約に対応して作成された索引のIDをセットする
//
// NOTES
//
// ARGUMENTS
//	ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
setIndexID(ID::Value iID_)
{
	m_iIndexID = iID_;
}

// FUNCTION public
//	Schema::Constraint::getReferedTableID -- 外部キー制約の参照する表IDを得る
//
// NOTES
//	外部キーでない制約に対してはID::Invalidを返す
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object::ID::Value
//
// EXCEPTIONS

Object::ID::Value
Constraint::
getReferedTableID() const
{
	return m_iReferedTableID;
}

// FUNCTION public
//	Schema::Constraint::getReferedIndexID -- 外部キー制約の参照する索引IDを得る
//
// NOTES
//	外部キーでない制約に対してはID::Invalidを返す
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object::ID::Value
//
// EXCEPTIONS

Object::ID::Value
Constraint::
getReferedIndexID() const
{
	return m_iReferedIndexID;
}

// FUNCTION public
//	Schema::Constraint::getReferedConstraintID -- 外部キー制約と対となる制約のIDを得る
//
// NOTES
//	外部キーでない制約に対してはID::Invalidを返す
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Object::ID::Value
//
// EXCEPTIONS

Object::ID::Value
Constraint::
getReferedConstraintID() const
{
	return m_iReferedConstraintID;
}

// FUNCTION public
//	Schema::Constraint::setReferedConstraintID -- 外部キー制約と対となる制約のIDをセットする
//
// NOTES
//
// ARGUMENTS
//	ID::Value iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
setReferedConstraintID(ID::Value iID_)
{
	m_iReferedConstraintID = iID_;
}

// FUNCTION public
//	Schema::Constraint::createName -- 制約の名前を生成する
//
// NOTES
//
// ARGUMENTS
//	const Name& cParentName_
//	
// RETURN
//	Object::Name
//
// EXCEPTIONS

Object::Name
Constraint::
createName(const Name& cParentName_)
{
	switch (_category) {
	case Category::OldPrimaryKey:
	case Category::PrimaryKey:
		{
			// 表名に主キーをあらわす部分を追加して制約の名前とする
			return Name(cParentName_).append(_TRMEISTER_U_STRING(NameParts::Constraint::PrimaryKey));
		}
	default:
		{
			break;
		}
	}
	// PrimaryKey以外に対してこの関数を呼んではいけない
	; _SYDNEY_ASSERT(false);
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Schema::Constraint::rename -- 名前を変更する
//
// NOTES
//
// ARGUMENTS
//	const Name& cName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
rename(const Name& cName_)
{
	setName(cName_);
}

//	FUNCTION public
//	Schema::Constraint::getColumnID -- 制約の列ID配列を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた制約の列ID配列
//
//	EXCEPTIONS
//		なし

const ModVector<Object::ID::Value>&
Constraint::
getColumnID() const
{
	if (!m_vecColumnID)
		const_cast<Constraint*>(this)->resetColumnID();
	; _SYDNEY_ASSERT(m_vecColumnID);

	return *m_vecColumnID;
}

//	FUNCTION public
//	Schema::Constraint::setColumnID -- 制約の列ID配列を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<Schema::Object::ID::Value>& vecID_
//			設定する制約の列ID配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Constraint::
setColumnID(const ModVector<ID::Value>& vecID_)
{
	if (!m_vecColumnID)
		const_cast<Constraint*>(this)->resetColumnID();
	; _SYDNEY_ASSERT(m_vecColumnID);

	*m_vecColumnID = vecID_;
}

//	FUNCTION public
//	Schema::Constraint::resetColumnID -- 列ID配列を初期化する
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
Constraint::
resetColumnID()
{
	if (!m_vecColumnID)
		m_vecColumnID = new ModVector<ID::Value>();
	else
		m_vecColumnID->clear();
}

//	FUNCTION public
//	Schema::Constraint::clearColumnID -- 列ID配列を破棄する
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
Constraint::
clearColumnID()
{
	delete m_vecColumnID, m_vecColumnID = 0;
	delete m_vecID, m_vecID = 0;
}

//	FUNCTION public
//	Schema::Constraint::getTable -- 制約が存在する表を表すクラスを得る
//
//	NOTES
//		生成前、中の制約や、排他制御がうまく行われていない場合を除けば、
//		制約が存在する表は必ず存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			制約が存在する表は存在しない
//
//	EXCEPTIONS

Table*
Constraint::getTable(Trans::Transaction& cTrans_) const
{
	return (!m_pTable) ?
		m_pTable = Table::get(getParentID(), getDatabase(cTrans_), cTrans_, true /* internal */)
		: m_pTable;
}

//	FUNCTION public
//	Schema::Constraint::serialize -- 
//		制約を表すクラスのシリアライザー
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
Constraint::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// 表の先頭から何番目か
		archiver << _position;

		// 制約の種別
		{
		int tmp = _category;
		archiver << tmp;
		}
		// clustered
		archiver << _clustered;

		// 列ID配列
		ModSize n = (m_vecColumnID) ? m_vecColumnID->getSize() : 0;
		archiver << n;
		for (ModSize i = 0; i < n; i++) {
			archiver << m_vecColumnID->at(i);
		}

		if ((_category == Category::ForeignKey)
			||
			(_category == Category::ReferedKey)) {
			// 対応する索引のID
			archiver << m_iIndexID;
			// 外部キーが参照する表と索引のID
			archiver << m_iReferedTableID;
			archiver << m_iReferedIndexID;
			// 外部キーと対となる制約のID
			archiver << m_iReferedConstraintID;
		}

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// 表の先頭から何番目か
		archiver >> _position;

		// 制約の種類
		{
		int tmp;
		archiver >> tmp;
		_category = static_cast<Category::Value>(tmp);
		}
		// clustered
		archiver >> _clustered;

		// 列ID配列
		{
		ModSize n;
		archiver >> n;
		if (n) {
			resetColumnID();
			m_vecColumnID->reserve(n);
			for (ModSize i = 0; i < n; i++) {
				ID::Value id;
				archiver >> id;
				m_vecColumnID->pushBack(id);
			}
		}
		}

		if ((_category == Category::ForeignKey)
			||
			(_category == Category::ReferedKey)) {
			// 対応する索引のID
			archiver >> m_iIndexID;
			// 外部キーが参照する表と索引のID
			archiver >> m_iReferedTableID;
			archiver >> m_iReferedIndexID;
			// 外部キーと対となる制約のID
			archiver >> m_iReferedConstraintID;
		}
	}
}

//	FUNCTION public
//	Schema::Constraint::getClassID -- このクラスのクラス ID を得る
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
Constraint::
getClassID() const
{
	return Externalizable::Category::Constraint +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Constraint::clear -- 制約を表すクラスのメンバーをすべて初期化する
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
Constraint::
clear()
{
	_position = 0;
	_category = Category::Unknown;
	_clustered = false;
	m_pTable = 0;
	destruct();
}

//	FUNCTION public
//	Schema::Constraint::makeLogData --
//		ログデータを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		Common::DataArrayData& cLogData_
//			値を設定するログデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Constraint::
makeLogData(Trans::Transaction& cTrans_, Common::DataArrayData& cLogData_) const
{
	// 制約のログデータ
	//	1．ID(v16.1以降)
	//	2．名前
	//	3．種別
	//	4．キーID配列
	//	5．ヒント(ログ専用)
	//	6. 索引定義
	cLogData_.reserve(Log::ValueNum); // 以下のコードが変わったらこの数値も変える
	cLogData_.pushBack(packMetaField(Meta::Constraint::ID));
	cLogData_.pushBack(packMetaField(Meta::Constraint::Name));
	cLogData_.pushBack(packMetaField(Meta::Constraint::Category));
	cLogData_.pushBack(packMetaField(Meta::Constraint::ColumnIDs));
	cLogData_.pushBack(packHint());
	if (Index* pIndex = getIndex(cTrans_)) {
		ModAutoPointer<Common::DataArrayData> pIndexLogData = new Common::DataArrayData;
		pIndex->makeLogData(cTrans_, *pIndexLogData);
		cLogData_.pushBack(pIndexLogData.release());
	}
}

// FUNCTION public
//	Schema::Constraint::getObjectID -- ログに記録されているIDを得る
//
// NOTES
//	makeLogDataを変更したv16.1以降でのみ正しい値が返る
//
// ARGUMENTS
//	const Common::DataArrayData& cLogData_
//	
// RETURN
//	Constraint::ID::Value
//
// EXCEPTIONS

//static
Constraint::ID::Value
Constraint::
getObjectID(const Common::DataArrayData& cLogData_)
{
	// ★注意★
	// makeLogDataの実装が変わったらここも変える
	if (cLogData_.getCount() >= Log::ValueNum0) {
		return LogData::getID(cLogData_.getElement(0));
	}
	return ID::Invalid;
}

// FUNCTION private
// packHint -- ヒントをログ用データに変換する
//
// NOTES
//	対応するIndexを作成してしまえば不要なので永続化の必要はない。

Common::Data::Pointer
Constraint::
packHint() const
{
	Utility::BinaryData& cArchiver = getArchiver();
	return cArchiver.put(getHint());
}

// FUNCTION private
// unpackHint -- ログ用データからヒントを取得する
//
// NOTES

bool
Constraint::
unpackHint(const Common::Data* pData_)
{
	if (pData_ && pData_->isNull()) {
		return true;

	} else if (pData_ && pData_->getType() == Common::DataType::Binary) {
		const Common::BinaryData* pBinary =
			_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);

		Utility::BinaryData& cArchiver = getArchiver();
		ModAutoPointer<Hint> pData =
			dynamic_cast<Hint*>(cArchiver.get(pBinary));
		if (pData.get())
			setHint(pData.release());
		return true;
	}
	return false;
}

//	FUNCTION public
//	Schema::Constraint::getHint -- 制約に指定されるヒントを得る
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
Constraint::
getHint() const
{
	return m_pHint;
}

//	FUNCTION public
//	Schema::Constraint::setHint -- 制約に指定されるヒントを設定する
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
Constraint::
setHint(Hint* pHint_)
{
	m_pHint = pHint_;
}

//	FUNCTION public
//	Schema::Constraint::clearHint --
//		制約を表すクラスに登録されているヒントを表すクラスを破棄する
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
Constraint::
clearHint()
{
	if (m_pHint)
		delete m_pHint, m_pHint = 0;
}

// FUNCTION public
//	Schema::Constraint::checkUndo -- Undo情報を検査して反映する
//
// NOTES
//
// ARGUMENTS
//	const Database& cDatabase_
//	const Table& cTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
checkUndo(const Database& cDatabase_, const Table& cTable_)
{
	using namespace Manager::RecoveryUtility;
	if (isPrimaryKey()) {
		// If parent table object has been undone for renaming,
		// the name of the table might be changed.
		if (Undo::isEntered(cDatabase_.getName(), cTable_.getID(), Undo::Type::RenameTable)) {
			// set name again
			setName(createName(cTable_.getName()));
		}
	}
}

// FUNCTION private
//	Schema::Constraint::createIndex -- create corresponding index
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	const Common::DataArrayData* pLogData_ /* = 0 */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
createIndex(Trans::Transaction& cTrans_, Table& cTable_,
			const Common::DataArrayData* pLogData_ /* = 0 */)
{
	Index::Pointer pIndex = (pLogData_)
		? Index::create(cTrans_, cTable_, *this, *pLogData_)
		: Index::create(cTrans_, cTable_, *this);
	; _SYDNEY_ASSERT(pIndex.get());

	// Status is 'created' or 'mounted'
	; _SYDNEY_ASSERT(pIndex->getStatus() == Status::Created
					 || pIndex->getStatus() == Status::Mounted);

	// Add the index to the table
	// [NOTES] adding to cache will be done after persisting
	cTable_.addIndex(pIndex, cTrans_);

	// set corresponding index ID for foreign key constraint
	setIndexID(pIndex->getID());

	// if constraint is foreign key, reverse constraint has to be created
	if (getCategory() == Category::ForeignKey) {
		Table* pForeignTable = Table::get(getReferedTableID(),
										  getDatabase(cTrans_),
										  cTrans_);
		; _SYDNEY_ASSERT(pForeignTable);
		Index* pForeignIndex =
			pForeignTable->getIndex(getReferedIndexID(), cTrans_);

		// get foreign constraint id from object
		ID::Value iForeignConstraintID = (pLogData_) ? getReferedConstraintID() : ID::Invalid;

		// create referedkey constraint using the index corresponds to the foreign key
		const Constraint::Pointer& pForeignConstraint =
			pForeignTable->addConstraint(Constraint::create(*pForeignTable, *pForeignIndex,
															*pIndex, cTrans_,
															iForeignConstraintID),
										 cTrans_);
		pForeignTable->touch();

		// set refering constraint ID
		setReferedConstraintID(pForeignConstraint->getID());
	}
}

// FUNCTION public
//	Schema::Constraint::checkForeignKey -- check an index whether it corresponds to a foreign key
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Table& cTable_
//	const Constraint& cConstraint_
//	const ModVector<ID::Value>& vecReferedColumnID_
//	
// RETURN
//	ID::Value
//		Invalid     ... not found
//		not Invalid ... index ID corresponding to the foreign key
//
// EXCEPTIONS

Object::ID::Value
Constraint::
checkForeignKey(Trans::Transaction& cTrans_,
				const Table& cTable_, const Constraint& cConstraint_,
				const ModVector<ID::Value>& vecReferedColumnID_)
{
	const Index* pIndex = cTable_.getIndex(cConstraint_.getName(), cTrans_);
	if (pIndex) {
		const ModVector<Key*>& vecKey = pIndex->getKey(cTrans_);
		ModSize n = vecKey.getSize();
		if (n == vecReferedColumnID_.getSize()) {
			// check only the number of keys are equal to that of referenced columns
			ModSize i = 0;
			for (; i < n; ++i) {
				if (vecKey[i]->getColumnID() != vecReferedColumnID_[i]) {
					break;
				}
			}
			if (i == n) {
				// Found the result
				return pIndex->getID();
			}
		}
	}
	return ID::Invalid;
}

// FUNCTION public
//	Schema::Constraint::getReferedIndexID -- get the index id to which foreign key refers
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Table& cReferedTable_
//	const Statement::TableConstraintDefinition& cStatement_
//	
// RETURN
//	ID::Value
//
// EXCEPTIONS

Object::ID::Value
Constraint::
getReferedIndexID(Trans::Transaction& cTrans_,
				  const Table& cReferedTable_,
				  const Statement::TableConstraintDefinition& cStatement_)
{
	// get the refered index
	if (cStatement_.getReferedColumnName()) {
		// refered columns are specified -> search for corresponding constraint
		int n = cStatement_.getReferedColumnName()->getCount();
		if (!m_vecColumnID
			|| (static_cast<ModSize>(n) != m_vecColumnID->getSize())) {
			// rerefing columns do not match to the refered columns
			SydInfoMessage << "The number of referenced columns ("
						   << n
						   << ") have to be equal to the number of refering columns."
						   << ModEndl;
			_SYDNEY_THROW0(Exception::InvalidReference);
		}
		ModVector<ID::Value> vecReferedColumnID;
		vecReferedColumnID.reserve(n);
		for (int i = 0; i < n; ++i) {
			Name cName(*cStatement_.getReferedColumnName()->getColumnNameAt(i)->getIdentifierString());
			Column* pReferedColumn = cReferedTable_.getColumn(cName, cTrans_);
			if (!pReferedColumn) {
				SydInfoMessage << "Referenced column \""
							   << cReferedTable_.getName() << '.' << cName
							   << "\" is not found";
				_SYDNEY_THROW1(Exception::ColumnNotFound, cName);
			}
			vecReferedColumnID.pushBack(pReferedColumn->getID());
		}

		// search for all the indexes which is unique and have the same key columns
		const ModVector<Constraint*>& vecConstraint = cReferedTable_.getConstraint(cTrans_);
		ModVector<Constraint*>::ConstIterator iterator = vecConstraint.begin();
		const ModVector<Constraint*>::ConstIterator last = vecConstraint.end();
		for (; iterator != last; ++iterator) {
			// check the constraint
			const Constraint* pConstraint = *iterator;
			ID::Value iIndexID = checkForeignKey(cTrans_, cReferedTable_, *pConstraint, vecReferedColumnID);
			if (iIndexID != ID::Invalid) {
				return iIndexID;
			}
		}

	} else {
		// if not columns are specified, primary key is used
		Constraint* pPrimaryKey = cReferedTable_.getPrimaryKeyConstraint(cTrans_);
		if (pPrimaryKey) {
			const Index* pIndex = cReferedTable_.getIndex(pPrimaryKey->getName(), cTrans_);
			; _SYDNEY_ASSERT(pIndex);
			if (pIndex->getKey(cTrans_).getSize() != m_vecColumnID->getSize()) {
				// rerefing columns do not match to the refered columns
				SydInfoMessage << "The number of keys of the primary key ("
							   << pIndex->getKey(cTrans_).getSize()
							   << ") have to be equal to the number of refering columns."
							   << ModEndl;
				_SYDNEY_THROW0(Exception::InvalidReference);
			}
			return pIndex->getID();
		}
	}

	// no appropriate index
	SydInfoMessage << "Foreign key: No corresponding constraint can be found" << ModEndl;
	_SYDNEY_THROW0(Exception::InvalidReference);
}

// FUNCTION public
//	Schema::Constraint::getIDs -- create an ID array from column IDs and foreign IDs
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const ModVector<ID::Value>&
//
// EXCEPTIONS

const ModVector<Object::ID::Value>&
Constraint::
getIDs() const
{
	if (!m_vecID) {
		m_vecID = new ModVector<ID::Value>;
	}
	*m_vecID = getColumnID();
	switch (getCategory()) {
	case Category::Unknown:
	case Category::OldPrimaryKey:
	case Category::OldUnique:
		{
			// do nothing
			break;
		}
	case Category::PrimaryKey:
	case Category::Unique:
		{
			// store corresponding index id
			m_vecID->pushBack(m_iIndexID);
			break;
		}
	case Category::ForeignKey:
	case Category::ReferedKey:
		{
			// store corresponding index id
			m_vecID->pushBack(m_iIndexID);
			// store referenced table, index and constraint id
			m_vecID->pushBack(m_iReferedTableID);
			m_vecID->pushBack(m_iReferedIndexID);
			m_vecID->pushBack(m_iReferedConstraintID);
			break;
		}
	}
	return *m_vecID;
}

// FUNCTION public
//	Schema::Constraint::setIDs -- set column IDs and foreign IDs from an ID array
//
// NOTES
//
// ARGUMENTS
//	const ModVector<ID::Value>& vecIDs_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Constraint::
setIDs(const ModVector<ID::Value>& vecIDs_)
{
	setColumnID(vecIDs_);
	switch (getCategory()) {
	case Category::Unknown:
	case Category::OldPrimaryKey:
	case Category::OldUnique:
		{
			// do nothing
			break;
		}
	case Category::PrimaryKey:
	case Category::Unique:
		{
			if (vecIDs_.getSize() < 2) {
				_SYDNEY_THROW0(Exception::LogItemCorrupted);
			}
			// corresponding index ID is stored
			m_iIndexID = m_vecColumnID->getBack();
			m_vecColumnID->popBack();
			break;
		}
	case Category::ForeignKey:
	case Category::ReferedKey:
		{
			if (vecIDs_.getSize() < 5) {
				_SYDNEY_THROW0(Exception::LogItemCorrupted);
			}
			m_iReferedConstraintID = m_vecColumnID->getBack();
			m_vecColumnID->popBack();
			m_iReferedIndexID = m_vecColumnID->getBack();
			m_vecColumnID->popBack();
			m_iReferedTableID = m_vecColumnID->getBack();
			m_vecColumnID->popBack();
			m_iIndexID = m_vecColumnID->getBack();
			m_vecColumnID->popBack();
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::LogItemCorrupted);
		}
	}
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

// メタデータベースにおける「制約」表の構造は以下のとおり
// create table Constraint_DBXXXX (
//		ID			id,
//		parent		id,
//		name		nvarchar,
//		category	int,
//		position	int,
//		columnIDs	int ARRAY[NO LIMIT],
//		time		timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Constraint>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Constraint>(Meta::MemberType::_type_, &Constraint::_get_, &Constraint::_set_)

	Meta::Definition<Constraint> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(ParentID),		// ParentID
		_DEFINE0(Name),			// Name
		_DEFINE0(Integer),		// Category
		_DEFINE0(Integer),		// Position
		_DEFINE2(IDArray, getIDs, setIDs), // ColumnIDs (+ IndexID)
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Constraint::getMetaFieldNumber --
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
Constraint::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Constraint::MemberNum);
}

//	FUNCTION public
//	Schema::Constraint::getMetaFieldDefinition --
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
Constraint::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Constraint::packMetaField --
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
Constraint::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Constraint>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			switch (iMemberID_) {
			case Meta::Constraint::Category:
				{
					return pack(static_cast<int>(getCategory()));
				}
			case Meta::Constraint::Position:
				{
					return pack(static_cast<int>(getPosition()));
				}
			}
			break;
		}
	case Meta::MemberType::IDArray:
		{
			return pack((this->*(cDef.m_funcGet._ids))());
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Constraint::unpackMetaField --
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
Constraint::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Constraint>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			int value;
			if (unpack(pData_, value)) {
				switch (iMemberID_) {
				case Meta::Constraint::Category:
					{
						if (value >= 0 && value < Category::ValueNum) {
							_category = static_cast<Category::Value>(value);
							return true;
						}
					}
				case Meta::Constraint::Position:
					{
						if (value >= 0) {
							_position = value;
							return true;
						}
					}
				}
			}
			break;
		}
	case Meta::MemberType::IDArray:
		{
			ModVector<ID::Value> vecID;
			if (unpack(pData_, vecID)) {
				(this->*(cDef.m_funcSet._ids))(vecID);
				return true;
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
// Copyright (c) 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
