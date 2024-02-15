// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Column.cpp -- 列関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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
#include "SyInclude.h"

#include "Schema/AutoRWLock.h"
#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/Default.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Field.h"
#include "Schema/File.h"
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
#include "Schema/Sequence.h"
#include "Schema/SystemTable_Column.h"
#include "Schema/Table.h"
#include "Schema/TupleID.h"
#include "Schema/Utility.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/Collation.h"
#include "Common/ColumnMetaData.h"
#include "Common/DataArrayData.h"
#include "Common/Message.h"
#include "Common/NullData.h"
#include "Common/SQLData.h"

#include "Exception/ColumnAlreadyDefined.h"
#include "Exception/ColumnLengthOutOfRange.h"
#include "Exception/ColumnPrecisionOutOfRange.h"
#include "Exception/ColumnScaleOutOfRange.h"
#include "Exception/LogItemCorrupted.h"
#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/TooLongObjectName.h"
#include "Exception/Unexpected.h"

#include "Os/RWLock.h"

#include "Statement/ColumnDefinition.h"
#include "Statement/ColumnConstraintDefinition.h"
#include "Statement/ColumnConstraintDefinitionList.h"
#include "Statement/Hint.h"
#include "Statement/Literal.h"
#include "Statement/Identifier.h"
#include "Statement/IntegerValue.h"
#include "Statement/ValueExpression.h"

#include "FileCommon/FileOption.h"
#include "Btree2/FileID.h"

#include "Trans/Transaction.h"

#include "ModVector.h"
#include "ModArchive.h"
#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	// Indexを調査してセットするフラグ
	struct _Flag {
		enum Value {
			Unique = 1,
			CaseInsensitive = Unique * 2,
		};
	};
}

/////////////////////////////////////
// Column::DataType
/////////////////////////////////////

//	FUNCTION public
//	Schema::Column::DataType::DataType -- コンストラクター
//
//	NOTES

Column::DataType::
DataType()
	: Super()
{}

//	FUNCTION public
//	Schema::Column::DataType::DataType -- コンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		const Common::SQLData& cType_
//			列の型を表すSQLDataオブジェクト

Column::DataType::
DataType(const Common::SQLData& cType_)
	: Super(cType_)
{}

//	FUNCTION public
//	Schema::Column::DataType::~Type -- デストラクター
//
//	NOTES

Column::DataType::
~DataType()
{}

//	FUNCTION public
//	Schema::Column::DataType::operator= -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Column::DataType& cType_
//			コピー元のColumn::DataType型オブジェクト

Column::DataType&
Column::DataType::
operator=(const DataType& cType_)
{
	Super::operator=(cType_);
	return *this;
}

//	FUNCTION public
//	Schema::Column::DataType::operator= -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//		const Common::SQLData& cType_
//			列の型を表すSQLDataオブジェクト

Column::DataType&
Column::DataType::
operator=(const Common::SQLData& cType_)
{
	Super::operator=(cType_);
	return *this;
}

//	FUNCTION public
//	Schema::Column::DataType::operator= -- 代入演算子
//
//	NOTES
//
//	ARGUMENTS
//		const TypeOld& cType_
//			コピー元のColumn::DataType型オブジェクト

Column::DataType&
Column::DataType::
operator=(const DataTypeOld& cType_)
{
	setType(cType_.getType());
	setFlag(cType_.getFlag());
	setLength(cType_.getLength());
	setScale(0);
	setMaxCardinality(cType_.getMaxCardinality());
	return *this;
}

// FUNCTION public
//	Schema::Column::DataType::check -- データ型として許されるか調べる
//
// NOTES
//
// ARGUMENTS
//	const Schema::Object::Name& cName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Column::DataType::
check(const Schema::Object::Name& cName_) const
{
	switch (getType()) {
	case Type::Char:
	case Type::NChar:
	case Type::Binary:
		{
			switch (getFlag()) {
			case Flag::Fixed:
				{
					// 固定長文字列型は400文字が最大
					// 固定長バイナリー型も400バイトが最大
					const int _maxFixedChar = 400;

					if (getLength() <= 0 || getLength() > _maxFixedChar) {
						_SYDNEY_THROW3(Exception::ColumnLengthOutOfRange, cName_, getLength(), _maxFixedChar);
					}
					break;
				}
			}
			break;
		}
	case Type::Decimal:
		{
			// length(precision) must no be larger than Decimal limit
			// scale must no be larger than length
			const int _maxDecimalPrecision = Common::SQLData::getMaxPrecision(getType());

			if (getLength() <= 0 || getLength() > _maxDecimalPrecision) {
				_SYDNEY_THROW3(Exception::ColumnPrecisionOutOfRange, cName_, getLength(),
							   getLength() <= 0 ? 1 : _maxDecimalPrecision);
			}
			if (getScale() > getLength()) {
				_SYDNEY_THROW3(Exception::ColumnScaleOutOfRange, cName_, getScale(), getLength());
			}
			break;
		}
	}
}

// FUNCTION public
//	Schema::Column::DataType::setFieldType -- データ型に対応するフィールド型と長さをセットする
//
// NOTES
//
// ARGUMENTS
//	Common::DataType::Type* pTypeTarget_
//	ModSize* pLengthTarget_
//	int* pScaleTarget_
//	bool* pTargetFixed_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Column::DataType::
setFieldType(Common::DataType::Type* pTypeTarget_,
			 ModSize* pLengthTarget_,
			 int* pScaleTarget_,
			 bool* pTargetFixed_) const
{
	switch (getType()) {
	case Common::SQLData::Type::Char:

		// Sydney では Unicode を使用しているので 2 を乗ずる
		// -> ファイル内ではcharなので乗ずるのをやめる
		//	  ただしNoEncodingFormが指定されている場合は引き続き2を乗ずる
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::String;
		if (pLengthTarget_) {
			if (Manager::Configuration::isNoEncodingForm())
				*pLengthTarget_ = sizeof(ModUnicodeChar) * getLength();
			else
				*pLengthTarget_ = getLength();
		}
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::NChar:

		// NChar は UCS2 で表現され、
		// UCS2 の 1 文字は 2 バイトで表現されるので、2 を乗ずる

		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::String;
		if (pLengthTarget_)
			*pLengthTarget_ = sizeof(ModUnicodeChar) * getLength();
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::Int:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::Integer;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::BigInt:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::Integer64;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::UInt:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::UnsignedInteger;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::Float:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::Double;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::DateTime:
	case Common::SQLData::Type::Timestamp:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::DateTime;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::UniqueIdentifier:

		// UniqueIdentifier は 'xxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxxx'
		// の形式の ASCII 文字列である

		// Sydney では Unicode を使用しているので 2 を乗ずる
		// -> v15では UniqueIdentifier はchar(36)の意味になったが、
		//	  Statementのレベルですでにそのように変換されるので
		//	  この実装は互換性のために以前の実装のままにする。

		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::String;
		if (pLengthTarget_)
			*pLengthTarget_ = sizeof(ModUnicodeChar) * 36;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::Binary:
	case Common::SQLData::Type::Image:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::Binary;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::NText:
	case Common::SQLData::Type::Fulltext:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::String;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::Language:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::Language;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::BLOB:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::Binary;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;

	case Common::SQLData::Type::NCLOB:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::String;
		if (pLengthTarget_)
			*pLengthTarget_ = 0;
		if (pScaleTarget_)
			*pScaleTarget_ = 0;
		break;
	case Common::SQLData::Type::Decimal:
		if (pTypeTarget_)
			*pTypeTarget_ = Common::DataType::Decimal;
		if (pLengthTarget_)
			*pLengthTarget_ = getLength();
		if (pScaleTarget_)
			*pScaleTarget_ = getScale();
		break;
	case Common::SQLData::Type::CLOB: // 未対応
	case Common::SQLData::Type::NoType:
	default:
		; _SYDNEY_ASSERT(false);
	}

	switch (getFlag()) {
	case Common::SQLData::Flag::None:
	case Common::SQLData::Flag::Fixed:
		{
			if (pTargetFixed_)
				*pTargetFixed_ = true;
			break;
		}
	case Common::SQLData::Flag::OldFixed:
	case Common::SQLData::Flag::OldVariable:
	case Common::SQLData::Flag::Variable:
	case Common::SQLData::Flag::Unlimited:
		{
			if (pTargetFixed_)
				*pTargetFixed_ = false;
			break;
		}
	}
}

/////////////////////////////////////
// Column::DataTypeOld
/////////////////////////////////////

