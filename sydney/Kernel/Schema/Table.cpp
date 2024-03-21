// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Table.cpp -- 表関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023, 2024 Ricoh Company, Ltd.
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

#include "Schema/Table.h"
#include "Schema/AccessFile.h"
#include "Schema/Area.h"
#include "Schema/AreaMap.h"
#include "Schema/AreaContent.h"
#include "Schema/AutoRWLock.h"
#include "Schema/BtreeFile.h"
#include "Schema/Column.h"
#include "Schema/ColumnMap.h"
#include "Schema/Constraint.h"
#include "Schema/ConstraintMap.h"
#include "Schema/Database.h"
#include "Schema/Default.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/FileMap.h"
#include "Schema/Hint.h"
#include "Schema/Index.h"
#include "Schema/IndexMap.h"
#include "Schema/LobFile.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Message_VerifyStarted.h"
#include "Schema/Message_VerifyFinished.h"
#include "Schema/Meta.h"
#include "Schema/NameParts.h"
#include "Schema/Object.h"
#include "Schema/Parameter.h"
#include "Schema/Partition.h"
#include "Schema/PathParts.h"
#include "Schema/RecordFile.h"
#include "Schema/Recovery.h"
#include "Schema/Sequence.h"
#include "Schema/SystemTable_Table.h"
#include "Schema/SystemTable_Column.h"
#include "Schema/SystemTable_Constraint.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/TupleID.h"
#include "Schema/Utility.h"
#include "Schema/VectorFile.h"
#include "Schema/FileVerify.h"

#include "Statement/AlterTableStatement.h"
#include "Statement/AlterTableAction.h"
#include "Statement/AreaOption.h"
#include "Statement/ColumnDefinition.h"
#include "Statement/DataValue.h"
#include "Statement/DropTableStatement.h"
#include "Statement/Hint.h"
#include "Statement/Identifier.h"
#include "Statement/Object.h"
#include "Statement/TableConstraintDefinition.h"
#include "Statement/TableDefinition.h"
#include "Statement/TableElementList.h"
#include "Statement/Type.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/BinaryData.h"
#include "Common/BitSet.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/StringArrayData.h"
#include "Common/UnsignedIntegerArrayData.h"
#include "Common/SQLData.h"

#include "Exception/AreaNotFound.h"
#include "Exception/ColumnNotFound.h"
#include "Exception/DefaultNeeded.h"
#include "Exception/DuplicateIdentity.h"
#include "Exception/InvalidAreaSpecification.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/OtherObjectDepending.h"
#include "Exception/TableAlreadyDefined.h"
#include "Exception/TableNotFound.h"
#include "Exception/TooLongObjectName.h"
#include "Exception/Unexpected.h"

#include "FileCommon/FileOption.h"

#include "Trans/Transaction.h"

#include "ModArchive.h"
#include "ModAutoPointer.h"
#include "ModVector.h"

#include <stdio.h>

_SYDNEY_USING
_SYDNEY_SCHEMA_USING
namespace {

// FileMapをIterationしながら行う処理で
// Iterationから除外するFileの条件を示す関数定義
namespace _Omit
{
	// 通常処理
	bool _file(File* pFile_)
	{
		return pFile_->getIndexID() != Object::ID::Invalid;
	}

	// リカバリー処理
	bool _fileRecover(File* pFile_, const Object::Name& cDatabaseName_)
	{
		using namespace Manager::RecoveryUtility;

		// DropまたはCreateがUndoされている索引のファイルはrecoverしない
		return (pFile_->getIndexID() != Object::ID::Invalid
				&& (Undo::isEntered(cDatabaseName_, pFile_->getIndexID(), Undo::Type::DropIndex)
					|| Undo::isEntered(cDatabaseName_, pFile_->getIndexID(), Undo::Type::CreateIndex)));
	}
}

namespace _Name
{

#define _TableName(__str__) Schema::Object::Name(NameParts::Table::System##__str__)
	const Schema::Object::Name _SystemTableNames[] = {
		Schema::Object::Name(),		// Unknown
		_TableName(Database),
		_TableName(Table),
		_TableName(Column),
		_TableName(Constraint),
		_TableName(Index),
		_TableName(Key),
		_TableName(File),
		_TableName(Field),
		_TableName(Area),
		_TableName(AreaContent),
		_TableName(Privilege),
		_TableName(Cascade),
		_TableName(Partition),
		_TableName(Function)
	};
#undef _TableName

	// システム表に対応する名前を得る
	const Schema::Object::Name& _getSystemTableName(Schema::Object::Category::Value eCategory_);
	// ある名前がどのシステム表に対応するかを得る
	Schema::Object::Category::Value _getSystemTableCategory(const Schema::Object::Name& cName_);
	// 表名の重複を調べる
	bool _checkExistence(Trans::Transaction& cTrans_, const Database& cDatabase_, const Table* pTable_, bool bNoCancel_ = false);

} // namespace _Name

namespace _ID {

	// システム表に対応するIDを得る
	Schema::Object::ID::Value _getSystemTableID(Schema::Object::Category::Value eCategory_);
	// IDがどのシステム表に対応するかを得る
	Schema::Object::Category::Value _getSystemTableCategory(Schema::Object::ID::Value iID_);

} // namespace _ID

namespace _Area
{
	// ファイルの格納場所として使われるエリアカテゴリーを得る
	AreaCategory::Value _getEffectiveCategory(const ModVector<Object::ID::Value>& vecAreaID_,
												AreaCategory::Value eCategory_);
	// ファイルの格納場所として使われるエリアIDを得る
	Object::ID::Value _getEffectiveID(const ModVector<Object::ID::Value>& vecAreaID_,
									  AreaCategory::Value eCategory_);

	// ファイルを抽出するのに使用する条件
	bool _findFile(File* pFile_, AreaCategory::Value eCategory_,
				   const ModVector<Object::ID::Value>& vecPrevAreaID_,
				   const ModVector<Object::ID::Value>& vecPostAreaID_);
} // namespace _Area

}

/////////////////////////////
// _Name
/////////////////////////////

//	FUNCTION local
//	$$::_Name::_getSystemTableName -- システム表に対応する名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value eCategory_
//			対象のシステム表
//
//	RETURN
//		システム表に対応する名前
//
//	EXCEPTIONS

inline
const Schema::Object::Name&
_Name::_getSystemTableName(Schema::Object::Category::Value eCategory_)
{
	return _SystemTableNames[eCategory_];
}

//	FUNCTION local
//	$$::_Name::_getSystemTableCategory -- ある名前がどのシステム表に対応するかを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& cName_
//			調べる名前
//
//	RETURN
//		Schema::Object::Category::Unknown
//			システム表にある名前ではない
//		Schema::Object::Category::Unknown以外
//			システム表にある名前である
//
//	EXCEPTIONS

Schema::Object::Category::Value
_Name::_getSystemTableCategory(const Schema::Object::Name& cName_)
{
	int i = Schema::Object::Category::ValueNum - 1;
	for (; i > Schema::Object::Category::Unknown; --i)
		if (cName_ ==
			_getSystemTableName(static_cast<Schema::Object::Category::Value>(i)))
			break;

	// ヒットしなければi == Unknownになっているのでそのまま返せばよい
	return static_cast<Schema::Object::Category::Value>(i);			
}

//	FUNCTION local
//	_Name::_checkExistence -- 表名の重複を調べる
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database& cDatabase_
//			表が属するデータベース
//		const Schema::Table* pTable_
//			作成しようとしている表
//		bool bNoCancel_ = false
//			trueなら重複時に常に例外。falseならパラメーターでチェックする
//
//	RETURN
//		true ... 同じ名前のものが存在している、または作成中である
//		false... 同じ名前のものはない
//
//	EXCEPTIONS
//		Exception::TableAlreadyDefined
//			同じ名前のものが存在しており、CanceledWhenDuplicatedがfalseである
bool
_Name::_checkExistence(Trans::Transaction& cTrans_,
					   const Database& cDatabase_,
					   const Table* pTable_,
					   bool bNoCancel_ /* = false */)
{
	if (pTable_->getName().getLength() > Manager::ObjectName::getMaxLength()) {
		// 名称が制限長を超えていたらエラー
		_SYDNEY_THROW2(Exception::TooLongObjectName,
					   pTable_->getName().getLength(), Manager::ObjectName::getMaxLength());
	}

	// まずシステム表と同じ名前でないか調べる
	if (_getSystemTableCategory(pTable_->getName()) != Object::Category::Unknown) {
		// システム表と同じ名前の表は作れない
		SydInfoMessage
			<< "Table definition of the same name as a SystemTable "
			<< pTable_->getName()
			<< ModEndl;
		_SYDNEY_THROW2(Exception::TableAlreadyDefined, pTable_->getName(), cDatabase_.getName());
	}

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (Manager::ObjectName::reserve(pTable_) == false) {

		if (!bNoCancel_ && Manager::Configuration::isCanceledWhenDuplicated()) {
			// trueを返し後の処理は何もしない
			SydInfoMessage
				<< "Table definition of the same name in progress("
				<< pTable_->getName()
				<< ") canceled"
				<< ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage
				<< "Table definition of the same name in progress("
				<< pTable_->getName()
				<< ")"
				<< ModEndl;
			_SYDNEY_THROW2(Exception::TableAlreadyDefined, pTable_->getName(), cDatabase_.getName());
		}
	}

	// さらに、同じ名前の表がすでにないか調べ、
	// 同時に現在の表をマネージャーに読み込んでおく
	// ★注意★
	// doAfterPersistの中でマネージャーに追加されるので
	// ここで読み込んでおかないと追加のときに不完全なTableを
	// 読み込んでしまう

	bool bFound = false;
	try {
		bFound = (cDatabase_.getTable(pTable_->getName(), cTrans_, true /* internal */) != 0);
	} catch (...) {
		Manager::ObjectName::withdraw(pTable_);
		_SYDNEY_RETHROW;
	}
	if (bFound) {

		// 作成中の登録からオブジェクトを外す
		Manager::ObjectName::withdraw(pTable_);

		if (!bNoCancel_ && Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated table definition("
						   << pTable_->getName()
						   << ") canceled"
						   << ModEndl;
			return true;
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated table definition("
						   << pTable_->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::TableAlreadyDefined, pTable_->getName(), cDatabase_.getName());
		}
	}
	return false;
}

///////////////////
// _ID
///////////////////

//	FUNCTION local
//	$$::_ID::_getSystemTableID -- システム表に対応するIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value eCategory_
//			対象のシステム表
//
//	RETURN
//		システム表に対応するID
//
//	EXCEPTIONS

inline
Schema::Object::ID::Value
_ID::_getSystemTableID(Schema::Object::Category::Value eCategory_)
{
	return Schema::Object::ID::Invalid - eCategory_;
}

//	FUNCTION local
//	$$::_ID::_getSystemTableCategory -- IDがどのシステム表に対応するかを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			調べるID
//
//	RETURN
//		Schema::Object::Category::Unknown
//			システム表に対応するIDではない
//		Schema::Object::Category::Unknown以外
//			システム表に対応するIDである
//
//	EXCEPTIONS

Schema::Object::Category::Value
_ID::_getSystemTableCategory(Schema::Object::ID::Value iID_)
{
	if (iID_ > _getSystemTableID(Schema::Object::Category::ValueNum)) {
		return static_cast<Schema::Object::Category::Value>
			(_getSystemTableID(Schema::Object::Category::Unknown) - iID_);
	} else {
		return Schema::Object::Category::Unknown;
	}
}

//////////////////////
// _Area
//////////////////////

//	FUNCTION local
//	_Area::_getEffectiveCategory -- ファイルの格納場所として使われるエリアカテゴリーを得る
//
//	NOTES

AreaCategory::Value
_Area::_getEffectiveCategory(const ModVector<Object::ID::Value>& vecAreaID_,
							 AreaCategory::Value eCategory_)
{
	Object::ID::Value iResult = vecAreaID_[static_cast<int>(eCategory_)];
	if (eCategory_ != AreaCategory::Default && iResult == Object::ID::Invalid) {
		do{
			eCategory_ = AreaCategory::getSuperValue(eCategory_);
			iResult  = vecAreaID_[static_cast<int>(eCategory_)];
		} while (eCategory_ != AreaCategory::Default && iResult == Object::ID::Invalid); 
	}
	return eCategory_;
}

//	FUNCTION local
//	_Area::_getEffectiveID -- ファイルの格納場所として使われるエリアIDを得る
//
//	NOTES

Object::ID::Value
_Area::_getEffectiveID(const ModVector<Object::ID::Value>& vecAreaID_,
					   AreaCategory::Value eCategory_)
{
	Object::ID::Value iResult = vecAreaID_[static_cast<int>(eCategory_)];
	if (eCategory_ != AreaCategory::Default && iResult == Object::ID::Invalid) {
		do{
			eCategory_ = AreaCategory::getSuperValue(eCategory_);
			iResult  = vecAreaID_[static_cast<int>(eCategory_)];
		} while (eCategory_ != AreaCategory::Default && iResult == Object::ID::Invalid); 
	}
	return iResult;
}

//	FUNCTION local
//	_Area::_findFile -- ファイルを抽出するのに使用する条件
//
//	NOTES

bool
_Area::_findFile(File* pFile_, AreaCategory::Value eCategory_,
				 const ModVector<Object::ID::Value>& vecPrevAreaID_,
				 const ModVector<Object::ID::Value>& vecPostAreaID_)
{
	// 索引にエリア指定があるものは結果に含めるべきではないが
	// エリアを使用しているファイルがあるかを呼び出し側で調べる必要があるので
	// ここではその条件は調べない
	return FileMap::findByAreaCategory(pFile_, eCategory_)
		|| (_getEffectiveCategory(vecPrevAreaID_, pFile_->getAreaCategory()) == eCategory_
			&& _getEffectiveCategory(vecPostAreaID_, pFile_->getAreaCategory()) == eCategory_);
}

/////////////////////////////
// Table
/////////////////////////////

//	FUNCTION public
//	Schema::Table::Table --	表を表すクラスのデフォルトコンストラクター
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

Table::
Table()
	: Object(Object::Category::Table),
	  m_pRowID(),
	  m_pIdentity(),
	  m_veciAreaID(AreaCategory::ValueNum, ID::Invalid),
	  m_vecpArea(0),
	  m_pPath(0),
	  m_pHint(0),
	  m_pReorganizedFiles(0),
	  m_bTemporary(false),
	  m_bSystem(false),
	  m_mapReferencingTable(),
	  m_mapReferencedTable(),
	  m_vecReferencingTable(0),
	  m_vecReferencedTable(0),
	  _columns(0),
	  _constraints(0),
	  _indices(0),
	  _files(0),
	  m_cRWLockForFile()
{ }

//	FUNCTION public
//	Schema::Table::Table -- 指定された名前の表を表すクラスのコンストラクター
//
//	NOTES
//		表を表すクラスを生成するだけで、「表」表は更新されない
//
//	ARGUMENTS
//		const Schema::Database&	database
//			表が存在するデータベースを表すクラス
//		const Schema::Object::Name& name
//			表の名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Table::
Table(const Database& database, const Name& name)
	: Object(Object::Category::Table, database.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, database.getID(), database.getID(),
			 name),
	  m_pRowID(),
	  m_pIdentity(),
	  m_veciAreaID(AreaCategory::ValueNum, ID::Invalid),
	  m_vecpArea(0),
	  m_pPath(0),
	  m_pHint(0),
	  m_pReorganizedFiles(0),
	  m_bTemporary(false),
	  m_bSystem(false),
	  m_mapReferencingTable(),
	  m_mapReferencedTable(),
	  m_vecReferencingTable(0),
	  m_vecReferencedTable(0),
	  _columns(0),
	  _constraints(0),
	  _indices(0),
	  _files(0),
	  m_cRWLockForFile()
{ }

//	FUNCTION public
//	Schema::Table::Table -- ログデータからのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Database&	database
//			表が存在するデータベースを表すクラス
//		const Schema::LogData& cLogData_
//			ログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Table::
Table(Trans::Transaction& cTrans_,
	  const Database& database, const LogData& cLogData_)
	: Object(Object::Category::Table, database.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, database.getID(), database.getID()),
	  m_pRowID(),
	  m_pIdentity(),
	  m_veciAreaID(AreaCategory::ValueNum, ID::Invalid),
	  m_vecpArea(0),
	  m_pPath(0),
	  m_pHint(0),
	  m_pReorganizedFiles(0),
	  m_bTemporary(false),
	  m_bSystem(false),
	  m_mapReferencingTable(),
	  m_mapReferencedTable(),
	  m_vecReferencingTable(0),
	  m_vecReferencedTable(0),
	  _columns(0),
	  _constraints(0),
	  _indices(0),
	  _files(0),
	  m_cRWLockForFile()
{
	// ログデータの内容を得る
	// ★注意★
	// makeLogDataの実装が変わったらここも変える

	// 名前、種別、ヒント、エリアID配列をログデータから反映しておく
	setName(cLogData_.getString(Log::Name));
	if (!unpackMetaField(cLogData_[Log::Create::Type].get(), Meta::Table::Type)
		|| !unpackMetaField(cLogData_[Log::Create::Hint].get(), Meta::Table::Hint)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	m_veciAreaID = cLogData_.getIDs(Log::Create::AreaIDs);
										// UNDO情報がある場合は後で上書きする
}

// FUNCTION public
//	Schema::Table::Table -- copy constructor
//
// NOTES
//
// ARGUMENTS
//	const Table& original
//		original object to be copied
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Table::
Table(const Table& original)
	: Object(Object::Category::Table, original.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, original.getDatabaseID(), original.getDatabaseID(),
			 original.getName()),
	  m_pRowID(),
	  m_pIdentity(),
	  m_veciAreaID(original.m_veciAreaID),
	  m_vecpArea(0),
	  m_pPath(0),
	  m_pHint(0),
	  m_pReorganizedFiles(0),
	  m_bTemporary(original.m_bTemporary),
	  m_bSystem(original.m_bSystem),
	  m_mapReferencingTable(original.m_mapReferencingTable),
	  m_mapReferencedTable(original.m_mapReferencedTable),
	  m_vecReferencingTable(0),
	  m_vecReferencedTable(0),
	  _columns(0),
	  _constraints(0),
	  _indices(0),
	  _files(0),
	  m_cRWLockForFile()
{
	if (original.getHint())
		m_pHint = new Hint(*original.getHint());
}

//	FUNCTION public
//	Schema::Table::~Table -- 表を表すクラスのデストラクター
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

Table::
~Table()
{
	destruct();
}

//	FUNCTION public
//		Schema::Table::getNewInstance -- オブジェクトを新たに取得する
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
Table*
Table::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Table> pObject = new Table;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION private
//	Schema::Table::destruct -- 表を表すクラスのデストラクター下位関数
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
Table::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteするところでresetを呼ぶ
	// ここでは領域を開放するのみ

	delete _columns, _columns = 0;
	delete _constraints, _constraints = 0;
	delete _indices, _indices = 0;
	delete _files, _files = 0;
	delete m_pPath, m_pPath = 0;

	clearArea();
	clearHint();

	delete m_pReorganizedFiles, m_pReorganizedFiles = 0;
	delete m_vecReferencingTable, m_vecReferencingTable = 0;
	delete m_vecReferencedTable, m_vecReferencedTable = 0;

	detachFiles();

	m_pRowID = Column::Pointer();
	m_pIdentity = Column::Pointer();
}

//	FUNCTION public
//	Schema::Table::detachFiles --
//		表が使っているファイルのdetach
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
Table::
detachFiles()
{
	clearSequence();
}

//	FUNCTION public
//	Schema::Table::create -- SQL の表定義から表を実際に定義する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Database&	database
//			表を定義するデータベースを表すクラス
//		Statement::TableDefinition&	statement
//			解析済の SQL の 表定義
//		Schema::LogData& cLogData_
//			ログに出力するデータを格納する変数
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		定義された表のスキーマオブジェクト
//
//	EXCEPTIONS

// static
Table*
Table::
create(Database& database, const Statement::TableDefinition& statement,
	   LogData& cLogData_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 与えられた表定義から、定義しようとしている表、
	// およびその表中の列を表すクラスを生成する
	//
	// また、表を構成するファイル、
	// およびそのファイル中のフィールドを表すクラスを生成する

	// 表定義から表名を得て、処理する

	Statement::Identifier* identifier = statement.getName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());

	ModAutoPointer<Table>
		table = new Table(database, *identifier->getIdentifier());
	; _SYDNEY_ASSERT(table.get());

	// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
	if (_Name::_checkExistence(cTrans_, database, table.get())) {
		return 0;
	}

	// 一時表かどうか
	table->setTemporary(Table::isToBeTemporary(statement.getName()));

	try {
		// IDをふり、状態を変える
		table->Object::create(cTrans_);
		SCHEMA_FAKE_ERROR("Schema::Table", "Create", "Created");

		if (!table->isTemporary()) {
			// エリア割り当て定義のログデータ
			// エリアの指定があったら名前からIDを取得して設定する
			// 同時にエリアに関するログデータを追加する
			if (Statement::AreaOption* pAreaOption = statement.getAreaOption()) {
				if (database.isSlaveStarted()) {
					// area is not allowed for replicated database
					_SYDNEY_THROW1(Exception::InvalidAreaSpecification, database.getName());
				}
				ModVector<ID::Value> dummy(AreaCategory::ValueNum);
				table->setArea(cTrans_, *pAreaOption, dummy, table->m_veciAreaID);
				table->createAreaContent(cTrans_);
			}
			SCHEMA_FAKE_ERROR("Schema::Table", "Create", "Area");
		}

		//////////////////////////////////////////////////
		//	表定義を解析し、列を表すクラスを生成する	//
		//////////////////////////////////////////////////

		// 処理する列の表の先頭からの位置
		Column::Position columnPosition = 0;

		// 列定義のログデータ
		ModAutoPointer<Common::DataArrayData> pColumnLogData = new Common::DataArrayData;

		// 表定義からスコープを得て、処理する

		switch (statement.getScope()) {
		case Statement::TableDefinition::Permanent:
		{
			// 永続的な表またはセッションローカルな一時表である

			// ★注意★
			// 一時表もロールバックの対象になるので
			// タプルIDを保持する必要がある

			// タプル ID を格納する列を表すクラスを生成する

			Column::Pointer column =
				Column::create(*table, columnPosition++,
							   Object::Name(NameParts::Column::TupleID),
							   Column::Category::TupleID,
							   Common::SQLData(Common::SQLData::Type::Int,
												   Common::SQLData::Flag::Fixed, 4, 0),
							   Default(),
							   cTrans_);
			SCHEMA_FAKE_ERROR("Schema::Table", "Create", "TupleIDColumn");

			// Schema_CancelWithDuplicate が true なら 0 が来る可能性が有る
			// ; _SYDNEY_ASSERT(column);

			// 状態は「生成」である

			; _SYDNEY_ASSERT(column.get() ?
							 (column->getStatus() == Status::Created
							  || column->getStatus() == Status::Mounted) : true);

			if ( column.get() )
			{
				// 表にこの列を表すクラスを追加する
				// ★注意★
				// キャッシュに加えるのは永続化の後

				(void) table->addColumn(column, cTrans_);

				// このタプル ID を生成するためのシーケンスを生成する

				(void) table->getTupleSequence(cTrans_);
				SCHEMA_FAKE_ERROR("Schema::Table", "Create", "TupleIDSequence");

				if (!table->isTemporary()) {
					// ログデータに追加する
					ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData;
					column->makeLogData(*pData);
					pColumnLogData->pushBack(pData.release());
				}
			}
			break;
		}
		case Statement::TableDefinition::LocalTemporary:
		case Statement::TableDefinition::GlobalTemporary:
			// not supported yet
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		// Create columns according to column definitions
		table->createColumn(cTrans_, statement.getElements(), *pColumnLogData);
		SCHEMA_FAKE_ERROR("Schema::Table", "Create", "Column");

		// 表定義からヒントを得て、処理する
		if (Statement::Hint* hint = statement.getHint())
			table->m_pHint = new Hint(*hint);

		// 表を構成するファイルを表すクラスを生成する
		// ログデータは後で作る
        table->createFile(cTrans_);
		SCHEMA_FAKE_ERROR("Schema::Table", "Create", "File");

		// Create constraints according to constraint definitions
		// [NOTES]
		//	Constraint should be created after record file is created
		//	because implicit indexes are created
		ModAutoPointer<Common::DataArrayData> pConstraintLogData = new Common::DataArrayData;
		table->createConstraint(cTrans_, statement.getElements(), *pConstraintLogData);

		SCHEMA_FAKE_ERROR("Schema::Table", "Create", "Index");

		if (!table->isTemporary()) {
			// すべてのオブジェクトができた時点で最後のIDを得る
			ID::Value iLastID = ObjectID::getLastValue(cTrans_, table->getDatabase(cTrans_));

			// ファイル定義のログデータ
			ModAutoPointer<Common::DataArrayData> pFileLogData = new Common::DataArrayData;
			table->makeFileLogData(cTrans_, table->getFile(cTrans_), *pFileLogData);

			// 表のログデータを作る
			//   - 表定義のログ
			//   - 列定義のログ
			//   - 制約定義のログ
			//   - 最後のID
			//   - ファイル定義のログ
			table->makeLogData(cTrans_, cLogData_);
			cLogData_.addData(pColumnLogData.release());
			cLogData_.addData(pConstraintLogData.release());
			cLogData_.addID(iLastID);
			cLogData_.addData(pFileLogData.release());
			; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Create::Num);
		}

	} catch (...) {

		// 作成中の登録から除く
		Manager::ObjectName::withdraw(table.get());

		// 作成を取り消す
		table->drop(cTrans_);

		_SYDNEY_RETHROW;
	}

	// 生成された表のスキーマオブジェクトを返す

	return table.release();
}

//	FUNCTION public
//	Schema::Table::create -- ログデータの表定義から表を定義する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		consnt Schema::Database&	database
//			表を定義するデータベースを表すクラス
//		const Schema::LogData& cLogData_
//			表定義のログデータ
//
//	RETURN
//		定義された表のスキーマオブジェクト
//
//	EXCEPTIONS

// static
Table*
Table::
create(Trans::Transaction& cTrans_, const Database& database,
	   const LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 与えられたログデータ表定義から、定義しようとしている表、
	// およびその表中の列を表すクラスを生成する

	ModAutoPointer<Table> table = new Table(cTrans_, database, cLogData_);
	; _SYDNEY_ASSERT(table.get());

	// 名前が重複することはありえないが
	// データベースにloadしておく必要があるので確認だけする
	if (database.getTable(table->getName(), cTrans_, true /* internal */)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// ALTER TABLEがUNDOされている場合、最終的なエリアIDの割り当てを使用する
	// ログに記録されているIDを得て最終的なエリアIDの割り当てがあるか調べる
	// 名前についても同様の処理を行う
	ID::Value id = getObjectID(cLogData_);
	table->checkUndo(database, id);

	// IDをふり、状態を変える
	table->Object::create(cTrans_, id);

	// エリア格納関係を作る
	table->createAreaContent(cTrans_);

	//////////////////////////////////
	//	列を表すクラスを生成する	//
	//////////////////////////////////

	// ログデータから列に関するデータを取り出す
	// ★注意★
	// makeLogDataの実装が変わったらここも変える
	{
		const Common::DataArrayData& cLogColumns =
			cLogData_.getDataArrayData(Log::Create::ColumnDefinitions);

		// 処理する列の表の先頭からの位置
		Column::Position columnPosition = 0;

		int n = cLogColumns.getCount();

		for (int i = 0; i < n; ++i) {
			const Common::DataArrayData& cLogColumn =
				LogData::getDataArrayData(cLogColumns.getElement(i));

			table->createColumn(cTrans_, cLogColumn, columnPosition++);
		}
	}

	// 表を構成するファイルを表すクラスを生成する
	const Common::DataArrayData* pLogFiles = 0;
	if (cLogData_.getCount() >= Log::Create::Num1) {
		pLogFiles = &(cLogData_.getDataArrayData(Log::Create::FileDefinitions));
	}
	table->createFile(cTrans_, pLogFiles);

	// ログデータから制約に関するデータを取り出す
	{
		const Common::DataArrayData& cLogConstraints =
			cLogData_.getDataArrayData(Log::Create::ConstraintDefinitions);

		// 処理する制約の表の先頭からの位置
		Constraint::Position constraintPosition = 0;

		int n = cLogConstraints.getCount();
		for (int i = 0; i < n; ++i) {
			const Common::DataArrayData& cLogConstraint =
				LogData::getDataArrayData(cLogConstraints.getElement(i));

			table->createConstraint(cTrans_, cLogConstraint, constraintPosition++);
		}
	}

	// 生成された表のスキーマオブジェクトを返す

	return table.release();
}

//	FUNCTION public
//	Schema::Table::create -- 表を実際に定義する
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
Table::
create(Trans::Transaction& cTrans_)
{
    // このメソッド内での処理進行状況を表す列挙形
    enum {
        None,
        Mkdir,									// ディレクトリの作成
        SequenceMade,							// シーケンスファイルの作成
        FileCreating,							// 表を構成するファイルの作成開始
        ValueNum
    } eStatus = None;

	ModVector<File*> vecpFile;
	Utility::File::AutoRmDir cAutoRmDir;
	cAutoRmDir.setDir(getPath(cTrans_));

    try {
		eStatus = Mkdir;
		SCHEMA_FAKE_ERROR("Schema::Table", "CreateFile", "Directory");

		if (getScope() == Object::Scope::Permanent
			|| getScope() == Object::Scope::SessionTemporary) {

			// シーケンスを記録するためのファイルを生成する
			createSequence(cTrans_);
		}
		eStatus = SequenceMade;
		SCHEMA_FAKE_ERROR("Schema::Table", "CreateFile", "Sequence");

        // 表を構成するファイルごとに処理する

        {
            const ModVector<File*>&	files = getFile(cTrans_);

            ModSize n = files.getSize();

            vecpFile.assign(n, reinterpret_cast<File *>(false));

            eStatus = FileCreating;

            for (ModSize i = 0; i < n; ++i) {
                ; _SYDNEY_ASSERT(files[i]);

                // 表を構成するファイルを生成する

                files[i]->create(cTrans_);
                vecpFile[i] = files[i];
            }
			SCHEMA_FAKE_ERROR("Schema::Table", "CreateFile", "File");
        }

    } catch(...) {
#ifdef DEBUG
        SydSchemaDebugMessage << "table create failed." << ModEndl;
#endif // DEBUG

        switch(eStatus) {
        case FileCreating:
        {
            // 作成したファイルを削除する

			ModSize nMax = vecpFile.getSize();

            for ( ModSize nCnt = 0; nCnt < nMax; nCnt++ ) {

                // 作成したファイルのみ削除する
                if ( vecpFile[nCnt] )
                    vecpFile[nCnt]->destroy(cTrans_, false /* do not delete directory */);
            }
        }
        case SequenceMade:
        {
            // 作成したシーケンスファイルの削除

            dropSequence(cTrans_);
        }
        case Mkdir:
#ifdef OBSOLETE
		{
			// 作成したディレクトリを削除する
			// （ディレクトリのみのはず）
			Utility::File::rmAll(getPath(cTrans_)); 

		}
#endif
		default:
			break;
		}

        _SYDNEY_RETHROW;
    }
	// 成功したのでエラー処理を解除する
	cAutoRmDir.disable();
}

//	FUNCTION public
//	Schema::Table::createSystem -- システム表を生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		Schema::Database& cDatabase_
//			メタデータベースを表すオブジェクト
//		Schema::Object::Category::Value eCategory_
//			システム表を生成するオブジェクトのカテゴリー
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
Table*
Table::
createSystem(Trans::Transaction& cTrans_,
			 const Database& cDatabase_,
			 Object::Category::Value eCategory_)
{
	// データベース表を表すオブジェクトを作成する
	ModAutoPointer<Table> pTable =
		new Table(cDatabase_, getSystemTableName(eCategory_));

	pTable->setID(getSystemTableID(eCategory_));
	pTable->setStatus(Status::Persistent);
	pTable->setSystem(true);

	return pTable.release();
}

//	FUNCTION public
//	Schema::Table::getName -- SQL の表破棄文から表名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Statement::DropTableStatement& statement
//			解析済の SQL の表破棄文
//
//	RETURN
//		表名
//
//	EXCEPTIONS

// static
Object::Name
Table::
getName(const Statement::DropTableStatement& statement)
{
	// 与えられた表破棄文から、破棄しようとしている表の名前を得る

	Statement::Identifier* identifier = statement.getName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());

	return *identifier->getIdentifier();
}

