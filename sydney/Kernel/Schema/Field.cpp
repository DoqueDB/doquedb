// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Field.cpp -- フィールド関連の関数定義
// 
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#include "Schema/Field.h"
#include "Schema/AutoRWLock.h"
#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/Default.h"
#include "Schema/File.h"
#include "Schema/Index.h"
#include "Schema/Key.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Meta.h"
#include "Schema/NameParts.h"
#include "Schema/Object.h"
#include "Schema/ObjectTemplate.h"
#include "Schema/Parameter.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/Table.h"
#include "Schema/TupleID.h"
#include "Schema/Utility.h"

#include "Common/Assert.h"
#include "Common/BinaryData.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/DataType.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"
#include "Common/SQLData.h"

#include "Exception/MetaDatabaseCorrupted.h"

#include "FileCommon/FileOption.h"

#include "Trans/Transaction.h"

#include "ModAutoPointer.h"
#include "ModVector.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{

// フィールドの種別から、そのフィールドの名前の形式を選択するための配列

const char* const namePrefix[] =
{
	"",
	NameParts::Field::ObjectID,
	NameParts::Field::Key,
	NameParts::Field::Data,
	"",
	""
};

// 関数の種別から、そのフィールドの名前の形式を選択するための配列

#define NAME(x) NameParts::Field::x
#define TYPE(x) Common::DataType::x

const struct _FunctionInfo_ {
	const char* m_pszNamePrefix;
	Common::DataType::Type m_eType;
	Common::DataType::Type m_eElementType;
	bool m_bNullable;
} _functionTable[] =
{
	{"",				TYPE(Undefined),	TYPE(Undefined),		true},
	{NAME(Score),		TYPE(Double),		TYPE(Undefined),		false},
	{NAME(Section),		TYPE(Array),		TYPE(UnsignedInteger),	false},
	{NAME(Word),		TYPE(Word),			TYPE(Undefined),		false},
	{NAME(WordDf),		TYPE(Integer),		TYPE(Undefined),		false},
	{NAME(WordScale),	TYPE(Double),		TYPE(Undefined),		false},
	{NAME(AverageLength), TYPE(Double),		TYPE(Undefined),		true},
	{NAME(AverageCharLength), TYPE(Double),	TYPE(Undefined),		true},
	{NAME(AverageWordCount), TYPE(Double),	TYPE(Undefined),		true},
	{NAME(Tf),			TYPE(Array),		TYPE(UnsignedInteger),	false},
	{NAME(Count),		TYPE(UnsignedInteger),TYPE(Undefined),		false},
	{NAME(ClusterId),	TYPE(Integer),		TYPE(Undefined),		false},
	{NAME(ClusterWord),	TYPE(Array),		TYPE(Word),				false},
	{NAME(Kwic),		TYPE(Integer),		TYPE(Undefined),		false},
	{NAME(Existence),	TYPE(Array),		TYPE(Integer),			false},
	{NAME(MinKey),		TYPE(Undefined),	TYPE(Undefined),		true},
	{NAME(MaxKey),		TYPE(Undefined),	TYPE(Undefined),		true},
	{NAME(NeighborId),	TYPE(Integer),		TYPE(Undefined),		false},
	{NAME(NeighborDistance), TYPE(Double),	TYPE(Undefined),		false},
	{"",				TYPE(Undefined),	TYPE(Undefined),		true},
};

#undef NAME
#undef TYPE

// スキーマからファイルドライバーに渡すヒントの定数
namespace _Hint
{
	const ModUnicodeString _notNull("not null");
	const ModUnicodeString _fixed("fixed");
} // namespace _Hint

Common::StringData::EncodingForm::Value
convertTypeToEncodingForm(const Column::DataType& type)
{
	Common::StringData::EncodingForm::Value encodingForm =
		Common::StringData::EncodingForm::Unknown;

	switch (type.getType()) {
	case Common::SQLData::Type::Char:
		encodingForm = Common::StringData::EncodingForm::UTF8;		break;
	case Common::SQLData::Type::NChar:
	case Common::SQLData::Type::UniqueIdentifier:
	case Common::SQLData::Type::NText:
	case Common::SQLData::Type::Fulltext:
	case Common::SQLData::Type::NCLOB:
		encodingForm = Common::StringData::EncodingForm::UCS2;		break;
	}
	return encodingForm;
}

} // namespace

//	FUNCTION public
//	Schema::Field::Field -- フィールドを表すクラスのデフォルトコンストラクター
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

Field::
Field()
	: Object(Object::Category::Field),
	  _position(0),
	  _category(Category::Unknown),
	  _function(Function::Undefined),
	  _permission(Permission::None),
	  _type(Common::DataType::Undefined),
	  _elementType(Common::DataType::Undefined),
	  _length(0),
	  _elementLength(0),
	  _default(),
	  _source(0),
	  _sourceID(Object::ID::Invalid),
	  _destinations(0),
	  _column(0),
	  _columnID(Object::ID::Invalid),
	  _key(0),
	  _keyID(Object::ID::Invalid),
	  m_iIsFirstKey(-1),
	  m_iScale(0),
	  m_bIsFixed(false),
	  m_pFile(0)
{ }

//	FUNCTION public
//	Schema::Field::Field --
//		ある列の値を格納するわけでもなく、
//		あるフィールドの複製先でもないフィールドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field::Type type
//			フィールドの型
//		Schema::Field::Length length
//			フィールドのバイト長
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし
	  
Field::
Field(const File& file, Field::Position position,
	  Field::Category::Value category, Field::Permission::Value permission,
	  Field::Type type, Field::Length length)
	: Object(Object::Category::Field, file.getScope(), file.getStatus(),
			 ID::Invalid, file.getID(), file.getDatabaseID()),
	  _position(position),
	  _category(category),
	  _function(Function::Undefined),
	  _permission(permission),
	  _type(type),
	  _elementType(Common::DataType::Undefined),
	  _length(length),
	  _elementLength(0),
	  _default(),
	  _source(0),
	  _sourceID(Object::ID::Invalid),
	  _destinations(0),
	  _column(0),
	  _columnID(Object::ID::Invalid),
	  _key(0),
	  _keyID(Object::ID::Invalid),
	  m_iIsFirstKey(-1),
	  m_iScale(0),
	  m_bIsFixed(false),
	  m_pFile(const_cast<File*>(&file))
{ }

//	FUNCTION public
//	Schema::Field::Field --
//		ある列の値を格納せずに、
//		あるフィールドの派生先であるフィールドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field&		source
//			フィールドの派生元であるフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Field::
Field(const File& file, Field::Position position,
	  Field::Category::Value category, Field::Permission::Value permission,
	  Field& source)
	: Object(Object::Category::Field, file.getScope(), file.getStatus(),
			 Object::ID::Invalid, file.getID(), file.getDatabaseID()),
	  _position(position),
	  _category(category),
	  _function(Function::Undefined),
	  _permission(permission),
	  _type(Common::DataType::Undefined),
	  _elementType(Common::DataType::Undefined),
	  _length(0),
	  _elementLength(0),
	  _default(),
	  _source(&source),
	  _sourceID(source.getID()),
	  _destinations(0),
	  _column(0),
	  _columnID(Object::ID::Invalid),
	  _key(0),
	  _keyID(Object::ID::Invalid),
	  m_iIsFirstKey(-1),
	  m_iScale(0),
	  m_bIsFixed(false),
	  m_pFile(const_cast<File*>(&file))
{ }

//	FUNCTION public
//	Schema::Field::Field --
//		ある列の値を格納し、
//		あるフィールドの派生先でないフィールドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Column&		column
//			フィールドを使用する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Field::
Field(const File& file, Field::Position position,
	  Field::Category::Value category, Field::Permission::Value permission,
	  Column& column)
	: Object(Object::Category::Field, file.getScope(), file.getStatus(),
			 ID::Invalid, file.getID(), file.getDatabaseID()),
	  _position(position),
	  _category(category),
	  _function(Function::Undefined),
	  _permission(permission),
	  _type(Common::DataType::Undefined),
	  _elementType(Common::DataType::Undefined),
	  _length(0),
	  _elementLength(0),
	  _default(),
	  _source(0),
	  _sourceID(Object::ID::Invalid),
	  _destinations(0),
	  _column(&column),
	  _columnID(column.getID()),
	  _key(0),
	  _keyID(Object::ID::Invalid),
	  m_iIsFirstKey(-1),
	  m_iScale(0),
	  m_bIsFixed(false),
	  m_pFile(const_cast<File*>(&file))
{
	setColumnAttribute(column);
}