//	FUNCTION public
//	Schema::Column::DataTypeOld::DataTypeOld -- コンストラクター
//
//	NOTES

Column::DataTypeOld::
DataTypeOld()
{}

//	FUNCTION public
//	Schema::Column::DataTypeOld::~DataTypeOld -- デストラクター
//
//	NOTES

Column::DataTypeOld::
~DataTypeOld()
{}

//	FUNCTION public
//	Schema::Column::DataTypeOld::getClassID -- クラスIDを得る
//
//	NOTES

int
Column::DataTypeOld::
getClassID() const
{
	return Schema::Externalizable::Category::ColumnType +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Column::DataTypeOld::serialize -- シリアライズする
//
//	NOTES
//
//	ARGUMENTS
//		ModArchive& cArchiver_
//			シリアライズの対象となるアーカイバー

void
Column::DataTypeOld::
serialize(ModArchive& cArchiver_)
{
	if (cArchiver_.isStore()) {

		; _SYDNEY_ASSERT(0);

	} else {

		{
		int tmp;
		cArchiver_ >> tmp;
		m_eType = static_cast<Common::SQLData::Type::Value>(tmp);
		}
		{
		int tmp;
		cArchiver_ >> tmp;
		m_eFlag = static_cast<Common::SQLData::Flag::Value>(tmp);
		}
		{
		int tmp;
		cArchiver_ >> tmp;
		m_iLength = tmp;
		}
		{
		int tmp;
		cArchiver_ >> tmp;
		m_iCardinality = tmp;
		}
	}
}

/////////////////////////////////////
// Column
/////////////////////////////////////

//	FUNCTION public
//	Schema::Column::Column -- 列を表すクラスのデフォルトコンストラクター
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

Column::
Column()
	: Object(Object::Category::Column),
	  _position(0),
	  _category(Category::Unknown),
	  m_pTable(0),
	  _field(0),
	  _fieldID(Object::ID::Invalid),
	  _keys(0),
	  m_bNullable(true),
	  m_pHint(0),
	  m_iFlag(-1),
	  m_pSequence(0)
{ }

//	FUNCTION public
//	Schema::Column::Column -- 列を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table&		table
//			列が存在する表を表すクラス
//		Schema::Column::Position	position
//			表の先頭からの列の位置
//		Schema::Object::Name&	name
//			列の名前
//		Schema::Column::Category::Value	category
//			列の種類
//		Common::SQLData&	type
//			列のデータ型
//		Schema::Default		def
//			列のデフォルト指定
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Column::
Column(const Table& table, Column::Position position,
	   const Name& name, Column::Category::Value category,
	   const Common::SQLData& type, const Default& def)
	: Object(Object::Category::Column, table.getScope(), table.getStatus(),
			 ID::Invalid, table.getID(), table.getDatabaseID(),
			 name),
	  _position(position),
	  _category(category),
	  _type(type),
	  _default(def),
	  m_pTable(const_cast<Table*>(&table)),
	  _field(0),
	  _fieldID(Object::ID::Invalid),
	  _keys(0),
	  m_bNullable(true),
	  m_pHint(0),
	  m_iFlag(-1),
	  m_pSequence(0)
{ }

//	FUNCTION public
//	Schema::Column::Column -- 列定義からの列を表すクラスのコンストラクター
//
//	NOTES
//		列を表すクラスを生成するだけで、「列」表は更新されない
//
//	ARGUMENTS
//		Schema::Table&		table
//			列が存在する表を表すクラス
//		Schema::Column::Position	position
//			表の先頭からの列の位置
//		Statement::ColumnDefinition&	statement
//			解析済の SQL の列定義
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Column::
Column(const Table& table, Column::Position position,
	   const Statement::ColumnDefinition& statement)
	: Object(Object::Category::Column, table.getScope(), table.getStatus(),
			 ID::Invalid, table.getID(), table.getDatabaseID()),
	  _position(position),
	  _category(Category::Normal),
	  m_pTable(const_cast<Table*>(&table)),
	  _field(0),
	  _fieldID(Object::ID::Invalid),
	  _keys(0),
	  m_bNullable(true),
	  m_pHint(0),
	  m_iFlag(-1),
	  m_pSequence(0)
{
	// 列定義から列名を得て、処理する

	Statement::Identifier* identifier = statement.getName();
	; _SYDNEY_ASSERT(identifier);
	; _SYDNEY_ASSERT(identifier->getIdentifier());
	setName(*identifier->getIdentifier());

	// 列定義からデータ型を得て、処理する

	_type = statement.getDataType();

	// 列定義からヒントを得て、処理する
	// This process should be executed before creating default value
	// because default value is casted to the type

	if (Statement::Hint* hint = statement.getHint()) {
		m_pHint = new Hint(*hint);
		// NonTruncateならCollationにNoPadを設定する
		if (m_pHint->getCategory() & Hint::Category::NonTruncate) {
			_type.setCollation(Common::Collation::Type::NoPad);
		}
	}

	// 列定義から列に対する制約の情報を得て、処理する
	if (Statement::ColumnConstraintDefinitionList* constraints = statement.getConstraints()) {
		int n = constraints->getCount();
		for (int i = 0; i < n; ++i) {
			Statement::ColumnConstraintDefinition* constraint = constraints->getColumnConstraintDefinitionAt(i);
			switch (constraint->getConstraintType()) {
			case Statement::ColumnConstraintDefinition::NotNull:
				{
					setNullable(false);
					break;
				}
			default:
				{
					_SYDNEY_THROW0(Exception::NotSupported);
				}
			}
		}
	}

	// 列定義からデフォルト値を得て、処理する
	Statement::ValueExpression* defaultValue = statement.getDefaultValue();
	if (defaultValue) {
		_default.setValue(getName(), *defaultValue, statement.isUseOnUpdate(), _type);
		if (_default.isIdentity()) {
			// Identity ColumnはNOT NULLをImplyする
			setNullable(false);
		}
	} else {
		// 列定義からconst値を得て、処理する
		Statement::ValueExpression* constValue = statement.getConstValue();
		if (constValue) {
			_default.setValue(getName(), *constValue, false, _type);
			_category = Category::Constant;
		}
	}
}

//	FUNCTION public
//	Schema::Column::Column -- ログデータからの列を表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table&		table
//			列が存在する表を表すクラス
//		Schema::Column::Position	position
//			表の先頭からの列の位置
//		const Common::DataArrayData&	cLogData_
//			列定義のログデータ
//
//	RETURN
//		なし
//
//	EXCEPTIONS

Column::
Column(const Table& table, Column::Position position,
	   const Common::DataArrayData& cLogData_)
	: Object(Object::Category::Column, table.getScope(), table.getStatus(),
			 ID::Invalid, table.getID(), table.getDatabaseID()),
	  _position(position),
	  _category(Category::Normal),
	  m_pTable(const_cast<Table*>(&table)),
	  _field(0),
	  _fieldID(Object::ID::Invalid),
	  _keys(0),
	  m_bNullable(true),
	  m_pHint(0),
	  m_iFlag(-1),
	  m_pSequence(0)
{
	// ログデータの内容を反映する
	// ★注意★
	// makeLogDataの実装が変わったらここも変える
	int i = 0;
	// IDはここでは使わない
	++i;
	if (!unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Name)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Category)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Default)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Flag)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Type)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Precision)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Scale)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::CharacterSet)
		|| !unpackMetaField(cLogData_.getElement(i++).get(), Meta::Column::Hint)) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}
}

// copy constructor with new data type (for alter column)
Column::
Column(const Column& original, const Common::SQLData& type)
	: Object(Object::Category::Column, original.getScope(), original.getStatus(),
			 original.getID(), original.getParentID(), original.getDatabaseID(),
			 original.getName()),
	  _position(original._position),
	  _category(original._category),
	  _type(type),
	  _default(original._default),
	  m_pTable(0),
	  _field(0),
	  _fieldID(original._fieldID),
	  _keys(0),
	  m_bNullable(original.m_bNullable),
	  m_pHint(0),
	  m_iFlag(original.m_iFlag),
	  m_pSequence(0)
{ }

//	FUNCTION public
//	Schema::Column::~Column -- 列を表すクラスのデストラクター
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

Column::
~Column()
{
	destruct();
}