//	FUNCTION public
//	Schema::Table::getName -- SQL の表変更文から表名を得る
//
//	NOTES
//
//	ARGUMENTS
//		Statement::AlterTableStatement& statement
//			解析済の SQL の表変更文
//
//	RETURN
//		表名
//
//	EXCEPTIONS

// static
Object::Name
Table::
getName(const Statement::AlterTableStatement& statement)
{
	Statement::Identifier* identifier = statement.getTableName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());

	return *identifier->getIdentifier();
}

//	FUNCTION public
//	Schema::Table::drop -- 表を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table& cTable_
//			破棄の対象となる表
//		Schema::LogData& cLogData_
//			ログデータを格納する変数
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
Table::
drop(Table& cTable_, LogData& cLogData_, Trans::Transaction& cTrans_)
{
	if (cTable_.isSystem()) return;

	// check whether there exist referencing tables
	if (cTable_.hasReferencingTable(cTrans_)) {
		bool bError = true;
		// search for this table inf referencing table for checking self-reference case
		if (cTable_.m_mapReferencingTable.getSize() == 1) {
			// search for this table
			TableReferenceMap::Iterator iterator = cTable_.m_mapReferencingTable.find(cTable_.getID());
			if (iterator != cTable_.m_mapReferencingTable.end()) {
				// this table is refered only by this table
				bError = false;
			}
		}
		if (bError) {
			// other tables referencing this table
			_SYDNEY_THROW1(Exception::OtherObjectDepending, cTable_.getName());
		}
	}
	cTable_.drop(cTrans_);
	cTable_.makeLogData(cTrans_, cLogData_);
}

//	FUNCTION public
//	Schema::Table::drop -- 表を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		bool bRecovery_ = false
//			リカバリー処理でのDROPか
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
drop(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	if (isSystem()) return;

	// 表を親オブジェクトとするスキーマオブジェクトを破棄する
	// ★注意★
	// これらのスキーマオブジェクトのキャッシュからの削除や
	// 領域の開放は表を表すクラスのデストラクターの中で行われる

	// まず、表についた索引をすべて破棄する
	// キーの定義はこの中で抹消される
	loadIndex(cTrans_).apply(ApplyFunction3<Index, Trans::Transaction&, bool, bool>
							 (&Index::drop, cTrans_, bRecovery_, false));

	// 表に属する列をすべて破棄する
	loadColumn(cTrans_).apply(ApplyFunction3<Column, Trans::Transaction&, bool, bool>
							  (&Column::drop, cTrans_, bRecovery_, false));

	// 表に属する制約をすべて破棄する
	loadConstraint(cTrans_).apply(ApplyFunction3<Constraint, Trans::Transaction&, bool, bool>
								  (&Constraint::drop, cTrans_, bRecovery_, false));

	// 表に属するファイルをすべて破棄する
	// フィールドはこの中で破棄される
	// ファイルはVectorでIterationする
	loadFile(cTrans_).apply(ApplyFunction4<File, Trans::Transaction&, bool, bool, bool>
							(&File::drop, cTrans_, bRecovery_, false, true),
							BoolFunction0<File>(&_Omit::_file));

	// Drop partition definition
	Partition* pPartition = getPartition(cTrans_);
	if (pPartition) {
		pPartition->drop(cTrans_, bRecovery_);
	} 

	// 表と関係するエリア格納関係をすべて破棄する
	{
		if (getScope() == Scope::Permanent) {
			for (int cat = 0; cat < AreaCategory::ValueNum; cat++)
				if (Area* pArea = getArea(static_cast<AreaCategory::Value>(cat), cTrans_))
					AreaContent::drop(*pArea, *this, static_cast<AreaCategory::Value>(cat), cTrans_, bRecovery_);
		}
	}

	// 状態を変更する
	Object::drop(bRecovery_);
}

//	FUNCTION public
//	Schema::Table::undoDrop -- 表の破棄マークをクリアする
//
//	NOTES
//		破棄は論理ログに出力する前なら破棄マークをクリアすることで
//		復旧可能である
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
Table::
undoDrop(Trans::Transaction& cTrans_)
{
	if (isSystem()) return;

	// 表を親オブジェクトとするスキーマオブジェクトの破棄マークをクリアする

	// まず、表についた索引のすべての破棄マークをクリアする
	// キーの定義はこの中でクリアされる
	loadIndex(cTrans_).apply(ApplyFunction1<Index, Trans::Transaction&>
							 (&Index::undoDrop, cTrans_));

	// 表に属する列のすべての破棄マークをクリアする
	loadColumn(cTrans_).apply(ApplyFunction0<Column>
							  (&Column::undoDrop));

	// 表に属する制約のすべての破棄マークをクリアする
	loadConstraint(cTrans_).apply(ApplyFunction0<Constraint>
								  (&Constraint::undoDrop));

	// 表に属するファイルのすべての破棄マークをクリアする
	// フィールドはこの中でクリアされる
	// ファイルはVectorでIterationする
	loadFile(cTrans_).apply(ApplyFunction1<File, Trans::Transaction&>
							(&File::undoDrop, cTrans_),
							BoolFunction0<File>(&_Omit::_file));

	// undoDrop partition definition
	Partition* pPartition = getPartition(cTrans_);
	if (pPartition) {
		pPartition->undoDrop();
	}

	// 表と関係するエリア格納関係のすべての破棄マークをクリアする
	{
		if (getScope() == Scope::Permanent) {
			for (int cat = 0; cat < AreaCategory::ValueNum; cat++)
				if (Area* pArea = getArea(static_cast<AreaCategory::Value>(cat), cTrans_))
					AreaContent::undoDrop(*pArea, *this, static_cast<AreaCategory::Value>(cat), cTrans_);
		}
	}

	// 状態変更を取り消す
	Object::undoDrop();
}

//	FUNCTION public
//	Schema::Table::destroy --
//		表を構成するファイルとそれを格納するディレクトリーを破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		bool bDestroyArea_ = false,
//			trueの場合表以下のディレクトリーを破棄する
//			falseの場合ファイルの削除のみ行う
//		bool bForce_ = true
//			trueの場合チェックポイントを待たずに即座に破棄する
//			falseの場合システムパラメーターによる
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
destroy(Trans::Transaction& cTrans_, bool bDestroyArea_, bool bForce_)
{
	if (isSystem()) return;

	// ディレクトリーを破棄すべきエリアを集める
	// HashMapのサイズはエリアの総数

	Database* pDatabase = getDatabase(cTrans_);
	; _SYDNEY_ASSERT(pDatabase);
	ModSize iAreaCount = pDatabase->loadArea(cTrans_).getSize();

	ModHashMap<ID::Value, Area*, ModHasher<ID::Value> > cAreaMap(iAreaCount, ModFalse);

	// 表を構成するファイルをすべて消す
	loadFile(cTrans_).apply(ApplyFunction3<File, Trans::Transaction&, bool, bool>
							(&File::destroy, cTrans_, true, bForce_),
							BoolFunction0<File>(&_Omit::_file));

	// 表についている索引をすべて消す
	loadIndex(cTrans_).apply(ApplyFunction3<Index, Trans::Transaction&, ModHashMap<ID::Value, Area*, ModHasher<ID::Value> >*, bool>
							 (&Index::destroy, cTrans_, &cAreaMap, bForce_));
										// ここで引数に与えているcAreaMapは
										// bDestroyArea_==falseのときは不要のように思えるが
										// Index::destroyの中でこの引数が0のときに
										// Indexに関係したディレクトリーを削除するようになっているので
										// 常に渡すようにした

	// 表に指定されているエリアのすべてを破棄すべきものに追加する

	if (bDestroyArea_) {
		for (int cat = 0; cat < AreaCategory::ValueNum; cat++)
			if (Area* pArea = getArea(static_cast<AreaCategory::Value>(cat), cTrans_))
				cAreaMap.insert(pArea->getID(), pArea);
	}

	// シーケンスファイルを消す

	if (getScope() == Object::Scope::Permanent
		|| getScope() == Object::Scope::SessionTemporary) {
		dropSequence(cTrans_, bForce_);
	}

	if (bDestroyArea_)	{

		// 破棄すべきエリアから表以下のディレクトリーを削除する

		while (!cAreaMap.isEmpty()) {
			; _SYDNEY_ASSERT(cAreaMap.getFront());
			Area::destroy(cTrans_, cAreaMap.getFront()->getPath(), getPathPart(cTrans_), bForce_);
			cAreaMap.popFront();
		}

		{
			// データベースのデータ格納ディレクトリーに対しても削除を行う
			pDatabase->destroy(cTrans_, Database::Path::Category::Data, getPathPart(cTrans_), bForce_);
		}
	}
}

//	FUNCTION public
//	Schema::Table::mount -- 表を構成するファイルを mount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const Schema::Object::Name& cDatabaseName_
//			表が属するデータベースの名称
//		bool bUndo_ = false
//			trueのときUNDOなので重ねてエラー処理しない	
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Table::
mount(Trans::Transaction& cTrans_, const Name& cDatabaseName_, bool bUndo_ /* = false */)
{
	if (isSystem()) return;

	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(cDatabaseName_, getID(), Undo::Type::DropTable)) {
		// DropがUNDOされていたらmountの対象にしない
		return;
	}

	enum {
		None,
		Files
	} eStatus = None;

	// 表を構成するファイルをすべて mount する
	// ★注意★
	// 索引を構成するファイルもすべて対象となる

	try {
		// 全てのファイルを mount
		// recovery用の特別な動作をする
		loadFile(cTrans_, true /* recovery */).apply(ApplyFunction1<File, Trans::Transaction&>(&File::mount, cTrans_),
													 ApplyFunction2<File, Trans::Transaction&, bool>(&File::unmount, cTrans_, false /* no retain */),
													 bUndo_);
		eStatus = Files;
		SCHEMA_FAKE_ERROR("Schema::Table", "Mount", "Table");

		// シーケンスファイルを mount する
		mountSequence(cTrans_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		switch (eStatus) {
		case Files:
			loadFile(cTrans_, true /* recovery */).apply(ApplyFunction2<File, Trans::Transaction&, bool>(&File::unmount, cTrans_, false /* no retain */));
			// thru.
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Table::unmount -- 表を構成するファイルを unmount する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const Schema::Object::Name& cDatabaseName_
//			表が属するデータベース名
//		bool bUndo_ = false
//			trueのときUNDOなので重ねてエラー処理しない	
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Table::
unmount(Trans::Transaction& cTrans_, const Name& cDatabaseName_, bool bUndo_ /* = false */)
{
	if (isSystem()) return;

	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(cDatabaseName_, getID(), Undo::Type::DropTable)) {
		// DropがUNDOされていたらunmountの対象にしない
		// ★注意★
		// 通常操作ではunmountの後にdropされることはない
		// この条件が満たされるのはmountのエラー処理の場合のみ
		return;
	}

	enum {
		None,
		Files
	} eStatus = None;

	// 表を構成するファイルをすべて unmount する
	// ★注意★
	// 索引を構成するファイルもすべて対象となる

	try {
		loadFile(cTrans_).apply(ApplyFunction2<File, Trans::Transaction&, bool>(&File::unmount, cTrans_, false /* no retain */),
								ApplyFunction1<File, Trans::Transaction&>(&File::mount, cTrans_),
								bUndo_);
		eStatus = Files;

		// シーケンスファイルを unmount する
		unmountSequence(cTrans_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		switch (eStatus) {
		case Files:
			loadFile(cTrans_).apply(ApplyFunction1<File, Trans::Transaction&>(&File::mount, cTrans_));
			// thru.
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Table::flush -- 表を構成するファイルを flush する
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
Table::
flush(Trans::Transaction& cTrans_)
{
	if (isSystem()) return;

	// 表を構成するファイルをすべて flush する
	// ★注意★
	// 索引を構成するファイルもすべて対象となる

	const FileMap& cMap = loadFile(cTrans_);

	{
		AutoRWLock l(getRWLockForFile());
		cMap.apply(ApplyFunction1<File, Trans::Transaction&>(&File::flush, cTrans_));
	}

	// シーケンスファイルを flush する
	flushSequence(cTrans_);
}

//	FUNCTION public
//	Schema::Table::sync -- 不要な版を破棄する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&	trans
//			不要な版を破棄する処理を行う
//			トランザクションのトランザクション記述子
//		bool&				incomplete
//			true
//				今回の同期処理で表を持つ
//				オブジェクトの一部に処理し残しがある
//			false
//				今回の同期処理で表を持つ
//				オブジェクトを完全に処理してきている
//
//				同期処理の結果、表を処理し残したかを設定する
//		bool&				modified
//			true
//				今回の同期処理で表を持つ
//				オブジェクトの一部が既に更新されている
//			false
//				今回の同期処理で表を持つ
//				オブジェクトはまだ更新されていない
//
//				同期処理の結果、表が更新されたかを設定する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
sync(Trans::Transaction& cTrans_, bool& incomplete, bool& modified)
{
	if (isSystem()) return;

	// 表および索引を構成する論理ファイル
	const FileMap& cMap = loadFile(cTrans_);

	{
	AutoRWLock l(getRWLockForFile());

	cMap.apply(ApplyFunction3<File, Trans::Transaction&, bool&, bool&>(&File::sync, cTrans_, incomplete, modified));
	}

	// タプル ID ファイル
	syncSequence(cTrans_, incomplete, modified);
}

//	FUNCTION public
//	Schema::Table::startBackup -- バックアップを開始する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		bool bRestorable_ = true
//		bool bUndo_ = false
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Table::
startBackup(Trans::Transaction& cTrans_, bool bRestorable_, bool bUndo_ /* = false */)
{
	if (isSystem()) return;

	enum {
		None,
		Files
	} eStatus = None;

	// 表を構成するファイルをすべて startBackup する
	// ★注意★
	// 索引を構成するファイルもすべて対象となる
	const FileMap& cMap = loadFile(cTrans_);

	try {
		{
			AutoRWLock l(getRWLockForFile());
			cMap.apply(ApplyFunction2<File, Trans::Transaction&, bool>(&File::startBackup, cTrans_, bRestorable_),
					   ApplyFunction1<File, Trans::Transaction&>(&File::endBackup, cTrans_),
					   bUndo_);
			eStatus = Files;
		}

		// シーケンスファイルを startBackup する
		startBackupSequence(cTrans_, bRestorable_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		switch (eStatus) {
		case Files:
			{
				AutoRWLock l(getRWLockForFile());
				cMap.apply(ApplyFunction1<File, Trans::Transaction&>(&File::endBackup, cTrans_));
			}
			// thru.
		case None:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Table::endBackup -- バックアップを終了する
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
Table::
endBackup(Trans::Transaction& cTrans_)
{
	if (isSystem()) return;

	// 表を構成するファイルをすべて endBackup する
	// ★注意★
	// 索引を構成するファイルもすべて対象となる
	const FileMap& cMap = loadFile(cTrans_);

	// エラーが起きたら自動的に利用不可にする
	Common::AutoCaller1<Database, bool> autoDisabler(getDatabase(cTrans_), &Database::setAvailability, false);
	{
		AutoRWLock l(getRWLockForFile());
		cMap.apply(ApplyFunction1<File, Trans::Transaction&>(&File::endBackup, cTrans_));
	}

	// シーケンスファイルを endBackup する
	endBackupSequence(cTrans_);

	// 成功したのでエラー処理のための構造を解放する
	autoDisabler.release();
}

//	FUNCTION public
//	Schema::Table::recover -- 障害から回復する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		const Trans::TimeStamp& cPoint_
//			回復するポイント
//		const Schema::Object::Name& cDatabaseName_
//			データベース名
//			DROPがUNDOされているか調べるために必要
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Table::
recover(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_,
		const Name& cDatabaseName_)
{
	if (isSystem()) return;

	using namespace Manager::RecoveryUtility;

	// DropまたはCreateがUndoされている表についてはrecoverしない
	if (!Undo::isEntered(cDatabaseName_, getID(), Undo::Type::DropTable)
		&& !Undo::isEntered(cDatabaseName_, getID(), Undo::Type::CreateTable)) {

		// 表を構成するファイルをすべて recover する
		// ★注意★
		// 索引を構成するファイルもすべて対象となる
		loadFile(cTrans_).apply(ApplyFunction2<File, Trans::Transaction&, const Trans::TimeStamp&>
								(&File::recover, cTrans_, cPoint_),
								BoolFunction1<File, const Name&>
								(&_Omit::_fileRecover, cDatabaseName_));

		// シーケンスファイルを recover する
		recoverSequence(cTrans_, cPoint_);
	}
}

//	FUNCTION public
//	Schema::Table::restore --
//		ある時点に開始された版管理するトランザクションが
//		参照する版を最新版とする
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
Table::
restore(Trans::Transaction& cTrans_, const Trans::TimeStamp& cPoint_)
{
	if (isSystem()) return;

	// 表を構成するファイルをすべて restore する
	// ★注意★
	// 索引を構成するファイルもすべて対象となる

	loadFile(cTrans_).apply(ApplyFunction2<File, Trans::Transaction&, const Trans::TimeStamp&>
							(&File::restore, cTrans_, cPoint_));

	// シーケンスファイルを restore する
	restoreSequence(cTrans_, cPoint_);
}

//	FUNCTION public
//	Schema::Table::alterArea -- SQL の表定義変更文からエリア移動により表を変更する準備をする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::AlterTableStatement& statement
//			解析済の SQL の表定義変更文
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
Table::
alterArea(Trans::Transaction& cTrans_,
		  Table& cTable_,
		  const Statement::AlterTableAction& statement,
		  ModVector<ID::Value>& vecPrevAreaID_,
		  ModVector<ID::Value>& vecPostAreaID_,
		  LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (cTable_.isSystem()) return false;

	Database* pDatabase = cTable_.getDatabase(cTrans_);
	if (pDatabase->isSlaveStarted()) {
		// area is not allowed for master database
		_SYDNEY_THROW1(Exception::InvalidAreaSpecification, pDatabase->getName());
	}

	bool bResult = false;
	; _SYDNEY_ASSERT(statement.getAction());

	switch (statement.getActionType()) {
	case Statement::AlterTableAction::SetArea:
		bResult = cTable_.setArea(cTrans_, *(_SYDNEY_DYNAMIC_CAST(Statement::AreaOption*, statement.getAction())),
								  vecPrevAreaID_, vecPostAreaID_);
		break;
	case Statement::AlterTableAction::DropArea:
		bResult = cTable_.dropArea(cTrans_, *(_SYDNEY_DYNAMIC_CAST(Statement::AreaOption*, statement.getAction())),
								   vecPrevAreaID_, vecPostAreaID_);
		break;
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}

	if (bResult) {
		// ログデータを作る
		cTable_.makeLogData(cTrans_, cLogData_);

		// 変更前後のエリアに関する情報をログに追加する
		//   - 変更前エリアID(Undo用)
		//   - 変更後エリアID(Undo用)
		//   - 変更前エリアパス(Undo用)
		//   - 変更後エリアパス(Undo用)
		//	 - 変更前後で移動するファイルの名称(Undo用)

		cLogData_.addIDs(vecPrevAreaID_);
		cLogData_.addIDs(vecPostAreaID_);
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, vecPrevAreaID_));
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, vecPostAreaID_));
		cLogData_.addData(cTable_.getMovedFiles(cTrans_, vecPrevAreaID_, vecPostAreaID_));

		; _SYDNEY_ASSERT(cLogData_.getCount() == Table::Log::Alter::Num);
	}

	return bResult;
}

//	FUNCTION public
//	Schema::Table::alterName -- SQL の表定義変更文から表の名前を変更する準備をする
//
//	NOTES
//
//	ARGUMENTS
//		Statement::AlterTableStatement& statement
//			解析済の SQL の表定義変更文
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
Table::
alterName(Trans::Transaction& cTrans_,
		  Table& cTable_,
		  const Statement::AlterTableAction& statement,
		  Name& cPostName_,
		  LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);
	; _SYDNEY_ASSERT(statement.getActionType() == Statement::AlterTableAction::Rename);

	if (cTable_.isSystem()) return false;

	bool bResult = false;

	; _SYDNEY_ASSERT(_SYDNEY_DYNAMIC_CAST(Statement::Identifier*, statement.getAction())->getIdentifier());
	const ModUnicodeString& cName = *(_SYDNEY_DYNAMIC_CAST(Statement::Identifier*, statement.getAction())->getIdentifier());

	if (cTable_.getName() != cName) {
		// 変更後の名前が使用されていないかをチェックするために一時的にTableオブジェクトを作る
		Database* pDatabase = cTable_.getDatabase(cTrans_);
		Table cAlterredTable(*pDatabase, cName);
		cAlterredTable.setID(cTable_.getID());

		// 作成中のオブジェクトとして登録し、同じ名前のものが登録中でないか調べる
		// ★注意★
		// 最後の引数がtrueなので重複していたら常に例外が飛ぶ
		(void) _Name::_checkExistence(cTrans_, *pDatabase, &cAlterredTable, true /* no cancel */);

		{ // AutoWithdrawのデストラクトがcAlterredTableより前であることを保証するためのスコープ

			// スコープから抜けたら自動的にwithdrawする
			Manager::ObjectName::AutoWithdraw w(&cAlterredTable);

			cPostName_ = cName;
			bResult = true;

			// ログデータを作る
			cTable_.makeLogData(cTrans_, cLogData_);

			// 変更後の名前と、操作時のエリアに関する情報をログに追加する
			// ★注意★
			// 変更前の名前はmakeLogDataで格納されている
			//   - 変更後名称(Undo用)
			//   - 変更時エリアID(Undo用)
			//   - 変更時エリアパス(Undo用)
			//	 - 変更前後で移動するファイルの名称(Undo用)

			cLogData_.addString(cPostName_);
			cLogData_.addIDs(cTable_.getAreaID());
			cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, cTable_.getAreaID()));
			cLogData_.addData(cTable_.getMovedFiles(cTrans_, cPostName_));

			; _SYDNEY_ASSERT(cLogData_.getCount() == Table::Log::Rename::Num);
		}
	}

	return bResult;
}

// FUNCTION public
//	Schema::Table::alterAddColumn -- Prepare altering a table adding columns
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	const Statement::AlterTableAction& cStatement_
//	ModVector<File::Pointer>& vecPrevFiles_
//		Center record and vector file before add column
//	ModVector<File::Pointer>& vecPostFiles_
//		New files created by add column
//	ModVector<Column::Pointer>& vecNewColumns_
//		All the colomuns added by add column
//	ModVector<Field*>& vecSourceField_
//		All the fields copied to new fields
//	ModVector<Field*>& vecTargetField_
//		All the fields newly created
//	LogData& cLogData_
//		logical log data corresponding to the alter table statement
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
alterAddColumn(Trans::Transaction& cTrans_,
			   Table& cTable_,
			   const Statement::AlterTableAction& cStatement_,
			   ModVector<File::Pointer>& vecPrevFiles_,
			   ModVector<File::Pointer>& vecPostFiles_,
			   ModVector<Column::Pointer>& vecNewColumns_,
			   ModVector<Field*>& vecSourceField_,
			   ModVector<Field*>& vecTargetField_,
			   LogData& cLogData_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory() != Trans::Transaction::Category::ReadOnly);
	; _SYDNEY_ASSERT(cStatement_.getAction());
	; _SYDNEY_ASSERT(cStatement_.getActionType() == Statement::AlterTableAction::AddColumn);

	if (cTable_.isSystem()) return false;

	bool bResult = false;

	// create a new centralized record file and vector file
	cTable_.createFileForAddColumn(cTrans_, vecPrevFiles_, vecPostFiles_,
								   vecSourceField_, vecTargetField_);
	; _SYDNEY_ASSERT(vecPrevFiles_.getSize() == vecPostFiles_.getSize());
	; _SYDNEY_ASSERT(vecPrevFiles_.getSize() > 0);

	// center record file is the first element
	File::Pointer pPrevRecordFile = vecPrevFiles_[0];
	File::Pointer pPostRecordFile = vecPostFiles_[0];

	Statement::TableElementList* pList = _SYDNEY_DYNAMIC_CAST(Statement::TableElementList*, cStatement_.getAction());
	; _SYDNEY_ASSERT(pList);
	int n = pList->getCount();
	; _SYDNEY_ASSERT(n);

	vecNewColumns_.reserve(n);

	Column::Position columnPosition = cTable_.loadColumn(cTrans_).getSize();
	// prepare log data for added columns
	ModAutoPointer<Common::DataArrayData> pColumnLogData = new Common::DataArrayData;
	pColumnLogData->reserve(n);

	for (int i = 0; i < n; ++i) {
		Statement::Object* object = pList->getAt(i);
		; _SYDNEY_ASSERT(object);
		; _SYDNEY_ASSERT(object->getType() == Statement::ObjectType::ColumnDefinition);

		// create a new column object and corresponding log data
		Statement::ColumnDefinition* pColumnDef =
			_SYDNEY_DYNAMIC_CAST(Statement::ColumnDefinition*, object);
		Column::Pointer pColumn = cTable_.createColumn(cTrans_, pColumnDef, columnPosition++, *pColumnLogData);
		if (pColumn.get()) {
			// add new column to the return value
			vecNewColumns_.pushBack(pColumn);
			SCHEMA_FAKE_ERROR("Schema::Table", "AddColumn", "ColumnAdded1");
			// if created column has a not-null constraint, that has to have a default value
			if (!pColumn->isNullable() && pColumn->getDefault().isNull()) {
				_SYDNEY_THROW1(Exception::DefaultNeeded, pColumn->getName());
			}

			// create a new field object
			cTable_.addField(cTrans_, *cTable_.getDatabase(cTrans_), *pColumn, pPostRecordFile, &vecPostFiles_);
			// add to result
			vecTargetField_.pushBack(pColumn->getField(cTrans_));

			bResult = true;
			SCHEMA_FAKE_ERROR("Schema::Table", "AddColumn", "FileAdded1");
		}
	}
	SCHEMA_FAKE_ERROR("Schema::Table", "AddColumn", "ColumnAdded");

	if (bResult) {
		// create fileid for the new record
		pPostRecordFile->setFileID(cTrans_);

		// obtain the last objectID for log data
		ID::Value iLastID = ObjectID::getLastValue(cTrans_, cTable_.getDatabase(cTrans_));

		// ファイル定義のログデータ
		ModAutoPointer<Common::DataArrayData> pFileLogData = new Common::DataArrayData;
		cTable_.makeFileLogData(cTrans_, vecPostFiles_, *pFileLogData);

		// create log data
		//   - altered table
		//   - added columns
		//   - original fields and files
		//   - the last id
		//   - array of area ids
		//   - array of area paths
		//   - post files
		//   - file name of original files
		//   - suffix used for saving original files
		//   - file definitions
		Database* pDatabase = cTable_.getDatabase(cTrans_);
		cTable_.makeLogData(cTrans_, cLogData_);
		cLogData_.addData(pColumnLogData.release());
		cLogData_.addID(iLastID);
		cLogData_.addIDs(cTable_.getAreaID());
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, cTable_.getAreaID()));
		cLogData_.addData(cTable_.getCreatedFiles(cTrans_, vecPostFiles_));
		cLogData_.addData(cTable_.getCreatedFiles(cTrans_, vecPrevFiles_));

		// save suffix for saving original files using object's timestamp (8digits)
		{
			const int iBufferSize = 9;
			char buffer[iBufferSize];
			::sprintf(buffer, "%.8x", cTable_.getTimestamp());
			cLogData_.addString(ModUnicodeString(buffer));
		}
		// file logs
		cLogData_.addData(pFileLogData.release());

		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::AlterAddColumn::Num);
	}
	SCHEMA_FAKE_ERROR("Schema::Table", "AddColumn", "LogCreated");

	return bResult;
}

