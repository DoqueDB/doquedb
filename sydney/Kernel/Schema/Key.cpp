// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Key.cpp -- キー関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2009, 2015, 2023 Ricoh Company, Ltd.
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

#include "Schema/Key.h"
#include "Schema/AutoRWLock.h"
#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/NameParts.h"
#include "Schema/Object.h"
#include "Schema/ObjectTemplate.h"
#include "Schema/Parameter.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/Table.h"

#include "Statement/ColumnName.h"
#include "Statement/Identifier.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"

#include "Exception/ColumnNotFound.h"
#include "Exception/InvalidIndexKey.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "FileCommon/FileOption.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {
namespace _Check
{
	//	FUNCTION local
	//	_Check::_isAllowed -- 
	//		指定された列が索引にキーとしてつけられるかを得る
	//
	//	NOTES
	//
	//	ARGUMENTS
	//		Trans::Transaction& cTrans_
	//			操作を行うトランザクション記述子
	//		const Schema::Index& cIndex_
	//			作成しようとしている索引
	//		Schema::Key::Position iPosition_
	//			作成しようとしているキーの位置
	//		const Schema::Table& cTable_
	//			作成しようとしている索引をつける表
	//		const Schema::Column& cColumn_
	//			索引のキーに使用としている列
	//
	//	RETURN
	//		なし
	//
	//	EXCEPTIONS

	bool
	_isAllowed(Trans::Transaction& cTrans_,
			   const Index& cIndex_, Key::Position iPosition_,
			   const Table& cTable_, const Column& cColumn_)
	{
		if (cColumn_.getCategory() == Column::Category::Constant
			|| cColumn_.isTupleID()) {
			// 定数列やROWIDに索引はつけられない
			return false;
		}
		bool bResult = true;
		switch (cIndex_.getCategory()) {
		case Index::Category::Normal:
		{
			// B+木には配列の列、バイナリをキーにはできない
			Field* pField = cColumn_.getField(cTrans_);
			bResult = (pField->getType() != Common::DataType::Array
					   && pField->getType() != Common::DataType::Binary);
			break;
		}
		case Index::Category::FullText:
		{
			// 全文はFullTextIndexの中で調べられている
			break;
		}
		case Index::Category::Array:
		{
			// Arrayにはバイナリー以外の配列しかキーにできない
			Field* pField = cColumn_.getField(cTrans_);
			bResult = (pField->getType() == Common::DataType::Array
					   && pField->getElementType() != Common::DataType::Binary);
			break;
		}
		default:
			break;
		}
		return bResult;
	}

} // namespace _Check
} // namespace

//	FUNCTION public
//	Schema::Key::Key -- キーを表すクラスのデフォルトコンストラクター
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

Key::
Key()
	: Object(Object::Category::Key),
	  _position(0),
	  _order(Order::Unknown),
	  _attribute(Attribute::None),
	  _column(0),
	  _columnID(Object::ID::Invalid),
	  _field(0),
	  _fieldID(Object::ID::Invalid),
	  m_pIndex(0)
{ }

//	FUNCTION public
//	Schema::Key::Key -- キー定義からのキーを表すクラスのコンストラクター
//
//	NOTES
//		キーを表すクラスを生成するだけで、「キー」表は更新されない
//
//	ARGUMENTS
//		Schema::Index&		index
//			キーが存在する索引を表すクラス
//		Schema::Index::Position	position
//			索引の先頭からのキーの位置
//		Schema::Table&		table
//			キーにした列が存在する表を表すクラス
//		Statement::ColumnName&	statement
//			解析済の SQL のキーにした列の名前指定
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Key::
Key(const Index& index, Key::Position position, const Table& table,
	const Statement::ColumnName& statement)
	: Object(Object::Category::Key, index.getScope(), index.getStatus(),
			 ID::Invalid, index.getID(), index.getDatabaseID()),
	  _position(position),
	  _order(Order::Unknown),
	  _attribute(Attribute::None),
	  _column(0),
	  _columnID(Object::ID::Invalid),
	  _field(0),
	  _fieldID(Object::ID::Invalid),
	  m_pIndex(const_cast<Index*>(&index))
{
	// キーの名前指定を処理する
	Statement::Identifier* identifier = statement.getIdentifier();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());
	Object::setName(*identifier->getIdentifier());
}