//	FUNCTION public
//		Schema::Column::getNewInstance -- オブジェクトを新たに取得する
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
Column*
Column::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Column> pObject = new Column;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION private
//	Schema::Column::destruct -- 列を表すクラスのデストラクター下位関数
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
Column::
destruct()
{
	// ★注意★
	// デストラクトのときは保持するオブジェクトを行儀よく片付ける必要はない
	// 必要ならばこのオブジェクトをdeleteするところでresetを呼ぶ
	// ここでは領域を開放するのみ

	clearSequence();

	delete _keys, _keys = 0;

	delete m_pHint, m_pHint = 0;
}

//	FUNCTION public
//	Schema::Column::create -- 列を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table&		table
//			列が存在する表を表すクラス
//		Schema::Column::Position	position
//			表の先頭からの列の位置
//		Schema::Object::Name&	name
//			列の名前
//		Schema::Column::Category::Value	category
//			列の種類
//		Common::SQLData&	type
//			列のデータ型
//		Schema::Default		def
//			列のデフォルト指定
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//
//	RETURN
//		生成された列を表すクラス
//
//	EXCEPTIONS
//		なし

// static
Column::Pointer
Column::
create(Table& table, Column::Position position,
	   const Name& name, Column::Category::Value category,
	   const Common::SQLData& type, const Default& def,
	   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 指定されたデータにより新たな列を表すクラスを生成する

	Pointer pColumn = new Column(table, position, name, category, type, def);

	; _SYDNEY_ASSERT(pColumn.get());

	// 同じ名前の列がないか調べる

	if (table.getColumn(pColumn->getName(), cTrans_))
		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated column definition("
						   << pColumn->getName()
						   << ")	canceled"
						   << ModEndl;
			return Pointer();
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated column definition("
						   << pColumn->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::ColumnAlreadyDefined, pColumn->getName(), table.getName());
		}

	// IDをふり、状態を変える
	pColumn->Object::create(cTrans_);

	return pColumn;
}

//	FUNCTION public
//	Schema::Column::create -- 列定義から列を表すクラスを生成する
//
//	NOTES
//		列を表すクラスを生成するだけで、「列」表は更新されない
//
//	ARGUMENTS
//		Schema::Table&		table
//			列が存在する表を表すクラス
//		Schema::Column::Position	position
//			表の先頭からの列の位置
//		Statement::ColumnDefinition&	statement
//			解析済の SQL の列定義
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//
//	RETURN
//		生成された列を表すクラス
//
//	EXCEPTIONS

Column::Pointer
Column::
create(Table& table, Column::Position position,
	   const Statement::ColumnDefinition& statement,
	   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// 列定義から列を表すクラスを作る

	Pointer pColumn = new Column(table, position, statement);

	; _SYDNEY_ASSERT(pColumn.get());

	if (pColumn->getName().getLength() > Manager::ObjectName::getMaxLength()) {
		// 名称が制限長を超えていたらエラー
		_SYDNEY_THROW2(Exception::TooLongObjectName,
					   pColumn->getName().getLength(), Manager::ObjectName::getMaxLength());
	}

	// 型の正当性を調べる
	pColumn->getType().check(pColumn->getName());

	// 同じ名前の列がないか調べる

	if (table.getColumn(pColumn->getName(), cTrans_))
		if (Manager::Configuration::isCanceledWhenDuplicated()) {
			// 0を返し後の処理は何もしない
			SydInfoMessage << "Duplicated column definition("
						   << pColumn->getName()
						   << ")	canceled"
						   << ModEndl;
			return Pointer();
		} else {
			// 例外を送出
			SydInfoMessage << "Duplicated column definition("
						   << pColumn->getName()
						   << ")"
						   << ModEndl;
			_SYDNEY_THROW2(Exception::ColumnAlreadyDefined, pColumn->getName(), table.getName());
		}

	// IDをふり、状態を変える
	pColumn->Object::create(cTrans_);

	return pColumn;
}

//	FUNCTION public
//	Schema::Column::create -- 列定義のログデータから列を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Table&		table
//			列が存在する表を表すクラス
//		Schema::Column::Position	position
//			表の先頭からの列の位置
//		const Common::DataArrayData& cLogData_
//			列定義のログデータ
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//		const Schema::Column* pOriginalColumn_ = 0
//			0でないときAlter Columnで変更前の列を指すオブジェクト
//
//	RETURN
//		生成された列を表すクラス
//
//	EXCEPTIONS

Column::Pointer
Column::
create(Table& table, Column::Position position,
	   const Common::DataArrayData& cLogData_, Trans::Transaction& cTrans_,
	   const Column* pOriginalColumn_ /* = 0 */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// ログデータから列を表すクラスを作る

	Pointer pColumn = new Column(table, position, cLogData_);

	; _SYDNEY_ASSERT(pColumn.get());

	ID::Value id = getObjectID(cLogData_);

	// IDをふり、状態を変える
	pColumn->Object::create(cTrans_, id);

	if (pOriginalColumn_) {
		pColumn->setFieldID(pOriginalColumn_->getFieldID());
		pColumn->setStatus(pOriginalColumn_->getStatus());
	}

	return pColumn;
}

//	FUNCTION public
//	Schema::Column::createSystem -- システム表の列を表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//		Schema::Table&		table
//			列が存在する表を表すクラス
//		Schema::Column::Position	position
//			表の先頭からの列の位置
//		Schema::Object::Name&	name
//			列の名前
//		Schema::Column::Category::Value	category
//			列の種類
//		Common::SQLData&	type
//			列のデータ型
//		Schema::Default		def
//			列のデフォルト指定
//		Schema::Object::ID::Value iObjectID_
//			列のオブジェクトID
//
//	RETURN
//		生成された列を表すクラス
//
//	EXCEPTIONS

// static
Column::Pointer
Column::
createSystem(Trans::Transaction& cTrans_,
			 Table& table, Position position,
			 const Name& name, Category::Value category,
			 const Common::SQLData& type,
			 const Default& def,
			 ID::Value iObjectID_)
{
	Pointer pColumn = new Column(table, position, name, category, type, def);
	pColumn->setID(iObjectID_);
	pColumn->setStatus(Status::Persistent);

	return pColumn;
}

// FUNCTION public
//	Schema::Column::drop -- Drop column definition
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
Column::
drop(Trans::Transaction& cTrans_, bool bRecovery_ /* = false */, bool bNoUnset_ /* = false */)
{
	switch (getStatus()) {
	case Status::Created:
		{
			// unmount sequence file so that the file descriptor become invalid
			// [NOTES] 
			// When a sequence file is created but not persisted, sequence file is empty.
			if (m_pSequence) {
				m_pSequence->unmount(cTrans_);
			}
			break;
		}
	default:
		break;
	}
	Object::drop(bRecovery_, bNoUnset_);
}

//	FUNCTION public
//	Schema::Column::getObjectID -- 列のログからIDを取得する
//
//	NOTES
//
//	ARGUMENTS
//	  	const Common::DataArrayData& cLogData_
//			列の生成を表すログデータ
//
//	RETURN
//		ログに記録されているID
//
//	EXCEPTIONS

// static
Object::ID::Value
Column::
getObjectID(const Common::DataArrayData& cLogData_)
{
	// ログデータの内容を得る
	// ★注意★
	// makeLogDataの実装が変わったらここも変える
	return LogData::getID(cLogData_.getElement(0));
}

//	FUNCTION public
//	Schema::Column::get -- あるスキーマオブジェクト ID の列を表すクラスを得る
//
//	NOTES
//		マウント中のデータベースに対してこの関数を使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			列のスキーマオブジェクト ID
//		Schema::Database* pDatabase_
//			列を探すデータベースのスキーマオブジェクト
//			値が0ならすべてのデータベースについて調べる
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//	
//	RETURN
//		0 以外の値
//			得られた列を格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID の列は存在しない
//
//	EXCEPTIONS

// static
Column*
Column::
get(ID::Value id_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	return ObjectTemplate::get<Column, SystemTable::Column, Object::Category::Column>(id_, pDatabase_, cTrans_);
}

//
//	FUNCTION public
//		Schema::Column::get -- スキーマオブジェクトIDを指定して列を得る
//
//	NOTES
//		マウント中のデータベースに対してこの関数を使用してはいけない
//
//	ARGUMENTS
//		Schema::Object::ID::Value iID_
//			取得する列のスキーマオブジェクトID
//		Schema::Object::ID::Value iDatabaseID_
//			取得する列が属するデータベースID
//		Trans::Transaction& cTrans_
//			列を取得しようとしているトランザクション記述子
//
//	RETURN
//		取得した列のオブジェクト
//
//	EXCEPTIONS
//		???
//