//	FUNCTION public
//	Schema::Field::Field --
//		ある列の値を格納し、
//		あるフィールドの派生先であるフィールドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field&		source
//			フィールドの派生元であるフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Column&		column
//			フィールドの派生元であるフィールドが値を保持する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Field::
Field(const File& file, Field::Position position,
	  Field::Category::Value category, Field::Permission::Value permission,
	  Field& source, Column& column)
	: Object(Object::Category::Field, file.getScope(), file.getStatus(),
			 ID::Invalid, file.getID(), file.getDatabaseID()),
	  _position(position),
	  _category(category),
	  _function(Function::Undefined),
	  _permission(permission),
	  _type(Common::DataType::Undefined),
	  _elementType(Common::DataType::Undefined),
	  _length(0),
	  _elementLength(0),
	  _default(),
	  _source(&source),
	  _sourceID(source.getID()),
	  _destinations(0),
	  _column(0),
	  _columnID(ID::Invalid),
	  _key(0),
	  _keyID(Object::ID::Invalid),
	  m_iIsFirstKey(-1),
	  m_iScale(0),
	  m_bIsFixed(false),
	  m_pFile(const_cast<File*>(&file))
{
	setColumnAttribute(column);
}

//	FUNCTION public
//	Schema::Field::Field --
//		あるキーの値を格納するフィールドを表すクラスのコンストラクター
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Key& cKey_
//			キーを表すクラス
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field&		source
//			フィールドの派生元であるフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Column&		column
//			フィールドの派生元であるフィールドが値を保持する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Field::
Field(const File& file, Field::Position position,
	  Key& cKey_, Field::Permission::Value permission,
	  Field& source, Column& column)
	: Object(Object::Category::Field, file.getScope(), file.getStatus(),
			 ID::Invalid, file.getID(), file.getDatabaseID()),
	  _position(position),
	  _category(Category::Key),
	  _function(Function::Undefined),
	  _permission(permission),
	  _type(Common::DataType::Undefined),
	  _elementType(Common::DataType::Undefined),
	  _length(0),
	  _elementLength(0),
	  _default(),
	  _source(&source),
	  _sourceID(source.getID()),
	  _destinations(0),
	  _column(0),
	  _columnID(ID::Invalid),
	  _key(&cKey_),
	  _keyID(cKey_.getID()),
	  m_iIsFirstKey(cKey_.getPosition() == 0 ? 1 : 0),
	  m_iScale(0),
	  m_bIsFixed(false),
	  m_pFile(const_cast<File*>(&file))
{
	setColumnAttribute(column);
}

//	FUNCTION public
//	Schema::Field::Field --
//		ある列の値を引数にした関数フィールドを表すクラスのコンストラクター
//
//	NOTES
//		関数フィールドは永続化されない
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		const Schema::Object::Name&		funcName
//			関数名
//		Schema::Column&		column
//			関数の引数に使われる列を表すクラス
//		Schema::Field::Type type
//			関数返り値の型
//		Schema::Field::Length length
//			関数返り値のバイト長
//
//	RETURN
//		なし
//
//	EXCEPTIONS
//		なし

Field::
Field(const File& file, Position position,
	  Function::Value function, Column& column)
	: Object(Object::Category::Field, Object::Scope::GlobalTemporary, file.getStatus(),
			 ID::Invalid, file.getID(), file.getDatabaseID()),
	  _position(position),
	  _category(Category::Function),
	  _function(function),
	  _permission(Permission::Getable),
	  _type(Common::DataType::Undefined),
	  _elementType(Common::DataType::Undefined),
	  _length(0),
	  _elementLength(0),
	  _default(),
	  _source(0),
	  _sourceID(ID::Invalid),
	  _destinations(0),
	  _column(&column),
	  _columnID(column.getID()),
	  _key(0),
	  _keyID(Object::ID::Invalid),
	  m_iIsFirstKey(-1),
	  m_iScale(0),
	  m_bIsFixed(false),
	  m_pFile(const_cast<File*>(&file))
{ }

// FUNCTION public
//	Schema::Field::Field -- constructor with copying original field
//
// NOTES
//
// ARGUMENTS
//	const File& file
//	Position position
//	Field& field
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Field::
Field(const File& file, Position position,
	  Field& field)
	: Object(Object::Category::Field, file.getScope(), file.getStatus(),
			 ID::Invalid, file.getID(), file.getDatabaseID()),
	  _position(position),
	  _category(field._category),
	  _function(field._function),
	  _permission(field._permission),
	  _type(field._type),
	  _elementType(field._elementType),
	  _length(field._length),
	  _elementLength(field._elementLength),
	  _default(field._default),
	  _source(field._source),
	  _sourceID(field._sourceID),
	  _destinations(0),
	  _column(field._column),
	  _columnID(field._columnID),
	  _key(0),
	  _keyID(field._keyID),
	  m_iIsFirstKey(field.m_iIsFirstKey),
	  m_iScale(field.m_iScale),
	  m_bIsFixed(field.m_bIsFixed),
	  m_pFile(const_cast<File*>(&file))
{ }

//	FUNCTION public
//	Schema::Field::~Field -- フィールドを表すクラスのデストラクター
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

Field::
~Field()
{
	destruct();
}

//	FUNCTION public
//		Schema::Field::getNewInstance -- オブジェクトを新たに取得する
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
Field*
Field::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Field> pObject = new Field;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION private
//	Schema::Field::destruct -- フィールドを表すクラスのデストラクター下位関数
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
Field::
destruct()
{
	// _defaultはObjectPointerなので0を入れればfreeされる
	_default = static_cast<Common::Data*>(0);
	clearDestination();
}

//	FUNCTION private
//	Schema::Field::setName -- フィールドの名前を設定する
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

const Object::Name&
Field::
setName(Trans::Transaction& cTrans_)
{
	if (getName().getLength() == 0) {
		const Name& cParentName =
			getColumn(cTrans_) ? getColumn(cTrans_)->getName()
			: getSource(cTrans_) ? getSource(cTrans_)->setName(cTrans_)
			: getFile(cTrans_)->getName();

		doSetName(cParentName);
	}
	return getName();
}

//	FUNCTION private
//	Schema::Field::doSetName -- フィールドの名前を設定する
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::Object::Name& cParentName_
//			名前のもとになる親オブジェクトの名前
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
doSetName(const Name& cParentName_)
{
	if (!isFunction())
		Object::setName(Name(namePrefix[getCategory()],
							 Common::LiteralCode)
						   .append(cParentName_));
	else
		Object::setName(Name(_functionTable[getFunction()].m_pszNamePrefix,
							 Common::LiteralCode)
						   .append(cParentName_));
}

//	FUNCTION public
//	Schema::Field::create --
//		ある列の値を格納するわけでもなく、
//		あるフィールドの複製先でもないフィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field::Type type
//			フィールドの型
//		Schema::Field::Length length
//			フィールドのバイト長
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer	  
Field::
create(File& file, Field::Position position,
	   Field::Category::Value category, Field::Permission::Value permission,
	   Field::Type type, Field::Length length,
	   Trans::Transaction& cTrans_,
	   ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// フィールドを表すクラスを新たに生成する
	Pointer pField = new Field(file, position, category, permission, type, length);

	; _SYDNEY_ASSERT(pField.get());

	// IDをふり、状態を変える
	pField->Object::create(cTrans_, iID_);

	// 種別からフィールド名を設定する
	pField->doSetName(file.getName());

	// ファイルにこのフィールドを表すクラスを追加する
	// ★注意★
	// キャッシュに登録するのは永続化の後

	file.addField(pField, cTrans_);

	return pField;
}

//	FUNCTION public
//	Schema::Field::create --
//		ある列の値を格納せずに、
//		あるフィールドの派生先であるフィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field&		source
//			フィールドの派生元であるフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer
Field::
create(File& file, Field::Position position,
	   Field::Category::Value category, Field::Permission::Value permission,
	   Field& source, Trans::Transaction& cTrans_,
	   ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// フィールドを表すクラスを新たに生成する
	Pointer pField = new Field(file, position, category, permission, source);

	; _SYDNEY_ASSERT(pField.get());

	// IDをふり、状態を変える
	pField->Object::create(cTrans_, iID_);

	// 種別からフィールド名を設定する
	pField->doSetName(source.getName());

	// 派生元からフィールドのデータ型を設定する
	pField->setType(source);

	// ファイルにこのフィールドを表すクラスを追加する
	// ★注意★
	// キャッシュに登録するのは永続化の後

	file.addField(pField, cTrans_);

	return pField;
}