//	FUNCTION public
//	Schema::Key::Key -- キー定義からのキーを表すクラスのコンストラクター
//
//	NOTES
//		キーを表すクラスを生成するだけで、「キー」表は更新されない
//
//	ARGUMENTS
//		Schema::Index&		index
//			キーが存在する索引を表すクラス
//		Schema::Index::Position	position
//			索引の先頭からのキーの位置
//		Schema::Table&		table
//			キーにした列が存在する表を表すクラス
//		Schema::Column&		column
//			キーにした列
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Key::
Key(const Index& index, Key::Position position, const Table& table,
	Column& column)
	: Object(Object::Category::Key, index.getScope(), index.getStatus(),
			 ID::Invalid, index.getID(), index.getDatabaseID()),
	  _position(position),
	  _order(Order::Unknown),
	  _attribute(Attribute::None),
	  _column(&column),
	  _columnID(column.getID()),
	  _field(0),
	  _fieldID(Object::ID::Invalid),
	  m_pIndex(const_cast<Index*>(&index))
{
	Object::setName(column.getName());
}

//	FUNCTION public
//	Schema::Key::~Key -- キーを表すクラスのデストラクター
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

Key::
~Key()
{ }

//	FUNCTION public
//		Schema::Key::getNewInstance -- オブジェクトを新たに取得する
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
Key*
Key::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Key> pObject = new Key;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION public
//	Schema::Key::create -- キー定義からキーのスキーマ情報を表すクラスを生成する
//
//	NOTES
//		キーを表すクラスを生成するだけで、「キー」表は更新されない
//
//	ARGUMENTS
//		Schema::Index&		index
//			キーが存在する索引を表すクラス
//		Schema::Index::Position	position
//			索引の先頭からのキーの位置
//		Schema::Table&		table
//			キーにした列が存在する表を表すクラス
//		Statement::ColumnName&	statement
//			解析済の SQL のキーにした列の名前指定
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたキーのスキーマ情報を表すクラス
//
//	EXCEPTIONS