// static
Column*
Column::
get(ID::Value iID_, ID::Value iDatabaseID_, Trans::Transaction& cTrans_)
{
	if (iID_ == Object::ID::Invalid)
		return 0;

	return get(iID_, Database::get(iDatabaseID_, cTrans_), cTrans_);
}

//	FUNCTION public
//	Schema::Column::isValid -- 陳腐化していないか
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
Column::
isValid(ID::Value iID_, ID::Value iDatabaseID_, Timestamp iTimestamp_,
		Trans::Transaction& cTrans_)
{
	Column* pColumn = get(iID_, iDatabaseID_, cTrans_);

	return (pColumn && pColumn->getTimestamp() == iTimestamp_);
}

//	FUNCTION public
//	Schema::Column::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Column::Pointer& pColumn_
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
Column::
doBeforePersist(const Pointer& pColumn_, Status::Value eStatus_, bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	// 何もしない
	;
}

//	FUNCTION public
//	Schema::Column::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Column::Pointer& pColumn_
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
Column::
doAfterPersist(const Pointer& pColumn_, Status::Value eStatus_, bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pColumn_.get());

	// deleteされる可能性があるのでデータベースIDをここで取得しておく
	ID::Value dbID = pColumn_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// データベースに列を表すクラスを
		// スキーマオブジェクトとして管理させる

		if (Database* pDatabase = pColumn_->getDatabase(cTrans_))
			pDatabase->addCache(pColumn_);
		break;
	}
	case Status::CreateCanceled:
	{
		// 表の登録から抹消する
		Table* pTable = pColumn_->getTable(cTrans_);
		; _SYDNEY_ASSERT(pTable);
		pTable->eraseColumn(pColumn_->getID());
		break;
	}
	case Status::Changed:
	{
		// 変更があったらクラス内でキャッシュしている情報をクリアしておく
		pColumn_->clearKey();
		pColumn_->clearFlag();
		break;
	}
	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 変更が削除だったらキャッシュや表の登録からの削除も行う

		// 状態を「実際に削除された」にする

		pColumn_->setStatus(Schema::Object::Status::ReallyDeleted);

		// AddColumnのエラー処理で使われる
		if (bNeedToErase_) {

			Database* pDatabase = pColumn_->getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			// 下位オブジェクトがあればそれを抹消してからdeleteする
			pColumn_->reset(*pDatabase);

			// キャッシュから抹消する
			// NeedToErase==falseのときは親オブジェクトのdeleteの中で
			// キャッシュから抹消される
			pDatabase->eraseCache(pColumn_->getID());

			// 表の登録から抹消する → deleteされる
			Table* pTable = pColumn_->getTable(cTrans_);
			; _SYDNEY_ASSERT(pTable);
			pTable->eraseColumn(pColumn_->getID());
		}
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::Column);
}

//	FUNCTION public
//	Schema::Column::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::ColumnPointer& pColumn_
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
Column::
doAfterLoad(const Pointer& pColumn_, Table& cTable_, Trans::Transaction& cTrans_)
{
	// 表へ読み出した列を表すクラスを追加する
	// また、データベースにこの列を表すクラスを
	// スキーマオブジェクトとして管理させる

	cTable_.getDatabase(cTrans_)->addCache(cTable_.addColumn(pColumn_, cTrans_));
}
// object method version (for apply function)
void
Column::
doAfterLoad(Table& cTable_, Trans::Transaction& cTrans_)
{
	doAfterLoad(Pointer(this), cTable_, cTrans_);
}

//	FUNCTION public
//	Schema::Column::reset --
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
Column::
reset(Database& cDatabase_)
{
	clearSequence();

	if (_keys)
		resetKey();
}

//	FUNCTION public
//	Schema::Column::getPosition -- 列の表の先頭からの位置を得る
//
//	NOTES
//		表の先頭からなん番目にその列が定義されているかを、列の位置とする
//		ただし、定義されている制約は数に入れない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた列の位置
//
//	EXCEPTIONS
//		なし

Column::Position
Column::getPosition() const
{
	return _position;
}

//	FUNCTION public
//	Schema::Column::getCategory -- 列の種類を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた列の種類
//
//	EXCEPTIONS
//		なし

Column::Category::Value
Column::
getCategory() const
{
	return _category;
}

// FUNCTION public
//	Schema::Column::getFunction -- 関数列の種別を得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//
// RETURN
//	Column::Function::Value
//
// EXCEPTIONS

Column::Function::Value
Column::
getFunction(Trans::Transaction& cTrans_) const
{
	if (getCategory() == Category::Function) {
		const Field* pField = getField(cTrans_);
		if (getParentID() == Table::getSystemTableID(Object::Category::Column)) {
			// system_column
			switch (pField->getPosition()) {
			case Meta::Column::MetaData:
				return Function::ColumnMetaData;
			default:
				break;
			}
		} else if (getParentID() == Table::getSystemTableID(Object::Category::File)) {
			// system_file
			switch (pField->getPosition()) {
			case Meta::File::FileSize:
				return Function::FileSize;
			default:
				break;
			}
		} else if (getParentID() == Table::getSystemTableID(Object::Category::Index)) {
			// system_index
			switch (pField->getPosition()) {
			case Meta::Index::HintString:
				return Function::IndexHint;
			default:
				break;
			}
		} else if (getParentID() == Table::getSystemTableID(Object::Category::Privilege)) {
			// system_privilege
			switch (pField->getPosition()) {
			case Meta::Privilege::Flags:
				return Function::PrivilegeFlag;
			case Meta::Privilege::Type:
				return Function::PrivilegeObjectType;
			case Meta::Privilege::ObjectIDs:
				return Function::PrivilegeObjectID;
			default:
				break;
			}
		} else if (getParentID() == Table::getSystemTableID(Object::Category::Partition)) {
			// system_partition
			switch (pField->getPosition()) {
			case Meta::Partition::Category:
				return Function::PartitionCategory;
			default:
				break;
			}
		} else if (getParentID() == Table::getSystemTableID(Object::Category::Function)) {
			// system_function
			switch (pField->getPosition()) {
			case Meta::Function::Routine:
				return Function::FunctionRoutine;
			default:
				break;
			}
		} else if (getParentID() == Table::getSystemTableID(Object::Category::Database)) {
			// system_database
			switch (pField->getPosition()) {
			case Meta::Database::Path:
				return Function::DatabasePath;
			case Meta::Database::MasterURL:
				return Function::DatabaseMasterURL;
			default:
				break;
			}
		}
	}
	return Function::Unknown;
}

//	FUNCTION public
//	Schema::Column::getFieldType -- 対応するCommon::Dataの型を得る
//
//	NOTES
		
void
Column::
getFieldType(Common::DataType::Type& eType_,
			 ModSize& iLength_, int& iScale_, bool& bFixed_,
			 Common::DataType::Type& eElementType_,
			 ModSize& iElementLength_, bool& bElementFixed_) const
{
	Common::DataType::Type* pTypeTarget = &eType_;
	ModSize* pLengthTarget = &iLength_;
	int* pScaleTarget = &iScale_;
	bool* pTargetFixed = &bFixed_;

	if (int maxCardinality = getType().getMaxCardinality()) {
		eType_ = Common::DataType::Array;
		iLength_ = maxCardinality < 0 ? static_cast<ModSize>(0) : static_cast<ModSize>(maxCardinality);
		bFixed_ = maxCardinality < 0 ? false : true;
		pTypeTarget = &eElementType_;
		pLengthTarget = &iElementLength_;
		pTargetFixed = &bElementFixed_;
	}

	getType().setFieldType(pTypeTarget, pLengthTarget, pScaleTarget, pTargetFixed);
	if (isTupleID())
		*pTypeTarget = TupleID().getType();
}

//	FUNCTION public
//	Schema::Column::getType -- 列のデータ型を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた列のデータ型
//
//	EXCEPTIONS
//		なし

const Column::DataType&
Column::getType() const
{
	return _type;
}

//	FUNCTION public
//	Schema::Column::isNullable -- 列にNotNull制約がついていないかを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true...列にNULL値を格納できる
//		false..列にNotNull制約がついている
//
//	EXCEPTIONS
//		なし

bool
Column::isNullable() const
{
	return m_bNullable && !isTupleID();
}

//	FUNCTION public
//	Schema::Column::setNullable -- 列にNotNull制約がついていないかを設定する
//
//	NOTES
//
//	ARGUMENTS
//		bool bNullable_
//			true...列にNULL値を格納できる
//			false..列にNotNull制約がついている
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