// FUNCTION public
//	Schema::Table::alterAddColumn -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	const LogData& cLogData_
//	ModVector<FilePointer>& vecPrevFiles_
//	ModVector<FilePointer>& vecPostFiles_
//	ModVector<Column::Pointer>& vecNewColumns_
//	ModVector<Field*>& vecSourceField_
//	ModVector<Field*>& vecTargetField_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
alterAddColumn(Trans::Transaction& cTrans_,
			   Table& cTable_,
			   const LogData& cLogData_,
			   ModVector<FilePointer>& vecPrevFiles_,
			   ModVector<FilePointer>& vecPostFiles_,
			   ModVector<Column::Pointer>& vecNewColumns_,
			   ModVector<Field*>& vecSourceField_,
			   ModVector<Field*>& vecTargetField_)
{
	if (cTable_.isSystem()) return false;

	bool bResult = false;

	// create a new centralized record file
	const Common::DataArrayData* pFileLog = 0;
	if (cLogData_.getCount() >= Log::AlterAddColumn::Num1) {
		pFileLog =
			&(cLogData_.getDataArrayData(Log::AlterAddColumn::FileDefinitions));
	}
	cTable_.createFileForAddColumn(cTrans_, vecPrevFiles_, vecPostFiles_,
								   vecSourceField_, vecTargetField_,
								   pFileLog);
	; _SYDNEY_ASSERT(vecPrevFiles_.getSize() == vecPostFiles_.getSize());
	; _SYDNEY_ASSERT(vecPrevFiles_.getSize() > 0);

	// center record file is the first element
	File::Pointer pPrevRecordFile = vecPrevFiles_[0];
	File::Pointer pPostRecordFile = vecPostFiles_[0];

	const Common::DataArrayData* pRecordFileLog = 0;
	if (pFileLog) {
		pRecordFileLog = &(LogData::getDataArrayData(pFileLog->getElement(0)));
	}

	const Common::DataArrayData& cLogColumns = cLogData_.getDataArrayData(Table::Log::AlterAddColumn::ColumnDefinitions);
	int n = cLogColumns.getCount();
	; _SYDNEY_ASSERT(n);

	vecNewColumns_.reserve(n);
	Column::Position columnPosition = cTable_.loadColumn(cTrans_).getSize();

	for (int i = 0; i < n; ++i) {
		const Common::DataArrayData& cLogColumn =
			LogData::getDataArrayData(cLogColumns.getElement(i));

		Column::Pointer pColumn = cTable_.createColumn(cTrans_, cLogColumn, columnPosition++);

		if (pColumn.get()) {
			// add new column to the return value
			vecNewColumns_.pushBack(pColumn);
			// if created column has a not-null constraint, that has to have a default value
			if (!pColumn->isNullable() && pColumn->getDefault().isNull()) {
				_SYDNEY_THROW0(Exception::LogItemCorrupted);
			}
			// create a new field object
			cTable_.addField(cTrans_, *cTable_.getDatabase(cTrans_), *pColumn, pPostRecordFile, &vecPostFiles_,
							 pFileLog, pRecordFileLog);
			// add to result
			vecTargetField_.pushBack(pColumn->getField(cTrans_));

			bResult = true;
		}
	}
	if (bResult) {
		// create fileid for the new record file
		pPostRecordFile->setFileID(cTrans_);
	}

	return bResult;
}

// FUNCTION public
//	Schema::Table::createFileForAddColumn -- get old centralized files(record/vector) and create new corresponding files 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	ModVector<File::Pointer>& vecPrevFiles_
//		Record file and vector file before add column
//	ModVector<File::Pointer>& vecPostFiles_
//		Record file and vector file after add column
//	ModVector<Field*>& vecSourceField_
//		All the fields copied to new fields
//	ModVector<Field*>& vecTargetField_
//		All the fields newly created
//	const Common::DataArrayData* pFileLogData_ /* = 0 */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
createFileForAddColumn(Trans::Transaction& cTrans_,
					   ModVector<File::Pointer>& vecPrevFiles_,
					   ModVector<File::Pointer>& vecPostFiles_,
					   ModVector<Field*>& vecSourceField_,
					   ModVector<Field*>& vecTargetField_,
					   const Common::DataArrayData* pFileLogData_ /* = 0 */)
{
	; _SYDNEY_ASSERT(!isSystem());
	; _SYDNEY_ASSERT(vecPrevFiles_.isEmpty());
	; _SYDNEY_ASSERT(vecPostFiles_.isEmpty());
	; _SYDNEY_ASSERT(vecSourceField_.isEmpty());
	; _SYDNEY_ASSERT(vecTargetField_.isEmpty());

	// get the centralized record file (= the file which includes rowid's field)
	; _SYDNEY_ASSERT(getTupleID(cTrans_));
	; _SYDNEY_ASSERT(getTupleID(cTrans_)->getField(cTrans_));
	; _SYDNEY_ASSERT(getTupleID(cTrans_)->getField(cTrans_)->getFile(cTrans_));
	File::Pointer pPrevCenterFile = getFieldFile(*getTupleID(cTrans_)->getField(cTrans_), cTrans_);
	vecPrevFiles_.pushBack(pPrevCenterFile);

	// for now, center file must be a record file.
	// get objectId field for following process.
	; _SYDNEY_ASSERT(pPrevCenterFile->getCategory() == File::Category::Record);  
	Field* pOID = pPrevCenterFile->getObjectID(cTrans_);

	// create new record file
	const Common::DataArrayData* pRecordLog = 0;
	if (pFileLogData_) {
		ModSize iPosition = vecPostFiles_.getSize();
		if (pFileLogData_->getCount() > iPosition) {
			pRecordLog =
				&(LogData::getDataArrayData(pFileLogData_->getElement(iPosition)));
		}
	}
	File::Pointer pPostCenterFile = File::createForAlter(cTrans_, *pPrevCenterFile, *this,
														 pRecordLog);

	//***
	// add corresponding old-new fields to results
	//***

	// OID field is not needed to be imported
	// Add other fields from original record file.
	const ModVector<Field*>& vecPrevCenterFields = pPrevCenterFile->getField(cTrans_);
	ModSize n = vecPrevCenterFields.getSize();
	vecSourceField_.reserve(n - 1);
	vecTargetField_.reserve(n); // in most cases, a new column will be added
	for (ModSize i = 1; i < n; ++i) {
		Field* pPrevField = vecPrevCenterFields[i];
		if (!pPrevField->isFunction()) { // skip function field
			Field::Pointer pPostField =
				pPostCenterFile->addField(*pPrevField, cTrans_,
										  pPostCenterFile->getNextFieldID(cTrans_, pRecordLog));
			vecSourceField_.pushBack(pPrevField);
			vecTargetField_.pushBack(pPostField.get());
		}
	}
	// (fileID should be created after all the columns are added)
	// pPostCenterFile->setFileID(cTrans_);
	// add the file into result vector
	vecPostFiles_.pushBack(pPostCenterFile);
	SCHEMA_FAKE_ERROR("Schema::Table", "AddColumn", "Created");

	// create new files corresponding to the files
	// which hold a destination of the OID
	const ModVector<Field*>& vecOIDDestination = pOID->getDestination(cTrans_);
	if (!vecOIDDestination.isEmpty()) {
		ModVector<Field*>::ConstIterator iterator = vecOIDDestination.begin();
		const ModVector<Field*>::ConstIterator last = vecOIDDestination.end();
		do {
			; _SYDNEY_ASSERT((*iterator)->getFile(cTrans_));
			File::Pointer pPrevFile = getFieldFile(*(*iterator), cTrans_);
			vecPrevFiles_.pushBack(pPrevFile);

			// create new file
			const Common::DataArrayData* pDestLog = 0;
			if (pFileLogData_) {
				ModSize iPosition = vecPostFiles_.getSize();
				if (pFileLogData_->getCount() > iPosition) {
					pDestLog =
						&(LogData::getDataArrayData(pFileLogData_->getElement(iPosition)));
				}
			}
			File::Pointer pPostFile = File::createForAlter(cTrans_, *pPrevFile, *this,
														   pDestLog);

			// create new fields
			const ModVector<Field*>& vecPrevFields = pPrevFile->getField(cTrans_);
			ModSize n = vecPrevFields.getSize();
			for (ModSize i = 0; i < n; ++i) {
				Field* pPrevField = vecPrevFields[i];
				Field::Pointer pPostField;
				// check the original source field
				Field* pPrevSource = pPrevField->getSource(cTrans_);
				if (pPrevSource && pPrevSource->getParentID() == pPrevCenterFile->getID()) {
					// set source ID pointing to the new center file
					Field* pPostSource = pPostCenterFile->getFieldByPosition(pPrevSource->getPosition(), cTrans_);
					pPostField = pPostFile->addField(pPrevField->getCategory(), Field::Permission::All,
													 *pPostSource, cTrans_,
													 pPostFile->getNextFieldID(cTrans_, pDestLog));
					// SourceField is null
					// because the value of the field will be inserted from the new center file
					vecSourceField_.pushBack(0);
					vecTargetField_.pushBack(pPostField.get());

				} else if (!pPrevField->isFunction()) { // skip function field
					// copy from original file
					pPostField = pPostFile->addField(*pPrevField, cTrans_,
													 pPostFile->getNextFieldID(cTrans_, pDestLog));
					vecSourceField_.pushBack(pPrevField);
					vecTargetField_.pushBack(pPostField.get());
				}
			}
			// create fileID for the file
			pPostFile->setFileID(cTrans_);
			// add to result
			vecPostFiles_.pushBack(pPostFile);
		} while (++iterator != last);
	}
	SCHEMA_FAKE_ERROR("Schema::Table", "AddColumn", "FieldAdded");
}

// FUNCTION public
//	Schema::Table::alterAlterColumn -- Prepare altering a table altering a column
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	const Statement::AlterTableAction& cStatement_
//	ColumnPointer& pPrevColumn_
//	ColumnPointer& pPostColumn_
//	LogData& cLogData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
alterAlterColumn(Trans::Transaction& cTrans_,
				 Table& cTable_,
				 const Statement::AlterTableAction& cStatement_,
				 ColumnPointer& pPrevColumn_,
				 ColumnPointer& pPostColumn_,
				 LogData& cLogData_)
{
	if (cTable_.isSystem()) return false;

	; _SYDNEY_ASSERT(cTrans_.getCategory() != Trans::Transaction::Category::ReadOnly);
	; _SYDNEY_ASSERT(cStatement_.getAction());
	; _SYDNEY_ASSERT(cStatement_.getActionType() == Statement::AlterTableAction::AlterColumn);

	bool bResult = false;

	Statement::ColumnDefinition* pColDef = _SYDNEY_DYNAMIC_CAST(Statement::ColumnDefinition*, cStatement_.getAction());
	; _SYDNEY_ASSERT(pColDef);

	// search for the column which has the name specified in the ColumnDefinition
	Statement::Identifier* pIdentifier = pColDef->getName();
	; _SYDNEY_ASSERT(pIdentifier);
	; _SYDNEY_ASSERT(pIdentifier->getIdentifier());

	Column* pTargetColumn = cTable_.getColumn(*pIdentifier->getIdentifier(), cTrans_);
	if (!pTargetColumn) {
		_SYDNEY_THROW1(Exception::ColumnNotFound, *pIdentifier->getIdentifier());
	}

	// If any index is defined on the column, the column cannot be alterred
	if (!pTargetColumn->getKey(cTrans_).isEmpty()) {
		SydInfoMessage
			<< "Column " << pTargetColumn->getName() << " has an index, cannot be altered"
			<< ModEndl;
		_SYDNEY_THROW1(Exception::OtherObjectDepending, pTargetColumn->getName());
	}

	// check whether new column type can be altered
	if (pTargetColumn->isAbleToAlter(*pColDef, pPostColumn_)) {

		; _SYDNEY_ASSERT(pPostColumn_.get());
		; _SYDNEY_ASSERT(pTargetColumn->getFileObjectID());

		pPostColumn_->setFileObjectID(*pTargetColumn->getFileObjectID());

		// set the previous column object
		pPrevColumn_ = cTable_.loadColumn(cTrans_).get(pTargetColumn->getID());
		; _SYDNEY_ASSERT(pPrevColumn_.get());

		bResult = true;

		// log data for the column definition
		ModAutoPointer<Common::DataArrayData> pPostColumnLogData = new Common::DataArrayData;
		pPostColumn_->makeLogData(*pPostColumnLogData);
		pPostColumn_->touch();

		// create common log data
		cTable_.makeLogData(cTrans_, cLogData_);

		// add log data special for 'alter column'
		//   - target column name
		//   - post column definition
		cLogData_.addString(pPostColumn_->getName());
		cLogData_.addData(pPostColumnLogData.release());

		; _SYDNEY_ASSERT(cLogData_.getCount() == Table::Log::AlterAlterColumn::Num);
	}

	return bResult;
}

// FUNCTION private
//	Schema::Table::alterAlterColumn -- from logdata
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	const LogData& cLogData_
//	ColumnPointer& pPrevColumn_
//	ColumnPointer& pPostColumn_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
alterAlterColumn(Trans::Transaction& cTrans_,
				 Table& cTable_,
				 const LogData& cLogData_,
				 ColumnPointer& pPrevColumn_,
				 ColumnPointer& pPostColumn_)
{
	if (cTable_.isSystem()) return false;

	bool bResult = false;

	// search for the column which has the name specified in the logdata
	Name cName = cLogData_.getString(Table::Log::AlterAlterColumn::TargetColumnName);
	Column* pTargetColumn = cTable_.getColumn(cName, cTrans_);
	if (!pTargetColumn) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// If any index is defined on the column, the column cannot be alterred
	if (!pTargetColumn->getKey(cTrans_).isEmpty()) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}

	// create post column from log data
	const Common::DataArrayData& cLogColumn = cLogData_.getDataArrayData(Table::Log::AlterAlterColumn::PostColumnDefinition);
	// use same position
	Column::Position columnPosition = pTargetColumn->getPosition();
	pPostColumn_ = cTable_.createColumn(cTrans_, cLogColumn, columnPosition, pTargetColumn);
	; _SYDNEY_ASSERT(pPostColumn_.get());
	; _SYDNEY_ASSERT(pTargetColumn->getFileObjectID());
	pPostColumn_->setFileObjectID(*pTargetColumn->getFileObjectID());
	pPostColumn_->touch();

	// set the previous column object
	pPrevColumn_ = cTable_.loadColumn(cTrans_).get(pTargetColumn->getID());
	; _SYDNEY_ASSERT(pPrevColumn_.get());

	return true;
}

// FUNCTION public
//	Schema::Table::alterAddConstraint -- Prepare altering a table adding a constraint
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	const Statement::AlterTableAction& cStatement_
//	Constraint::Pointer& pConstraint_
//	ModVector<Table*>& vecReferencedTable_
//	LogData& cLogData_
//		logical log data corresponding to the alter table statement
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
alterAddConstraint(Trans::Transaction& cTrans_,
				   Table& cTable_,
				   const Statement::AlterTableAction& cStatement_,
				   Constraint::Pointer& pConstraint_,
				   ModVector<Table*>& vecReferencedTable_,
				   LogData& cLogData_)
{
	if (cTable_.isSystem()) return false;

	; _SYDNEY_ASSERT(cTrans_.getCategory() != Trans::Transaction::Category::ReadOnly);
	; _SYDNEY_ASSERT(cStatement_.getAction());
	; _SYDNEY_ASSERT(cStatement_.getActionType() == Statement::AlterTableAction::AddTableConstraint);

	Statement::TableConstraintDefinition* pDef =
		_SYDNEY_DYNAMIC_CAST(Statement::TableConstraintDefinition*, cStatement_.getAction());
	; _SYDNEY_ASSERT(pDef);

	Constraint::Position iPosition = cTable_.loadConstraint(cTrans_).getSize();
	// prepare log data for added constraint
	ModAutoPointer<Common::DataArrayData> pLogData = new Common::DataArrayData;
	ModVector<File::Pointer> vecCreatedFiles;
	// create a new constraint object and corresponding log data
	pConstraint_ = cTable_.createConstraint(cTrans_, pDef, iPosition++, *pLogData);
	SCHEMA_FAKE_ERROR("Schema::Table", "AddConstraint", "ConstraintAdded");

	; _SYDNEY_ASSERT(pConstraint_->getIndex(cTrans_));
	; _SYDNEY_ASSERT(pConstraint_->getIndex(cTrans_)->getFile(cTrans_));
	const File* pConstraintFile = pConstraint_->getIndex(cTrans_)->getFile(cTrans_);
	vecCreatedFiles.pushBack(pConstraintFile); // use vector for future expansion

	ID::Value iID = pConstraint_->getReferedTableID();
	if (iID != ID::Invalid) {
		Table* pReferencedTable = Table::get(iID, cTable_.getDatabase(cTrans_), cTrans_);
		if (pReferencedTable) {
			vecReferencedTable_.pushBack(pReferencedTable);
		}
	}

	// obtain the last objectID for log data
	ID::Value iLastID = ObjectID::getLastValue(cTrans_, cTable_.getDatabase(cTrans_));

	// ファイル定義のログデータ
	ModAutoPointer<Common::DataArrayData> pFileLogData = new Common::DataArrayData;
	cTable_.makeFileLogData(cTrans_, vecCreatedFiles, *pFileLogData);

	// create log data
	//   - altered table
	//   - added constraints
	//   - the last id
	//   - array of area ids
	//   - array of area paths
	//   - created files
	Database* pDatabase = cTable_.getDatabase(cTrans_);
	cTable_.makeLogData(cTrans_, cLogData_);
	cLogData_.addData(pLogData.release());
	cLogData_.addID(iLastID);
	cLogData_.addIDs(cTable_.getAreaID());
	cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, cTable_.getAreaID()));
	cLogData_.addData(cTable_.getCreatedFiles(cTrans_, vecCreatedFiles));
	// file logs
	cLogData_.addData(pFileLogData.release());
	; _SYDNEY_ASSERT(cLogData_.getCount() == Log::AlterAddConstraint::Num);

	SCHEMA_FAKE_ERROR("Schema::Table", "AddConstraint", "LogCreated");

	return true;
}

// FUNCTION private
//	Schema::Table::alterAddConstraint -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	Constraint::Pointer& pConstraint_
//	ModVector<Table*>& vecReferencedTable_
//	const LogData& cLogData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
alterAddConstraint(Trans::Transaction& cTrans_,
				   Table& cTable_,
				   Constraint::Pointer& pConstraint_,
				   ModVector<Table*>& vecReferencedTable_,
				   const LogData& cLogData_)
{
	if (cTable_.isSystem()) return false;

	bool bResult = false;

	// get constraint log data
	const Common::DataArrayData& cLog =
		cLogData_.getDataArrayData(Log::AlterAddConstraint::ConstraintDefinitions);

	// get new constraint position and load existing constraints
	Constraint::Position iPosition = cTable_.loadConstraint(cTrans_).getSize();

	int n = cLog.getCount();
	if (n != 1) {
		// illegal log data
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	const Common::DataArrayData& cLogConstraint =
		LogData::getDataArrayData(cLog.getElement(0));

	// create constraint from the log data
	pConstraint_ = cTable_.createConstraint(cTrans_, cLogConstraint, iPosition++);
	ID::Value iID = pConstraint_->getReferedTableID();
	if (iID != ID::Invalid) {
		Table* pReferencedTable = Table::get(iID, cTable_.getDatabase(cTrans_), cTrans_);
		if (pReferencedTable) {
			vecReferencedTable_.pushBack(pReferencedTable);
		}
	}
	return true;
}

//	FUNCTION public
//	Schema::Table::move -- 表を実際に変更する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//      const ModVector<ID::Value>& vecpPrevAreaID_
//			変更前のエリアID配列
//      const ModVector<ID::Value>& vecpPostAreaID_
//			変更後のエリアID配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
move(Trans::Transaction& cTrans_,
	 const ModVector<ID::Value>& vecPrevAreaID_,
	 const ModVector<ID::Value>& vecPostAreaID_,
	 bool bUndo_, bool bRecovery_, bool bMount_)
{
	; _SYDNEY_ASSERT(!isSystem());

	int cat = 0;
	// 移動に関与するオブジェクトを覚えるための配列
	ModVector<AreaContent*> vecContent(AreaCategory::ValueNum, 0);
	ModVector<Area*> vecPrevArea(AreaCategory::ValueNum, 0);
	ModVector<Area*> vecPostArea(AreaCategory::ValueNum, 0);
	ModVector<bool> vecMoved(AreaCategory::ValueNum, false);

	Database* pDatabase = getDatabase(cTrans_);

	try {

		ModVector<ID::Value> vecChangeArea;		// 変更エリアリスト
		ModVector<ID::Value> vecNonEmpty;		// 使用中エリアリスト

		for ( ; cat < AreaCategory::ValueNum; cat++) {

			AreaCategory::Value eCat = static_cast<AreaCategory::Value>(cat);

			// 実際にファイルに適用されるエリアIDを得る
			ID::Value iPrevID = _Area::_getEffectiveID(vecPrevAreaID_, eCat);
			ID::Value iPostID = _Area::_getEffectiveID(vecPostAreaID_, eCat);

			// 指定に変更のあるもののみ処理する
			if (vecPrevAreaID_[cat] != vecPostAreaID_[cat]) {

				// 変更前後のIDからエリアを表すオブジェクトを得る
				Area* pPostArea = pDatabase->getArea(vecPostAreaID_[cat], cTrans_);
				vecPostArea[cat] = pPostArea;

				Area* pPrevArea = pDatabase->getArea(vecPrevAreaID_[cat], cTrans_);
				vecPrevArea[cat] = pPrevArea;

				// エリア格納関係を変更する
				vecContent[cat] =
					AreaContent::moveArea(cTrans_, pPrevArea, pPostArea,
										  this, eCat, bUndo_, bRecovery_, bMount_);
				SCHEMA_FAKE_ERROR("Schema::Table", "Move", "AreaContent");

				if (bMount_) {
					// MountでのDROP AREAに対応した変更では
					// パスを削除しないので使用中エリアのチェックはしない
				} else {
					// 移動後のエリアは使用中になる
					// (実際にファイルに適用されるエリアIDを使う)
					if (vecNonEmpty.find(iPostID) == vecNonEmpty.end())
						vecNonEmpty.pushBack(iPostID);
				}

				// ファイルを移動する
				// 索引で個別にエリアが指定されている場合はそのエリアIDが
				// vecNonEmptyに入る
				moveArea(cTrans_, eCat, iPrevID, iPostID,
						 vecPrevAreaID_, vecPostAreaID_,
						 &vecNonEmpty, bUndo_, bRecovery_, bMount_);
				vecMoved[cat] = true;
				SCHEMA_FAKE_ERROR("Schema::Table", "Move", "Area");

				// 変更エリアリストに追加
				// (実際にファイルに適用されるエリアIDを使う)
				if (vecChangeArea.find(iPrevID) == vecChangeArea.end())
					vecChangeArea.pushBack(iPrevID);

				// 変更後のエリア指定を設定する
				m_veciAreaID[cat] = vecPostAreaID_[cat];

			} else if (iPrevID == iPostID) {
				if (bMount_) {
					// MountでのDROP AREAに対応した変更では
					// パスを削除しないので使用中エリアのチェックはしない
				} else {
					// 変更のないエリアは使用中として登録する
					// (実際にファイルに適用されるエリアIDを使う)
					if (vecNonEmpty.find(iPrevID) == vecNonEmpty.end())
						vecNonEmpty.pushBack(iPrevID);
				}
			}
		}
		// 下位の関数にあるFakeErrorは呼ばれないかもしれないのでここでも設定しておく
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "Directory");
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "Moved");
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "Removed");
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "SetPath");
		

		// 変更されたエリアがあるならエリアオブジェクトの配列を初期化
		if ( vecChangeArea.getSize() > 0 )
			resetArea();

		if (bMount_) {
			// MountでのDROP AREAに対応した変更では
			// パスを削除しない
		} else {
			// 使用していないパスがあれば削除する
			ModSize n = vecNonEmpty.getSize();
			for (ModSize i = 0; i < n; ++i) {
				ModVector<ID::Value>::Iterator iterator = vecChangeArea.find(vecNonEmpty[i]);
				if (iterator != vecChangeArea.end()) {
					// 空でないエリアとして登録されているIDを対象から除外する
					vecChangeArea.erase(iterator);
				}
			}
			// 使用していないパス以下の表に関係するディレクトリーを削除する
			sweepArea(cTrans_, vecChangeArea, getName());
			SCHEMA_FAKE_ERROR("Schema::Table", "Move", "Sweep");
		}
	}
	catch ( ... ) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		for ( ; cat >= 0; cat-- ) {
			if (vecPrevAreaID_[cat] != vecPostAreaID_[cat]) {
				AreaCategory::Value eCat = static_cast<AreaCategory::Value>(cat);
				// エリアIDの設定を戻す
				m_veciAreaID[cat] = vecPrevAreaID_[cat];
				if (vecMoved[cat]) {
					// 元の位置に戻す
					ID::Value iPrevID = _Area::_getEffectiveID(vecPrevAreaID_, eCat);
					ID::Value iPostID = _Area::_getEffectiveID(vecPostAreaID_, eCat);
					moveArea(cTrans_, eCat, iPostID, iPrevID, vecPostAreaID_, vecPrevAreaID_,
							 0, true /* undo */, bRecovery_, bMount_);
				} else {
					// 移動していない場合、エリアにごみが残っている可能性があるので調べる
					ID::Value iPostID = _Area::_getEffectiveID(vecPostAreaID_, eCat);
					sweepUnusedArea(cTrans_, iPostID);
				}
				if (vecContent[cat]) {
					// 格納関係に対する変更をUNDOする
					AreaContent::undoMoveArea(cTrans_, vecPrevArea[cat], vecPostArea[cat],
											  this, eCat, vecContent[cat]);
				}
			}
		}
		untouch();

		// 使用していないパスの削除のエラー処理は使用する際に
		// ディレクトリが自動的に作成される為、特に行わない

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Table::sweepArea
//		-- Delete all the directories concerning a table which are not used because of moving files.
//
//	NOTES
//		Assuming all the directories are confirmed to be empty by the caller.
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			Transaction descriptor
//		const ModVector<ID::Value>
//			Array of area IDs to be sweeped
//		const Name& cName_
//			Table name
//	RETURN
//		Nothing
//
//	EXCEPTIONS
void
Table::
sweepArea(Trans::Transaction& cTrans_, const ModVector<ID::Value>& areaID_, const Name& cName_)
{
	; _SYDNEY_ASSERT(!isSystem());

	ModVector<ID::Value>::ConstIterator iterator = areaID_.begin();
	const ModVector<ID::Value>::ConstIterator& end = areaID_.end();

	Database* pDatabase = getDatabase(cTrans_);
	; _SYDNEY_ASSERT(pDatabase);

	for (; iterator != end; ++iterator) {
		if (*iterator == ID::Invalid) {
			// No area specification means using default path.
			Os::Path path(pDatabase->getDataPath());
			path.addPart(cName_);

			Utility::File::rmAll(path);

		} else {
			Area* pArea = pDatabase->getArea(*iterator, cTrans_);
			if (pArea) {
				ModSize n = pArea->getSize();
				for (ModSize i = 0; i < n; ++i) {
					Os::Path path(pArea->getPath(i));
					path.addPart(cName_);
					Utility::File::rmAll(path);
				}
			}
		}
	}
}

// FUNCTION private
//	Schema::Table::alterColumn -- Replace the altered column
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const ColumnPointer& pPrevColumn_
//	const ColumnPointer& pPostColumn_
//	bool bUndo_ /* = false */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
alterColumn(Trans::Transaction& cTrans_,
			const ColumnPointer& pPrevColumn_,
			const ColumnPointer& pPostColumn_,
			bool bUndo_ /* = false */)
{
	; _SYDNEY_ASSERT(!isSystem());

	// obtain all the related Field object
	Field* pColumnField = pPostColumn_->getField(cTrans_);
	const ModVector<Field*>& vecRelatedField = pColumnField->getDestination(cTrans_);
	ModSize n = vecRelatedField.getSize();

	File* pColumnFile = pColumnField->getFile(cTrans_);

	// processing status
	enum {
		None,							// initial value
		ColumnField,					// column field has changed
		ColumnFile,						// column field's file has changed
		ColumnErased,					// old column has been erased from the table
		ColumnAdded,					// new column has been added to the table
	} eStatus = None;
	ModSize i = 0;

	try {
		// set type of field
		pColumnField->setType(*pPostColumn_);
		pColumnField->touch();
		eStatus = ColumnField;
		SCHEMA_FAKE_ERROR("Schema::Table", "AlterColumn", "Field");

		pColumnFile->alterField(cTrans_, pColumnField);
		pColumnFile->touch();
		eStatus = ColumnFile;
		SCHEMA_FAKE_ERROR("Schema::Table", "AlterColumn", "File");

		for (; i < n;) {
			// change related field
			Field* pField = vecRelatedField[i++];
			pField->setType(*pPostColumn_);
			pField->touch();
			// change related file
			File* pFile = pField->getFile(cTrans_);
			pFile->alterField(cTrans_, pField);
			pFile->touch();
		}
		SCHEMA_FAKE_ERROR("Schema::Table", "AlterColumn", "RelatedFiles");

		// replace the column
		eraseColumn(pPrevColumn_->getID());
		eStatus = ColumnErased;
		SCHEMA_FAKE_ERROR("Schema::Table", "AlterColumn", "Erased");
		addColumn(pPostColumn_, cTrans_);
		eStatus = ColumnAdded;
		SCHEMA_FAKE_ERROR("Schema::Table", "AlterColumn", "Added");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());
		switch (eStatus) {
		case ColumnAdded:
			{
				eraseColumn(pPostColumn_->getID());
				// thru.
			}
		case ColumnErased:
			{
				addColumn(pPrevColumn_, cTrans_);
				// thru.
			}
		case ColumnFile:
			{
				if (i > 0) {
					// any field has processed -> undo that
					for (ModSize iErr = 0; iErr < i; ++iErr) {
						Field* pField = vecRelatedField[iErr];
						pField->setType(*pPrevColumn_);
						pField->untouch();
						File* pFile = pField->getFile(cTrans_);
						pFile->alterField(cTrans_, pField);
						pFile->untouch();
					}
				}
				pColumnFile->alterField(cTrans_, pColumnField);
				pColumnFile->untouch();
				// thru.
			}
		case ColumnField:
			{
				pColumnField->setType(*pPrevColumn_);
				pColumnField->untouch();
				// thru.
			}
		default:
			{
				break;
			}
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Table::get -- あるスキーマオブジェクト ID の表を表すクラスを得る
//
//	NOTES
//		使用するキャッシュを明示的に与える
//
//	ARGUMENTS
//		Schema::Object::ID::Value		id
//			表のスキーマオブジェクト ID
//		Schema::Database* pDatabase_
//			得たい表が属するデータベースのオブジェクトID
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		bool bInternal_ = false
//			trueのときUndo情報でDropされていても結果に含める
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の表は存在しない
//
//	EXCEPTIONS

// static
Table*
Table::
get(ID::Value id_, Database* pDatabase_, Trans::Transaction& cTrans_, bool bInternal_ /* = false */)
{
	// Tableはデータベース直下のオブジェクトなのでObjectTemplateを使えない
	if (id_ == Object::ID::Invalid)
		return 0;

	Table* table = 0;

	// まず、与えられたスキーマオブジェクト ID の
	// オブジェクトを表すクラスがデータベースに登録されていないか調べる

	Database* pDatabase =
		(pDatabase_) ? pDatabase_
		: Database::getTemporary(cTrans_);

	if (pDatabase) {
		table = pDatabase->getTable(id_, cTrans_, bInternal_);
	}

#ifdef OBSOLETE /* すべてのデータベースを調べる機能は現在使われていない */
	if (!table) {

		// 見つからなかった場合、
		// データベースの指定が0のときのみ
		// すべての永続データベースについてシステム表を検索する

		if (!pDatabase_) {

			// すべてのデータベースについて見つかるまで調べる
			// このときデータベースのIDが指定されたIDより大きい場合
			// それに含まれることはありえないので処理をとばす

			const ModVector<Database*>& database =
				Manager::ObjectTree::Database::get(cTrans_);

			ModSize n = database.getSize();
			for (ModSize i = 0; i < n; i++) {
				if (database[i] && database[i]->getID() < id_)
					if (table = database[i]->getTable(id_, cTrans_, bInternal_))
						break;
			}
		}
	}
#endif

	return table;
}

//	FUNCTION public
//	Schema::Table::get -- あるスキーマオブジェクト ID の表を表すクラスを得る
//
//	NOTES
//		使用するキャッシュを明示的に与える
//
//	ARGUMENTS
//		Schema::Object::ID::Value		id
//			表のスキーマオブジェクト ID
//		Schema::Object::ID::Value iDatabaseID_
//			得たい表が属するデータベースのオブジェクトID
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の表は存在しない
//
//	EXCEPTIONS

// static
Table*
Table::
get(ID::Value id_, ID::Value iDatabaseID_, Trans::Transaction& cTrans_, bool bInternal_ /* = false */)
{
	if (id_ == Object::ID::Invalid)
		return 0;

	return get(id_, Database::get(iDatabaseID_, cTrans_), cTrans_, bInternal_);
}

//	FUNCTION public
//	Schema::Table::isValid -- 陳腐化していないか
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
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
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
Table::
isValid(ID::Value iID_, ID::Value iDatabaseID_, Timestamp iTimestamp_,
		Trans::Transaction& cTrans_)
{
	Table* pTable = get(iID_, iDatabaseID_, cTrans_);

	return (pTable && pTable->getTimestamp() == iTimestamp_);
}

//	FUNCTION public
//	Schema::Table::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		Schema::Table* pTable_
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
Table::
doBeforePersist(const Pointer& pTable_, Status::Value eStatus_, bool bNeedToErase_, Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pTable_.get());

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::Changed:
	case Status::DeletedInRecovery:
	case Status::CreateCanceled:
	case Status::DeleteCanceled:
	{
		// 何もしない
		break;
	}
	case Status::Deleted:
	{
		// 表を構成するファイル、ディレクトリーを破棄する
		// ★注意★
		// ・エラーが起きてもログの内容から再実行できるように
		//   ファイルやディレクトリーを実際に「消す」操作は
		//   システム表から消す操作を永続化する前に行う
		
		pTable_->destroy(cTrans_, bNeedToErase_);

		break;
	}
	default:
		// 何もしない
		break;
	}
}