//	FUNCTION public
//	Schema::Field::create --
//		ある列の値を格納し、
//		あるフィールドの派生先でないフィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Column&		column
//			フィールドを使用する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer
Field::
create(File& file, Field::Position position,
	   Field::Category::Value category, Field::Permission::Value permission,
	   Column& column, Trans::Transaction& cTrans_,
	   ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// フィールドを表すクラスを新たに生成する
	Pointer pField = new Field(file, position, category, permission, column);

	; _SYDNEY_ASSERT(pField.get());

	// IDをふり、状態を変える
	pField->Object::create(cTrans_, iID_);

	// 種別からフィールド名を設定する
	pField->doSetName(column.getName());

	// 使用する列からフィールドのデータ型を設定する
	pField->setType(column);

	// ファイルにこのフィールドを表すクラスを追加する
	// ★注意★
	// キャッシュに登録するのは永続化の後

	file.addField(pField, cTrans_);

	return pField;
}

//	FUNCTION public
//	Schema::Field::create --
//		ある列の値を格納し、
//		あるフィールドの派生先であるフィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field&		source
//			フィールドの派生元であるフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Column&		column
//			フィールドの派生元であるフィールドが値を保持する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer
Field::
create(File& file, Field::Position position,
	   Field::Category::Value category, Field::Permission::Value permission,
	   Field& source, Column& column, Trans::Transaction& cTrans_,
	   ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// フィールドを表すクラスを新たに生成する
	Pointer pField = new Field(file, position, category, permission, source, column);

	; _SYDNEY_ASSERT(pField.get());

	// IDをふり、状態を変える
	pField->Object::create(cTrans_, iID_);

	// 種別からフィールド名を設定する
	pField->doSetName(column.getName());

	// 派生元からフィールドのデータ型を設定する
	pField->setType(source);

	// ファイルにこのフィールドを表すクラスを追加する
	// ★注意★
	// キャッシュに登録するのは永続化の後

	file.addField(pField, cTrans_);

	return pField;
}

//	FUNCTION public
//	Schema::Field::create --
//		あるキーの値を格納するフィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Key&	cKey_
//			フィールドが格納するキーを表すクラス
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer
Field::
create(File& file, Field::Position position,
	   Key& cKey_, Field::Permission::Value permission,
	   Trans::Transaction& cTrans_,
	   ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	Column* column = cKey_.getColumn(cTrans_);
	; _SYDNEY_ASSERT(column);

	Field* source = column->getField(cTrans_);
	; _SYDNEY_ASSERT(source);

	// フィールドを表すクラスを新たに生成する
	Pointer pField = new Field(file, position, cKey_, permission, *source, *column);
	; _SYDNEY_ASSERT(pField.get());

	// IDをふり、状態を変える
	pField->Object::create(cTrans_, iID_);

	// 種別からフィールド名を設定する
	pField->doSetName(column->getName());

	// 派生元からフィールドのデータ型を設定する
	pField->setType(*source);

	// ファイルにこのフィールドを表すクラスを追加する
	// ★注意★
	// キャッシュに登録するのは永続化の後
	file.addField(pField, cTrans_);

	return pField;
}

//	FUNCTION public
//	Schema::Field::create --
//		ある列の値を引数にした関数フィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Function::Value	function
//			関数を表す列挙子の値
//		Schema::Column&		column
//			関数の引数に使われる列を表すクラス
//		Trans::Transaction& cTrans_
//			操作しているトランザクション記述子
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer
Field::
create(File& file, Position position,
	   Function::Value function, Column& column,
	   Trans::Transaction& cTrans_)
{
	// 仮想列の場合は読み込み専用トランザクションでも作成される
	//; _SYDNEY_ASSERT(cTrans_.getCategory()
	//				 != Trans::Transaction::Category::ReadOnly);

	// フィールドを表すクラスを新たに生成する
	Pointer pField = new Field(file, position, function, column);

	; _SYDNEY_ASSERT(pField.get());

	// 仮想列のIDはFunctionの値を用いた固定値
	// Fileの中で一意であればよい
	Object::ID::Value iObjectID = Object::ID::Invalid - static_cast<int>(function);
	pField->setID(iObjectID);

	// 関数名と列名でフィールド名を設定する
	pField->doSetName(column.getName());

	// 関数名からフィールドのデータ型を設定する
	pField->setType(function);
	if (pField->getType() == Common::DataType::Undefined) {
		// functionからの設定がUndefinedならcolumnと同じ型にする
		bool bDummy1;
		bool bDummy2;
		column.getFieldType(pField->_type, pField->_length, pField->m_iScale, bDummy1,
							pField->_elementType, pField->_elementLength, bDummy2);
	}

	// ファイルにこのフィールドを表すクラスを追加する
	// ★注意★
	// キャッシュに登録するのは永続化の後

	file.addField(pField, cTrans_);

	return pField;
}

//static
Field::Pointer
Field::
create(File& file, Position position, Field& field,
	   Trans::Transaction& cTrans_,
	   ObjectID::Value iID_ /* = ObjectID::Invalid */)
{
	; _SYDNEY_ASSERT(cTrans_.getCategory()
					 != Trans::Transaction::Category::ReadOnly);

	// create a new object
	Pointer pField = new Field(file, position, field);

	; _SYDNEY_ASSERT(pField.get());

	pField->Object::create(cTrans_, iID_);
	pField->Object::setName(field.getName());

	file.addField(pField, cTrans_);

	return pField;
}

//	FUNCTION public
//	Schema::Field::createSystem --
//		システム表のある列の値を格納するフィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Column&		column
//			フィールドを使用する列を表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//		Schema::Field::Type type
//			フィールドの型
//		Schema::Object::ID::Value iObjectID_
//			オブジェクトIDの値
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer
Field::
createSystem(Trans::Transaction& cTrans_,
			 File& file, Field::Position position,
			 Field::Category::Value category, Field::Permission::Value permission,
			 Column& column, Type type, ID::Value iObjectID_)
{
	// フィールドを表すクラスを新たに生成する
	Pointer pField =
		new Field(file, position, category, permission, column);

	; _SYDNEY_ASSERT(pField.get());

	// IDを設定する
	pField->setID(iObjectID_);

	// 種別からフィールド名を設定する
	pField->doSetName(column.getName());

	// フィールドのデータ型を設定する
	pField->_type = type;

	// 状態を「永続」にする
	pField->setStatus(Status::Persistent);

	return pField;
}

//	FUNCTION public
//	Schema::Field::createSystem --
//		システム表の列に対応しない値を格納するフィールドを表すクラスを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::File&		file
//			フィールドが存在するファイルを表すクラス
//		Schema::Field::Position	position
//			フィールドのファイルの先頭からの位置
//		Schema::Field::Category::Value	category
//			フィールドの種別
//		Schema::Field::Permission::Value	permission
//			フィールドに許可されている操作
//		Schema::Field::Type type
//			フィールドの型
//		Schema::Object::ID::Value iObjectID_
//			オブジェクトIDの値
//
//	RETURN
//		生成されたフィールドを表すクラス
//
//	EXCEPTIONS
//		なし

// static
Field::Pointer
Field::
createSystem(Trans::Transaction& cTrans_,
			 File& file, Field::Position position,
			 Field::Category::Value category, Field::Permission::Value permission,
			 Type type, ID::Value iObjectID_)
{
	// フィールドを表すクラスを新たに生成する
	Pointer pField =
		new Field(file, position, category, permission, type, 0);

	; _SYDNEY_ASSERT(pField.get());

	// IDを設定する
	pField->setID(iObjectID_);

	// 種別からフィールド名を設定する
	pField->doSetName(file.getName());

	// フィールドのデータ型を設定する
	pField->_type = type;

	// 状態を「永続」にする
	pField->setStatus(Status::Persistent);

	return pField;
}

// FUNCTION public
//	Schema::Field::createSystem -- create field object for system file from source
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	File& file
//	Position position
//	Category::Value category
//	Permission::Value permission
//	Field& source
//	ID::Value iObjectID_
//	
// RETURN
//	Field::Pointer
//
// EXCEPTIONS

//static
Field::Pointer
Field::
createSystem(Trans::Transaction& cTrans_,
			 File& file, Position position,
			 Category::Value category,
			 Permission::Value permission,
			 Field& source,
			 ID::Value iObjectID_)
{
	// フィールドを表すクラスを新たに生成する
	Pointer pField =
		new Field(file, position, category, permission, source);

	; _SYDNEY_ASSERT(pField.get());

	// IDを設定する
	pField->setID(iObjectID_);

	// 種別からフィールド名を設定する
	pField->doSetName(source.getName());

	// フィールドのデータ型を設定する
	pField->setType(source);

	// 状態を「永続」にする
	pField->setStatus(Status::Persistent);

	return pField;
}