void
Column::setNullable(bool bNullable_)
{
	m_bNullable = bNullable_;
}

//	FUNCTION public
//	Schema::Column::isInHeap -- 列のヒントにHEAPがあるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true...列のヒントにHEAPがある
//		false..列のヒントにHEAPがない
//
//	EXCEPTIONS
//		なし

bool
Column::
isInHeap() const
{
	return getHint()
		&& (getHint()->getCategory() & Hint::Category::Heap);
}

//	FUNCTION public
//	Schema::Column::isNonTruncate -- 列のヒントにNONTRUNCATEがあるか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true...列のヒントにNONTRUNCATEがある
//		false..列のヒントにNONTRUNCATEがない
//
//	EXCEPTIONS
//		なし

bool
Column::
isNonTruncate() const
{
	return getHint()
		&& (getHint()->getCategory() & Hint::Category::NonTruncate);
}

//	FUNCTION public
//	Schema::Column::isLob -- LOB型の列か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true...LOB型の列である
//		false..LOB型の列ではない
//
//	EXCEPTIONS
//		なし

bool
Column::
isLob() const
{
	switch (_type.getType()) {
	case Common::SQLData::Type::BLOB:
	case Common::SQLData::Type::CLOB:
	case Common::SQLData::Type::NCLOB:
		return true;
	}
	return false;
}

// FUNCTION public
//	Schema::Column::isUnique -- 列の値がUNIQUEであるかを得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Column::
isUnique(Trans::Transaction& cTrans_) const
{
	investigateIndex(cTrans_);
	AutoRWLock l(getRWLock());
	return (m_iFlag & _Flag::Unique) != 0;
}

// FUNCTION public
//	Schema::Column::isCaseInsensitive -- 列の値が大文字小文字を区別しないかを得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Column::
isCaseInsensitive(Trans::Transaction& cTrans_) const
{
	investigateIndex(cTrans_);
	AutoRWLock l(getRWLock());
	return (m_iFlag & _Flag::CaseInsensitive) != 0;
}

// FUNCTION public
//	Schema::Column::isFixed -- 列の値が固定長であるかを得る
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
Column::
isFixed() const
{
	return getType().getFlag() == Column::DataType::Flag::Fixed;
}

namespace
{
	struct _Alterability {
		enum Value {
			NoNeed = 0,
			Alterable,
			CompareLength,
			NeverAlterable,
			ValueNum,
		};
	};
#define N _Alterability::NoNeed
#define A _Alterability::Alterable
#define C _Alterability::CompareLength
#define X _Alterability::NeverAlterable
	_Alterability::Value _alterabilityTable[Column::DataType::Flag::ValueNum][Column::DataType::Flag::ValueNum] =
	{
		// XX OF OV Ul Fx Vr  To/From
		{  X, X, X, X, X, X }, // XX(None)
		{  X, C, X, A, C, X }, // OF(OldFixed)
		{  X, X, C, A, X, C }, // OV(OldVariable)
		{  X, X, X, N, X, X }, // Ul(Unlimited)
		{  X, X, X, X, X, X }, // Fx(Fixed)
		{  X, X, X, A, X, C }, // Vr(Variable)
	};
#undef N
#undef A
#undef C
}