//	FUNCTION public
//	Schema::Table::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		Schema::Table* pTable_
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
Table::
doAfterPersist(const Pointer& pTable_, Status::Value eStatus_, bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pTable_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	const ObjectID::Value	dbId = pTable_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// データベースを表すクラスに、この表を表すクラスを登録する

		Database* pDatabase = pTable_->getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);

		// 表はキャッシュに入れない
		pDatabase->addTable(pTable_, cTrans_);
		break;
	}
	case Status::Changed:
	case Status::CreateCanceled:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{

		// 変更が削除だったらキャッシュやデータベースの登録からの削除も行う

		// 状態を「実際に削除された」にする

		pTable_->setStatus(Schema::Object::Status::ReallyDeleted);

		if (bNeedToErase_) {

			// ★注意★
			// タプルIDのシーケンスファイルをこの処理で消すこともできるが
			// 上記dropはファイルを消すほかにページのデタッチもしている

			Database* pDatabase = pTable_->getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			// resetはeraseTableに含まれるようになった
			// // 下位オブジェクトがあればそれを抹消してからdeleteする
			// pTable_->reset(*pDatabase);

			// データベースの登録から抹消する
			// 同時にオブジェクトが破棄される

			ObjectID::Value	iTableID = pTable_->getID();
			pDatabase->eraseTable(iTableID);

			// すべてのスナップショットから登録を抹消する
			Manager::ObjectSnapshot::eraseTable(dbId, iTableID);
		}
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbId, Object::Category::Table);
}

//	FUNCTION public
//	Schema::Table::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::TablePointer& pTable_
//			永続化したエリアのオブジェクト
//		Schema::Database& cDatabase_
//			エリアが属するデータベース
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
Table::
doAfterLoad(const Pointer& pTable_, Database& cDatabase_, Trans::Transaction& cTrans_)
{
	// UNDO情報に最終的なエリアID割り当てが登録されているときはそれに置き換える
	// 名前についても同様の処理を行う
	pTable_->checkUndo(cDatabase_, pTable_->getID());

	// データベースへ読み出した表を表すクラスを追加する
	// 表はキャッシュに入れない
	cDatabase_.addTable(pTable_, cTrans_);
}

//	FUNCTION public
//	Schema::Table::isToBeTemporary -- 表が一時表になるか
//
//	NOTES
//		createやdropに先立って対象の表が一時表かどうかを判定する
//
//	ARGUMENTS
//		const Statement::Identifier* pSt_
//			対象の表名を表すSQL構文要素
//
//	RETURN
//		trueの場合
//			この表は一時表として作られるまたは作られているものである
//		falseの場合
//			この表は永続表として作られるまたは作られているものである
//
//	EXCEPTIONS
//		なし

// static
bool
Table::
isToBeTemporary(const Statement::Identifier* pStatement_)
{
	; _SYDNEY_ASSERT(pStatement_);
	; _SYDNEY_ASSERT(pStatement_->getIdentifier());

	return isToBeTemporary(*pStatement_->getIdentifier());
}

// FUNCTION public
//	Schema::Table::isToBeTemporary -- 
//
// NOTES
//
// ARGUMENTS
//	const ModUnicodeString& cstrName_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Table::
isToBeTemporary(const ModUnicodeString& cstrName_)
{
	// 名前の先頭が'#'なら一時表である
	return cstrName_.getLength() > 0
		&& cstrName_[0] == NameParts::Table::Temporary;
}

//	FUNCTION public
//	Schema::Table::reset --
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
Table::
reset(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	if (_columns) {
		resetColumn(cDatabase_);
		delete _columns, _columns = 0;
	}
	if (_constraints) {
		resetConstraint(cDatabase_);
		delete _constraints, _constraints = 0;
	}
	if (_indices) {
		resetIndex(cDatabase_);
		delete _indices, _indices = 0;
	}
	if (_files) {
		resetFile(cDatabase_);
		delete _files, _files = 0;
	}
	if (m_vecpArea)
		clearArea();
}

//	FUNCTION public
//	Schema::Table::setTemporary
//		表の一時表属性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool
//			true  : 一時表である
//			false : 一時表でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Table::
setTemporary(bool bTemporary_)
{
	m_bTemporary = bTemporary_;
}

//	FUNCTION public
//	Schema::Table::isTemporary
//		表の一時表属性を取得する
//
//	NOTES
//		なし
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			true  : 一時表である
//			false : 一時表でない
//
//	EXCEPTIONS
//		なし

bool
Table::
isTemporary() const
{
	return m_bTemporary;
}

//	FUNCTION public
//	Schema::Table::setSystem
//		表のシステム表属性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool
//			true  : システム表である
//			false : システム表でない
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Table::
setSystem(bool bSystem_)
{
	m_bSystem = bSystem_;
	setScope(Scope::Meta);
}

//	FUNCTION public
//	Schema::Table::isSystem
//		表のシステム表属性を取得する
//
//	NOTES
//		なし
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		bool
//			true  : システム表である
//			false : システム表でない
//
//	EXCEPTIONS
//		なし

bool
Table::
isSystem() const
{
	return m_bSystem;
}

// FUNCTION public
//	Schema::Table::hasReferencingTable -- get whether referencing tables exist
//
// NOTES
//	referencinig table is a table which have a foreign key constraint referencing to this table
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Table::
hasReferencingTable(Trans::Transaction& cTrans_)
{
	loadConstraint(cTrans_);
	return m_mapReferencingTable.isEmpty() == ModFalse;
}

// FUNCTION public
//	Schema::Table::getReferencingTable -- get referencing tables
//
// NOTES
//	referencinig table is a table which have a foreign key constraint referencing to this table
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	const ModVector<Table*>&
//
// EXCEPTIONS

const ModVector<Table*>&
Table::
getReferencingTable(Trans::Transaction& cTrans_)
{
	loadConstraint(cTrans_);

	AutoRWLock l(getRWLock());
	if (!m_vecReferencingTable) {
		l.convert(Os::RWLock::Mode::Write);
		if (!m_vecReferencingTable) {
			m_vecReferencingTable = new ModVector<Table*>();
			ModSize n = m_mapReferencingTable.getSize();
			if (n) {
				Database* pDatabase = getDatabase(cTrans_);
				m_vecReferencingTable->reserve(n);
				TableReferenceMap::Iterator iterator = m_mapReferencingTable.begin();
				for (ModSize i = 0; i < n; ++i, ++iterator) {
					if (Table* pTable = Table::get((*iterator).first, pDatabase, cTrans_)) {
						m_vecReferencingTable->pushBack(pTable);
					}
				}
			}
		}
	}
	return *m_vecReferencingTable;
}

// FUNCTION public
//	Schema::Table::hasReferencedTable -- get whether referenced tables exist
//
// NOTES
//	referenced table is a table referenced by a foreign key constraint of this table
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Table::
hasReferencedTable(Trans::Transaction& cTrans_)
{
	loadConstraint(cTrans_);
	return m_mapReferencedTable.isEmpty() == ModFalse;
}

// FUNCTION public
//	Schema::Table::getReferencedTable -- get referenced tables
//
// NOTES
//	referenced table is a table referenced by a foreign key constraint of this table
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	const ModVector<Table*>&
//
// EXCEPTIONS

const ModVector<Table*>&
Table::
getReferencedTable(Trans::Transaction& cTrans_)
{
	loadConstraint(cTrans_);

	AutoRWLock l(getRWLock());
	if (!m_vecReferencedTable) {
		l.convert(Os::RWLock::Mode::Write);
		if (!m_vecReferencedTable) {
			m_vecReferencedTable = new ModVector<Table*>();
			ModSize n = m_mapReferencedTable.getSize();
			if (n) {
				Database* pDatabase = getDatabase(cTrans_);
				m_vecReferencedTable->reserve(n);
				TableReferenceMap::Iterator iterator = m_mapReferencedTable.begin();
				for (ModSize i = 0; i < n; ++i, ++iterator) {
					if (Table* pTable = Table::get((*iterator).first, pDatabase, cTrans_)) {
						m_vecReferencedTable->pushBack(pTable);
					}
				}
			}
		}
	}
	return *m_vecReferencedTable;
}

//	FUNCTION public
//	Schema::Table::propagateDatabaseAttribute
//		データベースの属性が変化したので対応した変更を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		const Database::Attribute& cAttribute_
//			変更後のデータベースの属性
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
propagateDatabaseAttribute(Trans::Transaction& cTrans_,
						   const Database::Attribute& cAttribute_)
{
	if (isSystem()) return;

	// 表に属するファイルに属性変化を伝播させる
	const FileMap& cMap = loadFile(cTrans_);

	{
		AutoRWLock l(getRWLockForFile());
		cMap.apply(ApplyFunction2<File, Trans::Transaction&, const Database::Attribute&>
				   (&File::propagateDatabaseAttribute, cTrans_, cAttribute_));
	}

	// ROWIDのファイルが内部で保持している構造を破棄してもらうために
	// unmountを呼ぶ
	unmountSequence(cTrans_);

	// ROWIDのためのシーケンスファイルの属性が変わるため
	// オブジェクトも一度クリアしておく
	// 次回必要になったときに作成しなおされる
	clearSequence();
}

//	FUNCTION public
//	Schema::Table::getHint -- 表に対するヒントを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		表に対するヒント
//
//	EXCEPTIONS
//		なし

const Hint*
Table::
getHint() const
{
	return m_pHint;
}

//	FUNCTION public
//	Schema::Table::getTupleSequence --
//		表のタプル ID を生成するためのシーケンスを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		得られたシーケンス
//
//	EXCEPTIONS

Sequence&
Table::
getTupleSequence(Trans::Transaction& cTrans_)
{
	Column* pRowID = getTupleID(cTrans_);
	if (!pRowID) {
		// There are no column which contains rowid.
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return pRowID->getSequence(cTrans_);
}

// FUNCTION public
//	Schema::Table::createSequence -- シーケンスファイルを作成する
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
Table::
createSequence(Trans::Transaction& cTrans_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).create(cTrans_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).create(cTrans_);
	}
}

// FUNCTION public
//	Schema::Table::dropSequence -- シーケンスファイルを破棄する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	bool bForce_ = true
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
dropSequence(Trans::Transaction& cTrans_, bool bForce_ /* = true */)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).drop(cTrans_, bForce_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).drop(cTrans_, bForce_);
	}
}

// FUNCTION public
//	Schema::Table::mountSequence -- シーケンスファイルをmountする
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
Table::
mountSequence(Trans::Transaction& cTrans_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).mount(cTrans_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).mount(cTrans_);
	}
}

// FUNCTION public
//	Schema::Table::unmountSequence -- シーケンスファイルをunmountする
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
Table::
unmountSequence(Trans::Transaction& cTrans_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).unmount(cTrans_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).unmount(cTrans_);
	}
}

// FUNCTION public
//	Schema::Table::flushSequence -- シーケンスファイルをflushする
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
Table::
flushSequence(Trans::Transaction& cTrans_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).flush(cTrans_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).flush(cTrans_);
	}
}

// FUNCTION public
//	Schema::Table::syncSequence -- シーケンスファイルをsyncする
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	bool& incomplete
//	bool& modified
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
syncSequence(Trans::Transaction& cTrans_, bool& incomplete, bool& modified)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).sync(cTrans_, incomplete, modified);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).sync(cTrans_, incomplete, modified);
	}
}

// FUNCTION public
//	Schema::Table::startBackupSequence -- シーケンスファイルをstartBackupする
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	bool bRestorable_ = true
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
startBackupSequence(Trans::Transaction& cTrans_,
					bool bRestorable_ /* = true */)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).startBackup(cTrans_, bRestorable_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).startBackup(cTrans_, bRestorable_);
	}
}

// シーケンスファイルをendBackupする
void
Table::
endBackupSequence(Trans::Transaction& cTrans_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).endBackup(cTrans_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).endBackup(cTrans_);
	}
}

// シーケンスファイルをrecoverする
void
Table::
recoverSequence(Trans::Transaction& cTrans_,
				const Trans::TimeStamp& cPoint_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).recover(cTrans_, cPoint_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).recover(cTrans_, cPoint_);
	}
}

// シーケンスファイルをrestoreする
void
Table::
restoreSequence(Trans::Transaction&	cTrans_,
				const Trans::TimeStamp&	cPoint_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイル
		pRowID->getSequence(cTrans_).restore(cTrans_, cPoint_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity columnを生成するシーケンスファイル
		pIdentity->getSequence(cTrans_).restore(cTrans_, cPoint_);
	}
}

//	FUNCTION public
//	Schema::Table::moveSequence --
//		シーケンスファイルを移動する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		const Os::Path& cPrevPath_
//		const Os::Path& cPostPath_
//			変更する前後のパス指定
//		const Name& cPrevName_
//		const Name& cPostName_
//			変更する前後の名前
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
moveSequence(Trans::Transaction& cTrans_,
			 const Os::Path& cPrevPath_,
			 const Os::Path& cPostPath_,
			 const Name& cPrevName_,
			 const Name& cPostName_,
			 bool bUndo_, bool bRecovery_)
{
	// Execute following only if the path or name is changed.
	if (cPrevPath_.compare(cPostPath_) == Os::Path::CompareResult::Identical
		&& cPrevName_ == cPostName_)
		return;

	if (bRecovery_) {
		// Redo時の処理ならsetPathのみ
		setPath(cTrans_, cPostPath_, cPostName_);
		return;
	}

	// 移動前のパス名を作る
	Os::Path cPrevPath(cPrevPath_);
	cPrevPath.addPart(cPrevName_);

	// オブジェクトに設定されているものと同じであるかを確認する
	if (cPrevPath.compare(getPath(cTrans_))
		!= Os::Path::CompareResult::Identical) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// 移動後のパスを作る
	Os::Path cPostPath(cPostPath_);
	cPostPath.addPart(cPostName_);

	enum {
		None,
		RowIDMoved,
		IdentityMoved,
		PathSet,
		ValueNum
	} eStatus = None;

	try {
		if (Column* pRowID = getTupleID(cTrans_)) {
			pRowID->moveSequence(cTrans_, cPrevPath, cPostPath, bUndo_, bRecovery_);
			eStatus = RowIDMoved;
			SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "RowIDMoved");
		}
		if (Column* pIdentity = getIdentity(cTrans_)) {
			pIdentity->moveSequence(cTrans_, cPrevPath, cPostPath, bUndo_, bRecovery_);
			eStatus = IdentityMoved;
			SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "IdentityMoved");
		}

		// パスを設定しなおす
		setPath(cTrans_, cPostPath_, cPostName_);
		eStatus = PathSet;
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "SetPath");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		switch (eStatus) {
		case PathSet:
			{
				// setPathはmoveの後でなければならないので
				// switch文の後で行う
				// thru.
			}
		case IdentityMoved:
			{
				// move reversely
				if (Column* pIdentity = getIdentity(cTrans_)) {
					pIdentity->moveSequence(cTrans_, cPostPath, cPrevPath, true /* undo */, bRecovery_);
				}
				// thru.
			}
		case RowIDMoved:
			{
				if (Column* pRowID = getTupleID(cTrans_)) {
					pRowID->moveSequence(cTrans_, cPostPath, cPrevPath, true /* undo */, bRecovery_);
				}
			}
		case None:
		default:
			{
				; // do nothing
				break;
			}
		}
		if (eStatus == PathSet)
			setPath(cTrans_, cPrevPath_, cPrevName_);

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

// FUNCTION public
//	Schema::Table::verifySequence -- シーケンスの検査を行う
//
// NOTES
//
// ARGUMENTS
//	Admin::Verification::Progress& cResult_
//	Trans::Transaction& cTrans_
//	Admin::Verification::Treatment::Value eTreatment_
//	TupleID::Value iMaxRowID_
//	Sequence::Signed::Value iMaxIdentity_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
verifySequence(Admin::Verification::Progress& cResult_,
			   Trans::Transaction& cTrans_,
			   Admin::Verification::Treatment::Value eTreatment_,
			   TupleID::Value iMaxRowID_,
			   Identity::Value iMaxIdentity_)
{
	if (Column* pRowID = getTupleID(cTrans_)) {
		// RowIdを生成するシーケンスファイルの検査
		pRowID->getSequence(cTrans_).verify(cResult_, cTrans_, eTreatment_, iMaxRowID_);
	}
	if (Column* pIdentity = getIdentity(cTrans_)) {
		// Identity Columnを生成するシーケンスファイルの検査
		pIdentity->getSequence(cTrans_).verify(cResult_, cTrans_, eTreatment_, iMaxIdentity_);
	}
}

//	FUNCTION public
//	Schema::Table::clearSequence --
//		シーケンスを表すクラスを破棄する
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
Table::
clearSequence()
{
	if (m_pRowID.get()) {
		m_pRowID->clearSequence();
	}
	if (m_pIdentity.get()) {
		m_pIdentity->clearSequence();
	}
}

//	FUNCTION public
//	Schema::Table::clearHint -- 表に対して指定されたヒントを消去する
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
Table::
clearHint()
{
	if (m_pHint)
		delete m_pHint, m_pHint = 0;
}

//	FUNCTION public
//	Schema::Table::isReorganizationInProgress -- 表に属するファイルが再構成中かを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true	この表に属するファイルが再構成中である
//		false	この表に属するファイルが再構成中ではない、
//				または実行中の再構成のうち余分なログ出力が必要なものはない
//
//	EXCEPTIONS

bool
Table::
isReorganizationInProgress() const
{
	return m_pReorganizedFiles && !m_pReorganizedFiles->isEmpty();
}

//	FUNCTION public
//	Schema::Table::getLogColumns -- 再構成後の反映のためにログ出力が必要な列を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		再構成後の反映のためにログ出力な列を表すスキーマオブジェクトのModVector
//
//	EXCEPTIONS

ModVector<Column*>
Table::
getLogColumns(Trans::Transaction& cTrans_) const
{
	AutoRWLock l(getRWLock());

	// この関数はisReorganizationInProgressがtrueのときにしか呼んではいけない
	; _SYDNEY_ASSERT(m_pReorganizedFiles); 

	ModVector<Column*> vecResult;
	Common::BitSet cAdded;

	ModVector<File*>::Iterator iterator = m_pReorganizedFiles->begin();
	const ModVector<File*>::Iterator& end = m_pReorganizedFiles->end();

	for (; iterator != end; ++iterator) {
		File* pFile = *iterator;
		ModVector<Field*> vecField = pFile->getField(Field::Category::Key, cTrans_);
		ModSize n = vecField.getSize();
		for (ModSize i = 0; i < n; i++) {
			// 対応するColumnにあたるまでSourceをたどる
			Field* pField = vecField[i];
			while (pField && pField->getColumnID() == ID::Invalid)
				pField = pField->getSource(cTrans_);
			; _SYDNEY_ASSERT(pField);
			// 追加されていなければ追加する
			if (!cAdded.test(static_cast<ModSize>(pField->getColumnID()))) {
				if (Column* pColumn = pField->getColumn(cTrans_)) {
					vecResult.pushBack(pColumn);
					cAdded.set(static_cast<ModSize>(pColumn->getID()));
				}
			}
		}
	}
	return vecResult;
}

//	FUNCTION public
//	Schema::Table::addReorganizedFile -- 再構成中のファイルを追加する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::File* pFile_
//			追加するファイル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
addReorganizedFile(Trans::Transaction& cTrans_, File* pFile_)
{
	; _SYDNEY_ASSERT(pFile_);

	// 複合索引のファイルについてのみ必要
	// undoExpunge対応のため複合索引でなくても必要になった

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (!m_pReorganizedFiles) {
		m_pReorganizedFiles = new ModVector<File*>;
	}
	m_pReorganizedFiles->pushBack(pFile_);
}

//	FUNCTION public
//	Schema::Table::eraseReorganizedFile -- 再構成中のファイルを削除する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::File* pFile_
//			削除するファイル
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
eraseReorganizedFile(Trans::Transaction& cTrans_, File* pFile_)
{
	; _SYDNEY_ASSERT(pFile_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (m_pReorganizedFiles) {
		ModVector<File*>::Iterator iterator = m_pReorganizedFiles->find(pFile_);
		if (iterator != m_pReorganizedFiles->end())
			m_pReorganizedFiles->erase(iterator);
	}
}

//	FUNCTION public
//	Schema::Table::getDefaultAreaID --
//		表を構成するファイルを格納するエリアのIDを得る
//
//	NOTES
//		エリアカテゴリーDefaultに対応するエリアのIDを得る
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid以外
//			エリアのオブジェクトID
//		Schema::Object::ID::Invalid
//			このクラスに保持されているIDに対応するエリアはすでに存在しない
//
//	EXCEPTIONS

Object::ID::Value
Table::
getDefaultAreaID() const
{
	return getAreaID(AreaCategory::Default);
}

//	FUNCTION public
//	Schema::Table::getAreaID --
//		表を構成する指定した種類のファイルを格納するエリアのIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eArea_
//			取得するエリアの対象を表す値
//		bool bEffective_ = false
//			trueの場合、対象の種類が無指定のときに使用される種類も調べる
//
//	RETURN
//		Schema::Object::ID::Invalid以外
//			エリアのオブジェクトID
//		Schema::Object::ID::Invalid
//			このクラスに保持されているIDに対応するエリアはすでに存在しない
//
//	EXCEPTIONS

Object::ID::Value
Table::
getAreaID(AreaCategory::Value eArea_, bool bEffective_ /* = false */) const
{
	return getAreaID(m_veciAreaID, eArea_, bEffective_);
}

//	FUNCTION public
//	Schema::Table::getAreaID --
//		表を構成する指定した種類のファイルを格納するエリアのIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		const ModVector<Schema::Object::ID::Value>& vecAreaID
//			エリア割り当てが記録された配列
//			サイズがAreaCategory::ValueNumであることが前提
//		Schema::AreaCategory::Value eArea_
//			取得するエリアの対象を表す値
//		bool bEffective_ = false
//			trueの場合、対象の種類が無指定のときに使用される種類も調べる
//
//	RETURN
//		Schema::Object::ID::Invalid以外
//			エリアのオブジェクトID
//		Schema::Object::ID::Invalid
//			このクラスに保持されているIDに対応するエリアはすでに存在しない
//
//	EXCEPTIONS

//static
Object::ID::Value
Table::
getAreaID(const ModVector<ID::Value>& vecAreaID,
		  AreaCategory::Value eArea_, bool bEffective_ /* = false */)
{
	; _SYDNEY_ASSERT(vecAreaID.getSize() >= static_cast<ModSize>(AreaCategory::ValueNum));
	return bEffective_ ?
		_Area::_getEffectiveID(vecAreaID, eArea_)
		: vecAreaID[eArea_];
}

//	FUNCTION public
//	Schema::Table::getAreaID --
//		表に設定されているエリアのIDをすべて得る
//
//	NOTES
//
//	ARGUMENTS
//		ModVecor<ID::Value>& vecID_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

const ModVector<Object::ID::Value>&
Table::
getAreaID() const
{
	return m_veciAreaID;
}

//	FUNCTION public
//	Schema::Table::getDefaultArea --
//		表を構成するファイルを格納するエリアを得る
//
//	NOTES
//		エリアカテゴリーDefaultで調べる
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0以外
//			エリアのオブジェクト
//		0
//			このクラスに保持されているIDに対応するエリアはすでに存在しない
//
//	EXCEPTIONS

Area*
Table::
getDefaultArea(Trans::Transaction& cTrans_) const
{
	return getArea(AreaCategory::Default, cTrans_);
}

//	FUNCTION public
//	Schema::Table::getArea --
//		表を構成する指定した種類のファイルを格納するエリアを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value eArea_
//			取得するエリアの対象を表す値
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0以外
//			エリアのオブジェクト
//		0
//			このクラスに保持されているIDに対応するエリアはすでに存在しない
//
//	EXCEPTIONS

Area*
Table::
getArea(AreaCategory::Value eArea_, Trans::Transaction& cTrans_) const
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (!m_vecpArea)
		const_cast<Table*>(this)->resetArea();

	Area*& refArea = m_vecpArea->at(eArea_);

	return (!refArea) ?
		refArea = Area::get(getAreaID(eArea_), getDatabase(cTrans_), cTrans_)
		: refArea;
}

//
//	FUNCTION public
//		Schema::Table::resetArea --
//
//	NOTES
//
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
Table::
resetArea()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (m_vecpArea) {
		m_vecpArea->clear();
	} else {
		m_vecpArea = new ModVector<Area*>();
		; _SYDNEY_ASSERT(m_vecpArea);
	}

	m_vecpArea->reserve(AreaCategory::ValueNum);
	for (int i = 0; i < AreaCategory::ValueNum; i++)
		m_vecpArea->pushBack(0);
}

//
//	FUNCTION public
//		Schema::Table::clearArea --
//
//	NOTES
//
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
Table::
clearArea()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (m_vecpArea) {
		resetArea();
		delete m_vecpArea, m_vecpArea = 0;
	}
}

//	FUNCTION public
//		Schema::Table:setAreaID --
//			指定エリア ID によってエリア割り当て情報の設定をする
//
//	NOTES
//	   	Table を create した後には呼び出さないこと
//
//	ARGUMENTS
//		const ModVector<ID::Value>& cAreaID_
//			エリア ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
setAreaID(const ModVector<ID::Value>& cAreaID_)
{
	clearArea();
	// エリアIDを設定する
	m_veciAreaID = cAreaID_;
}

//	FUNCTION public
//		Schema::Table:setArea --
//			SQL構文による指定にしたがってエリア割り当て情報の設定をする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		const Statement::AreaOption& cStatement_
//			エリア割り当て指定をするSQL構文要素
//		ModVector<Schema::Object::ID::Value>& vecPrevAreaID_
//		ModVector<Schema::Object::ID::Value>& vecPostAreaID_
//			移動前後のエリアID
//
//	RETURN
//		true ... 変更の必要がある
// 		false... 変更の必要がない
//
//	EXCEPTIONS
//		Exception::AreaNotFound
//			指定された名前のエリアはない
//		Exception::MetaDatabaseCorrupted
//			エリア格納関係の内容が正しくない

bool
Table::
setArea(Trans::Transaction& cTrans_,
		const Statement::AreaOption& cStatement_,
		ModVector<ID::Value>& vecPrevAreaID_,
		ModVector<ID::Value>& vecPostAreaID_)
{
	if (isSystem()) return false;

	bool bResult = false;
	Database* pDatabase = getDatabase(cTrans_);

	for (int cat = 0; cat < AreaCategory::ValueNum; cat++) {

		// Undo/Redoの仕様変更によりログひとつでAlter後の
		// Area指定すべてを取得できる必要ができた
		// したがってsetの指定の有無に関係なく修正前後のIDを入れる

		// まず修正前後に同じ値を入れる
		vecPrevAreaID_[cat] = vecPostAreaID_[cat] = m_veciAreaID[cat];

		// SQL構文要素から対応する種別のエリアの指定を得て
		// 対応する名前を持つエリアオブジェクトを探す

		AreaCategory::Value eCat = static_cast<AreaCategory::Value>(cat);

		if (Statement::Identifier* pAreaName =
			cStatement_.getAreaName(Table::convertAreaCategory(eCat))) {

			// ★注意★
			// エリア名が省略されているときはgetIdentifierが0を返す
			// setにおいてはエリア名の省略はエリア指定の省略と同義

			if (pAreaName->getIdentifier()) {
				Area* pPostArea = pDatabase->getArea(*pAreaName->getIdentifier(), cTrans_);
				if ( !pPostArea ) {
					// 指定された名称のエリアが存在しないので例外送出
					_SYDNEY_THROW2(Exception::AreaNotFound,
								   *pAreaName->getIdentifier(),
								   pDatabase->getName());
				}
				// 修正後のIDを設定しなおす
				vecPostAreaID_[cat] = pPostArea->getID();

				if (!bResult) {
					bResult = (vecPostAreaID_[cat] != vecPrevAreaID_[cat]);
				}
			}
		}
	}
	return bResult;
}

//	FUNCTION public
//		Schema::Table:dropArea --
//			SQL構文による指定にしたがってエリア割り当て情報の解除をする
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		const Statement::AreaOption& cStatement_
//			エリア割り当て指定を表すSQL構文要素
//		ModVector<Schema::Object::ID::Value>& vecPrevAreaID_
//		ModVector<Schema::Object::ID::Value>& vecPostAreaID_
//			変更前後のエリアID
//
//	RETURN
//		true ... 変更の必要がある
// 		false... 変更の必要がない
//
//	EXCEPTIONS