// static
Key::Pointer
Key::
create(Index& index, Key::Position position, const Table& table,
	   const Statement::ColumnName& statement,
	   Trans::Transaction& cTrans_,
	   ID::Value iID_ /* = ID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	Pointer pKey = new Key(index, position, table, statement);
	; _SYDNEY_ASSERT(pKey.get());

	// キーにした列を求めてセットする
	Column* column = table.getColumn(pKey->getName(), cTrans_);

	if (!column) {
		// キーに指定された列は存在しない
		_SYDNEY_THROW1(Exception::ColumnNotFound, pKey->getName());
	}

	if (!_Check::_isAllowed(cTrans_, index, position, table, *column)) {
		// 索引にできない列をキーにしようとしている
		_SYDNEY_THROW2(Exception::InvalidIndexKey, index.getName(), pKey->getName());
	}

	(void) pKey->setColumn(*column);

	// キーの属性を設定する
	pKey->setAttribute(index, column);

	// IDをふり、状態を変える
	pKey->Object::create(cTrans_, iID_);

	return pKey;
}

//	FUNCTION public
//	Schema::Key::create -- 制約定義からキーのスキーマ情報を表すクラスを生成する
//
//	NOTES
//		キーを表すクラスを生成するだけで、「キー」表は更新されない
//
//	ARGUMENTS
//		Schema::Index&		index
//			キーが存在する索引を表すクラス
//		Schema::Index::Position	position
//			索引の先頭からのキーの位置
//		Schema::Table&		table
//			キーにした列が存在する表を表すクラス
//		Schema::Object::ID::Value columnID
//			キーにした列のオブジェクトID
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたキーのスキーマ情報を表すクラス
//
//	EXCEPTIONS

// static
Key::Pointer
Key::
create(Index& index, Key::Position position, const Table& table,
	   ID::Value columnID, Trans::Transaction& cTrans_,
	   ID::Value iID_ /* = ID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// IDから列オブジェクトを得る
	Column* column = table.getColumnByID(columnID, cTrans_);
	; _SYDNEY_ASSERT(column);

	Pointer pKey = new Key(index, position, table, *column);
	; _SYDNEY_ASSERT(pKey.get());

	// キーの属性を設定する
	pKey->setAttribute(index, column);

	// IDをふり、状態を変える
	pKey->Object::create(cTrans_, iID_);

	return pKey;
}

//	FUNCTION public
//	Schema::Key::create -- ログデータからキーのスキーマ情報を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Index&		index
//			キーが存在する索引を表すクラス
//		Schema::Index::Position	position
//			索引の先頭からのキーの位置
//		Schema::Table&		table
//			キーにした列が存在する表を表すクラス
//		const Common::DataArrayData& cLogData_
//			キーのログデータ
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたキーのスキーマ情報を表すクラス
//
//	EXCEPTIONS

// static
Key::Pointer
Key::
create(Index& index, Key::Position position, const Table& table,
	   const Common::DataArrayData& cLogData_, Trans::Transaction& cTrans_,
	   ID::Value iID_ /* = ID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	if (cLogData_.getCount() >= Log::Num0) {

		// ログデータから名称を得る
		// ★注意★
		// makeLogDataの実装が変わったらここも変える
		int i = 0;

		// IDから列オブジェクトを得る
		Column* column = table.getColumnByID(LogData::getID(cLogData_.getElement(i++)), cTrans_);

		if (column) {
			Pointer pKey = new Key(index, position, table, *column);
			; _SYDNEY_ASSERT(pKey.get());

			// キーの属性を設定する
			pKey->setAttribute(index, column);

			// 残りのログデータの内容を反映する
			// ★注意★
			// makeLogDataの実装が変わったらここも変える
			if (pKey->unpackMetaField(cLogData_.getElement(i++).get(), Meta::Key::Order)) {

				if (cLogData_.getCount() >= Log::Num1) {
					iID_ = LogData::getID(cLogData_.getElement(Log::ID));
				}

				// IDをふり、状態を変える
				pKey->Object::create(cTrans_, iID_);

				return pKey;
			}
		}
	}
	// ここに来るのはログデータがおかしいとき
	_SYDNEY_THROW0(Exception::LogItemCorrupted);
}

//	FUNCTION public
//	Schema::Key::get -- あるスキーマオブジェクト ID のキーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			キーのスキーマオブジェクト ID
//		Schema::Database* pDatabase_
//			キーが属するデータベースのオブジェクト
//			値が0ならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//	
//	RETURN
//		0 以外の値
//			得られたキーを格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID のキーは存在しない
//
//	EXCEPTIONS

// static
Key*
Key::get(ID::Value id_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	return ObjectTemplate::get<Key, SystemTable::Key, Object::Category::Key>(id_, pDatabase_, cTrans_);
}

//	FUNCTION public
//	Schema::Key::get -- あるスキーマオブジェクト ID のキーを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			キーのスキーマオブジェクト ID
//		Schema::Object::ID::Value iDatabaseID_
//			キーが属するデータベースのオブジェクトID
//			値がID::Invalidならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//	
//	RETURN
//		0 以外の値
//			得られたキーを格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID のキーは存在しない
//
//	EXCEPTIONS

// static
Key*
Key::get(ID::Value id_, ID::Value iDatabaseID_, Trans::Transaction& cTrans_)
{
	if (id_ == Object::ID::Invalid)
		return 0;

	return get(id_, Database::get(iDatabaseID_, cTrans_), cTrans_);
}

//	FUNCTION public
//	Schema::Key::isValid -- 陳腐化していないか
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
//			操作を行うトランザクション記述子
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
Key::
isValid(ID::Value iID_, ID::Value iDatabaseID_,
		Timestamp iTimestamp_, Trans::Transaction& cTrans_)
{
	Key* pKey = get(iID_, iDatabaseID_, cTrans_);

	return (pKey && pKey->getTimestamp() == iTimestamp_);
}

//	FUNCTION public
//	Schema::Key::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Key::Pointer& pKey_
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
Key::
doBeforePersist(const Pointer& pKey_, Status::Value eStatus_, bool bNeedToErase_, Trans::Transaction& cTrans_)
{
	// 何もしない
	;
}

//	FUNCTION public
//	Schema::Key::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Key::Pointer& pKey_
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
Key::
doAfterPersist(const Pointer& pKey_, Status::Value eStatus_, bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pKey_.get());

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	const ObjectID::Value dbId = pKey_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// Nullabilityが列とキーで異なるとき、列オブジェクトに反映させる
		Column* pColumn = pKey_->getColumn(cTrans_);
		; _SYDNEY_ASSERT(pColumn);
		if (pColumn->isNullable() != pKey_->isNullable()) {
			pColumn->setNullable(pKey_->isNullable());
			// 変更を伝える
			pColumn->touch();
		}

		// データベースにこのキーを表すクラスを
		// スキーマオブジェクトとして管理させる

		if (Database* pDatabase = pKey_->getDatabase(cTrans_))
			pDatabase->addCache(pKey_);
		break;
	}
	case Status::Changed:
	case Status::CreateCanceled:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらキャッシュや索引の登録からの削除も行う

		// 状態を「実際に削除された」にする

		pKey_->setStatus(Status::ReallyDeleted);

		// (キーがつく列に変更を伝播する)
		// キーがつく列への変更の伝播はフィールド経由で行われるのでここでは不要
		// なので該当の処理は削除した

#ifdef OBSOLETE // 現在はKeyだけを消去する再構成はないので以下のコードが意味を持つことはない
		if (bNeedToErase_) {
			Database* pDatabase = pKey_->getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			// 下位オブジェクトがあればそれを抹消してからdeleteする
			pKey_->reset(*pDatabase);

			// キャッシュから抹消する
			// NeedToErase==falseのときは親オブジェクトのdeleteの中で
			// キャッシュから抹消される
			pDatabase->eraseCache(pKey_->getID());

			// 索引の登録から抹消する → deleteされる
			Index* pIndex = pKey_->getIndex(cTrans_);
			; _SYDNEY_ASSERT(pIndex);
			pIndex->eraseKey(pKey_->getID());
		}
#endif
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbId, Object::Category::Key);
}

//	FUNCTION public
//	Schema::Key::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::KeyPointer& pKey_
//			読み出したオブジェクト
//		Schema::Index& cIndex_
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
Key::
doAfterLoad(const Pointer& pKey_, Index& cIndex_, Trans::Transaction& cTrans_)
{
	// 名前を設定する
	pKey_->setName(cTrans_);

	// 属性を設定する
	pKey_->setAttribute(cIndex_, pKey_->getColumn(cTrans_));

	// データベースへ読み出したキーを表すクラスを追加する
	// また、データベースにこのキーを表すクラスを
	// スキーマオブジェクトとして管理させる
	cIndex_.getDatabase(cTrans_)->addCache(cIndex_.addKey(pKey_, cTrans_));
}

//	FUNCTION public
//	Schema::Key::reset --
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
//		なし

void
Key::
reset(Database& cDatabase_)
{
	;
}

//	FUNCTION public
//	Schema::Key::getPosition -- キーの索引の先頭からの位置を得る
//
//	NOTES
//		索引の先頭からなん番目にそのキーが定義されているかを、キーの位置とする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたキーの位置
//
//	EXCEPTIONS
//		なし

Key::Position
Key::
getPosition() const
{
	return _position;
}

//	FUNCTION public
//	Schema::Key::getOrder -- キーの値の順序を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたキーの値の順序
//
//	EXCEPTIONS
//		なし

Key::Order::Value
Key::
getOrder() const
{
	return _order;
}

//	FUNCTION public
//	Schema::Key::isSearchable -- キーが検索キーになるかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true .. 検索キーになる
//
//	EXCEPTIONS
//		なし

bool
Key::
isSearchable() const
{
	return (_attribute & Attribute::Searchable);
}

// FUNCTION public
//	Schema::Key::isNullable -- キーがNULL可能かを得る
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
Key::
isNullable() const
{
	return !(_attribute & Attribute::NotNull);
}

// FUNCTION public
//	Schema::Key::setNullable -- キーがNULL可能かを設定する
//
// NOTES
//
// ARGUMENTS
//	bool v_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Key::
setNullable(bool v_)
{
	// この関数が呼ばれるのはKeyが作成中のときでなければならない
	if (getStatus() != Status::Created) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	if (v_ == false) _attribute |= Attribute::NotNull;
}

//	FUNCTION public
//	Schema::Key::getColumnID -- キーにした列のスキーマオブジェクト ID を得る
//
//	NOTES
//		生成前、中のキーや、排他制御がうまく行われていない場合を除けば、
//		キーにした列は必ずひとつ存在するはずである
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			キーにした列のスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			キーにした列が存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Key::
getColumnID() const
{
	return _columnID;
}

//	FUNCTION public
//	Schema::Key::setColumnID -- キーにした列のスキーマオブジェクト ID を設定する
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

inline // 内部でしか使用しない
void
Key::
setColumnID(ID::Value id_)
{
	_columnID = id_;
}

//	FUNCTION public
//	Schema::Key::getIndex -- キーが存在する索引を表すクラスを得る
//
//	NOTES
//		生成前、中のキーや、排他制御がうまく行われていない場合を除けば、
//		キーが存在する索引は必ず存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた索引を格納する領域の先頭アドレス
//		0
//			キーが存在する索引は存在しない
//
//	EXCEPTIONS

Index*
Key::
getIndex(Trans::Transaction& cTrans_) const
{
	return (!m_pIndex) ?
		m_pIndex = Index::get(getParentID(), getDatabase(cTrans_), cTrans_)
		: m_pIndex;
}

//	FUNCTION public
//	Schema::Key::getColumn -- キーにした列を得る
//
//	NOTES
//		生成前、中のキーや、排他制御がうまく行われていない場合を除けば、
//		キーにした列は必ずひとつ存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた列を格納する領域の先頭アドレス
//		0
//			キーにした列が存在しない
//
//	EXCEPTIONS
//		なし

Column*
Key::
getColumn(Trans::Transaction& cTrans_) const
{
	if (!_column) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_column) ?
			_column = Column::get(_columnID, getDatabase(cTrans_), cTrans_)
			: _column;
	}
	return _column;
}

//	FUNCTION public
//	Schema::Key::setColumn -- キーにした列を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Column&		column
//			設定する列
//
//	RETURN
//		設定した列
//
//	EXCEPTIONS
//		なし

const Column&
Key::
setColumn(Column& column)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	_columnID = column.getID();
	return *(_column = &column);
}

//	FUNCTION public
//	Schema::Key::getField -- キーの値を格納するフィールドを得る
//
//	NOTES
//		生成前、中のキーや、排他制御がうまく行われていない場合を除けば、
//		キーの値を格納するフィールドは必ずひとつ存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			キーの値を格納するフィールドが存在しない
//
//	EXCEPTIONS
//		なし

Field*
Key::getField(Trans::Transaction& cTrans_) const
{
	if (!_field) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_field) ?
			_field = Field::get(_fieldID, getDatabase(cTrans_), cTrans_)
			: _field;
	}
	return _field;
}

//	FUNCTION public
//	Schema::Key::setField -- キーの値を格納するフィールドを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field&		field
//			設定するキーの値を格納するフィールド
//
//	RETURN
//		設定したフィールド
//
//	EXCEPTIONS
//		なし

const Field&
Key::
setField(Field& field)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	_fieldID = field.getID();
	return *(_field = &field);
}

//	FUNCTION public
//	Schema::Key::getFieldID -- キーの値を格納するフィールドのスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			キーの値を格納するフィールドのスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			キーの値を格納するフィールドが存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Key::
getFieldID() const
{
	return _fieldID;
}

//	FUNCTION public
//	Schema::Key::setFieldID -- キーの値を格納するフィールドのスキーマオブジェクト ID を設定する
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

inline // 内部でしか使用しない
void
Key::
setFieldID(ID::Value id_)
{
	_fieldID = id_;
}

//	FUNCTION public
//	Schema::Key::appendColumn -- KeyMapをIterationしながらColumnを集めるための関数
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<Column*>& vecColumns_
//			結果を集めるVector
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Key::
appendColumn(ModVector<Column*>& vecColumns_, Trans::Transaction& cTrans_)
{
	vecColumns_.pushBack(getColumn(cTrans_));
}

#ifdef OBSOLETE // Fieldを得る機能は使用されない

//	FUNCTION public
//	Schema::Key::appendField -- KeyMapをIterationしながらFieldを集めるための関数
//
//	NOTES
//
//	ARGUMENTS
//		ModVector<Field*>& vecFields_
//			結果を集めるVector
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Key::
appendField(ModVector<Field*>& vecFields_, Trans::Transaction& cTrans_)
{
	vecFields_.pushBack(getField(cTrans_));
}
#endif

//	FUNCTION public
//	Schema::Key::serialize -- 
//		キーを表すクラスのシリアライザー
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
Key::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// 索引の先頭から何番目か

		archiver << _position;

		// キー値の順序
		{
		int tmp = _order;
		archiver << tmp;
		}
		// キーにした列のスキーマオブジェクト ID

		archiver << _columnID;

		// キーの値を格納するフィールドのスキーマオブジェクト ID

		archiver << _fieldID;

		// キーの属性
		{
		int tmp = _attribute;
		archiver << tmp;
		}
	} else {

		// メンバーをすべて初期化しておく

		clear();

		// 索引の先頭から何番目か

		archiver >> _position;

		// キー値の順序
		{
		int tmp;
		archiver >> tmp;
		_order = static_cast<Order::Value>(tmp);
		}
		// キーにした列のスキーマオブジェクト ID

		archiver >> _columnID;

		// キーの値を格納するフィールドのスキーマオブジェクト ID

		archiver >> _fieldID;

		// キーの属性
		{
		int tmp;
		archiver >> tmp;
		_attribute = static_cast<Attribute::Value>(tmp);
		}
	}
}

//	FUNCTION public
//	Schema::Key::getClassID -- このクラスのクラス ID を得る
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
Key::
getClassID() const
{
	return Externalizable::Category::Key +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Key::isLessThan -- ソートに使う比較関数
//
//	NOTES
//
//	ARGUMENTS
//		const Object& cOther_
//
//	RETURN
//		true .. thisの方がソート順で先である
//
//	EXCEPTIONS
//		なし

bool
Key::
isLessThan(const Object& cOther_) const
{
	; _SYDNEY_ASSERT(cOther_.getCategory() == Object::Category::Key);

	// キーは位置の順に並べる
	return (getPosition() < _SYDNEY_DYNAMIC_CAST(const Key&, cOther_).getPosition());
}

//	FUNCTION public
//	Schema::Key::clear -- キーを表すクラスのメンバーをすべて初期化する
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
Key::
clear()
{
	_position = 0;
	_order = Order::Unknown;
	_column = 0;
	_columnID = Object::ID::Invalid;
	_field = 0;
	_fieldID = Object::ID::Invalid;
	m_pIndex = 0;
}

//	FUNCTION public
//	Schema::Key::makeLogData --
//		ログデータを作る
//
//	NOTES
//
//	ARGUMENTS
//		Common::DataArrayData& cLogData_
//			値を設定するログデータ
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Key::
makeLogData(Common::DataArrayData& cLogData_) const
{
	// キー定義を表すログデータ
	//		1．列ID
	//		2．ソート順
	//		3. ID
	cLogData_.reserve(Log::Num); // 以下のコードが変わったらここも変える
	cLogData_.pushBack(packMetaField(Meta::Key::ColumnID));
	cLogData_.pushBack(packMetaField(Meta::Key::Order));
	cLogData_.pushBack(LogData::createID(getID()));
}

//	FUNCTION private
//	Schema::Key::setName --
//		キーの名称を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Key::
setName(Trans::Transaction& cTrans_)
{
	if (Column* pColumn = getColumn(cTrans_)) {
		Object::setName(pColumn->getName());
	}
}

//	FUNCTION private
//	Schema::Key::setAttribute --
//		キーの属性を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Index& cIndex_
//			キーが属する索引
//		const Column* pColumn_
//			キーがついている列
//
//	RETURN
//	なし
//
//	EXCEPTIONS

void
Key::
setAttribute(const Index& cIndex_,
			 const Column* pColumn_)
{
	// 全文で位置が0でないものはSearchableにしない
	// 列にNullableがついていたらキーにも反映する
	_attribute =
		((cIndex_.getCategory() == Index::Category::FullText && getPosition() > 0)
		 ? Attribute::None : Attribute::Searchable)
		|
		((pColumn_ == 0 || pColumn_->isNullable())
		 ? Attribute::None : Attribute::NotNull);
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

// メタデータベースにおける「キー」表の構造は以下のとおり
// create table Key_DBXXXX (
//		ID			id,
//		parent		id,
//		position	int,
//		order		int,
//		column		id,
//		field		id
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Key>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Key>(Meta::MemberType::_type_, &Key::_get_, &Key::_set_)

	Meta::Definition<Key> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(ParentID),		//ParentID
		_DEFINE0(Integer),		//Position
		_DEFINE0(Integer),		//Order
		_DEFINE2(ID, getColumnID, setColumnID), //ColumnID
		_DEFINE2(ID, getFieldID, setFieldID), //FieldID
		_DEFINE0(Timestamp),	//Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Key::getMetaFieldNumber --
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
Key::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Key::MemberNum);
}

//	FUNCTION public
//	Schema::Key::getMetaFieldDefinition --
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
Key::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Key::packMetaField --
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
Key::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Key>& cDef = _vecDefinition[iMemberID_];

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
			case Meta::Key::Position:
				{
					return pack(static_cast<int>(getPosition()));
				}
			case Meta::Key::Order:
				{
					return pack(static_cast<int>(getOrder()));
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
//	Schema::Key::unpackMetaField --
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
Key::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Key>& cDef = _vecDefinition[iMemberID_];

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
				case Meta::Key::Order:
					{
						if (value >= 0 && value < Order::ValueNum) {
							_order = static_cast<Order::Value>(value);
							return true;
						}
						break;
					}
				case Meta::Key::Position:
					{
						if (value >= 0) {
							_position = value;
							return true;
						}
						break;
					}
				}
			}
		}
	default:
		break;
	}
	return false;
}

//
// Copyright (c) 2000, 2001, 2002, 2005, 2006, 2007, 2009, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