// Check whether new type is alterable
bool
Column::
isAbleToAlter(const Statement::ColumnDefinition& cDefinition_,
			  ColumnPointer& pNewColumn_)
{
	// Get the current and new type specification from ColumnDefinition
	DataType cOldType = getType();
	DataType cNewType = cDefinition_.getDataType();

	// At least, type and cardinality must be the same
	if (cOldType.getType() == cNewType.getType()
		&& cOldType.getMaxCardinality() == cNewType.getMaxCardinality()) {

		switch (_alterabilityTable[cOldType.getFlag()][cNewType.getFlag()]) {
		case _Alterability::NoNeed:
			{
				// no need to be altered
				return false;
			}
		case _Alterability::Alterable:
			{
				// alterable
				pNewColumn_ = new Column(*this, cNewType);
				return true;
			}
		case _Alterability::CompareLength:
			{
				if (cOldType.getLength() == cNewType.getLength())
					// no need to be altered
					return false;
				if (cOldType.getLength() < cNewType.getLength()
					|| cNewType.getLength() <= 0 /* unlimited */) {
					// new type is longer than old type
					pNewColumn_ = new Column(*this, cNewType);
					return true;
				}
				// otherwise, throw an exception
				break;
			}
		case _Alterability::NeverAlterable:
		default:
			{
				break;
			}
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

//	FUNCTION public
//	Schema::Column::getDefault -- 列のデフォルト指定を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた列のデフォルト指定
//
//	EXCEPTIONS
//		なし

const Default&
Column::getDefault() const
{
	return _default;
}

// FUNCTION public
//	Plan::Column::getIndexField -- 索引フィールドを得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//		処理を行うトランザクション記述子
//	ModVector<Field*>& vecField_
//		列についている索引ファイルの対応フィールドを格納するModVector
//	
// RETURN
//	true ... 結果が1つ以上ある
//	false ... 結果がない1つ以上ある
//
// EXCEPTIONS

bool
Column::getIndexField(Trans::Transaction& cTrans_,
					  ModVector<Field*>& vecField_)
{
	if (getTable(cTrans_)->isVirtual()) {
		// 仮想表に対しては処理しない
		return false;
	}

	if (isTupleID()) {
		// 列がRowIDなら索引ではなくVectorファイルのフィールドを返す
		const ModVector<Field*>& vecDestination = getField(cTrans_)->getDestination(cTrans_);
		// Destinationのうち所属するファイルがVectorであるものを返り値に入れる
		if (!vecDestination.isEmpty()) {
			ModVector<Field*>::ConstIterator iterator = vecDestination.begin();
			const ModVector<Field*>::ConstIterator& last = vecDestination.end();
			do {
				if ((*iterator)->getStatus() == Field::Status::Persistent
					&& (*iterator)->getFile(cTrans_)->getCategory() == File::Category::Vector) {
					vecField_.pushBack(*iterator);
				}
			} while (++iterator != last);
		}
	} else if (getScope() == Scope::Meta) {
		// system table's index file
		const ModVector<Field*>& vecDestination = getField(cTrans_)->getDestination(cTrans_);
		// Destinationのうち所属するファイルがBtreeかVectorであるものを返り値に入れる
		if (!vecDestination.isEmpty()) {
			ModVector<Field*>::ConstIterator iterator = vecDestination.begin();
			const ModVector<Field*>::ConstIterator& last = vecDestination.end();
			do {
				if ((*iterator)->getStatus() == Field::Status::Persistent
					&& ((*iterator)->getFile(cTrans_)->getCategory() == File::Category::Vector
						|| (*iterator)->getFile(cTrans_)->getCategory() == File::Category::Btree)) {
					vecField_.pushBack(*iterator);
				}
			} while (++iterator != last);
		}
	} else {
		// 列がRowIDでなければ索引のキーに対応するものを結果とする
		const ModVector<Key*>& vecKey = getKey(cTrans_);
		if (!vecKey.isEmpty()) {
			ModSize n = vecKey.getSize();
			vecField_.reserve(n);
			ModVector<Key*>::ConstIterator iterator = vecKey.begin();
			do {
				if ((*iterator)->getStatus() == Field::Status::Persistent
					&& !(*iterator)->getIndex(cTrans_)->isOffline())
					vecField_.pushBack((*iterator)->getField(cTrans_));
				++iterator;
			} while (--n > 0);
		}
	}
	// 返り値が空かどうかをreturn valueとする
	return !vecField_.isEmpty();
}

//	FUNCTION public
//	Schema::Column::isTupleID -- タプル ID を格納する列か調べる
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		true
//			タプル ID を格納する列である
//		false
//			タプル ID を格納する列でない
//
//	EXCEPTIONS
//		なし

bool
Column::
isTupleID() const
{
	return _category == Category::TupleID;
}

// FUNCTION public
//	Schema::Column::isIdentity -- Identity Column を格納する列か
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
Column::
isIdentity() const
{
	return getDefault().isIdentity();
}

// FUNCTION public
//	Schema::Column::isCurrentTimestamp -- Defaultがcurrent timestampである列か
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
Column::
isCurrentTimestamp() const
{
	return getDefault().isFunction()
		&& (getDefault().getFunction() == Statement::ValueExpression::func_Current_Timestamp);
}

// FUNCTION public
//	Schema::Column::getSequence -- 値を取得するSequenceオブジェクトを得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Sequence&
//
// EXCEPTIONS

Sequence&
Column::
getSequence(Trans::Transaction& cTrans_)
{
	if (!m_pSequence) {
		// Get the Table object out side of the critical section
		Table* pTable = getTable(cTrans_);

		// Check the member again in the critical section
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		if (!m_pSequence) {

			// Create path name for a sequence file

			Os::Path cPath(pTable->getPath(cTrans_));

			if (isTupleID()) {
				// Add file name
				cPath.addPart(PathParts::Sequence::TupleID);

				// Create a Sequence object using the path name
				m_pSequence = new Sequence(cPath, getDatabaseID(), getParentID(), getID(),
										   Sequence::Unsigned::MaxValue,
										   !getDatabase(cTrans_)->isUnmounted(),
										   getScope(),
										   getDatabase(cTrans_)->isReadOnly());

			} else if (isIdentity()) {
				// Add file name
				cPath.addPart(PathParts::Sequence::Identity);

				// Create a Sequence object using the path name
				m_pSequence = new Sequence(cPath, getDatabaseID(), getParentID(), getID(),
										   getDefault(),
										   !getDatabase(cTrans_)->isUnmounted(),
										   getScope(),
										   getDatabase(cTrans_)->isReadOnly());
			} else {
				_SYDNEY_THROW0(Exception::NotSupported);
			}
		}
	}
	; _SYDNEY_ASSERT(m_pSequence);
	return *m_pSequence;
}

// FUNCTION public
//	Schema::Column::moveSequence -- シーケンスファイルを移動する
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Os::Path& cPrevPath_
//	const Os::Path& cPostPath_
//	bool bUndo_
//	bool bRecovery_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Column::
moveSequence(Trans::Transaction& cTrans_,
			 const Os::Path& cPrevPath_,
			 const Os::Path& cPostPath_,
			 bool bUndo_, bool bRecovery_)
{
	// ファイル名を追加する
	Os::Path cPrevPath(cPrevPath_);
	Os::Path cPostPath(cPostPath_);

	if (isTupleID()) {
		cPrevPath.addPart(PathParts::Sequence::TupleID);
		cPostPath.addPart(PathParts::Sequence::TupleID);
	} else if (isIdentity()) {
		cPrevPath.addPart(PathParts::Sequence::Identity);
		cPostPath.addPart(PathParts::Sequence::Identity);
	} else {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	enum {
		None,
		Created,
		Moved,
		Removed,
		ValueNum
	} eStatus = None;

	Utility::File::AutoRmDir cAutoRmDir;
	cAutoRmDir.setDir(cPostPath);

	try {
		eStatus = Created;
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "Directory");
										// FakeErrorのキーがSchema::Tableになっているのは
										// 古いテスト環境とあわせるため
		// 移動する
		getSequence(cTrans_).move(cTrans_, cPostPath, bUndo_, bRecovery_);
		// 移動したら移動後のパスを自動で破棄できない
		cAutoRmDir.disable();
		eStatus = Moved;
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "Moved");

		// 移動前のディレクトリーを破棄する
		Utility::File::rmAll(cPrevPath);
		eStatus = Removed;
		SCHEMA_FAKE_ERROR("Schema::Table", "MoveTupleSequence", "Removed");

	} catch (...) {

		_BEGIN_REORGANIZE_RECOVERY(getDatabaseID());

		switch (eStatus) {
		case Removed:
		case Moved:
			getSequence(cTrans_).move(cTrans_, cPrevPath, true);
			// 移動したら再び移動後のパスを自動で破棄できる
			cAutoRmDir.enable();
			// thru.
		case Created:
		default:
			break;
		}

		_END_REORGANIZE_RECOVERY(getDatabaseID());

		_SYDNEY_RETHROW;
	}
}

// FUNCTION public
//	Schema::Column::clearSequence -- Sequenceオブジェクトを破棄する
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
Column::
clearSequence()
{
	delete m_pSequence, m_pSequence = 0;
}

//	FUNCTION public
//	Schema::Column::getHint -- 列のヒントを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0以外
//			列のヒント
//		0
//			列にはヒントが設定されていない
//
//	EXCEPTIONS
//		なし

const Hint*
Column::
getHint() const
{
	return m_pHint;
}

//	FUNCTION public
//	Schema::Column::getTable -- 列が存在する表を表すクラスを得る
//
//	NOTES
//		生成前、中の列や、排他制御がうまく行われていない場合を除けば、
//		列が存在する索引は必ず存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた表を格納する領域の先頭アドレス
//		0
//			列が存在する表は存在しない
//
//	EXCEPTIONS

Table*
Column::getTable(Trans::Transaction& cTrans_) const
{
	return m_pTable ? m_pTable
		: (m_pTable =
		   Table::get(getParentID(), getDatabase(cTrans_), cTrans_, true /* internal */));
}

//	FUNCTION public
//	Schema::Column::getField -- 列の値を格納するフィールドを得る
//
//	NOTES
//		生成前、中の列や、排他制御がうまく行われていない場合を除けば、
//		列の値を格納するフィールドは必ずひとつ存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			列の値を格納するフィールドが存在しない
//
//	EXCEPTIONS
//		なし

Field*
Column::getField(Trans::Transaction& cTrans_) const
{
	if (_category != Category::Constant) {
		if (!_field) {
			AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
			// 書き込みロックの中でもう一度調べる

			return (!_field) ?
				_field = Field::get(_fieldID, getDatabase(cTrans_), cTrans_)
				: _field;
		}
	}
	return _field;
}

// FUNCTION public
//	Schema::Column::getField -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	File* pFile_
//	
// RETURN
//	Field*
//
// EXCEPTIONS

Field*
Column::
getField(Trans::Transaction& cTrans_,
		 File* pFile_) const
{
	Field* pField = getField(cTrans_);
	if (pField->getFile(cTrans_) == pFile_) {
		return pField;
	}
	const ModVector<Field*>& vecDestination = getField(cTrans_)->getDestination(cTrans_);
	// Destinationのうち所属するファイルがVectorであるものを返り値に入れる
	if (!vecDestination.isEmpty()) {
		ModVector<Field*>::ConstIterator iterator = vecDestination.begin();
		const ModVector<Field*>::ConstIterator& last = vecDestination.end();
		do {
			if ((*iterator)->getFile(cTrans_) == pFile_) {
				return *iterator;
			}
		} while (++iterator != last);
	}
	return 0;
}

//	FUNCTION public
//	Schema::Column::getFieldID -- 列の値を格納するフィールドのIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		フィールドのID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Column::getFieldID() const
{
	return _fieldID;
}

//	FUNCTION public
//	Schema::Column::setField -- 列の値を格納するフィールドを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field&		field
//			設定する列の値を格納するフィールド
//
//	RETURN
//		設定したフィールド
//
//	EXCEPTIONS
//		なし

const Field&
Column::
setField(Field& field)
{
	; _SYDNEY_ASSERT(_category != Category::Constant);
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	_fieldID = field.getID();
	return *(_field = &field);
}

//	FUNCTION public
//	Schema::Column::setFieldID -- 列の値を格納するフィールドのIDを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value id_
//			フィールドのID
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

inline // 内部でしか使わない
void
Column::setFieldID(ID::Value id_)
{
	_fieldID = id_;
	_field = 0;
}

//	FUNCTION public
//	Schema::Column::getKey -- 列に対する索引のキーを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction&			cTrans_
//			操作をしようとしているトランザクション記述子
//
//	RETURN
//		列に対する索引のキーをひとつづつ要素とするベクター
//		列に対して索引のキーがひとつもないとき、空のベクター
//
//	EXCEPTIONS

const ModVector<Key*>&
Column::getKey(Trans::Transaction& cTrans_) const
{
	AutoRWLock l(getRWLock());
	if (!_keys) {

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!_keys) {

			// この列に対するキーは登録されていないことにする

			const_cast<Column*>(this)->resetKey();
			; _SYDNEY_ASSERT(_keys);

			// 列が属する表についているすべての索引について
			// それに属するキーのうち、
			// この列についているものをすべて追加する

			; _SYDNEY_ASSERT(getTable(cTrans_));

			const ModVector<Index*>& vecIndices =
				getTable(cTrans_)->getIndex(cTrans_);
			ModSize n = vecIndices.getSize();
		
			for (ModSize i = 0; i < n; i++) {
				ModVector<Key*> vecKeys = vecIndices[i]->getKey(*this, cTrans_);
				if (vecKeys.getSize())
					_keys->insert(_keys->end(), vecKeys.begin(), vecKeys.end());
			}
		}
	}
	return *_keys;
}

#ifdef OBSOLETE // _keysへの挿入にはModVector::insertを使っているのでaddKeyは使用しない

//	FUNCTION public
//	Schema::Column::addKey --
//		列を表すクラスに、その列に対するキーを表すクラスを登録する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Key&		key
//			登録するキーを表すクラス
//			これは、呼び出し中に更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		追加したキーを表すクラス
//
//	EXCEPTIONS


Key&
Column::
addKey(Key& key)
{
	; _SYDNEY_ASSERT(_category != Category::Constant);

	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (!_keys) {

		// 列に対するキーを表すクラスを登録する
		// ベクターがないので、生成しておく

		resetKey();
		; _SYDNEY_ASSERT(_keys);
	}
	_keys->pushBack(&key);

	return key;
}
#endif

//	FUNCTION public
//	Schema::Column::resetKey --
//		列に対するキーを表すクラスが登録されていないことにする
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
Column::
resetKey()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_keys)

		// 列に対するキーを表すクラスを登録するベクターを空にする

		_keys->clear();
	else {
		// 列に対するキーを表すクラスを登録するベクターを生成する

		_keys = new ModVector<Key*>();
		; _SYDNEY_ASSERT(_keys);
	}
}