//	FUNCTION public
//	Schema::Field::get --
//		あるスキーマオブジェクト ID のフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			フィールドのスキーマオブジェクト ID
//		Schema::Database* pDatabase_
//			フィールドが属するデータベース
//			値が0ならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID のフィールドは存在しない
//
//	EXCEPTIONS

// static
Field*
Field::
get(ID::Value id_, Database* pDatabase_, Trans::Transaction& cTrans_)
{
	return ObjectTemplate::get<Field, SystemTable::Field, Object::Category::Field>(id_, pDatabase_, cTrans_);
}

//	FUNCTION public
//	Schema::Field::get --
//		あるスキーマオブジェクト ID のフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::ID::Value	id
//			フィールドのスキーマオブジェクト ID
//		Schema::Object::ID::Value iDatabaseID_
//			フィールドが属するデータベースのID
//			値がID::Invalidならすべてのデータベースについて調べる
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたフィールドを格納する領域の先頭アドレス
//		0
//			指定されたスキーマオブジェクト ID のフィールドは存在しない
//
//	EXCEPTIONS

// static
Field*
Field::
get(ID::Value id_, ID::Value iDatabaseID_, Trans::Transaction& cTrans_)
{
	if (id_ == Object::ID::Invalid)
		return 0;

	return get(id_, Database::get(iDatabaseID_, cTrans_), cTrans_);
}

//	FUNCTION public
//	Schema::Field::isValid -- 陳腐化していないか
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
Field::
isValid(ID::Value iID_, ID::Value iDatabaseID_, Timestamp iTimestamp_,
		Trans::Transaction& cTrans_)
{
	Field* pField = get(iID_, iDatabaseID_, cTrans_);

	return (pField && pField->getTimestamp() == iTimestamp_);
}

//	FUNCTION public
//	Schema::Field::doBeforePersist -- 永続化前に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Field::Pointer& pField_
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
Field::
doBeforePersist(const Pointer& pField_, Status::Value eStatus_, bool bNeedToErase_, Trans::Transaction& cTrans_)
{
	// 何もしない
	;
}

//	FUNCTION public
//	Schema::Field::doAfterPersist -- 永続化後に行う処理
//
//	NOTES
//		一時オブジェクトでもこの処理は行う
//
//	ARGUMENTS
//		const Schema::Field::Pointer& pField_
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
Field::
doAfterPersist(const Pointer& pField_, Status::Value eStatus_, bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pField_.get());

	// 読み込んだフィールドが仮想列なら無視する
	if (pField_->isFunction()) {
		return;
	}

	// deleteされる可能性があるのでここでデータベースIDを取得しておく
	ID::Value dbID = pField_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// データベースのキャッシュに登録する

		if (Database* pDatabase = pField_->getDatabase(cTrans_))
			pDatabase->addCache(pField_);
		break;
	}
	case Status::Changed:
	{
		pField_->clearColumn();
		break;
	}
	case Status::CreateCanceled:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// 状態を「実際に削除された」にする

		pField_->setStatus(Status::ReallyDeleted);

#ifdef OBSOLETE // 現在はFieldだけを消去する再構成はないので以下のコードが意味を持つことはない
		if (bNeedToErase_) {
			Database* pDatabase = pField_->getDatabase(cTrans_);
			; _SYDNEY_ASSERT(pDatabase);

			pField_->reset(*pDatabase);

			// キャッシュから抹消する
			// NeedToErase==falseのときは親オブジェクトのdeleteの中で
			// キャッシュから抹消される
			pDatabase->eraseCache(pField_->getID());

			// ファイルの登録から抹消する → deleteされる

			File* pFile = pField_->getFile(cTrans_);
			; _SYDNEY_ASSERT(pFile);
			pFile->eraseField(pField_->getID());
		}
#endif
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(dbID, Object::Category::Field);
}

//	FUNCTION public
//	Schema::Field::doAfterLoad -- 読み込んだ後に行う処理
//
//	NOTES
//
//	ARGUMENTS
//		const Schema::FieldPointer& pField_
//			読み出したオブジェクト
//		Schema::File& cFile_
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
Field::
doAfterLoad(const Pointer& pField_, File& cFile_, Trans::Transaction& cTrans_)
{
	// 読み込んだフィールドが仮想列なら別途追加されるので無視する
	if (pField_->isFunction()) {
		return;
	}

	// NotAvailableでこの関数が呼ばれている場合、Drop database中なので以下の処理はしない
	if (Schema::Database::isAvailable(pField_->getDatabaseID())) {

		// 列IDが設定されているとき対応する列が存在するかを検査する
		Schema::Field* pColumnField = pField_.get();
		while (pColumnField
			   && pColumnField->getColumnID() == Schema::Object::ID::Invalid
			   && pColumnField->getSourceID() != Schema::Object::ID::Invalid)
			pColumnField = pColumnField->getSource(cTrans_);

		if (pColumnField && pColumnField->getColumnID() != Schema::Object::ID::Invalid) {
			Column* pColumn = pColumnField->getColumn(cTrans_);
			if (!pColumn) {
				_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
			} else {
				// 列があれば列から設定される属性をセットする
				pField_->setColumnAttribute(*pColumn);
			}
		}

		// キーIDが設定されているとき
		//	対応するファイルは索引を構成するファイルか
		//	対応するキーが存在するか
		// を検査する
		if (pField_->getKeyID() != Schema::Object::ID::Invalid) {
			if (Schema::Index* pIndex = cFile_.getIndex(cTrans_)) {
				if (pIndex->getKey(pField_->getKeyID(), cTrans_)) {
					goto NoProblem;
				}
			}
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}
	}
 NoProblem:

	// データベースへ読み出したフィールドを表すクラスを追加する
	// また、データベースにこのフィールドを表すクラスを
	// スキーマオブジェクトとして管理させる
	cFile_.getDatabase(cTrans_)->addCache(cFile_.addField(pField_, cTrans_));
}

//	FUNCTION public
//	Schema::Field::reset --
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
Field::reset(Database& cDatabase_)
{
	if (_destinations)
		resetDestination();
}

//	FUNCTION public
//	Schema::Field::getPosition -- フィールドのファイルの先頭からの位置を得る
//
//	NOTES
//		ファイルの先頭からなん番目にそのフィールドが定義されているかを、
//		フィールドの位置とする
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたフィールドの位置
//
//	EXCEPTIONS
//		なし

Field::Position
Field::
getPosition() const
{
	return _position;
}

//	FUNCTION public
//	Schema::Field::getCategory -- フィールドの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたフィールドの種別
//
//	EXCEPTIONS
//		なし

Field::Category::Value
Field::
getCategory() const
{
	return _category;
}

//	FUNCTION public
//	Schema::Field::getFunction -- フィールドの種別を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたフィールドの種別
//
//	EXCEPTIONS
//		なし

Field::Function::Value
Field::
getFunction() const
{
	return _function;
}

//	FUNCTION public
//	Schema::Field::getType -- フィールドの値のデータ型を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られたフィールドの値のデータ型
//
//	EXCEPTIONS
//		なし

Field::Type
Field::
getType() const
{
	return _type;
}

//	FUNCTION public
//	Schema::Field::getElementType -- 配列フィールドの値の要素のデータ型を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		得られた配列フィールドの要素のデータ型
//
//	EXCEPTIONS
//		なし

Field::Type
Field::
getElementType() const
{
	return _elementType;
}

//	FUNCTION public
//	Schema::Field::getLength -- フィールドの最大長を得る
//
//	NOTES
//		現状では、フィールドの値のデータ型が
//		Common::DataType::String、Array 以外のときは、得られる値には意味がない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			フィールドは可変長フィールドで、得られた値は最大長(B 単位)を表す
//			またはフィールドは配列フィールドで、得られた値は最大要素数を表す
//		0
//			フィールドは無制限可変長フィールドであることを表す
//			またはフィールドは要素数無制限の配列フィールドであることを表す
//
//	EXCEPTIONS
//		なし

Field::Length
Field::
getLength() const
{
	return _length;
}

//	FUNCTION public
//	Schema::Field::getElementLength -- 配列フィールドの要素の最大長を得る
//
//	NOTES
//		現状では、フィールドの値のデータ型が
//		Common::DataType::String 以外のときは、得られる値には意味がない
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			フィールドは可変長フィールドで、得られた値は最大長(B 単位)を表す
//		0
//			フィールドは無制限可変長フィールドであることを表す
//
//	EXCEPTIONS
//		なし