bool
Table::
dropArea(Trans::Transaction& cTrans_,
		 const Statement::AreaOption& cStatement_,
		 ModVector<ID::Value>& vecPrevAreaID_,
		 ModVector<ID::Value>& vecPostAreaID_)
{
	if (isSystem()) return false;

	bool bResult = false;
	for (int cat = 0; cat < AreaCategory::ValueNum; cat++) {

		// Undo/Redoの仕様変更によりログひとつでAlter後の
		// Area指定すべてを取得できる必要ができた
		// したがってsetの指定の有無に関係なく修正前後のIDを入れる

		// まず修正前後に同じ値を入れる
		vecPrevAreaID_[cat] = vecPostAreaID_[cat] = m_veciAreaID[cat];

		// SQL構文要素から対応する種別のエリアの指定を得て
		// 指定があるものについてエリア指定を解除する

		AreaCategory::Value eCat = static_cast<AreaCategory::Value>(cat);
		if (Statement::Identifier* pAreaName =
			cStatement_.getAreaName(Table::convertAreaCategory(eCat))) {

			// ★注意★
			// dropにおいてはエリア名の指定は無効であるが、
			// ここではログを出して無視する

			if (pAreaName->getIdentifier()) {
				SydInfoMessage
					<< "Specified area name in drop area: "
					<< *pAreaName->getIdentifier()
					<< " ignored." << ModEndl;
			}

			// 修正後のIDをInvalidにする
			vecPostAreaID_[cat] = ID::Invalid;

			if (!bResult) {
				bResult = (vecPostAreaID_[cat] != vecPrevAreaID_[cat]);
			}
		}
	}
	return bResult;
}

//	FUNCTION public
//		Schema::Table::moveArea --
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		AreaCategory::Value eCat_
//			移動対象のエリア種別
//		ID::Value iPrevAreaID_,
//		ID::Value iPostAreaID_,
//			ファイルに対応する移動前後のエリアID
//		const ModVector<ID::Value>& vecPrevAreaID_
//		const ModVector<ID::Value>& vecPostAreaID_
//			移動前後の表に対するエリアID割り当て
//		ModVector<ID::Value>* pvecNonEmptyArea_ = 0
//			使用中のオブジェクトがあることを示す配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???

void
Table::
moveArea(Trans::Transaction& cTrans_, AreaCategory::Value eCat_,
		 ID::Value iPrevAreaID_, ID::Value iPostAreaID_,
		 const ModVector<ID::Value>& vecPrevAreaID_,
		 const ModVector<ID::Value>& vecPostAreaID_,
		 ModVector<ID::Value>* pvecNonEmptyArea_,
		 bool bUndo_, bool bRecovery_, bool bMount_)
{
	; _SYDNEY_ASSERT(!isSystem());

    // ファイルを新しいエリアに移動する
    switch (eCat_) {
    case AreaCategory::LogicalLog:
    case AreaCategory::PhysicalLog:
    case AreaCategory::FileMin:
		{
			// 非使用
			break;
		}
	case AreaCategory::Default:
		{
			moveAreaDefault(cTrans_, iPrevAreaID_, iPostAreaID_, bUndo_, bRecovery_, bMount_);
			// thru.
		}
    case AreaCategory::Heap:
		{
			// エリア種別が同じファイルについて移動を行う
			moveAreaFile(cTrans_, eCat_, iPrevAreaID_, iPostAreaID_,
						 vecPrevAreaID_, vecPostAreaID_,
						 pvecNonEmptyArea_,
						 bUndo_, bRecovery_, bMount_);
			break;
		}
    case AreaCategory::Index:
    case AreaCategory::FullText:
		{
			// 索引を構成するファイルを移動する
			// ただし、索引に個別に指定があれば何もしない
			moveAreaIndex(cTrans_, eCat_, iPrevAreaID_, iPostAreaID_,
						  vecPrevAreaID_, vecPostAreaID_,
						  pvecNonEmptyArea_,
						  bUndo_, bRecovery_, bMount_);
			break;
		}
    default:
        ; _SYDNEY_ASSERT(false);
        break;
    }
}

//	FUNCTION public
//		Schema::Table::movePath -- パス指定の変更を下位オブジェクトに反映する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		const Os::Path& cPrevPath_
//		const Os::Path& cPostPath_
//			移動前後のパス
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???

void
Table::
movePath(Trans::Transaction& cTrans_, const Os::Path& cPrevPath_, const Os::Path& cPostPath_,
		 bool bUndo_, bool bRecovery_)
{
	if (isSystem()) return;

	// Defaultのエリアカテゴリーでエリアが指定してあるか調べる

	ID::Value iID = getDefaultAreaID();

	// Tableでエリアが指定してあるならば
	// この表およびそれに属する索引を構成するファイルは
	// すべてエリア指定があることになるので
	// パス指定の変更には影響されない
	// したがって、エリアの指定がないときのみ処理すればよい

	if (iID == ID::Invalid) {

		// 移動前のディレクトリー
		Os::Path cPrevDir(cPrevPath_);
		cPrevDir.addPart(getPathPart(cTrans_));
		// 移動先のディレクトリー
		Os::Path cNewDir(cPostPath_);
		cNewDir.addPart(getPathPart(cTrans_));

		// エラー処理でどこまで進んだかを得るための状態変数
		enum {
			None,						// 初期値
			DirectoryCreated,			// ディレクトリーを作成した
			RowID,						// タプルIDのファイルを移動した
			Move,						// ファイルの移動を開始した
			Removed,					// 移動前のディレクトリーを破棄した
			ValueNum
		} eStatus = None;

		// 表に属するファイルをすべて得ておく
		const ModVector<File*>& vecFiles = getFile(cTrans_);
		ModVector<File*>::ConstIterator iterator = vecFiles.begin();

		Utility::File::AutoRmDir cAutoRmDir;
		cAutoRmDir.setDir(cNewDir);

		try {
			eStatus = DirectoryCreated;
			SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "Directory");

			// シーケンスファイルを移動する

			if (getScope() == Object::Scope::Permanent
				|| getScope() == Object::Scope::SessionTemporary) {

				// シーケンスファイルを移動
				moveSequence(cTrans_, cPrevPath_, cPostPath_, getName(), getName(), bUndo_, bRecovery_);

				// 構成するファイルがひとつでも移動したら移動後のパスを破棄できない
				cAutoRmDir.disable();
				eStatus = RowID;
				SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "RowID");
			}

			const ModVector<File*>::ConstIterator end = vecFiles.end();

			for (; iterator != end; ++iterator) {
				(*iterator)->movePath(cTrans_, cNewDir, bUndo_, bRecovery_);
				// 構成するファイルがひとつでも移動したら移動後のパスを破棄できない
				cAutoRmDir.disable();
				eStatus = Move;
			}
			SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "Moved");

			// 移動前のディレクトリーを破棄する
			Utility::File::rmAll(cPrevDir);
			eStatus = Removed;
			SCHEMA_FAKE_ERROR("Schema::Table", "MovePath", "Removed");

			// メンバーとして保持しているトップディレクトリーをクリアしておく
			delete m_pPath, m_pPath = 0;

		} catch (...) {

			_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

			switch (eStatus) {
			case Removed:
			case Move:
				{
					// エラーの起きたファイルまでを元に戻す
					ModVector<File*>::ConstIterator errIterator = vecFiles.begin();
					for (; errIterator != iterator; ++errIterator) {
						(*errIterator)->movePath(cTrans_, cPrevDir, true);
					}
					// thru
				}
			case RowID:
				{
					if (getScope() == Object::Scope::Permanent
						|| getScope() == Object::Scope::SessionTemporary) {
						// シーケンスを保持するファイルを戻す
						moveSequence(cTrans_, cPostPath_, cPrevPath_, getName(), getName(), true /* undo */, bRecovery_);
					}
					// すべてが移動したら移動後のパスを破棄できる
					cAutoRmDir.enable();
				}
			case DirectoryCreated:
			case None:
			default:
				break;
			}

			_END_REORGANIZE_RECOVERY(getDatabaseID());

			_SYDNEY_RETHROW;
		}
	}
}

// FUNCTION public
//	Schema::Table::rename -- 名前を変更する
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
Table::
rename(const Name& cPostName_)
{
	setName(cPostName_);
}

//	FUNCTION public
//		Schema::Table::moveRename -- 名前変更を下位オブジェクトに反映する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		const Schema::Object::Name& cPrevName_
//		const Schema::Object::Name& cPostName_
//			移動前後の名前
//		bool bUndo_ = false
//			trueの場合エラー処理中なので重ねてのエラー処理はしない
//		bool bRecovery_ = false
//			trueなら変更前のパスにファイルがなくても変更後にあればOKとする
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???

void
Table::
moveRename(Trans::Transaction& cTrans_, const Name& cPrevName_, const Name& cPostName_,
		   bool bUndo_, bool bRecovery_)
{
	if (isSystem()) return;

	int cat = 0;
	// Used area list
	ModVector<ID::Value> vecUsedArea;

	// Status which indicates the progression
	enum {
		None,			// Initial value
		Moved,			// More than one of other files have been moved
		Removed,		// All the directories corresponding to the old name has been deleted
		Renamed,		// renamed
		ValueNum
	} eStatus = None;

	try {
		for (; cat < AreaCategory::ValueNum; ++cat) {
			AreaCategory::Value eCat = static_cast<AreaCategory::Value>(cat);

			// Get area id which is actually assigned to files
			//     whose corresponding area category is eCat
			ID::Value iAreaID = _Area::_getEffectiveID(m_veciAreaID, eCat);

			// Add to used area list
			if (vecUsedArea.find(iAreaID) == vecUsedArea.end())
					vecUsedArea.pushBack(iAreaID);

			SCHEMA_FAKE_ERROR("Schema::Table", "MoveRename", "Directory");

			// Move files
			moveRename(cTrans_, eCat, iAreaID, cPrevName_, cPostName_, bUndo_, bRecovery_);
			eStatus = Moved;
		}
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveRename", "Moved");

		if (!bRecovery_ && cPrevName_ != cPostName_)
			// Delete all the directories corresponding to the old name.
			sweepArea(cTrans_, vecUsedArea, cPrevName_);
		eStatus = Removed;
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveRename", "Removed");

		// Modify the name
		rename(cPostName_);
		eStatus = Renamed;
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveRename", "Renamed");

		// Clear the path cache
		delete m_pPath, m_pPath = 0;

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		switch (eStatus) {
		case Renamed:
			{
				rename(cPrevName_);
				// thru.
			}
		case Removed:
		case Moved:
			{
				// Undo moving for the all categories.
				for (--cat; cat >= 0; --cat) {
					AreaCategory::Value eCat = static_cast<AreaCategory::Value>(cat);
					ID::Value iAreaID = _Area::_getEffectiveID(m_veciAreaID, eCat);
					moveRename(cTrans_, eCat, iAreaID, cPostName_, cPrevName_, true /* undo */, bRecovery_);
				}
				// thru.
			}
		case None:
			{
				// Delete all the directories corresponding to the new name.
				if (!bRecovery_ && cPrevName_ != cPostName_)
					sweepArea(cTrans_, vecUsedArea, cPostName_);
				break;
			}
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

// FUNCTION public
//	Schema::Table::moveRename -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Name& cPrevName_
//	bool bUndo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
moveRename(Trans::Transaction& cTrans_, AreaCategory::Value eCat_, ID::Value iAreaID_,
		   const Name& cPrevName_, const Name& cPostName_,
		   bool bUndo_, bool bRecovery_)
{
	if (isSystem()) return;

	// Move files according to renaming the table
    switch (eCat_) {
    case AreaCategory::LogicalLog:
    case AreaCategory::PhysicalLog:
    case AreaCategory::FileMin:
		{
			// Not used
			break;
		}
	case AreaCategory::Default:
		{
			moveRenameDefault(cTrans_, iAreaID_, cPrevName_, cPostName_, bUndo_, bRecovery_);
			// thru.
		}
    case AreaCategory::Heap:
    case AreaCategory::Index:
    case AreaCategory::FullText:
		{
			// Move files which have a area category which equals to eCat_
			moveRenameFile(cTrans_, eCat_, iAreaID_, cPrevName_, cPostName_,
						   bUndo_, bRecovery_);
			break;
		}
    default:
        ; _SYDNEY_ASSERT(false);
        break;
    }
}

// FUNCTION public
//	Schema::Table::moveRenameDefault -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	ID::Value iAreaID_
//	bool bUndo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
moveRenameDefault(Trans::Transaction& cTrans_,
				  ID::Value iAreaID_, const Name& cPrevName_, const Name& cPostName_,
				  bool bUndo_, bool bRecovery_)
{
	Database* pDatabase = getDatabase(cTrans_);

	// シーケンスファイルを移動する
	Os::Path cPath(getDataPath(cTrans_, pDatabase, iAreaID_));
	moveSequence(cTrans_, cPath, cPath, cPrevName_, cPostName_,
				 bUndo_, bRecovery_);
}

// FUNCTION public
//	Schema::Table::moveRenameFile -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Name& cPrevName_
//	bool bUndo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
moveRenameFile(Trans::Transaction& cTrans_, AreaCategory::Value eCat_, ID::Value iAreaID_,
			   const Name& cPrevName_, const Name& cPostName_,
			   bool bUndo_, bool bRecovery_)
{
	ModVector<File*> file;

	// Get primary index object if exist
	Constraint* primaryConstraint = getPrimaryKeyConstraint(cTrans_);
	Index* primaryIndex = 0;
	Name cPrevPrimaryName;
	Name cPostPrimaryName;
	int iMoved = -1;
	int iMovedByPrimary = -1;

	try {
		if (primaryConstraint) {
			// get index corresponding to the primary key constraint
			cPrevPrimaryName = primaryConstraint->createName(cPrevName_);
			cPostPrimaryName = primaryConstraint->createName(cPostName_);
			primaryIndex = getIndex(cPrevPrimaryName, cTrans_);
			; _SYDNEY_ASSERT(primaryIndex || getIndex(cPostPrimaryName, cTrans_));

			// change the name of constraint
			primaryConstraint->rename(cPostPrimaryName);
			primaryConstraint->touch();
		}

		file = getFile(eCat_, m_veciAreaID, m_veciAreaID, cTrans_);
		int n = file.getSize();

		for (int i = 0; i < n; ++i) {
			File* pFile = file[i];

			// Is the index is for primary key?
			if (iMovedByPrimary < 0 && primaryIndex
				&& pFile->getIndexID() == primaryIndex->getID()) {
				// Move the file by renaming the primary index itself.
				primaryIndex->moveRename(cTrans_, cPrevName_, cPostName_, cPrevPrimaryName, cPostPrimaryName,
										 bUndo_, bRecovery_);
				// Index should be touched here.
				primaryIndex->touch();
				iMovedByPrimary = i;
			} else {
				Name cPrevFileName(pFile->getName());
				Name cPostFileName(pFile->getIndexID() == ID::Invalid
								   ? pFile->createName(cTrans_, cPostName_)
								   : pFile->getName());
				pFile->moveRename(cTrans_, cPrevName_, cPostName_, cPrevFileName, cPostFileName,
								  bUndo_, bRecovery_);
			}
			iMoved = i;
		}

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		// revert the name of primary key constraint
		if (primaryConstraint) {
			primaryConstraint->rename(cPrevPrimaryName);
			primaryConstraint->untouch();
		}

		// Return the moved files to the original path.
		for (int i = 0; i <= iMoved; ++i) {
			File* pFile = file[i];
			if (iMovedByPrimary == i) {
				// Undo moving the primary key file
				primaryIndex->moveRename(cTrans_, cPostName_, cPrevName_, cPostPrimaryName, cPrevPrimaryName,
										 true /* undo */, bRecovery_);
				primaryIndex->untouch();
			} else {
				// Undo moving other files
				Name cPrevFileName(pFile->getIndexID() == ID::Invalid
								   ? pFile->createName(cTrans_, cPrevName_)
								   : pFile->getName());
				Name cPostFileName(pFile->getName());
				pFile->moveRename(cTrans_, cPostName_, cPrevName_, cPostFileName, cPrevFileName,
								  true /* undo */, bRecovery_);
			}
		}
		// If moved category is Default, undo moving rowid
		if (eCat_ == AreaCategory::Default) {
			moveRenameDefault(cTrans_, iAreaID_, cPostName_, cPrevName_, true /* undo */, bRecovery_);
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//		Schema::Table::sweepUnusedArea -- エリア以下のディレクトリーを必要に応じて消す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		Schema::Object::ID::Value	iAreaID_
//			調べるエリアID
//		bool bForce_ = true
//			trueのとき即座に破棄する
//			falseのときCheckpointに可能になったら破棄するよう依頼する
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		???

void
Table::
sweepUnusedArea(Trans::Transaction& cTrans_, ID::Value iAreaID_, bool bForce_ /* = true */)
{
	// 表に属するファイルを得る
	const ModVector<File*>& vecFiles = getFile(cTrans_);
	ModSize n = vecFiles.getSize();
	bool bDelete = true;
	for (ModSize i = 0; i < n; ++i) {
		// 調べているエリアIDと一致するファイルがあったら抜ける
		File* pFile = vecFiles[i];
		if (pFile->getStatus() != Status::Deleted
			&& pFile->getStatus() != Status::DeletedInRecovery
			&& pFile->getStatus() != Status::ReallyDeleted
			&& pFile->getAreaID() == iAreaID_) {
			bDelete = false;
			break;
		}
	}

	if (bDelete) {
		// エリア以下に存在するファイルはないので表名で表されるディレクトリーを消す
		Database* pDatabase = getDatabase(cTrans_);
		Area* pArea = pDatabase->getArea(iAreaID_, cTrans_);

		if (pArea) {
			Area::destroy(cTrans_, pArea->getPath(), getPathPart(cTrans_), bForce_);

		} else {

			Database* pDatabase = getDatabase(cTrans_);
			pDatabase->destroy(cTrans_, Database::Path::Category::Data,
							   getPathPart(cTrans_), bForce_);
		}
	}
}

//	FUNCTION public
//	Schema::Table::convertAreaCategory
//			-- CategoryをStatementのAreaタイプに変換する
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
Table::
convertAreaCategory(AreaCategory::Value eCategory_)
{
	switch (eCategory_) {
	case AreaCategory::Default:
		return Statement::AreaOption::Default;
	case AreaCategory::Heap:
		return Statement::AreaOption::Heap;
	case AreaCategory::Index:
		return Statement::AreaOption::Index;
	case AreaCategory::FullText:
		return Statement::AreaOption::FullText;
	case AreaCategory::LogicalLog:
		return Statement::AreaOption::LogicalLog;
	case AreaCategory::PhysicalLog:
		return Statement::AreaOption::PhysicalLog;
	case AreaCategory::FileMin:
		return Statement::AreaOption::Table; // dummy
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
	// never reach
	return Statement::AreaOption::Default;
}

//	FUNCTION public
//		Schema::Table::getPathPart --
//			ファイルの格納場所の表固有部分を作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		ファイルを格納するパス名のうち表固有部分
//
//	EXCEPTIONS
//		???

const ModUnicodeString&
Table::
getPathPart(Trans::Transaction& cTrans_) const
{
	return getName();
}

//	FUNCTION public
//		Schema::Table::getPath --
//			表を構成するファイルの格納場所のトップディレクトリーパス名を得る
//
//	NOTES
//		以下の順で得られた文字列に表固有部分を追加して得る
//			・エリアカテゴリーDefaultで得られるエリア
//			・データベースにパスカテゴリーDataで指定されるパス
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		トップディレクトリーのパス名
//
//	EXCEPTIONS

const Os::Path&
Table::
getPath(Trans::Transaction& cTrans_) const
{
	if (!m_pPath) {
		// エリアとデータベースの指定からデータを格納するパスを得る
		const_cast<Table*>(this)->setPath(cTrans_, getDataPath(cTrans_, getDatabase(cTrans_), getDefaultAreaID()), getName());
	}
	return *m_pPath;
}

//	FUNCTION public
//		Schema::Table::getPath --
//			表を構成するファイルの格納場所のトップディレクトリーパス名を得る
//
//	NOTES
//		以下の順で得られた文字列に表固有部分を追加して得る
//			・エリアカテゴリーDefaultで得られるエリア
//			・データベースにパスカテゴリーDataで指定されるパス
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
//
//	RETURN
//		表のデータを格納するディレクトリーのパス名
//
//	EXCEPTIONS

// static
Os::Path
Table::
getPath(const ModVector<ModUnicodeString>& vecDatabasePath_,
		const ModVector<ModUnicodeString>& vecAreaPath_,
		const Schema::Object::Name& cDatabaseName_,
		const Schema::Object::Name& cTableName_)
{
	if (vecAreaPath_.getSize() > 0) {
		return Os::Path(vecAreaPath_[0]).addPart(cTableName_);
	} else {
		return Database::getDataPath(vecDatabasePath_, cDatabaseName_).addPart(cTableName_);
	}
}

//	FUNCTION public
//		Schema::Table::getDataPath --
//			データベースとエリアの指定からデータを格納するパス名を得る
//
//	NOTES
//		以下の順で得られた文字列を使う
//			・エリアのパス指定0番目
//			・データベースにパスカテゴリーDataで指定されるパス
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			トランザクション記述子
//		Schema::Database* pDatabase_
//			表が属するデータベース
//		Schema::Object::ID::Value iAreaID_
//			表に指定されているエリアID
//			指定されていない場合はID::Invalid
//
//	RETURN
//		データを格納するディレクトリーのパス名
//
//	EXCEPTIONS

//static
Os::Path
Table::
getDataPath(Trans::Transaction& cTrans_,
			Database* pDatabase_, ID::Value iAreaID_)
{
	if (iAreaID_ != ID::Invalid) {
		// REDO中は最終的なパス指定を使用する
		ModVector<ModUnicodeString> vecPath;
		if (Manager::RecoveryUtility::Path::getUndoAreaPath(pDatabase_->getName(), iAreaID_, vecPath)) {
			return vecPath[0];
		} else {
			if (Area* pArea = pDatabase_->getArea(iAreaID_, cTrans_)) {
				return pArea->getPath(0);
			}
		}
		// UNDO情報に登録されておらず、エリアオブジェクトも得られなかったときはデータベースのパスを使う
	}
	return pDatabase_->getDataPath();
}

//	FUNCTION public
//		Schema::Table::setPath --
//			表を構成するファイルの格納場所のトップディレクトリーパス名を設定する
//
//	NOTES
//		与えられた文字列に表固有部分を追加して設定する
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModUnicodeString&			cPath_
//			設定するパス名
//
//	RETURN
//
//	EXCEPTIONS
//		???

void
Table::
setPath(Trans::Transaction& cTrans_,
		const ModUnicodeString& cPath_,
		const Name& cName_)
{
	ModAutoPointer<Os::Path> pPath = new Os::Path(cPath_);
	pPath->addPart(cName_);

	if (m_pPath) {
		if (m_pPath->compare(*pPath) == Os::Path::CompareResult::Identical) {
			// 設定の必要はない
			return;
		}
		delete m_pPath, m_pPath = 0;

		// シーケンスファイルのオブジェクトもクリアする必要がある
		clearSequence();
	}
	m_pPath = pPath.release();
}

//	FUNCTION public
//		Schema::Table::clearPath --
//			表を構成するファイルの格納場所のトップディレクトリーパス名をクリアする
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
Table::
clearPath()
{
	delete m_pPath, m_pPath = 0;

	// シーケンスファイルのオブジェクトもクリアする必要がある
	clearSequence();
}

//	FUNCTION public
//	Schema::Table::getColumn --
//		表に属するすべての列を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に属する列を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

const ModVector<Column*>&
Table::
getColumn(Trans::Transaction& cTrans_) const
{
	return const_cast<Table*>(this)->loadColumn(cTrans_).getView(getRWLock());
}

//	FUNCTION public
//	Schema::Table::getColumn --
//		表に属するすべての列を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Column::Category::Value columnCategory
//			取得する列のカテゴリー
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に属する列を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Column*>
Table::
getColumn(Column::Category::Value columnCategory,
		  Trans::Transaction& cTrans_) const
{
	ModVector<Column*> v;

	const ColumnMap& cMap = const_cast<Table*>(this)->loadColumn(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction1<Column, Column::Category::Value>
				 (ColumnMap::findByCategory, columnCategory));
	return v;
}

//	FUNCTION public
//	Schema::Table::getColumnByID --
//		表に属する列のうち、
//		指定したオブジェクトIDを持つ列を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	iColumnID_
//			このオブジェクトIDを持つ列を表すクラスを得る
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた列を格納する領域の先頭アドレス
//		0
//			表には指定されたオブジェクトIDの列は存在しない
//
//	EXCEPTIONS

Column*
Table::
getColumnByID(ID::Value iColumnID_, Trans::Transaction& cTrans_) const
{
	if (iColumnID_ == ID::Invalid)
		return 0;

	const ColumnMap& cMap = const_cast<Table*>(this)->loadColumn(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(iColumnID_).get();
}

//	FUNCTION public
//	Schema::Table::getColumn --
//		表に属する列のうち、
//		指定した名前を持つ列を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	columnName
//			列の名前
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた列を格納する領域の先頭アドレス
//		0
//			表には指定された名前の列は存在しない
//
//	EXCEPTIONS

Column*
Table::
getColumn(const Name& columnName, Trans::Transaction& cTrans_) const
{
	const ColumnMap& cMap = const_cast<Table*>(this)->loadColumn(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Column, const Name&>(_Bool::_findByName<Column>, columnName));
}

//	FUNCTION public
//	Schema::Table::getColumnByPosition --
//		表に属する列のうち、
//		指定した位置にある列を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Column::Position columnPosition
//			列の位置
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた列を格納する領域の先頭アドレス
//		0
//			表には指定された位置の列は存在しない
//
//	EXCEPTIONS

Column*
Table::
getColumnByPosition(Column::Position columnPosition,
					Trans::Transaction& cTrans_) const
{
	const ColumnMap& cMap = const_cast<Table*>(this)->loadColumn(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Column, Column::Position>
					 (ColumnMap::findByPosition, columnPosition));
}

//	FUNCTION public
//	Schema::Table::addColumn --
//		表を表すクラスの列として、
//		指定された列を表すクラスを追加する
//
//	NOTES
//		「列」表は更新されない
//
//	ARGUMENTS
//		const Schema::Column::Pointer&		column
//			追加する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加した列を表すクラス
//
//	EXCEPTIONS

const Column::Pointer&
Table::
addColumn(const Column::Pointer& pColumn_, Trans::Transaction& cTrans_)
{
	// 「列」表のうち、この表に関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられた列を追加する

	(void) loadColumn(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (pColumn_->isTupleID()) {
		; _SYDNEY_ASSERT(!m_pRowID.get());
		m_pRowID = pColumn_;

	} else if (pColumn_->isIdentity()) {
		if (m_pIdentity.get()) {
			// すでにIdentity Columnが定義されている
			_SYDNEY_THROW2(Exception::DuplicateIdentity, getName(), m_pIdentity->getName());
		}
		m_pIdentity = pColumn_;

	} else {
		// do nothing
		;
	}
	_columns->insert(pColumn_);

	return pColumn_;
}

//	FUNCTION public
//	Schema::Table::eraseColumn --
//		表を表すクラスからある列を表すクラスの登録を抹消する
//
//	NOTES
//		「列」表は更新されない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	columnID
//			登録を抹消する列のオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
eraseColumn(ID::Value columnID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_columns)
		_columns->erase(columnID);
}

//	FUNCTION public
//	Schema::File::resetColumn --
//		表には列を表すクラスが登録されていないことにする
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
Table::resetColumn()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_columns) {

		if (getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_columns->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_columns->reset();

	} else {
		// 列を表すクラスを登録するハッシュマップを生成する

		_columns = new ColumnMap;
		; _SYDNEY_ASSERT(_columns);
	}
}

//	FUNCTION public
//	Schema::Table::resetColumn --
//		表には列を表すクラスが登録されていないことにする
//
//	NOTES
//		「列」表は更新されない
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
resetColumn(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_columns) {

		// ハッシュマップに登録されている列を表すクラスがあれば、
		// すべて破棄し、ハッシュマップを空にする

		m_pRowID = Column::Pointer();
		m_pIdentity = Column::Pointer();
		_columns->reset(cDatabase_);

	} else {
		// 列を表すクラスを登録するハッシュマップを生成する

		_columns = new ColumnMap;
		; _SYDNEY_ASSERT(_columns);
	}
}

//	FUNCTION public
//	Schema::Table::clearColumn --
//		表を表すクラスに登録されている列を表すクラスと、
//		その管理用のベクターを破棄する
//
//	NOTES
//		「列」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
clearColumn(Trans::Transaction& cTrans_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_columns) {

		// ハッシュマップに登録されている列を表すクラスがあれば、
		// すべて破棄し、ハッシュマップも破棄する

		resetColumn(*getDatabase(cTrans_));
		delete _columns, _columns = 0;
	}
}

//	FUNCTION public
//	Schema::Table::getTupleID --
//		表のタプル ID を格納する列を表すクラスを得る
//
//	NOTES
//		生成前、中の表や、排他制御がうまく行われていない場合を除けば、
//		表にはタプル ID を格納する列が必ずひとつ存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたタプル ID を格納する列を格納する領域の先頭アドレス
//		0
//			表にはタプル ID を格納する列は存在しない
//
//	EXCEPTIONS

Column*
Table::
getTupleID(Trans::Transaction& cTrans_) const
{
	// ロードしておく
	(void) const_cast<Table&>(*this).loadColumn(cTrans_);

	return m_pRowID.get();
}

// FUNCTION public
//	Schema::Table::getIdentity -- Identity Column列を得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Column*
//
// EXCEPTIONS

Column*
Table::
getIdentity(Trans::Transaction& cTrans_) const
{
	// ロードしておく
	(void) const_cast<Table&>(*this).loadColumn(cTrans_);

	return m_pIdentity.get();
}

//	FUNCTION public
//	Schema::Table::getConstraint --
//		表に存在するすべての制約を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に存在する制約を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

const ModVector<Constraint*>&
Table::
getConstraint(Trans::Transaction& cTrans_) const
{
	return const_cast<Table*>(this)->loadConstraint(cTrans_).getView(getRWLock());
}

//	FUNCTION public
//	Schema::Table::getConstraint --
//		表に属する制約のうち、
//		指定したオブジェクトIDを持つ制約を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	iConstraintID_
//			このオブジェクトIDを持つ制約を表すクラスを得る
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた制約を格納する領域の先頭アドレス
//		0
//			表には指定されたオブジェクトIDの制約は存在しない
//
//	EXCEPTIONS

Constraint*
Table::
getConstraint(ID::Value iConstraintID_, Trans::Transaction& cTrans_) const
{
	if (iConstraintID_ == ID::Invalid)
		return 0;

	const ConstraintMap& cMap = const_cast<Table*>(this)->loadConstraint(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(iConstraintID_).get();
}

//	FUNCTION public
//	Schema::Table::getConstraint --
//		表に属する制約のうち、
//		指定した名前を持つ制約を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	constraintName
//			制約の名前
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた制約を格納する領域の先頭アドレス
//		0
//			表には指定された名前の制約は存在しない
//
//	EXCEPTIONS

Constraint*
Table::
getConstraint(const Name& constraintName, Trans::Transaction& cTrans_) const
{
	const ConstraintMap& cMap = const_cast<Table*>(this)->loadConstraint(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Constraint, const Name&>(_Bool::_findByName<Constraint>, constraintName));
}

//	FUNCTION public
//	Schema::Table::getConstraint --
//		表に属するすべての制約を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Constraint::Category::Value constraintCategory
//			取得する制約のカテゴリー
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に属する制約を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Constraint*>
Table::
getConstraint(Constraint::Category::Value constraintCategory,
			  Trans::Transaction& cTrans_) const
{
	ModVector<Constraint*> v;

	const ConstraintMap& cMap = const_cast<Table*>(this)->loadConstraint(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction1<Constraint, Constraint::Category::Value>
				 (ConstraintMap::findByCategory, constraintCategory));
	return v;
}

#ifdef OBSOLETE // Constraintを外部が参照することはない
//	FUNCTION public
//	Schema::Table::getConstraint --
//		表に属する制約のうち、
//		指定した位置にある制約を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Constraint::Position constraintPosition
//			制約の位置
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた制約を格納する領域の先頭アドレス
//		0
//			表には指定された位置の制約は存在しない
//
//	EXCEPTIONS

Constraint*
Table::
getConstraint(Constraint::Position constraintPosition,
			  Trans::Transaction& cTrans_) const
{
	const ConstraintMap& cMap = return const_cast<Table*>(this)->loadConstraint(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.find(BoolFunction1<Constraint, Constraint::Position>
				 (ConstraintMap::findByPosition, constraintPosition));
}
#endif

// FUNCTION public
//	Schema::Table::getPrimaryKeyConstraint -- Primary Key制約の取得
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Constraint*
//
// EXCEPTIONS

Constraint*
Table::
getPrimaryKeyConstraint(Trans::Transaction& cTrans_) const
{
	const ConstraintMap& cMap = const_cast<Table*>(this)->loadConstraint(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolMemberFunction0<Constraint>(&Constraint::isPrimaryKey));
}

//	FUNCTION public
//	Schema::Table::addConstraint --
//		表を表すクラスの制約として、
//		指定された制約を表すクラスを追加する
//
//	NOTES
//		「制約」表は更新されない
//
//	ARGUMENTS
//		const Schema::Constraint::Pointer&		constraint
//			追加する制約を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加した制約を表すクラス
//
//	EXCEPTIONS

const Constraint::Pointer&
Table::
addConstraint(const Constraint::Pointer& constraint, Trans::Transaction& cTrans_)
{
	// 「制約」表のうち、この表に関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられた制約を追加する

	(void) loadConstraint(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_constraints->insert(constraint);

	if ((constraint->getCategory() == Constraint::Category::ForeignKey)
		||
		(constraint->getCategory() == Constraint::Category::ReferedKey)) {
		addReference(*constraint);
	}

	return constraint;
}

//	FUNCTION public
//	Schema::Table::eraseConstraint --
//		表を表すクラスからある制約を表すクラスの登録を抹消する
//
//	NOTES
//		「制約」表は更新されない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	constraintID
//			登録を抹消する制約のオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
eraseConstraint(ID::Value constraintID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_constraints) {
		ConstraintMap::Iterator iterator = _constraints->find(constraintID);
		if (iterator != _constraints->end()) {
			Constraint::Pointer pConstraint = ConstraintMap::getValue(iterator);
			if ((pConstraint->getCategory() == Constraint::Category::ForeignKey)
				||
				(pConstraint->getCategory() == Constraint::Category::ReferedKey)) {
				eraseReference(*pConstraint);
			}
			_constraints->erase(constraintID);
		}
	}
}

//	FUNCTION public
//	Schema::File::resetConstraint --
//		表には制約を表すクラスが登録されていないことにする
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
Table::resetConstraint()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_constraints) {

		if (getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_constraints->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_constraints->reset();

	} else {
		// 制約を表すクラスを登録するハッシュマップを生成する

		_constraints = new ConstraintMap;
		; _SYDNEY_ASSERT(_constraints);
	}
}

//	FUNCTION public
//	Schema::Table::resetConstraint --
//		表には制約を表すクラスが登録されていないことにする
//
//	NOTES
//		「制約」表は更新されない
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
resetConstraint(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_constraints) {

		_constraints->reset(cDatabase_);

	} else {
		// 制約を表すクラスを登録するハッシュマップを生成する

		_constraints = new ConstraintMap;
		; _SYDNEY_ASSERT(_constraints);
	}
}

//	FUNCTION public
//	Schema::Table::clearConstraint --
//		表を表すクラスに登録されている制約を表すクラスと、
//		その管理用のハッシュマップを破棄する
//
//	NOTES
//		「制約」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
clearConstraint(Trans::Transaction& cTrans_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_constraints) {

		// ハッシュマップに登録されている制約を表すクラスがあれば、
		// すべて破棄し、ハッシュマップも破棄する

		resetConstraint(*getDatabase(cTrans_));
		delete _constraints, _constraints = 0;
	}
}

//	FUNCTION public
//	Schema::Table::getIndex --
//		表に存在するすべての索引を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に存在する索引を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

const ModVector<Index*>&
Table::
getIndex(Trans::Transaction& cTrans_) const
{
	BoolFunction0<Index>::Func func = _Bool::_Deleted;
	return const_cast<Table*>(this)->loadIndex(cTrans_).getView(
		getRWLock(), BoolFunction0<Index>(func));
}

//	FUNCTION public
//	Schema::Table::getIndexArea --
//		表に存在する指定されたエリア ID の索引を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//	   	ID::Value areaID_
//			エリア ID
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に存在する索引を定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS
ModVector<Index*>
Table::
getIndexArea(ID::Value areaID_, Trans::Transaction& cTrans_) const
{
	ModVector<Index*> v;

	const IndexMap& cMap = const_cast<Table*>(this)->loadIndex(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction1<Index, ID::Value>
				 (IndexMap::findByAreaID, areaID_));
	return v;
}

//	FUNCTION public
//	Schema::Table::getIndex --
//		表に存在する索引のうち、
//		あるエリア種別に対応する索引を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value	eCategory_
//			このエリア種別に該当する索引を得る
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		指定されたエリア種別に該当する索引を並べたベクター
//
//	EXCEPTIONS

ModVector<Index*>
Table::
getIndex(AreaCategory::Value eCategory_, Trans::Transaction& cTrans_) const
{
	ModVector<Index*> v;

	const IndexMap& cMap = const_cast<Table*>(this)->loadIndex(cTrans_);

	AutoRWLock l(getRWLock());
	cMap.extract(v,
				 BoolFunction1<Index, AreaCategory::Value>
				 (IndexMap::findByAreaCategory, eCategory_));
	return v;
}

//	FUNCTION public
//	Schema::Table::getIndex --
//		表に存在する索引のうち、
//		あるスキーマオブジェクト ID の索引を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	indexID
//			索引のスキーマオブジェクト ID
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた索引を格納する領域の先頭アドレス
//		0
//			表には指定された
//			スキーマオブジェクト ID の索引は存在しない
//
//	EXCEPTIONS

Index*
Table::
getIndex(ID::Value indexID, Trans::Transaction& cTrans_) const
{
	if (indexID == ID::Invalid)
		return 0;

	const IndexMap& cMap = const_cast<Table*>(this)->loadIndex(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.get(indexID).get();
}

//	FUNCTION public
//	Schema::Table::getIndex --
//		表に存在する索引のうち、ある名前の表を表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	indexName
//			表の名前
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた索引を格納する領域の先頭アドレス
//		0
//			表には指定された名前の索引は存在しない
//
//	EXCEPTIONS

Index*
Table::
getIndex(const Name& indexName, Trans::Transaction& cTrans_) const
{
	const IndexMap& cMap = const_cast<Table*>(this)->loadIndex(cTrans_);

	AutoRWLock l(getRWLock());
	return cMap.find(BoolFunction1<Index, const Name&>(_Bool::_findByName<Index>, indexName));
}

//	FUNCTION public
//	Schema::Table::addIndex --
//		表を表すクラスの索引として、
//		指定された索引を表すクラスを追加する
//
//	NOTES
//		「索引」表は更新されない
//
//	ARGUMENTS
//		const Schema::Index::Pointer&		index
//			追加する索引を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加した索引を表すクラス
//
//	EXCEPTIONS

const Index::Pointer&
Table::
addIndex(const Index::Pointer& index, Trans::Transaction& cTrans_)
{
	// 「索引」表のうち、この表に関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられた索引を追加する

	(void) loadIndex(cTrans_);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_indices->insert(index);

	return index;
}

//	FUNCTION public
//	Schema::Table::eraseIndex --
//		表を表すクラスからある索引を表すクラスの登録を抹消する
//
//	NOTES
//		「索引」表は更新されない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	indexID
//			登録を抹消する索引のオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
eraseIndex(ID::Value indexID)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_indices)
		_indices->erase(indexID);
}

// FUNCTION public
//	Schema::Table::eraseIndex -- ある索引の抹消(resetつき)
//
// NOTES
//
// ARGUMENTS
//	Database& cDatabase_
//	ID::Value iIndexID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
eraseIndex(Database& cDatabase_, ID::Value iIndexID_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	if (_indices) {
		_indices->erase(cDatabase_, iIndexID_);
	}
}

//	FUNCTION public
//	Schema::File::resetIndex --
//		表には索引を表すクラスが登録されていないことにする
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
Table::resetIndex()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_indices) {

		if (getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_indices->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_indices->reset();

	} else {
		// 索引を表すクラスを登録するハッシュマップを生成する

		_indices = new IndexMap;
		; _SYDNEY_ASSERT(_indices);
	}
}

//	FUNCTION public
//	Schema::Table::resetIndex --
//		表には索引を表すクラスが登録されていないことにする
//
//	NOTES
//		「索引」表は更新されない
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
resetIndex(Database& cDatabase_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_indices) {
		_indices->reset(cDatabase_);

	} else {
		// 索引を表すクラスを登録するハッシュマップを生成する

		_indices = new IndexMap;
		; _SYDNEY_ASSERT(_indices);
	}
}

//	FUNCTION public
//	Schema::Table::clearIndex --
//		表を表すクラスに登録されている索引を表すクラスと、
//		その管理用のハッシュマップを破棄する
//
//	NOTES
//		「索引」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
clearIndex(Trans::Transaction& cTrans_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_indices) {

		// ハッシュマップに登録されている索引を表すクラスがあれば、
		// すべて破棄し、ハッシュマップも破棄する

		resetIndex(*getDatabase(cTrans_));
		delete _indices, _indices = 0;
	}
}