//	FUNCTION public
//	Schema::Column::clearKey --
//		列に対するキーを表すクラスを管理するためのベクターを破棄する
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
Column::
clearKey()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 列に対するキーを表すクラスを登録するベクターを破棄する

	delete _keys, _keys = 0;
}

//	FUNCTION public
//	Schema::Column::setHint -- 列のヒントを設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Hint* pHint_
//			設定する列のヒント
//
//	RETURN
//			なし
//
//	EXCEPTIONS
//		なし

void
Schema::Column::setHint(Hint* pHint_)
{
	if (m_pHint)
		delete m_pHint, m_pHint = 0;

	m_pHint = pHint_;

	// NonTruncateならCollationにNoPadを設定する
	if (m_pHint->getCategory() & Hint::Category::NonTruncate) {
		_type.setCollation(Common::Collation::Type::NoPad);
	}
}

// FUNCTION public
//	Schema::Column::setMetaData -- ColumnMetaDataをセットする
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Common::ColumnMetaData& cMetaData_
//	
// RETURN
//	なし
//
// EXCEPTIONS

void
Column::
setMetaData(Trans::Transaction& cTrans_, Common::ColumnMetaData& cMetaData_) const
{
	const Table* pTable = getTable(cTrans_);
	; _SYDNEY_ASSERT(pTable);
	const Database* pDatabase = pTable->getDatabase(cTrans_);
	; _SYDNEY_ASSERT(pDatabase);

	cMetaData_.setColumnName(getName());
	cMetaData_.setTableName(pTable->getName());
	cMetaData_.setDatabaseName(pDatabase->getName());

	// RowIdの列に対して設定する
	// ★注意★
	// 今のところ自動採番はRowIdだけ
	if (isTupleID()) {
		cMetaData_.setAutoIncrement();
		cMetaData_.setReadOnly();
		cMetaData_.setUnique();
	}

	// 符号なしか
	if (isTupleID() || getType().getType() == Common::SQLData::Type::UInt)
		cMetaData_.setUnsigned();

	// 検索不可か
	// ★注意★
	// 今のところ検索不可の列はない

	// 読み込み専用か
	const Field* pField = getField(cTrans_);
	if (pField == 0 || !pField->isPutable())
		cMetaData_.setReadOnly();

	// Nullをセットできないか
	if (!isNullable())
		cMetaData_.setNotNullable();

	// Uniqueか
	if (isUnique(cTrans_))
		cMetaData_.setUnique();
	// 大文字小文字が区別されないか
	if (isCaseInsensitive(cTrans_))
		cMetaData_.setCaseInsensitive();
}

//	FUNCTION public
//	Schema::Column::serialize -- 
//		列を表すクラスのシリアライザー
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
Column::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// 表の先頭から何番目か

		archiver << _position;

		// 列の種別
		{
		int tmp = _category;
		archiver << tmp;
		}
		// 列の型

		archiver << _type;

		// 列のデフォルト指定

		archiver << _default;

		// nullの可不可
		archiver << m_bNullable;

		// 列のヒント

		int hasHint = (m_pHint) ? 1 : 0;
		archiver << hasHint;
		if (hasHint) archiver << *m_pHint;

		// 列の値を格納するフィールドのスキーマオブジェクト ID

		archiver << _fieldID;

		// 列に関係するキーがあるか
		bool hasKey = (!_keys || _keys->getSize() > 0);
		archiver << hasKey;

	} else {

		// メンバーをすべて初期化しておく

		clear();

		// 表の先頭から何番目か

		archiver >> _position;

		// 列の種別
		{
		int tmp;
		archiver >> tmp;
		_category = static_cast<Category::Value>(tmp);
		}
		// 列の型

		archiver >> _type;

		// 列のデフォルト指定

		archiver >> _default;

		// nullの可不可
		archiver >> m_bNullable;

		// 列のヒント

		int hasHint;
		archiver >> hasHint;
		if (hasHint) {
			Hint cHint;
			archiver >> cHint;
			m_pHint = new Hint(cHint);

			// NonTruncateならCollationにNoPadを設定する
			if (m_pHint->getCategory() & Hint::Category::NonTruncate) {
				_type.setCollation(Common::Collation::Type::NoPad);
			}
		}

		// 列の値を格納するフィールドのスキーマオブジェクト ID

		archiver >> _fieldID;

		// 列に関係するキーがあるか
		bool hasKey;
		archiver >> hasKey;
		if (!hasKey)
			// _keysを空にしておく
			resetKey();
	}
}

//	FUNCTION public
//	Schema::Column::getClassID -- このクラスのクラス ID を得る
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
Column::
getClassID() const
{
	return Externalizable::Category::Column +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Column::isLessThan -- ソートに使う比較関数
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
Column::
isLessThan(const Object& cOther_) const
{
	// 列は位置の順に並べる
	return (Object::getCategory() < cOther_.getCategory()
			|| (Object::getCategory() == cOther_.getCategory()
				&& (getParentID() < cOther_.getParentID()
					|| (getParentID() == cOther_.getParentID()
						&& getPosition() < _SYDNEY_DYNAMIC_CAST(const Column&, cOther_).getPosition()))));
}

//	FUNCTION public
//	Schema::Column::clear -- 列を表すクラスのメンバーをすべて初期化する
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
Column::
clear()
{
	_position = 0;
	_category = Category::Unknown;

	_type.clear();

	_default.clear();
	_field = 0;
	_fieldID = Object::ID::Invalid;

	m_pTable = 0;

	destruct();
}

//	FUNCTION public
//	Schema::Column::makeLogData --
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
Column::
makeLogData(Common::DataArrayData& cLogData_) const
{
	//	列定義のログデータ
	//		0．ID
	//		1．名前
	//		2．種別
	//		3．デフォルト値
	//		4．フラグ
	//		5．データ型
	//		6．precision
	//		7．scale
	//		8．character set
	//		9．ヒント

	cLogData_.reserve(10); // 以下のコードが変わったらこの数値も変える
	cLogData_.pushBack(packMetaField(Meta::Column::ID));
	cLogData_.pushBack(packMetaField(Meta::Column::Name));
	cLogData_.pushBack(packMetaField(Meta::Column::Category));
	cLogData_.pushBack(packMetaField(Meta::Column::Default));
	cLogData_.pushBack(packMetaField(Meta::Column::Flag));
	cLogData_.pushBack(packMetaField(Meta::Column::Type));
	cLogData_.pushBack(packMetaField(Meta::Column::Precision));
	cLogData_.pushBack(packMetaField(Meta::Column::Scale));
	cLogData_.pushBack(packMetaField(Meta::Column::CharacterSet));
	cLogData_.pushBack(packMetaField(Meta::Column::Hint));
}

// FUNCTION public
//	Schema::Column::investigateIndex -- 列についているIndexを調べないと分からない特性を調べる
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

void
Column::
investigateIndex(Trans::Transaction& cTrans_) const
{
	// ロックが競合しないように排他の区間を短くする
	int iFlag;
	{
		AutoRWLock l(getRWLock());
		iFlag = m_iFlag;
	}

	if (iFlag < 0) {
		// 未調査なので調べる
		iFlag = 0;

		// Uniqueか
		// -> ColumnにキーがひとつのBtree索引がついていて、その索引がUniqueならUnique
		// 大文字小文字が区別されないか
		// -> ColumnにBtree索引がついていて、その索引にヒントがついていたらFileIDを調べる

		const ModVector<Key*> vecKey = getKey(cTrans_);
		if (!vecKey.isEmpty()) {
			ModVector<Key*>::ConstIterator iterator = vecKey.begin();
			const ModVector<Key*>::ConstIterator& last = vecKey.end();
			do {
				const Index* pIndex = (*iterator)->getIndex(cTrans_);
				if (pIndex->isUnique() && pIndex->getKey(cTrans_).getSize() == 1)
					iFlag += _Flag::Unique;
				if (pIndex->getCategory() == Index::Category::Normal) {
					// B+木索引がついている
					const File* pFile = pIndex->getFile(cTrans_);
					if (pFile->getHint() != 0) {
						const LogicalFile::FileID& cFileID = pFile->getFileID();
						// バージョンが0でなく、Normalizedが設定されていたら大文字小文字の区別なし
						if (cFileID.getInteger(_SYDNEY_FILE_PARAMETER_KEY(FileCommon::FileOption::Version::Key)) > 0
							&& cFileID.getBoolean(_SYDNEY_FILE_PARAMETER_KEY(Btree2::FileID::KeyID::Normalized))) {
							iFlag += _Flag::CaseInsensitive;
						}
					}
				}
			} while (++iterator != last);
		}
		{
			AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
			m_iFlag = iFlag;
		}
	}
}

// 調査結果をクリアする
void
Column::
clearFlag()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	m_iFlag = -1;
}