Field::Length
Field::
getElementLength() const
{
	return _elementLength;
}

//	FUNCTION public
//	Schema::Field::setLength --
//		ある列の値を格納するフィールドの最大長を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Column&		column
//			この列の値をフィールドに格納する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
setLength(const Schema::Column& column)
{
	setType(column);
}

//	FUNCTION public
//	Schema::Field::setLength --
//		あるフィールドを派生元とするフィールドの最大長を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field&		source
//			派生元であるフィールドを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
setLength(const Schema::Field& source)
{
	setType(source);
}

// FUNCTION public
//	Schema::Field::getScale -- get Scale value of Decimal type field
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Field::Scale
//
// EXCEPTIONS

Field::Scale
Field::
getScale() const
{
	return m_iScale;
}

//	FUNCTION public
//	Schema::Field::getDefaultData -- フィールドの値のデフォルト値を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		0 以外の値
//			得られたデフォルト値を格納する領域の先頭アドレス
//		0
//			このフィールドにはデフォルト値の指定がされていない
//
//	EXCEPTIONS
//		なし

const Common::Data*
Field::
getDefaultData() const
{
	return _default.get();
}

//	FUNCTION public
//	Schema::Field::setDefaultData --
//		ある列の値を格納するフィールドのデフォルト値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Column&		column
//			この列の値をフィールドに格納する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
setDefaultData(const Schema::Column& column)
{
	setType(column);
}

//	FUNCTION public
//	Schema::Field::setDefaultData --
//		あるフィールドを派生元とするフィールドのデフォルト値を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field&		source
//			派生元であるフィールドを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
setDefaultData(const Schema::Field& source)
{
	setType(source);
}

//	FUNCTION public
//	Schema::Field::getSourceID --
//		フィールドの派生元のフィールドのスキーマオブジェクト ID を得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		Schema::Object::ID::Invalid 以外
//			派生元のフィールドのスキーマオブジェクト ID
//		Schema::Object::ID::Invalid
//			このフィールドを派生先とするフィールドは存在しない
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Field::
getSourceID() const
{
	return _sourceID;
}

//	FUNCTION public
//	Schema::Field::setSourceID --
//		フィールドの派生元のフィールドのスキーマオブジェクト ID を設定する
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

void
Field::
setSourceID(ID::Value id_)
{
	; _SYDNEY_ASSERT(id_ != getID());
	_sourceID = id_;
	_source = 0;
}

//	FUNCTION public
//	Schema::Field::getColumnID --
//		フィールドが値を格納している列のオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		列のオブジェクトID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Field::
getColumnID() const
{
	return _columnID;
}

//	FUNCTION public
//	Schema::Field::setColumnID --
//		フィールドが値を格納している列のオブジェクトIDを設定する
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

void
Field::
setColumnID(ID::Value id_)
{
	_columnID = id_;
	_column = 0;
}

//	FUNCTION public
//	Schema::Field::getKeyID --
//		フィールドが値を格納しているキーのオブジェクトIDを得る
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		キーのオブジェクトID
//
//	EXCEPTIONS
//		なし

Object::ID::Value
Field::
getKeyID() const
{
	return _keyID;
}

//	FUNCTION public
//	Schema::Field::setKeyID --
//		フィールドが値を格納しているキーのオブジェクトIDを設定する
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
Field::
setKeyID(ID::Value id_)
{
	_keyID = id_;
}

//	FUNCTION public
//	Schema::Field::isObjectID --
//		フィールドがオブジェクト ID を値として格納するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ture
//			オブジェクト ID を値として格納する
//		false
//			オブジェクト ID を値として格納しない
//
//	EXCEPTIONS
//		なし

bool
Field::
isObjectID() const
{
	return _category == Category::ObjectID;
}

//	FUNCTION public
//	Schema::Field::isKey --
//		フィールドがファイルのキーを値として格納するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ture
//			ファイルのキーを値として格納する
//		false
//			ファイルのキーを値として格納しない
//
//	EXCEPTIONS
//		なし

bool
Field::
isKey() const
{
	return _category == Category::Key;
}

//	FUNCTION public
//	Schema::Field::isData --
//		フィールドがファイルのデータを値として格納するか
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ture
//			ファイルのデータを値として格納する
//		false
//			ファイルのデータを値として格納しない
//
//	EXCEPTIONS
//		なし

bool
Field::
isData() const
{
	return _category == Category::Data;
}

// FUNCTION public
//	Schema::Field::isFunction -- 関数フィールドか
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Field::
isFunction() const
{
	return _category == Category::Function;
}

//	FUNCTION public
//	Schema::Field::isGetable --	フィールドは読み込み可か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ture
//			読み込み可である
//		false
//			読み込み不可である
//
//	EXCEPTIONS
//		なし

bool
Field::
isGetable() const
{
	return _permission & Permission::Getable;
}

//	FUNCTION public
//	Schema::Field::isPutable --	フィールドは書き込み可か
//
//	NOTES
//
//	ARGUMENTS
//		なし
//
//	RETURN
//		ture
//			書き込み可である
//		false
//			書き込み不可である
//
//	EXCEPTIONS
//		なし

bool
Field::
isPutable() const
{
	return _permission & Permission::Putable;
}

//	FUNCTION public
//	Schema::Field::setType --
//		ある列の値を格納するフィールドの値のデータ型を設定する
//
//	NOTES
//		フィールドが指定された列の値を格納するものとして、
//		その列の SQL のデータ型から、
//		その値を格納可能なフィールドのデータ型、
//		および最大長を求め、設定する
//
//	ARGUMENTS
//		Schema::Column&		column
//			この列の値をフィールドに格納する
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
setType(const Column& column)
{
	bool bDummy1;
	bool bDummy2;
	column.getFieldType(_type, _length, m_iScale, bDummy1, _elementType, _elementLength, bDummy2);

	Common::Data::Pointer src = column.getDefault().getConstant();
	if (src.get())
		_default = src->cast((_type == Common::DataType::Array && src->getType() != Common::DataType::Array)
							 ? _elementType : _type);
}

//	FUNCTION public
//	Schema::Field::setType --
//		あるフィールドを派生元とするフィールドの値のデータ型を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field&		source
//			派生元であるフィールドを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
setType(const Field& source)
{
	_type = source._type;
	_elementType = source._elementType;
	_length = source._length;
	_elementLength = source._elementLength;
	_default = (source._default.get()) ? source._default->copy() : Common::Data::Pointer();
}

//	FUNCTION public
//	Schema::Field::setType --
//		ある関数の返り値を表すフィールドのデータ型を設定する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Field::Function::Value function
//			関数を表す列挙子の値
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
setType(Function::Value function)
{
	(void) getFunctionType(function, _type, _elementType);
}

//	FUNCTION public
//	Schema::Field::getEncodingForm --
//		フィールドの文字列データの符号化形式を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction		trans
//			操作するトランザクションのトランザクション記述子
//
//	RETURN
//		Common::StringData::EncodingForm::Unknown 以外
//			得られた符号化形式
//		Common::StringData::EncodingForm::Unknown
//			フィールドは文字列データを格納しない
//
//	EXCEPTIONS

Common::StringData::EncodingForm::Value
Field::getEncodingForm(Trans::Transaction& trans)
{
	if (getType() == Common::DataType::String)
		if (Column* column = getRelatedColumn(trans))
			return convertTypeToEncodingForm(column->getType());

	return Common::StringData::EncodingForm::Unknown;
}

//	FUNCTION public
//	Schema::Field::getElementEncodingForm --
//		配列フィールドの要素の文字列データの符号化形式を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction		trans
//			操作するトランザクションのトランザクション記述子
//
//	RETURN
//		Common::StringData::EncodingForm::Unknown 以外
//			得られた符号化形式
//		Common::StringData::EncodingForm::Unknown
//			配列フィールドの要素は文字列データを格納しない
//
//	EXCEPTIONS

Common::StringData::EncodingForm::Value
Field::getElementEncodingForm(Trans::Transaction& trans)
{
	; _SYDNEY_ASSERT(getType() == Common::DataType::Array);

	if (getElementType() == Common::DataType::String)
		if (Column* column = getRelatedColumn(trans))
			return convertTypeToEncodingForm(column->getType());

	return Common::StringData::EncodingForm::Unknown;
}