//	FUNCTION public
//	Schema::Table::getFile --
//		表に存在するすべてのファイルを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に存在するファイルを定義順にひとつづつ要素とするベクター
//
//	EXCEPTIONS

const ModVector<Schema::File*>&
Table::
getFile(Trans::Transaction& cTrans_) const
{
	BoolFunction0<File>::Func func = _Bool::_Deleted;
	return const_cast<Table*>(this)->loadFile(cTrans_).getView(
		getRWLockForFile(), BoolFunction0<File>(func));
}

//	FUNCTION public
//	Schema::Table::getFile --
//		表に存在するファイルのうち、
//		あるエリア種別に対応するファイルを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::AreaCategory::Value	eCategory_
//			このエリア種別に該当するファイルを得る
//		const ModVector<Schema::Object::ID::Value>& vecPrevAreaID_
//			表に割り当てられているエリアを表す配列
//			呼び出し側の状況によりm_veciAreaIDだったりログから取得した配列だったりする
//		const ModVector<Schema::Object::ID::Value>& vecPostAreaID_
//			ALTERで移動するファイルを得るときには移動後のエリア配列も渡される
//			ALTER以外の場合はPrevと同じ内容の配列が渡される
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		指定されたエリア種別に該当するファイルを並べたベクター
//
//	EXCEPTIONS

ModVector<File*>
Table::
getFile(AreaCategory::Value eCategory_,
		const ModVector<ID::Value>& vecPrevAreaID_,
		const ModVector<ID::Value>& vecPostAreaID_,
		Trans::Transaction& cTrans_) const
{
	ModVector<File*> v;

	const FileMap& cMap = const_cast<Table*>(this)->loadFile(cTrans_);

	AutoRWLock l(getRWLockForFile());
	cMap.extract(v,
				 BoolFunction3<File, AreaCategory::Value, const ModVector<ID::Value>&, const ModVector<ID::Value>&>
				 (_Area::_findFile, eCategory_, vecPrevAreaID_, vecPostAreaID_));
	return v;
}

//	FUNCTION public
//	Schema::Table::getFile --
//		表に存在するファイルのうち、
//		あるスキーマオブジェクト ID のファイルを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	fileID
//			ファイルのスキーマオブジェクト ID
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたファイルを格納する領域の先頭アドレス
//		0
//			表には指定された
//			スキーマオブジェクト ID のファイルは存在しない
//
//	EXCEPTIONS

File*
Table::
getFile(ID::Value fileID, Trans::Transaction& cTrans_) const
{
	if (fileID == ID::Invalid)
		return 0;

	const FileMap& cMap = const_cast<Table*>(this)->loadFile(cTrans_);

	AutoRWLock l(getRWLockForFile());
	return cMap.get(fileID).get();
}

//	FUNCTION public
//	Schema::Table::getFile --
//		表に存在するファイルのうち、
//		ある名前のファイルを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Name&	fileName
//			ファイルの名前
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたファイルを格納する領域の先頭アドレス
//		0
//			表には指定された名前のファイルは存在しない
//
//	EXCEPTIONS

File*
Table::
getFile(const Name& fileName, Trans::Transaction& cTrans_) const
{
	const FileMap& cMap = const_cast<Table*>(this)->loadFile(cTrans_);

	AutoRWLock l(getRWLockForFile());
	return cMap.find(BoolFunction1<File, const Name&>(_Bool::_findByName<File>, fileName));
}

//	FUNCTION public
//	Schema::Table::getIndexFile --
//		表に存在するファイルのうち、
//		ある索引をじつげんするファイルを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	indexID
//			索引のスキーマオブジェクト ID
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたファイルを格納する領域の先頭アドレス
//		0
//			表には指定された
//			スキーマオブジェクト ID のファイルは存在しない
//
//	EXCEPTIONS

File::Pointer
Table::
getIndexFile(ID::Value indexID, Trans::Transaction& cTrans_) const
{
	const FileMap& cMap = const_cast<Table*>(this)->loadFile(cTrans_);

	AutoRWLock l(getRWLockForFile());

	FileMap::ConstIterator iterator = cMap.begin();
	const FileMap::ConstIterator& end = cMap.end();
	for (; iterator != end; ++iterator) {
		const File::Pointer& pFile = FileMap::getValue(iterator);
		if (pFile.get() && pFile->getIndexID() == indexID)
			return pFile;
	}
	return File::Pointer();
}

// FUNCTION public
//	Schema::Table::getFieldFile -- フィールドを保持するファイルを得る
//
// NOTES
//
// ARGUMENTS
//	const Schema::Field& cField_
//		フィールドのスキーマオブジェクトID
//	Trans::Transaction& cTrans_
//		トランザクション記述子
//	
// RETURN
//	File::Pointer
//
// EXCEPTIONS

File::Pointer
Table::
getFieldFile(const Field& cField_, Trans::Transaction& cTrans_) const
{
	const FileMap& cMap = const_cast<Table*>(this)->loadFile(cTrans_);

	AutoRWLock l(getRWLockForFile());

	FileMap::ConstIterator iterator = cMap.find(cField_.getParentID());
	if (iterator != cMap.end())
		return FileMap::getValue(iterator);
	return File::Pointer();
}

//	FUNCTION public
//	Schema::Table::addFile --
//		表を表すクラスのファイルとして、
//		指定されたファイルを表すクラスを追加する
//
//	NOTES
//		「ファイル」表は更新されない
//
//	ARGUMENTS
//		const Schema::File::Pointer&		file
//			追加するファイルを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		追加したファイルを表すクラス
//
//	EXCEPTIONS

const File::Pointer&
Table::
addFile(const File::Pointer& file, Trans::Transaction& cTrans_)
{
	// 「ファイル」表のうち、この表に関する部分を
	// 読み出していなければ、まず読み出してから、
	// 与えられたファイルを追加する

	(void) loadFile(cTrans_);

	AutoRWLock l(getRWLockForFile(), Os::RWLock::Mode::Write);
	_files->insert(file);

	return file;
}

//	FUNCTION public
//	Schema::Table::eraseFile --
//		表を表すクラスからあるファイルを表すクラスの登録を抹消する
//
//	NOTES
//		「ファイル」表は更新されない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	fileID
//			登録を抹消するファイルのオブジェクト ID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
eraseFile(ID::Value fileID)
{
	AutoRWLock l(getRWLockForFile(), Os::RWLock::Mode::Write);

	if (_files)
		_files->erase(fileID);
}

//	FUNCTION public
//	Schema::File::resetFile --
//		表にはファイルを表すクラスが登録されていないことにする
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
Table::resetFile()
{
	AutoRWLock l(getRWLockForFile(), Os::RWLock::Mode::Write);

	if (_files) {

		if (getScope() == Scope::Permanent)
			Manager::ObjectTree::Database::decrementCacheSize(_files->getSize());

		// ベクターを空にする
		// キャッシュには入っていないので
		// 他のもののようにループしながら
		// キャッシュから削除して回る必要はない
		_files->reset();

	} else {
		// ファイルを表すクラスを登録するハッシュマップを生成する

		_files = new FileMap;
		; _SYDNEY_ASSERT(_files);
	}
}

//	FUNCTION public
//	Schema::Table::resetFile --
//		表にはファイルを表すクラスが登録されていないことにする
//
//	NOTES
//		「ファイル」表は更新されない
//
//	ARGUMENTS
//		Database& cDatabase_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
resetFile(Database& cDatabase_)
{
	AutoRWLock l(getRWLockForFile(), Os::RWLock::Mode::Write);

	if (_files) {
		_files->reset(cDatabase_);

	} else {
		// ファイルを表すクラスを登録するハッシュマップを生成する

		_files = new FileMap;
		; _SYDNEY_ASSERT(_files);
	}
}

//	FUNCTION public
//	Schema::Table::clearFile --
//		表を表すクラスに登録されているファイルを表すクラスと、
//		その管理用のハッシュマップを破棄する
//
//	NOTES
//		「ファイル」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
clearFile(Trans::Transaction& cTrans_)
{
	AutoRWLock l(getRWLockForFile(), Os::RWLock::Mode::Write);

	if (_files) {

		// ハッシュマップに登録されているファイルを表すクラスがあれば、
		// すべて破棄し、ハッシュマップも破棄する

		resetFile(*getDatabase(cTrans_));
		delete _files, _files = 0;

	}
}

// FUNCTION public
//	Schema::Table::getPartition -- 分散ルールを表すクラスを得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Partition*
//
// EXCEPTIONS

Partition*
Table::
getPartition(Trans::Transaction& cTrans_) const
{
	AutoRWLock l(getRWLock());
	if (!m_pPartition.get()) {
		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		if (!m_pPartition.get()) {
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			Partition::Pointer pPartition = pDatabase->getTablePartition(getID(), cTrans_);
			if (pPartition.get()) {
				return const_cast<Table*>(this)->setPartition(pPartition).get();
			}
		}
	}
	return m_pPartition.get();
}

// FUNCTION public
//	Schema::Table::setPartition -- 分散ルールを表すクラスを設定する
//
// NOTES
//
// ARGUMENTS
//	const PartitionPointer& pPartition_
//	
// RETURN
//	const PartitionPointer&
//
// EXCEPTIONS

const PartitionPointer&
Table::
setPartition(const PartitionPointer& pPartition_)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	m_iPartitionID = pPartition_->getID();
	return m_pPartition = pPartition_;
}

// FUNCTION public
//	Schema::Table::clearPartition -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
clearPartition()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	m_iPartitionID = ID::Invalid;
	m_pPartition = Partition::Pointer();
}

// FUNCTION public
//	Schema::Table::getPartitionID -- 
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
Table::
getPartitionID()
{
	return m_iPartitionID;
}

// FUNCTION public
//	Schema::Table::setPartitionID -- 
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

inline // 内部でしか使わない
void
Table::
setPartitionID(ID::Value id_)
{
	m_iPartitionID = id_;
}

#ifdef OBSOLETE // Fieldを得る機能は使用されない

//	FUNCTION public
//	Schema::Table::getSource --
//		表に存在するすべての列のそれぞれについて、
//		列の値を格納するフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//
//	RETURN
//		表に存在する列の値を格納するフィールドをひとつづつ要素とするベクター
//
//	EXCEPTIONS

ModVector<Field*>
Table::
getSource(Trans::Transaction& cTrans_) const
{
	const ModVector<Column*>& columns = getColumn(cTrans_);

	ModVector<Field*>	v;
	ModSize	n = columns.getSize();
	v.reserve(n);

	for (ModSize i = 0; i < n; ++i) {
		; _SYDNEY_ASSERT(columns[i]);
		if (columns[i]->getField(cTrans_)) {
			v.pushBack(columns[i]->getField(cTrans_));
		}
	}

	return v;
}
#endif

//	FUNCTION public
//	Schema::Table::loadColumn --
//		表に存在するすべての列を表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		表に存在する列をひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const ColumnMap&
Table::
loadColumn(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());
	if (!_columns) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!_columns) {
			// 「列」表のうち、この表に関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが一時オブジェクトかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			if ((getScope() != Scope::Permanent
				 || getStatus() == Status::Created)
				&& !bRecovery_)
				resetColumn(*pDatabase);
			else {
				SystemTable::Column(*pDatabase).load(cTrans_, *this, bRecovery_);
			}
			; _SYDNEY_ASSERT(_columns);
		}
	}
	return *_columns;
}

//	FUNCTION public
//	Schema::Table::loadConstraint --
//		表に存在するすべての制約を表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		表に存在する制約をひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const ConstraintMap&
Table::
loadConstraint(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());

	if (!_constraints) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!_constraints) {
			// 「制約」表のうち、この表に関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが一時オブジェクトかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			if ((getScope() != Scope::Permanent
				 || getStatus() == Status::Created)
				&& !bRecovery_)
				resetConstraint(*pDatabase);
			else {
				SystemTable::Constraint(*pDatabase).load(cTrans_, *this, bRecovery_);
			}
			; _SYDNEY_ASSERT(_constraints);
		}
	}
	return *_constraints;
}

//	FUNCTION public
//	Schema::Table::loadIndex --
//		表に存在するすべての索引を表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		表に存在する索引をひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const IndexMap&
Table::
loadIndex(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLock());

	if (!_indices) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!_indices) {
			// 「索引」表のうち、この表に関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが一時オブジェクトかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			if ((getScope() != Scope::Permanent
				 || getStatus() == Status::Created)
				&& !bRecovery_)
				resetIndex(*pDatabase);
			else {
				SystemTable::Index(*pDatabase).load(cTrans_, *this, bRecovery_);
			}
			; _SYDNEY_ASSERT(_indices);
		}
	}
	return *_indices;
}

//	FUNCTION public
//	Schema::Table::loadFile --
//		表に存在するすべてのファイルを表すクラスを読み出す
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		bool bRecovery_ = false
//			trueの場合Mount後のリカバリーのための特別な動作を行う
//
//	RETURN
//		表に存在するファイルをひとつづつ要素とするハッシュマップ
//
//	EXCEPTIONS

const FileMap&
Table::
loadFile(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */)
{
	AutoRWLock l(getRWLockForFile());

	if (!_files) {
		l.convert(Os::RWLock::Mode::Write);

		// 書き込みロックの中で再度調べる
		if (!_files) {
			// 「ファイル」表のうち、この表に関する部分を
			// 一度も読み出していないので、まず、読み出す
			// これが一時オブジェクトかもしくは作成後永続化していないなら
			// 読み出す必要はないのでベクターを初期化するのみ
			Database* pDatabase = getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			if ((getScope() != Scope::Permanent
				 || getStatus() == Status::Created)
				&& !bRecovery_)
				resetFile(*pDatabase);
			else {
				SystemTable::File(*pDatabase).load(cTrans_, *this, bRecovery_);

				// Check the fileID for EncodingForm and FieldLength inconsistency
				// *Warning*
				//    This method must be called after all the file objects are loaded
				FileMap::Iterator iterator = _files->begin();
				const FileMap::Iterator end = _files->end();
				for (; iterator != end; ++iterator)
					FileMap::getValue(iterator)->checkFieldType(cTrans_);

				// loadしたらキャッシュサイズを調べる
				if (Manager::ObjectTree::Database::checkCacheSize()) {
					// 超えていたらキャッシュをクリアする
					Manager::ObjectTree::Database::clearCache();
				}
			}
			; _SYDNEY_ASSERT(_files);
		}
	}
	return *_files;
}

//	FUNCTION public
//	Schema::Table::doPreLoad --
//		表を構成するスキーマオブジェクトをあらかじめすべて読み込む
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をするトランザクション記述子
//		Schema::Database& cDatabase_
//			表が属するデータベース
//			doPreloadがgetDatabaseから呼ばれるので、
//			内部でgetDatabase()を使うことができない
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
doPreLoad(Trans::Transaction& cTrans_, Database& cDatabase_)
{
	bool output = false;

	// 表に属する列と制約を読み込む

	if (!_columns)
		SystemTable::Column(cDatabase_).load(cTrans_, *this);

	if (!_constraints)
		SystemTable::Constraint(cDatabase_).load(cTrans_, *this);

	// 表に属する索引をすべて読み込み、それぞれについてキーを読み込む
	if (!_indices) {

		SystemTable::Index(cDatabase_).load(cTrans_, *this);

		IndexMap::Iterator iterator = _indices->begin();
		const IndexMap::Iterator& end = _indices->end();

		for (; iterator != end; ++iterator) {
			const Index::Pointer& pIndex = IndexMap::getValue(iterator);
			if (pIndex.get())
				SystemTable::Key(cDatabase_).load(cTrans_, *pIndex);
		}
	}
	// 表に属するファイルをすべて読み込み、それぞれについてフィールドを読み込む
	if (!_files) {

		SystemTable::File(cDatabase_).load(cTrans_, *this);

		FileMap::Iterator iterator = _files->begin();
		const FileMap::Iterator& end = _files->end();

		for (; iterator != end; ++iterator) {
			const File::Pointer& pFile = FileMap::getValue(iterator);
			if (pFile.get())
				SystemTable::Field(cDatabase_).load(cTrans_, *pFile);
		}
	}
}

//	FUNCTION public
//	Schema::Table::serialize --
//		表を表すクラスのシリアライザー
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
Table::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// 表のヒント

		bool hasHint = (m_pHint) ? 1 : 0;
		archiver << hasHint;
		if (hasHint) archiver << *m_pHint;

		// エリア情報を格納する
		for (int i = 0; i < AreaCategory::ValueNum; i++)
			archiver << m_veciAreaID[i];

		// 一時表か
		archiver << m_bTemporary;

		// columns
		{
			ModSize n = (_columns) ? _columns->getSize() : 0;
			archiver << n;
			if (n) {
				Utility::OutputArchive& out =
					dynamic_cast<Utility::OutputArchive&>(archiver);

				_columns->writeObject(out);
			}
		}

		// constraints
		{
			ModSize n = (_constraints) ? _constraints->getSize() : 0;
			archiver << n;
			if (n) {
				Utility::OutputArchive& out =
					dynamic_cast<Utility::OutputArchive&>(archiver);

				_constraints->writeObject(out);
			}
		}

		// indices
		{
			ModSize n = (_indices) ? _indices->getSize() : 0;
			archiver << n;
			if (n) {
				Utility::OutputArchive& out =
					dynamic_cast<Utility::OutputArchive&>(archiver);

				_indices->writeObject(out);
			}
		}

		// files
		{
			ModSize n = (_files) ? _files->getSize() : 0;
			archiver << n;
			if (n) {
				Utility::OutputArchive& out =
					dynamic_cast<Utility::OutputArchive&>(archiver);

				_files->writeObject(out);
			}
		}

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// 表のヒント

		bool hasHint;
		archiver >> hasHint;
		if (hasHint) {
			Hint cHint;
			archiver >> cHint;
			m_pHint = new Hint(cHint);
		}

		// エリア情報を得る
		for (int i = 0; i < AreaCategory::ValueNum; i++)
			archiver >> m_veciAreaID[i];

		// 一時表か
		archiver >> m_bTemporary;

		// columns
		{
			ModSize n;
			archiver >> n;
			if (n) {
				Utility::InputArchive& in =
					dynamic_cast<Utility::InputArchive&>(archiver);

				resetColumn();
				_columns->readObject(in, n);
			}
		}

		// constraints
		{
			ModSize n;
			archiver >> n;
			if (n) {
				Utility::InputArchive& in =
					dynamic_cast<Utility::InputArchive&>(archiver);

				resetConstraint();
				_constraints->readObject(in, n);
			}
		}

		// indices
		{
			ModSize n;
			archiver >> n;
			if (n) {
				Utility::InputArchive& in =
					dynamic_cast<Utility::InputArchive&>(archiver);

				resetIndex();
				_indices->readObject(in, n);
			}
		}

		// files
		{
			ModSize n;
			archiver >> n;
			if (n) {
				Utility::InputArchive& in =
					dynamic_cast<Utility::InputArchive&>(archiver);

				resetFile();
				_files->readObject(in, n);
			}
		}
	}
}