////////////////////////////////////////////////////////////
// メタデータベースのための定義
////////////////////////////////////////////////////////////

// メタデータベースにおける「列」表の構造は以下のとおり
// create table Column_DBXXXX (
//		ID			id,
//		parent		id,
//		name		nvarchar,
//		category	int,
//		position	int,
//		field		id,
//		default		<data>, -- Common::Data
//		flag		int,
//		type		int,
//		length		int,
//		precision	int,
//		scale		int,
//		characterSet	int,
//		hint		<data>, -- Common::Data
//		time		timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Column>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Column>(Meta::MemberType::_type_, &Column::_get_, &Column::_set_)

	Meta::Definition<Column> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(ParentID),		// ParentID
		_DEFINE0(Name),			// Name
		_DEFINE0(Integer),		// Category,
		_DEFINE0(Integer),		// Position,
		_DEFINE2(ID, getFieldID, setFieldID),// FieldID
		_DEFINE0(Binary),		// Default
		_DEFINE0(Integer),		// Flag
		_DEFINE0(Binary),		// Type
		_DEFINE0(Integer),		// Precision
		_DEFINE0(Integer),		// Scale
		_DEFINE0(Integer),		// CharacterSet
		_DEFINE0(Binary),		// Hint,
		_DEFINE0(Timestamp),	// Timestamp
		_DEFINE0(Unknown),	// MemberNum
		_DEFINE0(StringArray),	// MetaData
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Column::getMetaFieldNumber --
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
Column::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Column::MemberNum);
}

//	FUNCTION public
//	Schema::Column::getMetaFieldDefinition --
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
Column::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Column::packMetaField --
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
Column::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Column>& cDef = _vecDefinition[iMemberID_];

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
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Column::unpackMetaField --
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
Column::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Column>& cDef = _vecDefinition[iMemberID_];

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
	default:
		break;
	}
	return false;
}

Common::Data::Pointer
Column::
packIntegerMetaField(int iMemberID_) const
{
	switch (iMemberID_) {
	case Meta::Column::Category:
		{
			return pack(static_cast<int>(getCategory()));
		}
	case Meta::Column::Position:
		{
			return pack(static_cast<int>(getPosition()));
		}
	case Meta::Column::Flag:
		{
			// フラグ
			//		NULLの可不可は最下位ビットで、
			//		none/variable/fixed/unlimitedは残りのビットで表す
			//
			//		0.. null可、  none
			//		1.. null不可、none
			//		2.. null可、  fixed
			//		3.. null不可、fixed
			//		4.. null可、  variable
			//		5.. null不可、variable
			//		6.. null可、  unlimited
			//		7.. null不可、unlimited

			int flag = static_cast<int>(_type.getFlag()) * 2;
			if (!m_bNullable) ++flag;
			return pack(flag);
		}
	case Meta::Column::Precision:
	case Meta::Column::Scale:
	case Meta::Column::CharacterSet:
		{
			//		非使用:nullを入れる
			return Common::NullData::getInstance();
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

Common::Data::Pointer
Column::
packBinaryMetaField(int iMemberID_) const
{
	Utility::BinaryData& cArchiver = getArchiver();
	switch (iMemberID_) {
	case Meta::Column::Default:
		{
			const Default& cDefault = getDefault();
			return cArchiver.put(&cDefault);
		}
	case Meta::Column::Type:
		{
			return cArchiver.put(&_type);
		}
	case Meta::Column::Hint:
		{
			return cArchiver.put(getHint());
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

bool
Column::
unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		// Nullが許されるのはPrecision、Scale、CharacterSet
		return (iMemberID_ == Meta::Column::Precision
				|| iMemberID_ == Meta::Column::Scale
				|| iMemberID_ == Meta::Column::CharacterSet);
	} else {
		int value;
		if (unpack(pData_, value)) {
			switch (iMemberID_) {
			case Meta::Column::Category:
				{
					if (value >= 0 && value < Category::ValueNum) {
						_category = static_cast<Category::Value>(value);
						return true;
					}
					break;
				}
			case Meta::Column::Position:
				{
					if (value >= 0) {
						_position = value;
						return true;
					}
					break;
				}
			case Meta::Column::Flag:
				{
					// フラグ
					//		NULLの可不可は最下位ビットで、
					//		none/variable/fixed/unlimitedは残りのビットで表す
					//
					//		0.. null可、  none
					//		1.. null不可、none
					//		2.. null可、  fixed(old)
					//		3.. null不可、fixed(old)
					//		4.. null可、  variable(old)
					//		5.. null不可、variable(old)
					//		6.. null可、  unlimited
					//		7.. null不可、unlimited
					//		8.. null可、  fixed
					//		9.. null不可、fixed
					//	   10.. null可、  variable
					//	   11.. null不可、variable

					if (value >= 0 && value < static_cast<int>(Common::SQLData::Flag::ValueNum) * 2) {
						_type.setFlag(static_cast<Common::SQLData::Flag::Value>(value / 2));
						m_bNullable = (value % 2 == 0);
						return true;
					}
					break;
				}
			case Meta::Column::Precision:
			case Meta::Column::Scale:
			case Meta::Column::CharacterSet:
				{
					//		非使用:何もしない
					return true;
				}
			default:
				break;
			}
		}
	}
	return false;
}

bool
Column::
unpackBinaryMetaField(const Common::Data* pData_, int iMemberID_)
{
	if (pData_ && pData_->isNull()) {
		// Nullが許されるのはHintのみ
		return (iMemberID_ == Meta::Column::Hint);

	} else if (pData_ && pData_->getType() == Common::DataType::Binary) {
		const Common::BinaryData* pBinary =
			_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);

		Utility::BinaryData& cArchiver = getArchiver();

		switch (iMemberID_) {
		case Meta::Column::Default:
			{
				ModAutoPointer<Default> pData =
					dynamic_cast<Default*>(cArchiver.get(pBinary));
				if (pData.get())
					_default = *pData;
				return true;
			}
		case Meta::Column::Type:
			{
				ModAutoPointer<Common::Externalizable> pData = cArchiver.get(pBinary);

				if (Common::SQLData* pType = dynamic_cast<Common::SQLData*>(pData.get())) {
					_type = *pType;
					return true;
				} else if (DataTypeOld* pTypeOld = dynamic_cast<DataTypeOld*>(pData.get())) {
					_type = *pTypeOld;
					return true;
				}
				break;
			}
		case Meta::Column::Hint:
			{
				ModAutoPointer<Hint> pData =
					dynamic_cast<Hint*>(cArchiver.get(pBinary));
				if (pData.get()) {
					m_pHint = pData.release();
					// NonTruncateならCollationにNoPadを設定する
					if (m_pHint->getCategory() & Hint::Category::NonTruncate) {
						_type.setCollation(Common::Collation::Type::NoPad);
					}
				}
				return true;
			}
		default:
			break;
		}
	}
	return false;
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