//	FUNCTION public
//	Schema::Field::getFile -- フィールドが存在するファイルを表すクラスを得る
//
//	NOTES
//		生成前、中のフィールドや、排他制御がうまく行われていない場合を除けば、
//		フィールドが存在するファイルは必ず存在するはずである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたファイルを格納する領域の先頭アドレス
//		0
//			フィールドが存在するファイルは存在しない
//
//	EXCEPTIONS

File*
Field::getFile(Trans::Transaction& cTrans_) const
{
	return (!m_pFile) ?
		m_pFile = File::get(getParentID(), getDatabase(cTrans_), cTrans_)
		: m_pFile;
}

//	FUNCTION public
//	Schema::Field::getSource --
//		フィールドの派生元であるフィールドを表すクラスを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた派生元であるフィールドを格納する領域の先頭アドレス
//		0
//			このフィールドを派生先とするフィールドは存在しない
//
//	EXCEPTIONS
//		なし

Field*
Field::getSource(Trans::Transaction& cTrans_) const
{
	if (_sourceID != ID::Invalid && !_source) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_source) ?
			_source = Field::get(_sourceID, getDatabase(cTrans_), cTrans_)
			: _source;
	}
	return _source;
}

//	FUNCTION public
//	Schema::Field::getDestination -- フィールドの派生先を得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		フィールドの派生先であるフィールドをひとつづつ要素とするベクター
//		フィールドの派生先がひとつもないとき、空のベクター
//
//	EXCEPTIONS

const ModVector<Field*>&
Field::getDestination(Trans::Transaction& cTrans_) const
{
	AutoRWLock l(getRWLock());
	if (!_destinations) {

		l.convert(Os::RWLock::Mode::Write);
		// 書き込みロックの中で再度調べる
		if (!_destinations) {
			// このフィールドの派生先は登録されていないことにする
			const_cast<Field*>(this)->resetDestination();
			; _SYDNEY_ASSERT(_destinations);

			// 同じ表に属するファイルのすべてについて
			// このフィールドを派生元とする
			// フィールドを表すクラスをすべて得て、登録する
			// ★注意★
			// Source-Destinationの関係にあるフィールドは
			// 同じ表に属しているとの前提を設けている。
			// これが成り立たない場合、同じデータベースに属するすべての
			// 表について調べる、またはすべてのデータベースについて調べる
			// ということをすることになる。

			; _SYDNEY_ASSERT(getFile(cTrans_));
			; _SYDNEY_ASSERT(getFile(cTrans_)->getTable(cTrans_));

			const ModVector<File*>& vecFiles =
				getFile(cTrans_)->getTable(cTrans_)->getFile(cTrans_);

			ModSize n = vecFiles.getSize();
		
			for (ModSize i = 0; i < n; i++) {
				ModVector<Field*> vecFields =
					vecFiles[i]->getField(*this, cTrans_);
				if (vecFields.getSize())
					_destinations->insert(_destinations->end(),
										  vecFields.begin(), vecFields.end());
			}
		}
	}
	return *_destinations;
}

//	FUNCTION public
//	Schema::Field::addDestination --
//		フィールドの派生先として、指定されたフィールドを表すクラスを追加する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Field&		field
//			派生先として追加するフィールドを表すクラス
//			これは、呼び出し中には更新されないが、
//			その後、更新される可能性がある
//
//	RETURN
//		追加したフィールドを表すクラス
//
//	EXCEPTIONS

Field&
Field::
addDestination(Trans::Transaction& cTrans_, Field& field)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	// 派生先のフィールドを表すクラスを登録する
	if (!_destinations) {

		// 現在の派生先をリストアップしておく

		(void)getDestination(cTrans_);
		; _SYDNEY_ASSERT(_destinations);
	}
	_destinations->pushBack(&field);

	return field;
}

//	FUNCTION public
//	Schema::Field::eraseDestination --
//		フィールドの派生先から指定されたフィールドの登録を抹消する
//
//	NOTES
//		「フィールド」表は更新されない
//
//	ARGUMENTS
//		Schema::Field&		field
//			登録を抹消するフィールドを表すクラス
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
eraseDestination(const Field& field)
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_destinations) {
		ModVector<Field*>::Iterator	iterator =
			_destinations->find(const_cast<Field*>(&field));
		if (iterator != _destinations->end())
			(void) _destinations->erase(iterator);
	}
}

//	FUNCTION public
//	Schema::Field::resetDestination --
//		フィールドの派生先であるフィールドを表す
//		クラスが登録されていないことにする
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
Field::
resetDestination()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);

	if (_destinations)

		// フィールドの派生先であるフィールドを表す
		// クラスを登録するベクターを空にする

		_destinations->clear();
	else {
		// フィールドの派生先であるフィールドを表す
		// クラスを登録するベクターを生成する

		_destinations = new ModVector<Field*>();
		; _SYDNEY_ASSERT(_destinations);
	}
}

//	FUNCTION public
//	Schema::Field::clearDestination --
//		フィールドの派生先であるフィールドを表すクラスを
//		管理するためのベクターを破棄する
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
Field::
clearDestination()
{
	// フィールドの派生先であるフィールドを表す
	// クラスを登録するベクターを破棄する
	delete _destinations, _destinations = 0;
}

//	FUNCTION public
//	Schema::Field::getColumn --
//		フィールドが格納している値がどの列のものかを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られた列を格納する領域の先頭アドレス
//		0
//			このフィールドは列の値を格納していない
//
//	EXCEPTIONS
//		なし

Column*
Field::getColumn(Trans::Transaction& cTrans_) const
{
	if (!_column && _columnID != ID::Invalid) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_column) ?
			_column = Column::get(_columnID, getDatabase(cTrans_), cTrans_)
			: _column;
	}
	return _column;
}

// clear cache for corresponding column
void
Field::
clearColumn()
{
	AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
	_column = 0;
}

// FUNCTION public
//	Schema::Field::getRelatedColumn -- フィールドに関係する列を得る
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Source-destination関係もたどって得られる列
//
// EXCEPTIONS

Column*
Field::
getRelatedColumn(Trans::Transaction& cTrans_) const
{
	const Field* pField = this;
	while (pField->getColumnID() == ID::Invalid
		   && pField->getSourceID() != ID::Invalid) {
		pField = pField->getSource(cTrans_);
	}
	; _SYDNEY_ASSERT(pField);
	return pField->getColumn(cTrans_);
}

//	FUNCTION public
//	Schema::Field::getKey --
//		フィールドが格納している値がどのキーのものかを得る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		0 以外の値
//			得られたキーを格納する領域の先頭アドレス
//		0
//			このフィールドはキーの値を格納していない
//
//	EXCEPTIONS
//		なし

Key*
Field::getKey(Trans::Transaction& cTrans_) const
{
	if (!_key) {
		AutoRWLock l(getRWLock(), Os::RWLock::Mode::Write);
		// 書き込みロックの中でもう一度調べる

		return (!_key) ?
			_key = Key::get(_keyID, getDatabase(cTrans_), cTrans_)
			: _key;
	}
	return _key;
}

//	FUNCTION public
//	Schema::Field::isFirstKey --	フィールドは索引の第一キーを値として格納するか
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		ture
//			索引の第一キーを値として格納する
//		false
//			索引の第一キーを値として格納しない
//
//	EXCEPTIONS
//		なし

bool
Field::isFirstKey(Trans::Transaction& cTrans_) const
{
	if (m_iIsFirstKey < 0) {
		// 未調査
		if (isKey()) {
			if (Key* pKey = getKey(cTrans_)) {
				m_iIsFirstKey = (pKey->getPosition() == 0) ? 1 : 0;

			} else {
				// 同じファイルに属するフィールドで
				// 自分より前にキーがなければ最初のキーである
				File* pFile = getFile(cTrans_);
				; _SYDNEY_ASSERT(pFile);
				const ModVector<Field*>& vecFields = pFile->getField(cTrans_);
				ModSize n = vecFields.getSize();
				for (ModSize i = 0; i < n; ++i) {
					if (vecFields[i]->isKey()) {
						m_iIsFirstKey = (vecFields[i] == this) ? 1 : 0;
						break;
					}
				}
			}

		} else {
			m_iIsFirstKey = 0;
		}
	}
	return (m_iIsFirstKey == 1);
}

// FUNCTION public
//	Schema::Field::isStringKey -- 文字列型キーフィールドか
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
Field::
isStringKey() const
{
	return isKey()
		&& (getType() == Common::DataType::String
			|| getElementType() == Common::DataType::String);
}

//	FUNCTION public
//	Schema::Field::isTupleID --	フィールドはタプル ID を値として格納するか
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		ture
//			タプル ID を値として格納する
//		false
//			タプル ID を値として格納しない
//
//	EXCEPTIONS
//		なし