//	FUNCTION public
//	Schema::Table::getClassID -- このクラスのクラス ID を得る
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
Table::
getClassID() const
{
	return Externalizable::Category::Table +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Table::clear --
//		表を表すクラスのメンバーをすべて初期化する
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
Table::
clear()
{
	destruct();
}

// FUNCTION private
//	Schema::Table::createColumn -- Create column objects according to TableElementList
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Statement::TableElementList* pElement_
//	Common::DataArrayData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
createColumn(Trans::Transaction& cTrans_,
			 Statement::TableElementList* pElement_,
			 Common::DataArrayData& cLogData_)
{
	if (pElement_) {

		Column::Position iPos = _columns ? _columns->getSize() : 0;

		// count of all the table elements
		int	n = pElement_->getCount();

		// prepare log data
		// [NOTES]
		//	size might be larger than necessary value,
		//	but reducing allocation count is more important.
		cLogData_.reserve(n);

		// create column objects
		for (int i = 0; i < n; ++i) {
			Statement::Object* pObject = pElement_->getAt(i);
			; _SYDNEY_ASSERT(pObject);

			switch (pObject->getType()) {
			case Statement::ObjectType::ColumnDefinition:
				{
					// ColumnDefinition
					Statement::ColumnDefinition* pDefinition =
						_SYDNEY_DYNAMIC_CAST(Statement::ColumnDefinition*, pObject);

					createColumn(cTrans_, pDefinition, iPos++, cLogData_);

					break;
				}
			default:
				{
					break;
				}
			}
		}
	}
}

//	FUNCTION private
//	Schema::Table::createColumn --
//		列定義から列を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		Statement::ColumnDefinition* pColumnDefinition_
//			列定義のStatement
//		Column::Position iPosition_
//			表内の列の位置を格納する変数
//		Common::DataArrayData& cLogData_
//			列定義のログデータを格納する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Column::Pointer
Table::
createColumn(Trans::Transaction& cTrans_,
			 Statement::ColumnDefinition* pColumnDefinition_,
			 Column::Position iPosition_,
			 Common::DataArrayData& cLogData_)
{
	Column::Pointer column =
		Column::create(*this, iPosition_, *pColumnDefinition_, cTrans_);

	; _SYDNEY_ASSERT(column.get() || Manager::Configuration::isCanceledWhenDuplicated());

	// 状態は「生成」である
	; _SYDNEY_ASSERT(!column.get()
					 || (column->getStatus() == Status::Created ||
						 column->getStatus() == Status::Mounted));

	if (column.get()) {
		// 表にこの列を表すクラスを追加する
		// ★注意★
		// キャッシュに加えるのは永続化の後
						
		(void) addColumn(column, cTrans_);

		// ログデータを加える
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData;
		column->makeLogData(*pData);
		cLogData_.pushBack(pData.release());
	}
	return column;
}

// FUNCTION public
//	Schema::Table::createColumn -- ログデータから列オブジェクトを生成する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Common::DataArrayData& cLogData_
//	Column::Position iPosition_
//	const Column* pOriginalColumn_ /* = 0 */
//	
// RETURN
//	Column::Pointer
//
// EXCEPTIONS

Column::Pointer
Table::
createColumn(Trans::Transaction& cTrans_,
			 const Common::DataArrayData& cLogData_,
			 Column::Position iPosition_,
			 const Column* pOriginalColumn_ /* = 0 */)
{
	Column::Pointer pColumn = Column::create(*this, iPosition_, cLogData_, cTrans_, pOriginalColumn_);

	// 状態は「生成」である
	; _SYDNEY_ASSERT(!pColumn.get()
					 || (pOriginalColumn_ && pColumn->getStatus() == pOriginalColumn_->getStatus())
					 || (!pOriginalColumn_
						 && (pColumn->getStatus() == Status::Created
							 || pColumn->getStatus() == Status::Mounted)));

	// 表にこの列を表すクラスを追加する
	// ★注意★
	// キャッシュに加えるのは永続化の後

	if ( pColumn.get() )
		(void) addColumn(pColumn, cTrans_);

	return pColumn;
}

// FUNCTION public
//	Schema::Table::createConstraint -- create constraint objects according to TableElementList
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Statement::TableElementList* pElement_
//	Common::DataArrayData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
createConstraint(Trans::Transaction& cTrans_,
				 Statement::TableElementList* pElement_,
				 Common::DataArrayData& cLogData_)
{
	if (pElement_) {

		Constraint::Position iPos = _constraints ? _constraints->getSize() : 0;

		// count of all the table elements
		int	n = pElement_->getCount();

		// prepare log data
		// [NOTES]
		//	size might be larger than necessary value,
		//	but reducing allocation count is more important.
		cLogData_.reserve(n);

		// create column objects
		for (int i = 0; i < n; ++i) {
			Statement::Object* pObject = pElement_->getAt(i);
			; _SYDNEY_ASSERT(pObject);

			switch (pObject->getType()) {
			case Statement::ObjectType::TableConstraintDefinition:
				{
					// ColumnDefinition
					Statement::TableConstraintDefinition* pDefinition =
						_SYDNEY_DYNAMIC_CAST(Statement::TableConstraintDefinition*, pObject);

					createConstraint(cTrans_, pDefinition, iPos++, cLogData_);

					break;
				}
			default:
				{
					break;
				}
			}
		}
	}
}

//	FUNCTION private
//	Schema::Table::createConstraint --
//		制約定義から制約を表すクラスを生成する
//
//	NOTES
//		制約に使用する索引はファイルのオブジェクトが生成されてから定義される
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作をするトランザクション記述子
//		Statement::TableConstraintDefinition* pConstraintDefinition_
//			制約定義のStatement
//		Column::Position& iPosition_
//			表内の制約の位置を格納する変数
//		Common::DataArrayData& cLogData_
//			制約定義のログデータを格納する変数
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Constraint::Pointer
Table::
createConstraint(Trans::Transaction& cTrans_,
				 Statement::TableConstraintDefinition* pConstraintDefinition_,
				 Constraint::Position iPosition_,
				 Common::DataArrayData& cLogData_)
{
	Constraint::Pointer constraint =
		Constraint::create(*this, iPosition_, *pConstraintDefinition_, cTrans_);

	// 制約の種類によっては無視する場合があるので
	// 他のもののようにアサートでは受けない

	if (constraint.get()) {
		// 状態は「生成」である
		; _SYDNEY_ASSERT(constraint->getStatus() == Status::Created);

		// 表にこの制約を表すクラスを追加する
		// ★注意★
		// キャッシュに加えるのは永続化の後
		(void) addConstraint(constraint, cTrans_);

		// ログデータを加える
		ModAutoPointer<Common::DataArrayData> pData = new Common::DataArrayData;
		constraint->makeLogData(cTrans_, *pData);
		cLogData_.pushBack(pData.release());
	}
	return constraint;
}

// FUNCTION private
//	Schema::Table::createConstraint -- ログデータから制約オブジェクトを生成する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Common::DataArrayData& cLogData_
//	Constraint::Position iPosition_
//	
// RETURN
//	Constraint::Pointer
//
// EXCEPTIONS

Constraint::Pointer
Table::
createConstraint(Trans::Transaction& cTrans_,
				 const Common::DataArrayData& cLogData_,
				 Constraint::Position iPosition_)
{
	Constraint::Pointer pConstraint = Constraint::create(*this, iPosition_, cLogData_, cTrans_);
	; _SYDNEY_ASSERT(pConstraint.get());

	// 状態は「生成」である
	; _SYDNEY_ASSERT(pConstraint->getStatus() == Status::Created
					 || pConstraint->getStatus() == Status::Mounted);

	// 表にこの制約を表すクラスを追加する
	// ★注意★
	// キャッシュに加えるのは永続化の後
	return addConstraint(pConstraint, cTrans_);
}

// FUNCTION private
//	Schema::Table::createFile -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Common::DataArrayData* pLogData_ /* = 0 */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
createFile(Trans::Transaction& cTrans_,
		   const Common::DataArrayData* pLogData_ /* = 0 */)
{
    // 表ひとつに対して、レコードファイルをひとつ作る
    // 一時表のときに「一時的である」という指示を付加するのはFile::create
    // での処理
    //
    //【注意】	垂直分割が実装されると、こうはいかない
	const Common::DataArrayData* pFileLogData = 0;
	if (pLogData_) {
		ModSize iPosition = getFile(cTrans_).getSize();
		if (pLogData_->getCount() > iPosition) {
			pFileLogData =
				&(LogData::getDataArrayData(pLogData_->getElement(iPosition)));
		}
	}
    const File::Pointer& file =
        File::create(cTrans_,
					 RecordFile::create(cTrans_, *getDatabase(cTrans_), *this, m_pHint),
					 *this,
					 pFileLogData);
    
    ; _SYDNEY_ASSERT(file.get());
    
    // 状態は「生成」である
    
    ; _SYDNEY_ASSERT(file->getStatus() == Status::Created
					 || file->getStatus() == Status::Mounted);

    // 表を構成する列の値を格納するフィールドを、
    // レコードファイルへ加えていく

    const ModVector<Column*>& columns = getColumn(cTrans_);

    ModSize n = columns.getSize();
    for (ModSize i = 0; i < n; ++i) {
        Column* column = columns[i];
        ; _SYDNEY_ASSERT(column);

        // const列はファイルに格納しない
		if (column->getCategory() == Column::Category::Constant) {
			continue;
		}

		// add a field using a column
		addField(cTrans_, *getDatabase(cTrans_), *column, file, 0, pLogData_, pFileLogData);
    }

	// 生成する表に対応するレコードファイルのファイル ID を設定する

	file->setFileID(cTrans_);
}

// FUNCTION private
//	Schema::Table::createVectorFile -- create Vector file
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	Column& cColumn_
//	const File::Pointer& pFile_
//	const Common::DataArrayData* pFileLogData_ /* = 0 */
//	const Common::DataArrayData* pfVectorLogData_ /* = 0 */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
createVectorFile(Trans::Transaction& cTrans_,
				 Database& cDatabase_, Column& cColumn_,
				 const File::Pointer& pFile_,
				 const Common::DataArrayData* pFileLogData_ /* = 0 */,
				 const Common::DataArrayData* pVectorLogData_ /* = 0 */)
{
	File::Pointer pVectorFile =
		File::create(cTrans_,
					 VectorFile::create(cTrans_, cDatabase_, *this, m_pHint, 0,
										true // hasAllTuples
					 ),
					 *this,
					 pVectorLogData_);
	; _SYDNEY_ASSERT(pVectorFile.get());
	; _SYDNEY_ASSERT(pVectorFile->getStatus() == Status::Created
					 || pVectorFile->getStatus() == Status::Mounted);

	{
		Field* source = cColumn_.getField(cTrans_);
		; _SYDNEY_ASSERT(source);

		ID::Value iSourceID = pVectorLogData_ ? pVectorFile->getNextFieldID(cTrans_, pVectorLogData_) : ID::Invalid;
		(void)pVectorFile->addField(Field::Category::Key,
									Field::Permission::All, *source, cColumn_,
									cTrans_,
									iSourceID);
	}
	{
		Field* source = pFile_->getObjectID(cTrans_);
		; _SYDNEY_ASSERT(source);
                
		ID::Value iSourceID = pVectorLogData_ ? pVectorFile->getNextFieldID(cTrans_, pVectorLogData_) : ID::Invalid;
		(void)pVectorFile->addField(Field::Category::Data,
									Field::Permission::All, *source,
									cTrans_,
									iSourceID);
	}

	// create the fileid of the conversion file
	pVectorFile->setFileID(cTrans_);
}

// FUNCTION private
//	Schema::Table::createHeapFile -- create Heap file
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	Column& cColumn_
//	const File::Pointer& pFile_
//	const Common::DataArrayData* pFileLogData_ /* = 0 */
//	const Common::DataArrayData* pHeapLogData_ /* = 0 */
//	
// RETURN
//	Field::Pointer
//
// EXCEPTIONS

Field::Pointer
Table::
createHeapFile(Trans::Transaction& cTrans_,
			   Database& cDatabase_, Column& cColumn_,
			   const File::Pointer& pFile_,
			   const Common::DataArrayData* pFileLogData_ /* = 0 */,
			   const Common::DataArrayData* pHeapLogData_ /* = 0 */)
{
	File::Pointer pHeapFile =
		File::create(cTrans_,
					 RecordFile::create(cTrans_, cDatabase_, *this, m_pHint, 0, &cColumn_),
					 *this,
					 pHeapLogData_);
	; _SYDNEY_ASSERT(pHeapFile.get());
	; _SYDNEY_ASSERT(pHeapFile->getStatus() == Status::Created
					 || pHeapFile->getStatus() == Status::Mounted);
 
	ID::Value iFieldID = pHeapLogData_ ? pHeapFile->getNextFieldID(cTrans_, pHeapLogData_) : ID::Invalid;
	Field::Pointer pColumnField = pHeapFile->addField(cColumn_, cTrans_, iFieldID);

	// Add the field copying the objectid of heap file
	Field* source = pHeapFile->getObjectID(cTrans_);
	; _SYDNEY_ASSERT(source);

	ID::Value iSourceID = pFileLogData_ ? pFile_->getNextFieldID(cTrans_, pFileLogData_) : ID::Invalid;
	(void)pFile_->addField(Field::Category::Data,
						   Field::Permission::All, *source, cTrans_, iSourceID);

	// create fileid of the heap file
	pHeapFile->setFileID(cTrans_);

	return pColumnField;
}

// FUNCTION private
//	Schema::Table::createLobFile -- create Lob file
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	Column& cColumn_
//	const File::Pointer& pFile_
//	const Common::DataArrayData* pFileLogData_ /* = 0 */
//	const Common::DataArrayData* pLobLogData_ /* = 0 */
//	
// RETURN
//	Field::Pointer
//
// EXCEPTIONS

Field::Pointer
Table::
createLobFile(Trans::Transaction& cTrans_,
			  Database& cDatabase_, Column& cColumn_,
			  const File::Pointer& pFile_,
			  const Common::DataArrayData* pFileLogData_ /* = 0 */,
			  const Common::DataArrayData* pLobLogData_ /* = 0 */)
{
	File::Pointer pLobFile =
		File::create(cTrans_,
					 LobFile::create(cTrans_, cDatabase_, *this, m_pHint, 0, &cColumn_),
					 *this,
					 pLobLogData_);
	; _SYDNEY_ASSERT(pLobFile.get());
	; _SYDNEY_ASSERT(pLobFile->getStatus() == Status::Created
					 || pLobFile->getStatus() == Status::Mounted);

	ID::Value iFieldID = pLobLogData_ ? pLobFile->getNextFieldID(cTrans_, pLobLogData_) : ID::Invalid;
	Field::Pointer pColumnField = pLobFile->addField(cColumn_, cTrans_, iFieldID);

	// Add the field copying the objectid of lob file
	Field* source = pLobFile->getObjectID(cTrans_);
	; _SYDNEY_ASSERT(source);

	ID::Value iSourceID = pFileLogData_ ? pFile_->getNextFieldID(cTrans_, pFileLogData_) : ID::Invalid;
	(void)pFile_->addField(Field::Category::Data,
						   Field::Permission::All, *source, cTrans_, iSourceID);

	pLobFile->setFileID(cTrans_);

	return pColumnField;
}

// FUNCTION private
//	Schema::Table::makeFileLogData -- make log data for files
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const ModVector<File*>& vecFiles_
//	Common::DataArrayData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template<class E>
void
Table::
makeFileLogData(Trans::Transaction& cTrans_,
				const ModVector<E>& vecFiles_,
				Common::DataArrayData& cLogData_)
{
	typename ModVector<E>::ConstIterator iterator = vecFiles_.begin();
	const typename ModVector<E>::ConstIterator last = vecFiles_.end();
	for (; iterator != last; ++iterator) {
		ModAutoPointer<Common::DataArrayData> pElement = new Common::DataArrayData;
		(*iterator)->makeLogData(cTrans_, *pElement);
		cLogData_.pushBack(pElement.release());
	}
}

// FUNCTION private
//	Schema::Table::addField -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	Column& cColumn_
//	const FilePointer& pFile_
//	ModVector<FilePointer>* pvecNewFiles_ /* = 0 */
//	const Common::DataArrayData* pFileLogData_ /* = 0 */
//	const Common::DataArrayData* pTargetFileLogData_ /* = 0 */
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
addField(Trans::Transaction& cTrans_, Database& cDatabase_, Column& cColumn_,
		 const FilePointer& pFile_,
		 ModVector<FilePointer>* pvecNewFiles_ /* = 0 */,
		 const Common::DataArrayData* pFileLogData_ /* = 0 */,
		 const Common::DataArrayData* pTargetFileLogData_ /* = 0 */)
{
	Field::Pointer pColumnField;

	if (cColumn_.getType().getType() == Common::SQLData::Type::BLOB
		|| cColumn_.getType().getType() == Common::SQLData::Type::CLOB
		|| cColumn_.getType().getType() == Common::SQLData::Type::NCLOB) {

		// If the column is a LOB column, create a LOB file apart from the center file
		const Common::DataArrayData* pLobLogData = 0;
		if (pFileLogData_) {
			ModSize iPosition = pvecNewFiles_ ? pvecNewFiles_->getSize() : getFile(cTrans_).getSize();
			pLobLogData = &(LogData::getDataArrayData(pFileLogData_->getElement(iPosition)));
		}

		pColumnField = createLobFile(cTrans_, cDatabase_, cColumn_, pFile_,
									 pTargetFileLogData_, pLobLogData);

		if (pvecNewFiles_)
			pvecNewFiles_->pushBack(pColumnField->getFile(cTrans_));

	} else if (cColumn_.isInHeap()) {

		// If the column has 'HEAP' hint, create a heap file apart from the center file
		// hint heap will be ignored for LOB columns

		const Common::DataArrayData* pHeapLogData = 0;
		if (pFileLogData_) {
			ModSize iPosition = pvecNewFiles_ ? pvecNewFiles_->getSize() : getFile(cTrans_).getSize();
			pHeapLogData = &(LogData::getDataArrayData(pFileLogData_->getElement(iPosition)));
		}

		pColumnField = createHeapFile(cTrans_, cDatabase_, cColumn_, pFile_,
									  pTargetFileLogData_, pHeapLogData);

		if (pvecNewFiles_)
			pvecNewFiles_->pushBack(pColumnField->getFile(cTrans_));

	} else {

		pColumnField = pFile_->addField(cColumn_, cTrans_,
										pFile_->getNextFieldID(cTrans_, pTargetFileLogData_));
	}

	// set the field of the column
	(void) cColumn_.setField(*pColumnField);
        
	if (cColumn_.isTupleID()) {
		// If the table is not a temporary table,
		// create a vector file to be used in conversion from rowid into oid

		; _SYDNEY_ASSERT(!pvecNewFiles_);

		const Common::DataArrayData* pVectorLogData = 0;
		if (pFileLogData_) {
			ModSize iPosition = getFile(cTrans_).getSize();
			pVectorLogData =
				&(LogData::getDataArrayData(pFileLogData_->getElement(iPosition)));
		}
		createVectorFile(cTrans_, cDatabase_, cColumn_, pFile_,
						 pTargetFileLogData_, pVectorLogData);

	}
}

// FUNCTION public
//	Schema::Table::addReference -- 
//
// NOTES
//	Added id are stored in order of id value
//
// ARGUMENTS
//	Constraint& cConstraint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
addReference(Constraint& cConstraint_)
{
	; _SYDNEY_ASSERT(cConstraint_.getTableID() == getID());

	ID::Value iID = cConstraint_.getReferedTableID();
	; _SYDNEY_ASSERT(iID != ID::Invalid);

	if (cConstraint_.getCategory() == Constraint::Category::ForeignKey) {
		TableReferenceMap::Iterator iterator = m_mapReferencedTable.find(iID);
		if (iterator == m_mapReferencedTable.end()) {
			m_mapReferencedTable[iID] = 1;
		} else {
			++((*iterator).second);
		}
		delete m_vecReferencedTable, m_vecReferencedTable = 0;
	} else {
		; _SYDNEY_ASSERT(cConstraint_.getCategory() == Constraint::Category::ReferedKey);
		TableReferenceMap::Iterator iterator = m_mapReferencingTable.find(iID);
		if (iterator == m_mapReferencingTable.end()) {
			m_mapReferencingTable[iID] = 1;
		} else {
			++((*iterator).second);
		}
		delete m_vecReferencingTable, m_vecReferencingTable = 0;
	}
}

// FUNCTION public
//	Schema::Table::eraseReference -- 
//
// NOTES
//
// ARGUMENTS
//	Constraint& cConstraint_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Table::
eraseReference(Constraint& cConstraint_)
{
	; _SYDNEY_ASSERT(cConstraint_.getTableID() == getID());

	ID::Value iID = cConstraint_.getReferedTableID();
	; _SYDNEY_ASSERT(iID != ID::Invalid);

	if (cConstraint_.getCategory() == Constraint::Category::ForeignKey) {
		TableReferenceMap::Iterator iterator = m_mapReferencedTable.find(iID);
		if (iterator != m_mapReferencedTable.end()) {
			if (--(*iterator).second <= 0) {
				m_mapReferencedTable.erase(iterator);
				delete m_vecReferencedTable, m_vecReferencedTable = 0;
			}
		}
	} else {
		; _SYDNEY_ASSERT(cConstraint_.getCategory() == Constraint::Category::ReferedKey);
		TableReferenceMap::Iterator iterator = m_mapReferencingTable.find(iID);
		if (iterator != m_mapReferencingTable.end()) {
			if (--(*iterator).second <= 0) {
				m_mapReferencingTable.erase(iterator);
				delete m_vecReferencingTable, m_vecReferencingTable = 0;
			}
		}
	}
}

//	FUNCTION private
//	Schema::Table::createAreaContent --
//		エリア格納関係をあらわすクラスを生成する
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
Table::
createAreaContent(Trans::Transaction& cTrans_)
{
	Database* pDatabase = getDatabase(cTrans_);
	ModSize n = m_veciAreaID.getSize();

	for (ModSize i = 0; i < n; ++i) {
		if (m_veciAreaID[i] != ID::Invalid) {
			Area* pArea = pDatabase->getArea(m_veciAreaID[i], cTrans_);

			// リカバリー中の場合エリアが存在しない場合がある
			if (pArea) {
				// すでにある場合は何もしない
				AreaContent* pContent =
					pArea->getContent(getID(), static_cast<AreaCategory::Value>(i),
									  cTrans_);

				if (!pContent) {
					// 格納場所の対応関係を表すクラスを新たに追加する
					// ★注意★
					// ここで追加したAreaContentはこの後のエラーでTable::dropが呼ばれると
					// その中で作成が取り消されるのでここでエラー処理をする必要はない
					AreaContent::Pointer pContent =
						AreaContent::create(*pArea, *this,
											static_cast<AreaCategory::Value>(i), cTrans_);

					// 状態は「生成」である
					; _SYDNEY_ASSERT(pContent->getStatus() == Status::Created
									 || pContent->getStatus() == Status::Mounted);
				}
			}
			else
			{
				// 存在しない場合は AreaID を無効とし、無視する

				SydMessage << "Area 'ID=" << m_veciAreaID[i]
						   << "' not found in database '"
						   << pDatabase->getName() << "'"
						   << ModEndl;
				
				m_veciAreaID[i] = ID::Invalid;
			}
		}
	}
}

//	FUNCTION private
//	Schema::Table::moveAreaDefault -- カテゴリーがDefaultであるファイルの移動を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		ID::Value iPrevAreaID_,
//		ID::Value iPostAreaID_,
//			ファイルに対応する移動前後のエリアID
//		const ModVector<ID::Value>& vecPrevAreaID_
//		const ModVector<ID::Value>& vecPostAreaID_
//			移動前後の表に対するエリアID割り当て
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
moveAreaDefault(Trans::Transaction& cTrans_,
				ID::Value iPrevAreaID_, ID::Value iPostAreaID_,
				bool bUndo_, bool bRecovery_, bool bMount_)
{
	Database* pDatabase = getDatabase(cTrans_);

	if (bMount_ || bRecovery_) {
		// MOUNTまたはREDOのDROP AREAに伴う処理ではファイルの移動は行わない
		// パス設定だけを書き換える
		setPath(cTrans_, getDataPath(cTrans_, pDatabase, iPostAreaID_), getName());

	} else {
		// シーケンスファイルを移動する
		moveSequence(cTrans_,
					 getDataPath(cTrans_, pDatabase, iPrevAreaID_),
					 getDataPath(cTrans_, pDatabase, iPostAreaID_),
					 getName(), getName(),
					 bUndo_, bRecovery_);
	}
}

//	FUNCTION private
//	Schema::Table::moveAreaFile -- 指定されたカテゴリーに対応するファイルの移動を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		AreaCategory::Value eCat_
//			移動対象のエリア種別
//		ID::Value iPrevAreaID_,
//		ID::Value iPostAreaID_,
//			ファイルに対応する移動前後のエリアID
//		const ModVector<ID::Value>& vecPrevAreaID_
//		const ModVector<ID::Value>& vecPostAreaID_
//			移動前後の表に対するエリアID割り当て
//		ModVector<ID::Value>* pvecNonEmptyArea_ = 0
//			使用中のオブジェクトがあることを示す配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
moveAreaFile(Trans::Transaction& cTrans_, AreaCategory::Value eCat_,
			 ID::Value iPrevAreaID_, ID::Value iPostAreaID_,
			 const ModVector<ID::Value>& vecPrevAreaID_,
			 const ModVector<ID::Value>& vecPostAreaID_,
			 ModVector<ID::Value>* pvecNonEmptyArea_,
			 bool bUndo_, bool bRecovery_, bool bMount_)
{
	ModVector<File*> file;
	ModVector<ModSize> vecMoved;

	try {
		file = getFile(eCat_, vecPrevAreaID_, vecPostAreaID_, cTrans_);
		ModSize n = file.getSize();
		vecMoved.reserve(n);

		for (ModSize i = 0; i < n; ++i) {
			File* pFile = file[i];
			if (pFile->getIndexID() != ID::Invalid) {
				// 索引のエリア指定を調べる
				ID::Value indexAreaID = pFile->getIndex(cTrans_)->getAreaID(pFile->getAreaCategory(), false, cTrans_);
				if (indexAreaID != ID::Invalid ) {
					// 使用しているオブジェクトがあることを示す配列に追加する
					if (pvecNonEmptyArea_
						&& pvecNonEmptyArea_->find(indexAreaID) == pvecNonEmptyArea_->end())
						pvecNonEmptyArea_->pushBack(indexAreaID);

					// 索引にエリア指定があるものは移動対象にしない
					continue;
				}
			}

			// ファイルを移動する
			file[i]->moveArea(cTrans_, iPrevAreaID_, iPostAreaID_, bUndo_, bRecovery_, bMount_);
			vecMoved.pushBack(i);
		}
		if (!bRecovery_ && !bMount_)
			sweepUnusedArea(cTrans_, iPrevAreaID_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		// 移動したファイルを元に戻す
		ModSize n = vecMoved.getSize();
		for (ModSize i = 0; i < n; ++i) {
			file[vecMoved[i]]->moveArea(cTrans_, iPostAreaID_, iPrevAreaID_,
										true /* undo */, bRecovery_, bMount_);
		}
		// タプルIDのファイルも移動していた場合はそれも元に戻す
		if (eCat_ == AreaCategory::Default) {
			moveAreaDefault(cTrans_, iPostAreaID_, iPrevAreaID_, true /* undo */, bRecovery_, bMount_);
		}
		if (!bRecovery_ && !bMount_)
			sweepUnusedArea(cTrans_, iPostAreaID_);

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION private
//	Schema::Table::moveAreaIndex -- 指定されたカテゴリーに対応する索引の移動を行う
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		AreaCategory::Value eCat_
//			移動対象のエリア種別
//		ID::Value iPrevAreaID_,
//		ID::Value iPostAreaID_,
//			ファイルに対応する移動前後のエリアID
//		const ModVector<ID::Value>& vecPrevAreaID_
//		const ModVector<ID::Value>& vecPostAreaID_
//			移動前後の表に対するエリアID割り当て
//		ModVector<ID::Value>* pvecNonEmptyArea_ = 0
//			使用中のオブジェクトがあることを示す配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
moveAreaIndex(Trans::Transaction& cTrans_, AreaCategory::Value eCat_,
			  ID::Value iPrevAreaID_, ID::Value iPostAreaID_,
			  const ModVector<ID::Value>& vecPrevAreaID_,
			  const ModVector<ID::Value>& vecPostAreaID_,
			  ModVector<ID::Value>* pvecNonEmptyArea_,
			  bool bUndo_, bool bRecovery_, bool bMount_)
{
	ModVector<Index*> index;
	ModVector<ModSize> vecMoved;

	try {
		index = getIndex(eCat_, cTrans_);
		ModSize n = index.getSize();
		vecMoved.reserve(n);

		for (ModSize i = 0; i < n; i++) {
			ID::Value indexAreaID = index[i]->getAreaID(eCat_, false, cTrans_);
			if (indexAreaID == ID::Invalid) {
				index[i]->moveArea(cTrans_, iPrevAreaID_, iPostAreaID_, bUndo_, bRecovery_, bMount_);
				vecMoved.pushBack(i);
			} else {
				// 使用しているオブジェクトがあることを示す配列に追加する
				if (pvecNonEmptyArea_
					&& pvecNonEmptyArea_->find(indexAreaID) == pvecNonEmptyArea_->end())
					pvecNonEmptyArea_->pushBack(indexAreaID);
			}
		}
		if (!bRecovery_ && !bMount_)
			sweepUnusedArea(cTrans_, iPrevAreaID_);

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		ModSize n = vecMoved.getSize();
		for (ModSize i = 0; i < n; ++i) {
			index[vecMoved[i]]->moveArea(cTrans_, iPostAreaID_, iPrevAreaID_,
										 true /* undo */, bRecovery_, bMount_);
		}
		if (!bRecovery_ && !bMount_)
			sweepUnusedArea(cTrans_, iPostAreaID_);

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

//	FUNCTION public
//	Schema::Table::verify --
//		表の整合性検査を行う
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
Table::
verify(Admin::Verification::Progress& cResult_,
	   Trans::Transaction& cTrans_,
	   Admin::Verification::Treatment::Value eTreatment_)
{
	if (isSystem()) {
		// システム表はこのメソッドでは検査しない
		return;
	}

	// 表に属するすべてのファイルについて以下を検査する
	//  1．論理ファイルのverify(cascade指定時)
	//	2．あるファイルのKeyまたはObjectIDをValueに持つファイル
	//	   という関係でファイルを並べたときに
	//		F1 -> F2 -> ... -> F1
	//	   となる場合、各タプルについてすべてのファイルでfetchでき、
	//	   最初と最後のF1のタプルが一致していることを確認する
	//
	//		--> これは以下のようにして調べる
	//			2-1．ROWIDを保持するレコードファイルをScanアクセスして
	//				 読み込んだデータをフィールドごとにSource-Destinatio関係の
	//				 あるフィールドと関係づけて記録する
	//			2-2．残りのファイルについてファイルのキーがすべて関係づけられていたら
	//				 それによりFetchしてデータが得られればその値を比較する
	// ...

	// 呼び出し側で検査の経過が良好であることを保証する必要がある
	; _SYDNEY_ASSERT(cResult_.isGood());

	// 対象の名称を設定する
	Name cName(getDatabase(cTrans_)->getName());
	cName.append(Common::UnicodeChar::usPeriod);
	cName.append(getName());

	cResult_.setSchemaObjectName(cName);

	SydSchemaVerifyMessage
		<< "Verify " << cName
		<< ModEndl;

	// 途中経過を出す
	_SYDNEY_VERIFY_INFO(cResult_, "", Message::VerifyStarted(getName()), eTreatment_);

	try {

		// RowIdの最大値
		TupleID::Value iMaxRowID = TupleID::Invalid;
		// Identityの最大値(incrementが負なら最小値)
		Sequence::Signed::Value iMaxIdentity = Sequence::Signed::Invalid;
		Sequence::Signed::Value* pMaxIdentity = 0;
		Sequence::Signed::Value* pMinIdentity = 0;

		Column* pIdentity = getIdentity(cTrans_);
		if (pIdentity) {
			Sequence& cIdentitySequence = pIdentity->getSequence(cTrans_);
			if (cIdentitySequence.isGetMax()
				|| pIdentity->getDefault().isUseAlways()) {
				if (cIdentitySequence.isAscending()) {
					pMaxIdentity = &iMaxIdentity;
				} else {
					pMinIdentity = &iMaxIdentity;
				}
			}
		}

		// この表に属するファイルすべてについて調べる
		// Cascadeなら論理ファイルについて調べる
		FileVerify cVerify(*this, getFile(cTrans_));
		cVerify.verify(cResult_, cTrans_, eTreatment_, &iMaxRowID, pMaxIdentity, pMinIdentity);
		if (!cResult_.isGood())
			return;
		SCHEMA_FAKE_ERROR("Schema::Table", "Verify", "File");

		if ( !isTemporary() )
		{
			// シーケンスファイルについて調べる
			verifySequence(cResult_, cTrans_, eTreatment_, iMaxRowID, iMaxIdentity);
			if (!cResult_.isGood())
				return;
		}

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
//	Schema::Table::makeLogData --
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
Table::
makeLogData(Trans::Transaction& cTrans_, LogData& cLogData_) const
{
	// 全てに共通のデータ
	//	1. 名前
	//	2. ID
	cLogData_.addString(getName());
	cLogData_.addID(getID());

	//	3. データベースID(UNDO用)
	Database* pDatabase = getDatabase(cTrans_);
	cLogData_.addID(pDatabase->getID());

	//	4．データベースパス配列(UNDO用)
	ModVector<ModUnicodeString> vecDatabasePath;
	pDatabase->getPath(vecDatabasePath);
	cLogData_.addStrings(vecDatabasePath);
	
	switch (cLogData_.getSubCategory()) {
	case LogData::Category::CreateTable:
	{
		//	 表作成
		//		5．種別
		//		6．ヒント

		cLogData_.addData(packMetaField(Meta::Table::Type));
		cLogData_.addData(packMetaField(Meta::Table::Hint));

		// エリアID配列(REDO用)
		cLogData_.addIDs(m_veciAreaID);
		// エリアパス配列(UNDO用)
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, m_veciAreaID));

		// 列定義、制約定義はTable::createで作成される

		break;
	}
	case LogData::Category::DropTable:
	{
		//	 表の破棄
		//		5．エリアパス配列(UNDO用)
		//		6. 索引のID配列(UNDO用)
		// ★注意★
		// エリアパスの配列は表に割り当てられているものだけではなく
		// 索引に割り当てられているものも含める
		// カテゴリーと対応して記録されている必要はない
		// 索引はDeletedのものも入れるのでMapから調べる
		// ロックの機構によりこのオブジェクトを触るスレッドは1つであるから
		// 排他制御は不要
		ModVector<ID::Value> vecAreaID;
		ModVector<ID::Value> vecIndexIDs;

		const IndexMap&
			cIndices = const_cast<Table*>(this)->loadIndex(cTrans_);

		IndexMap::ConstIterator iterator = cIndices.begin();
		const IndexMap::ConstIterator& end = cIndices.end();

		ModSize n = m_veciAreaID.getSize();
		vecAreaID.reserve(n + cIndices.getSize());
		vecIndexIDs.reserve(cIndices.getSize());

		// 表に割り当てられているエリアIDを入れる
		for (ModSize i = 0; i < n; ++i) {
			if (vecAreaID.find(m_veciAreaID[i]) == vecAreaID.end()) {
				vecAreaID.pushBack(m_veciAreaID[i]);
			}
		}
		// 索引ID配列を作るとともに索引に割り当てられているエリアIDを入れる
		for (; iterator != end; ++iterator) {
			Index* pIndex = IndexMap::getValue(iterator).get();
			if (pIndex) {
				vecIndexIDs.pushBack(pIndex->getID());
				ID::Value iAreaID = pIndex->getAreaID(pIndex->getAreaCategory(),
													  false /* do not look up parent */,
													  cTrans_);
				if (vecAreaID.find(iAreaID) == vecAreaID.end()) {
					vecAreaID.pushBack(iAreaID);
				}
			}
		}
		// 索引ID配列
		cLogData_.addIDs(vecIndexIDs);
		// エリアIDをパス配列にする
		cLogData_.addData(Area::getPathArray(cTrans_, *pDatabase, vecAreaID));

		; _SYDNEY_ASSERT(cLogData_.getCount() == Log::Drop::Num);

		break;
	}
	case LogData::Category::AlterTable:
	case LogData::Category::RenameTable:
	case LogData::Category::AddColumn:
	case LogData::Category::AlterColumn:
	case LogData::Category::AddConstraint:
	case LogData::Category::DropConstraint:
	{
		//	表のエリア割り当てや名前などの変更
		//	ここで設定すべきものはない
		//
		//		移動前後のエリア指定および移動するファイル名称は
		//		Reorganize.cppの中で設定される

		break;
	}
	default:
		; _SYDNEY_ASSERT(false);
		break;
	}
}

//	FUNCTION public
//	Schema::Table::getMovedFiles -- ログデータに記録するための移動するファイル名称一覧を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const ModVector<ID::Value>& vecPrevAreaID_
//		const ModVector<ID::Value>& vecPostAreaID_
//			移動前後のエリア指定配列
//
//	RETURN
//		移動するファイル名称一覧を表すCommon::Data::Pointer
//
//	EXCEPTIONS

Common::Data::Pointer
Table::
getMovedFiles(Trans::Transaction& cTrans_,
			  const ModVector<ID::Value>& vecPrevAreaID_,
			  const ModVector<ID::Value>& vecPostAreaID_) const
{
	// ★注意★
	// 配列要素はエリアカテゴリーの順に並んでいることが前提

	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData;

	ModSize n = vecPrevAreaID_.getSize();
	; _SYDNEY_ASSERT(vecPostAreaID_.getSize() == n);
	; _SYDNEY_ASSERT(n <= ModSize(AreaCategory::ValueNum));

	pResult->reserve(n*3);
	for (ModSize i = 0; i < n; ++i) {
		ID::Value iPrevAreaID = vecPrevAreaID_[i];
		ID::Value iPostAreaID = vecPostAreaID_[i];
		if (iPrevAreaID != iPostAreaID) {
			// 移動するはずなのでファイル名一覧を作る
			ModVector<ID::Value> vecParentID;
			ModVector<ID::Value> vecFileID;
			ModVector<ModUnicodeString> vecFileName;
			getMovedFiles(cTrans_, static_cast<AreaCategory::Value>(i), vecParentID, vecFileID, vecFileName);

			// 返り値に追加する
			pResult->pushBack(LogData::createIDs(vecParentID));
			pResult->pushBack(LogData::createIDs(vecFileID));
			pResult->pushBack(LogData::createStrings(vecFileName));

		} else {
			// 移動していない場合は位置を合わせるためにNullDataを同じ数だけ入れておく
			pResult->pushBack(LogData::createNull());
			pResult->pushBack(LogData::createNull());
			pResult->pushBack(LogData::createNull());
		}
	}
	return pResult.release();
}

//	FUNCTION public
//	Schema::Table::getMovedFiles -- ログデータに記録するための移動するファイル名称一覧を得る(Rename用)
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Schema::Object::Name& cPostName_
//			変更後の名前
//
//	RETURN
//		移動するファイル名称一覧を表すCommon::Data::Pointer
//
//	EXCEPTIONS

Common::Data::Pointer
Table::
getMovedFiles(Trans::Transaction& cTrans_,
			  const Name& cPostName_) const
{
	// ★注意★
	// 配列要素はエリアカテゴリーの順に並んでいることが前提

	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData;

	const ModVector<ID::Value>& vecAreaID = getAreaID();
	ModSize n = vecAreaID.getSize();
	; _SYDNEY_ASSERT(n <= ModSize(AreaCategory::ValueNum));

	pResult->reserve(n*4);
	for (ModSize i = 0; i < n; ++i) {
		// すべて移動するはずなのでファイル名一覧を作る
		ModVector<ID::Value> vecParentID;
		ModVector<ID::Value> vecFileID;
		ModVector<ModUnicodeString> vecPrevFileName;
		ModVector<ModUnicodeString> vecPostFileName;
		getMovedFiles(cTrans_, static_cast<AreaCategory::Value>(i), cPostName_, vecParentID, vecFileID, vecPrevFileName, vecPostFileName);

		// 返り値に追加する
		pResult->pushBack(LogData::createIDs(vecParentID));
		pResult->pushBack(LogData::createIDs(vecFileID));
		pResult->pushBack(LogData::createStrings(vecPrevFileName));
		pResult->pushBack(LogData::createStrings(vecPostFileName));
	}
	return pResult.release();
}

//	FUNCTION private
//	Schema::Table::getMovedFiles -- ログデータに記録するための移動するファイル名称一覧を得る
//
//	NOTES
//		ここで加えられるファイル名はエリアなどのパスに加えられるパス名である
//		たとえばTという表のBTR_Xというファイルが移動するのであれば
//		"T/BTR_X"という文字列が記録される
//		-> 名称変更に伴う移動でも同じメソッドを使うため、表名による部分は別途格納するように変更した
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::AreaCategory::Value eCategory_
//			調査対象のエリアカテゴリー
//		ModVector<Schema::Object::ID::Value>& vecID_
//		ModVector<ModUnicodeString>& vecFileName_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
getMovedFiles(Trans::Transaction& cTrans_,
			  AreaCategory::Value eCategory_,
			  ModVector<ID::Value>& vecParentID_,
			  ModVector<ID::Value>& vecFileID_,
			  ModVector<ModUnicodeString>& vecFileName_) const
{
	// 移動するファイルの一覧を作る
	// ★注意★
	// moveAreaの実装を変更したらここも変える

    switch (eCategory_) {
    case AreaCategory::LogicalLog:
    case AreaCategory::PhysicalLog:
    case AreaCategory::FileMin:
    {
        // 非使用
		break;
    }
	case AreaCategory::Default:
	{
		if (getTupleID(cTrans_)) {
			// タプルIDを保持するファイルのファイル名を加える
			// IDには表のIDを使う
			vecParentID_.pushBack(getID());
			vecFileID_.pushBack(ID::Invalid);
			vecFileName_.pushBack(PathParts::Sequence::TupleID);
		}
		if (getIdentity(cTrans_)) {
			// Identity Columnを保持するファイルのファイル名を加える
			// IDには表のIDを使う
			vecParentID_.pushBack(getID());
			vecFileID_.pushBack(ID::Invalid);
			vecFileName_.pushBack(PathParts::Sequence::Identity);
		}
		// thru.
	}
    case AreaCategory::Heap:
    {
        // エリア種別が同じファイルについてファイル名を加える
		// IDには表のIDを使う
		// ★注意★
		// この関数はリカバリーで呼ばれることはないので
		// getFileに渡す表の割り当て配列はメンバーのものを使って構わない
        ModVector<File*> file = getFile(eCategory_, m_veciAreaID, m_veciAreaID, cTrans_);
        ModSize n = file.getSize();

		for (ModSize i = 0; i < n; i++) {
			if (file[i]->getIndexID() != ID::Invalid) {
				// 索引のエリア指定を調べる
				ID::Value indexAreaID = file[i]->getIndex(cTrans_)->getAreaID(file[i]->getAreaCategory(), false, cTrans_);
				if (indexAreaID != ID::Invalid ) {
					// 索引にエリア指定があるものは対象にしない
					continue;
				}
			}
			vecParentID_.pushBack(getID());
			vecFileID_.pushBack(file[i]->getID());
			vecFileName_.pushBack(file[i]->getName());
		}
        break;
    }
    case AreaCategory::Index:
    case AreaCategory::FullText:
    {
        // 索引を構成するファイルのファイル名を加える
        // ただし、索引に個別に指定があれば何もしない
		// IDには索引のIDを使う

        ModVector<Index*> index = getIndex(eCategory_, cTrans_);
        ModSize n = index.getSize();
		for (ModSize i = 0; i < n; i++) {
			ID::Value indexAreaID = index[i]->getAreaID(eCategory_, false, cTrans_);
			if (indexAreaID == ID::Invalid) {
				File* pIndexFile = index[i]->getFile(cTrans_);
				vecParentID_.pushBack(index[i]->getID());
				vecFileID_.pushBack(pIndexFile->getID());
				vecFileName_.pushBack(pIndexFile->getName());
			}
		}
        break;
    }
    default:
        ; _SYDNEY_ASSERT(false);
        break;
    }
	; _SYDNEY_ASSERT(vecParentID_.getSize() == vecFileName_.getSize());
	; _SYDNEY_ASSERT(vecFileID_.getSize() == vecFileName_.getSize());
}

//	FUNCTION private
//	Schema::Table::getMovedFiles -- ログデータに記録するための移動するファイル名称一覧を得る(rename用)
//
//	NOTES
//		ここで加えられるファイル名はエリアなどのパスに加えられるパス名である
//		たとえばTという表のBTR_Xというファイルが移動するのであれば
//		"T/BTR_X"という文字列が記録される
//		-> 名称変更に伴う移動でも同じメソッドを使うため、表名による部分は別途格納するように変更した
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::AreaCategory::Value eCategory_
//			調査対象のエリアカテゴリー
//		ModVector<Schema::Object::ID::Value>& vecID_
//		ModVector<ModUnicodeString>& vecFileName_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Table::
getMovedFiles(Trans::Transaction& cTrans_,
			  AreaCategory::Value eCategory_,
			  const Name& cPostName_,
			  ModVector<ID::Value>& vecParentID_,
			  ModVector<ID::Value>& vecFileID_,
			  ModVector<ModUnicodeString>& vecPrevFileName_,
			  ModVector<ModUnicodeString>& vecPostFileName_) const
{
	// 移動するファイルの一覧を作る
	// ★注意★
	// moveAreaの実装を変更したらここも変える

    switch (eCategory_) {
    case AreaCategory::LogicalLog:
    case AreaCategory::PhysicalLog:
    case AreaCategory::FileMin:
    {
        // 非使用
		break;
    }
	case AreaCategory::Default:
	{
		if (getTupleID(cTrans_)) {
			// タプルIDを保持するファイルのファイル名を加える
			// IDには表のIDを使う
			vecParentID_.pushBack(getID());
			vecFileID_.pushBack(ID::Invalid);
			vecPrevFileName_.pushBack(PathParts::Sequence::TupleID);
			vecPostFileName_.pushBack(PathParts::Sequence::TupleID);
		}
		if (getIdentity(cTrans_)) {
			// Identity columnを保持するファイルのファイル名を加える
			// IDには表のIDを使う
			vecParentID_.pushBack(getID());
			vecFileID_.pushBack(ID::Invalid);
			vecPrevFileName_.pushBack(PathParts::Sequence::Identity);
			vecPostFileName_.pushBack(PathParts::Sequence::Identity);
		}
		// thru.
	}
    case AreaCategory::Heap:
    {
        // エリア種別が同じファイルについてファイル名を加える
		// IDには表のIDを使う
		// ★注意★
		// この関数はリカバリーで呼ばれることはないので
		// getFileに渡す表の割り当て配列はメンバーのものを使って構わない
        ModVector<File*> file = getFile(eCategory_, m_veciAreaID, m_veciAreaID, cTrans_);
        ModSize n = file.getSize();

		for (ModSize i = 0; i < n; i++) {
			if (file[i]->getAreaCategory() != eCategory_)
				continue;
			vecParentID_.pushBack(getID());
			vecFileID_.pushBack(file[i]->getID());
			vecPrevFileName_.pushBack(file[i]->getName());
			vecPostFileName_.pushBack(file[i]->createName(cTrans_, cPostName_));
		}
        break;
    }
    case AreaCategory::Index:
	{
		// 主キー制約があったら制約に対応するIndexオブジェクトを得ておく
		Constraint* primaryConstraint = getPrimaryKeyConstraint(cTrans_);

        // 索引を構成するファイルのファイル名を加える
		// IDには索引のIDを使う
        ModVector<Index*> index = getIndex(eCategory_, cTrans_);
        ModSize n = index.getSize();
		for (ModSize i = 0; i < n; i++) {
			ID::Value indexAreaID = index[i]->getAreaID(eCategory_, false, cTrans_);
			File* pIndexFile = index[i]->getFile(cTrans_);
			vecParentID_.pushBack(index[i]->getID());
			vecFileID_.pushBack(pIndexFile->getID());
			vecPrevFileName_.pushBack(pIndexFile->getName());
			if (primaryConstraint && index[i]->getName() == primaryConstraint->getName()) {
				vecPostFileName_.pushBack(pIndexFile->createName(cTrans_, primaryConstraint->createName(cPostName_)));

				// 主キー索引自体の名前も記録する
				vecParentID_.pushBack(index[i]->getID());
				vecFileID_.pushBack(index[i]->getID());
				vecPrevFileName_.pushBack(index[i]->getName());
				vecPostFileName_.pushBack(primaryConstraint->createName(cPostName_));
			}
			else
				vecPostFileName_.pushBack(pIndexFile->getName());
		}
        break;
	}
    case AreaCategory::FullText:
    {
        // 索引を構成するファイルのファイル名を加える
		// IDには索引のIDを使う

        ModVector<Index*> index = getIndex(eCategory_, cTrans_);
        ModSize n = index.getSize();
		for (ModSize i = 0; i < n; i++) {
			ID::Value indexAreaID = index[i]->getAreaID(eCategory_, false, cTrans_);
			File* pIndexFile = index[i]->getFile(cTrans_);
			vecParentID_.pushBack(index[i]->getID());
			vecFileID_.pushBack(pIndexFile->getID());
			vecPrevFileName_.pushBack(pIndexFile->getName());
			vecPostFileName_.pushBack(pIndexFile->getName());
		}
        break;
    }
    default:
        ; _SYDNEY_ASSERT(false);
        break;
    }
	; _SYDNEY_ASSERT(vecParentID_.getSize() == vecPrevFileName_.getSize());
	; _SYDNEY_ASSERT(vecParentID_.getSize() == vecPostFileName_.getSize());
	; _SYDNEY_ASSERT(vecFileID_.getSize() == vecParentID_.getSize());
}

//	FUNCTION private
//	Schema::Table::getMovedFiles -- ログデータから移動したファイル名称一覧を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Common::DataArrayData& cData_
//			ログデータ中の移動ファイルに関して記録されているデータ
//		int iCategory_
//			取得対象のエリアカテゴリー
//		ModVector<Schema::Object::ID::Value>& vecID_
//		ModVector<ModUnicodeString>& vecFileName_
//			返り値を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
void
Table::
getMovedFiles(const Common::DataArrayData& cData_,
			  int iCategory_,
			  ModVector<ID::Value>& vecParentID_,
			  ModVector<ID::Value>& vecFileID_,
			  ModVector<ModUnicodeString>& vecPrevFileName_,
			  ModVector<ModUnicodeString>& vecPostFileName_,
			  bool bRename_ /* = false */)
{
	// MovedFileにはひとつのカテゴリーにつき3つまたは4つずつの要素を格納している
	const int iNum = bRename_ ? 4 : 3;
	if (cData_.getCount() < (iCategory_ + 1) * iNum) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
	for (int i = 0; i < iNum; ++i) {
		const Common::Data::Pointer& pElement = cData_.getElement(iCategory_ * iNum + i);
		switch (i) {
		case 0:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::UnsignedInteger) {
					vecParentID_ = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData&, *pElement).getValue();
				}
				break;
			}
		case 1:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::UnsignedInteger) {
					vecFileID_ = _SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerArrayData&, *pElement).getValue();
				}
				break;
			}
		case 2:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::String) {
					vecPrevFileName_ = _SYDNEY_DYNAMIC_CAST(const Common::StringArrayData&, *pElement).getValue();
					if (iNum == 3)
						vecPostFileName_ = vecPrevFileName_;
				}
				break;
			}
		case 3:
			{
				if (pElement.get()
					&& pElement->getType() == Common::DataType::Array
					&& pElement->getElementType() == Common::DataType::String) {
					vecPostFileName_ = _SYDNEY_DYNAMIC_CAST(const Common::StringArrayData&, *pElement).getValue();
				}
				break;
			}
		}
	}
}