bool
Field::isTupleID(Trans::Transaction& cTrans_) const
{
	return (getColumn(cTrans_))
		? getColumn(cTrans_)->isTupleID()
		: ((getKey(cTrans_))
		   ? getKey(cTrans_)->getColumn(cTrans_)->isTupleID()
		   : ((getSource(cTrans_))
			  ? getSource(cTrans_)->isTupleID(cTrans_)
			  : false));
}

//	FUNCTION public
//	Schema::Field::isNullable --	フィールドはNULL値を格納する可能性があるか
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		ture
//			NULL値を格納する可能性がある
//		false
//			NULL値を格納しない
//
//	EXCEPTIONS
//		なし

bool
Field::isNullable(Trans::Transaction& cTrans_) const
{
	return isObjectID() ? false
		: ((getKey(cTrans_))
		   ? getKey(cTrans_)->isNullable()
		   : ((getColumn(cTrans_))
			  ? getColumn(cTrans_)->isNullable()
			  : ((getSource(cTrans_))
				 ? getSource(cTrans_)->isNullable(cTrans_)
				 : true))); // 何とも関係していなければNullが入る可能性がある
}

// FUNCTION public
//	Schema::Field::isFixed -- 固定長のフィールドか
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Field::
isFixed() const
{
	return m_bIsFixed;
}

// FUNCTION public
//	Schema::Field::hasFunction -- 関数を処理できるか
//
// NOTES
//
// ARGUMENTS
//	Function::Value eFunction_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Field::
hasFunction(Function::Value eFunction_,
			Trans::Transaction& cTrans_) const
{
	File* pFile = getFile(cTrans_);
	if (pFile->isHasFunctionField(eFunction_)) {
		switch (eFunction_) {
		case Function::Count:
			{
				if (pFile->getCategory() == File::Category::Vector) {
					return true;
				}
				// go thru.
			}
		case Function::Score:
		case Function::Section:
		case Function::Word:
		case Function::WordDf:
		case Function::WordScale:
		case Function::AverageLength:
		case Function::AverageCharLength:
		case Function::AverageWordCount:
		case Function::Tf:
		case Function::ClusterId:
		case Function::ClusterWord:
		case Function::Kwic:
		case Function::Existence:
			{
				// should be string key field
				return getType() == Common::DataType::String
					|| getElementType() == Common::DataType::String;
			}
		case Function::MinKey:
		case Function::MaxKey:
			{
				// should be first key
				Key* pKey = getKey(cTrans_);
				return pKey && pKey->getPosition() == 0;
			}
		case Function::NeighborId:
		case Function::NeighborDistance:
			{
				// should be array and first key
				Key* pKey = getKey(cTrans_);
				return pKey && pKey->getPosition() == 0
					&& getType() == Common::DataType::Array;
			}
		default:
			{
				break;
			}
		}
	}
	return false;
}

// FUNCTION public
//	Schema::Field::hasFunction2 -- 
//
// NOTES
//
// ARGUMENTS
//	LogicalFile::TreeNodeInterface::Type eFunction_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Field::
hasFunction2(LogicalFile::TreeNodeInterface::Type eFunction_,
			 Trans::Transaction& cTrans_) const
{
	File* pFile = getFile(cTrans_);
	if (pFile->isHasFunctionField(eFunction_)) {
		switch (eFunction_) {
		case LogicalFile::TreeNodeInterface::Count:
			{
				if (pFile->getCategory() == File::Category::Vector) {
					return true;
				}
				// go thru.
			}
		case LogicalFile::TreeNodeInterface::Score:
		case LogicalFile::TreeNodeInterface::Section:
		case LogicalFile::TreeNodeInterface::Word:
		case LogicalFile::TreeNodeInterface::WordDf:
		case LogicalFile::TreeNodeInterface::WordScale:
		case LogicalFile::TreeNodeInterface::AverageLength:
		case LogicalFile::TreeNodeInterface::AverageCharLength:
		case LogicalFile::TreeNodeInterface::AverageWordCount:
		case LogicalFile::TreeNodeInterface::Tf:
		case LogicalFile::TreeNodeInterface::ClusterID:
		case LogicalFile::TreeNodeInterface::FeatureValue:
		case LogicalFile::TreeNodeInterface::Kwic:
		case LogicalFile::TreeNodeInterface::Existence:
			{
				// should be string key field
				return getType() == Common::DataType::String
					|| getElementType() == Common::DataType::String;
			}
		case LogicalFile::TreeNodeInterface::NeighborID:
		case LogicalFile::TreeNodeInterface::NeighborDistance:
			{
				// should be array key field
				return getType() == Common::DataType::Array;
			}
		case LogicalFile::TreeNodeInterface::Min:
		case LogicalFile::TreeNodeInterface::Max:
			{
				// should be first key
				Key* pKey = getKey(cTrans_);
				return pKey && pKey->getPosition() == 0;
			}
		default:
			{
				break;
			}
		}
	}
	return false;
}

//	FUNCTION public
//	Schema::Field::touchRelated -- 
//		関係するオブジェクトを陳腐化する
//
//	NOTES
//
//	ARGUMENTS
//		Schema::Object::Status::Value eStatus_
//			陳腐化のもとになった状態
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
Field::
touchRelated(Status::Value eStatus_, Trans::Transaction& cTrans_)
{
	if (Field* pSource = getSource(cTrans_)) {

		switch (eStatus_) {
		case Status::Created:
		case Status::Mounted:
		case Status::DeleteCanceled:
			// SourceのDestination配列を解放して陳腐化する
			pSource->clearDestination();
			break;
		case Status::Deleted:
		case Status::DeletedInRecovery:
		case Status::CreateCanceled:
			// SourceのDestination配列から除いて陳腐化する
			pSource->eraseDestination(*this);
			break;
		default:
			break;
		}

		if (eStatus_ != Status::DeleteCanceled
			&& eStatus_ != Status::CreateCanceled) {
			// 変更があったことを伝える
			pSource->touch();
			if (Column* pColumn = pSource->getColumn(cTrans_)) {
				pColumn->touch();
			}
		}
	}
}

//	FUNCTION public
//	Schema::Field::serialize -- 
//		フィールドを表すクラスのシリアライザー
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
Field::
serialize(ModArchive& archiver)
{
	// まず、スキーマオブジェクト固有の情報をシリアル化する

	Object::serialize(archiver);

	if (archiver.isStore()) {

		// ファイルの先頭から何番目か

		archiver << _position;

		// フィールドの種別
		{
		int tmp = _category;
		archiver << tmp;
		}
		// 関数フィールドの種別
		{
		int tmp = _function;
		archiver << tmp;
		}
		// フィールドに対して許可されている操作
		{
		int tmp = _permission;
		archiver << tmp;
		}
		// フィールドの派生元のフィールドのスキーマオブジェクト ID

		archiver << _sourceID;

		// フィールドを使用する列のスキーマオブジェクト ID

		archiver << _columnID;

		// フィールドを使用するキーのスキーマオブジェクト ID

		archiver << _keyID;

		//【注意】	フィールドの型、最大長、デフォルト値は、
		//			フィールドの派生元、使用する列、
		//			オブジェクト ID フィールドかどうか
		//			を調べることにより、求めることができるが、
		//			将来的に求められなくなる可能性もあるので、記録する

		// フィールドの型
		{
		int tmp = _type;
		archiver << tmp;
		tmp = _elementType;
		archiver << tmp;
		}
		// フィールドの最大長

		archiver << _length;
		archiver << _elementLength;

		// フィールドのデフォルト値
		{
		Common::OutputArchive& out =
			dynamic_cast<Common::OutputArchive&>(archiver);
		out.writeObject(_default.get());
		}
	} else {

		// メンバーをすべて初期化しておく

		clear();

		// ファイルの先頭から何番目か

		archiver >> _position;

		// フィールドの種別
		{
		int tmp;
		archiver >> tmp;
		_category = static_cast<Category::Value>(tmp);
		}
		// 関数フィールドの種別
		{
		int tmp;
		archiver >> tmp;
		_function = static_cast<Function::Value>(tmp);
		}
		// フィールドに対して許可されている操作
		{
		int tmp;
		archiver >> tmp;
		_permission = static_cast<Permission::Value>(tmp);
		}
		// フィールドの派生元のフィールドのスキーマオブジェクト ID

		archiver >> _sourceID;

		// フィールドを使用する列のスキーマオブジェクト ID

		archiver >> _columnID;

		// フィールドを使用するキーのスキーマオブジェクト ID

		archiver >> _keyID;

		// フィールドの型
		{
		int tmp;
		archiver >> tmp;
		_type = static_cast<Type>(tmp);
		archiver >> tmp;
		_elementType = static_cast<Type>(tmp);
		}
		// フィールドの最大長

		archiver >> _length;
		archiver >> _elementLength;

		// フィールドのデフォルト値
		{
		Common::InputArchive& in =
			dynamic_cast<Common::InputArchive&>(archiver);
		_default = dynamic_cast<Common::Data*>(in.readObject());
		}
	}
}

//	FUNCTION public
//	Schema::Field::getClassID -- このクラスのクラス ID を得る
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
Field::
getClassID() const
{
	return Externalizable::Category::Field +
		Common::Externalizable::SchemaClasses;
}

//	FUNCTION public
//	Schema::Field::isLessThan -- ソートに使う比較関数
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
Field::
isLessThan(const Object& cOther_) const
{
	// フィールドは位置の順に並べる
	return (Object::getCategory() < cOther_.getCategory()
			|| (Object::getCategory() == cOther_.getCategory()
				&& (getParentID() < cOther_.getParentID()
					|| (getParentID() == cOther_.getParentID()
						&& getPosition() < _SYDNEY_DYNAMIC_CAST(const Field&, cOther_).getPosition()))));
}

// FUNCTION public
//	Schema::Field::getFunctionType -- Functionに対応する型を得る関数
//
// NOTES
//
// ARGUMENTS
//	Function::Value eFunction_
//	Type& cType_
//	Type& cElementType_
//	
// RETURN
//	bool true ... nullable
//
// EXCEPTIONS

//static
bool
Field::
getFunctionType(Function::Value eFunction_, Type& cType_, Type& cElementType_)
{
	const _FunctionInfo_& cElement = _functionTable[eFunction_];
	cType_ = cElement.m_eType;
	cElementType_ = cElement.m_eElementType;
	return cElement.m_bNullable;
}

//	FUNCTION public
//	Schema::Field::clear -- フィールドを表すクラスのメンバーをすべて初期化する
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
Field::
clear()
{
	_position = 0;
	_category = Category::Unknown;
	_function = Function::Undefined;
	_permission = Permission::None;
	_type = Common::DataType::Undefined;
	_elementType = Common::DataType::Undefined;
	_length = 0;
	_elementLength = 0;
	_source = 0;
	_sourceID = Object::ID::Invalid;
	_column = 0;
	_columnID = Object::ID::Invalid;
	_key = 0;
	_keyID = Object::ID::Invalid;
	m_iIsFirstKey = -1;
	m_pFile = 0;

	destruct();
}

// スキーマで判定してファイルドライバーに渡すField Hintのための定数
//static
const ModUnicodeString&
Field::
getHintNotNull()
{
	return _Hint::_notNull;
}

//static
const ModUnicodeString&
Field::
getHintFixed()
{
	return _Hint::_fixed;
}

void
Field::
setColumnAttribute(const Column& cColumn_)
{
	if (cColumn_.isFixed())
		m_bIsFixed = true;
	if (cColumn_.getType().getFlag() == Column::DataType::Flag::OldFixed
		&& cColumn_.getType().getType() == Column::DataType::Type::UniqueIdentifier)
		// Old version has created a wrong length to the field
		setType(cColumn_);
	if (cColumn_.getType().getType() == Common::SQLData::Type::Decimal) {
		// set scale
		m_iScale = cColumn_.getType().getScale();
	}
}

////////////////////////////////////////////
// データベースファイル化のためのメソッド //
////////////////////////////////////////////

// メタデータベースにおける「フィールド」表の構造は以下のとおり
// create table Field_DBXXXX (
//		ID			id,
//		parent		id,
//		name		nvarchar,
//		category	int,
//		function	int,
//		position	int,
//		permission	int,
//		source		id,
//		column		id,
//		key			id,
//		type		int,
//		elementType	int,
//		length		int,
//		elementLength	int,
//		default		<binary>,
//		time		timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Field>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Field>(Meta::MemberType::_type_, &Field::_get_, &Field::_set_)

	Meta::Definition<Field> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE0(ParentID),		// ParentID
		_DEFINE0(Integer),		// Category
		_DEFINE0(Integer),		// Function
		_DEFINE0(Integer),		// Position
		_DEFINE0(Integer),		// Permission
		_DEFINE2(ID, getSourceID, setSourceID), // SourceID
		_DEFINE2(ID, getColumnID, setColumnID), // ColumnID
		_DEFINE2(ID, getKeyID, setKeyID), //KeyID
		_DEFINE0(Integer),		// Type
		_DEFINE0(Integer),		// ElementType
		_DEFINE0(UnsignedInteger), // Length
		_DEFINE0(UnsignedInteger), // ElementLength
		_DEFINE0(Binary),		// Default
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

//	FUNCTION public
//	Schema::Field::getMetaFieldNumber --
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
Field::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Field::MemberNum);
}

//	FUNCTION public
//	Schema::Field::getMetaFieldDefinition --
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
Field::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

//	FUNCTION public
//	Schema::Field::packMetaField --
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
Field::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Field>& cDef = _vecDefinition[iMemberID_];

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
	case Meta::MemberType::UnsignedInteger:
		{
			switch (iMemberID_) {
			case Meta::Field::Length:
				{
					return pack(static_cast<unsigned int>(_length));
				}
			case Meta::Field::ElementLength:
				{
					return pack(static_cast<unsigned int>(_elementLength));
				}
			}
			break;
		}
	case Meta::MemberType::Binary:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Field::Default);
			return getArchiver().put(_default.get());
		}
	default:
		// これ以外の型はないはず
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

//	FUNCTION private
//	Schema::Field::unpackMetaField --
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
Field::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Field>& cDef = _vecDefinition[iMemberID_];

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
	case Meta::MemberType::UnsignedInteger:
		{
			unsigned int value;
			if (unpack(pData_, value)) {
				switch (iMemberID_) {
				case Meta::Field::Length:
					{
						_length = static_cast<Length>(value);
						return true;
					}
				case Meta::Field::ElementLength:
					{
						_elementLength = static_cast<Length>(value);
						return true;
					}
				}
			}
			break;
		}
	case Meta::MemberType::Binary:
		{
			; _SYDNEY_ASSERT(iMemberID_ == Meta::Field::Default);
			if (pData_ && pData_->isNull()) {
				_default = Common::Data::Pointer();
				return true;

			} else if (pData_ && pData_->getType() == Common::DataType::Binary) {
				const Common::BinaryData* pBinary =
					_SYDNEY_DYNAMIC_CAST(const Common::BinaryData*, pData_);

				_default = dynamic_cast<Common::Data*>(getArchiver().get(pBinary));
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
Field::
packIntegerMetaField(int iMemberID_) const
{
	switch (iMemberID_) {
	case Meta::Field::Category:
		{
			return pack(static_cast<int>(getCategory()));
		}
	case Meta::Field::Function:
		{
			return pack(static_cast<int>(getFunction()));
		}
	case Meta::Field::Position:
		{
			return pack(static_cast<int>(getPosition()));
		}
	case Meta::Field::Permission:
		{
			return pack(static_cast<int>(_permission));
		}
	case Meta::Field::Type:
		{
			return pack(static_cast<int>(_type));
		}
	case Meta::Field::ElementType:
		{
			return pack(static_cast<int>(_elementType));
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

bool
Field::
unpackIntegerMetaField(const Common::Data* pData_, int iMemberID_)
{
	int value;
	if (unpack(pData_, value)) {
		switch (iMemberID_) {
		case Meta::Field::Category:
			{
				if (value >= 0 && value < Category::ValueNum) {
					_category = static_cast<Category::Value>(value);
					return true;
				}
				break;
			}
		case Meta::Field::Function:
			{
				if (value >= 0 && value < Function::ValueNum) {
					_function = static_cast<Function::Value>(value);
					return true;
				}
				break;
			}
		case Meta::Field::Position:
			{
				if (value >= 0) {
					_position = value;
					return true;
				}
				break;
			}
		case Meta::Field::Permission:
			{
				if (value >= 0 && value <= Permission::All) {
					_permission = static_cast<Permission::Value>(value);
					return true;
				}
				break;
			}
		case Meta::Field::Type:
			{
				_type = static_cast<Common::DataType::Type>(value);
				return true;
			}
		case Meta::Field::ElementType:
			{
				_elementType = static_cast<Common::DataType::Type>(value);
				return true;
			}
		default:
			break;
		}
	}
	return false;
}

//
// Copyright (c) 2000, 2001, 2002, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