Common::Data::Pointer
Table::
getCreatedFiles(Trans::Transaction& cTrans_,
				const ModVector<File::Pointer>& vecPostFiles_) const
{
	ModAutoPointer<Common::DataArrayData> pResult = new Common::DataArrayData;

	// areaCategory, FileID, FileNameを順番にセットする
	ModSize n = vecPostFiles_.getSize();
	for (ModSize i = 0; i < n; ++i) {
		const File::Pointer& pFile = vecPostFiles_[i];
		pResult->pushBack(LogData::createInteger(pFile->getAreaCategory()));
		pResult->pushBack(LogData::createID(pFile->getID()));
		pResult->pushBack(LogData::createString(pFile->getName()));
	}
	return pResult.release();
}

//	FUNCTION public
//	Schema::Table::getName -- ログデータから表名を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表破棄または変更のログデータ
//
//	RETURN
//		表名
//
//	EXCEPTIONS

// static
Object::Name
Table::
getName(const LogData& cLogData_)
{
	return cLogData_.getString(Log::Name);
}

//	FUNCTION public
//	Schema::Table::getObjectID -- ログデータから Schema ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表破棄または変更のログデータ
//
//	RETURN
//		Schema ID
//
//	EXCEPTIONS
ObjectID::Value
Table::
getObjectID(const LogData& cLogData_)
{
	return cLogData_.getID(Log::ID);
}

//	FUNCTION public
//	Schema::Table::getDatabaseID -- ログデータからデータベースIDを得る
//
//	NOTES
//		UNDO処理でデータベースに関するUNDO情報を取得するために使用する
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表のログデータ
//
//	RETURN
//		Schema ID
//
//	EXCEPTIONS
ObjectID::Value
Table::
getDatabaseID(const LogData& cLogData_)
{
	return cLogData_.getID(Log::DatabaseID);
}

//	FUNCTION public
//	Schema::Table::getDatabasePath -- ログデータからデータベースパスを得る
//
//	NOTES
//		UNDO処理でデータベースに関するUNDO情報を取得するために使用する
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表のログデータ
//		ModVector<ModUnicodeString>& vecPath_
//			ログに書き込んだ時点のデータベースのパス指定を格納する配列
//
//	RETURN
//		なし
//
//	EXCEPTIONS
void
Table::
getDatabasePath(const LogData& cLogData_,
				ModVector<ModUnicodeString>& vecPath_)
{
	vecPath_ = cLogData_.getStrings(Log::DatabasePaths);
}

//	FUNCTION public
//	Schema::Table::getAreaID -- ログデータからエリアIDリストを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表のログデータ
//		int iIndex_
//			エリアIDが格納されているログの位置
//		ModVector<Object::ID::Value>& vecAreaID_
//			エリアIDを格納するVector
//
//	RETURN
//		なし
//
//	EXCEPTIONS

//static
void
Table::
getAreaID(const LogData& cLogData_, int iIndex_, ModVector<ID::Value>& vecAreaID_)
{
	vecAreaID_ = cLogData_.getIDs(iIndex_);
}

//	FUNCTION public
//	Schema::Table::getAreaPath -- ログデータからエリアパスリストを得る
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
Table::
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
//	Schema::Table::getEffectiveAreaPath -- ログデータからエリアパスリストを得る
//
//	NOTES
//		エリアが設定されていないカテゴリーでもそのSuperValueまでたどって調べる
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表のログデータ
//		int iIndex_
//			エリアパスが格納されているログの位置
//		int iCategory_
//			取得するエリアのカテゴリー(ログを作ったときに渡した配列上の位置)
//		const ModVector<Schema::Object::ID::Value>& vecAreaID_
//			エリアIDを格納するVector
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
Table::
getEffectiveAreaPath(const LogData& cLogData_, int iIndex_, int iCategory_,
					 const ModVector<ID::Value>& vecAreaID_,
					 ModVector<ModUnicodeString>& vecAreaPath_)
{
	// ファイルに適用されるカテゴリーを使う
	return getAreaPath(cLogData_, iIndex_,
					   _Area::_getEffectiveCategory(vecAreaID_, static_cast<AreaCategory::Value>(iCategory_)),
					   vecAreaPath_);
}

//	FUNCTION public
//	Schema::Table::getIndexID -- ログデータから索引IDリストを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::LogData& cLogData_
//			表破棄のログデータ
//
//	RETURN
//		IDリスト
//
//	EXCEPTIONS
const ModVector<Object::ID::Value>&
Table::
getIndexID(const LogData& cLogData_)
{
	return cLogData_.getIDs(Log::Drop::IndexIDs);
}

//	FUNCTION public
//	Schema::Table::getSystemTableName --
//		システム表の名前を得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value eCategory_
//			対象のオブジェクト種
//
//	RETURN
//		対応する名前
//
//	EXCEPTIONS

//static
const Object::Name&
Table::
getSystemTableName(Object::Category::Value eCategory_)
{
	return _Name::_getSystemTableName(eCategory_);
}

//	FUNCTION public
//	Schema::Table::getSystemTableID --
//		システム表のIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Category::Value eCategory_
//			対象のオブジェクト種
//
//	RETURN
//		対応するID
//
//	EXCEPTIONS

//static
Object::ID::Value
Table::
getSystemTableID(Object::Category::Value eCategory_)
{
	return _ID::_getSystemTableID(eCategory_);
}

//	FUNCTION public
//	Schema::Table::getSystemTableCategory --
//		名前がどのシステム表に対応するかを得るのIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& cName_
//			調べる名前
//
//	RETURN
//		Schema::Object::Category::Unknown
//			システム表にある名前ではない
//		Schema::Object::Category::Unknown以外
//			システム表にある名前である
//
//	EXCEPTIONS

//static
Object::Category::Value
Table::
getSystemTableCategory(const Name& cName_)
{
	return _Name::_getSystemTableCategory(cName_);
}

//	FUNCTION public
//	Schema::Table::getSystemTableCategory --
//		名前がどのシステム表に対応するかを得るのIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& cName_
//			調べる名前
//
//	RETURN
//		Schema::Object::Category::Unknown
//			システム表にある名前ではない
//		Schema::Object::Category::Unknown以外
//			システム表にある名前である
//
//	EXCEPTIONS

//static
Object::Category::Value
Table::
getSystemTableCategory(ID::Value iID_)
{
	return _ID::_getSystemTableCategory(iID_);
}

//	FUNCTION private
//	Schema::Table::checkUndo --
//		Undo情報を検査し反映する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Database& cDatabase_
//			表が属するデータベースオブジェクト
//		Schema::Object::ID::Value iID_
//			検査に使用するID
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Table::
checkUndo(const Database& cDatabase_, ID::Value iID_)
{
	using namespace Manager::RecoveryUtility;
	if (Undo::isEntered(cDatabase_.getName(), iID_, Undo::Type::AlterTable)) {
		ModVector<Object::ID::Value> vecUndoAreaID;
		if (Manager::RecoveryUtility::ID::getUndoAreaID(cDatabase_.getName(), iID_, vecUndoAreaID)) {
			// Alter後のエリアID割り当てが登録されているときはログデータのパスではなく
			// Alter後のエリアID割り当てを使用する
			setAreaID(vecUndoAreaID);
		}
	}
	if (Undo::isEntered(cDatabase_.getName(), iID_, Undo::Type::RenameTable)) {
		Name cUndoName;
		if (Manager::RecoveryUtility::Name::getUndoName(cDatabase_.getName(), iID_, cUndoName)) {
			// Alter後の名前が登録されているときはログデータの名前ではなく
			// Alter後の名前を使用する
			setName(cUndoName);
		}
	}
}

// ファイルオブジェクト操作用のRWLockを得る
Os::RWLock&
Table::
getRWLockForFile() const
{
	return const_cast<Os::RWLock&>(m_cRWLockForFile);
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

// メタデータベースにおける「表」表の構造は以下のとおり
// create table Table_DBXXXX (
//		ID			id,
//		name		nvarchar,
//		type		int,
//		area		<id array>, -- エリアIDの配列
//		hint		nvarchar
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Table>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Table>(Meta::MemberType::_type_, &Table::_get_, &Table::_set_)

	Meta::Definition<Table> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(Name),			// Name
		_DEFINE0(Integer),		// Type
		_DEFINE2(IDArray, getAreaID, setAreaID), // AreaIDs,
		_DEFINE0(Binary),		// Hint,
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Table::getMetaFieldNumber --
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
Table::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Table::MemberNum);
}

//	FUNCTION public
//	Schema::Table::getMetaFieldDefinition --
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
Table::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Table::packMetaField --
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
Table::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Table>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Table::Type);

			// 当面、Scopeだけを反映させる
			// 他にViewなどの種別を表すものが加わったらここに加える
			return pack(static_cast<int>(getScope()));
		}
	case Meta::MemberType::Binary:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Table::Hint);
			return getArchiver().put(getHint());
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
//	Schema::Table::unpackMetaField --
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
Table::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Table>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Objectの情報を使う
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Table::Type);

			// 当面、Scopeだけを反映させる
			// 他にViewなどの種別を表すものが加わったらここに加える
			int value;
			if (unpack(pData_, value)) {
				if (value >= 0 && value < Scope::ValueNum) {
					setScope(static_cast<Scope::Value>(value));
					return true;
				}
			}
			break;
		}
	case Meta::MemberType::Binary:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Table::Hint);

			if (pData_ && pData_->isNull()) {
				return true;

			} else if (pData_ && pData_->getType() == Common::DataType::Binary) {
				const Common::BinaryData* pBinary =
					_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);

				ModAutoPointer<Hint> pData =
					dynamic_cast<Hint*>(getArchiver().get(pBinary));
				if (pData.get())
					m_pHint = pData.release();
				return true;
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
		break;
	}
	return false;
}

//
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023, 2024 Ricoh Company, Ltd.
// All rights reserved.
//
