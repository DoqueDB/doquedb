// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// SystemDatabase.cpp -- メタデータベース関連の関数定義
// 
// Copyright (c) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2014, 2023 Ricoh Company, Ltd.
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

#include "Schema/SystemDatabase.h"
#include "Schema/Area.h"
#include "Schema/AreaContent.h"
#include "Schema/BtreeFile.h"
#include "Schema/Cascade.h"
#include "Schema/Column.h"
#include "Schema/Constraint.h"
#include "Schema/Database.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Function.h"
#include "Schema/Index.h"
#include "Schema/Key.h"
#include "Schema/Message.h"
#include "Schema/Meta.h"
#include "Schema/NameParts.h"
#include "Schema/Partition.h"
#include "Schema/Privilege.h"
#include "Schema/RecordFile.h"
#include "Schema/SystemTable.h"
#include "Schema/Table.h"
#include "Schema/VectorFile.h"

#include "Statement/IntegerValue.h"

#include "Common/Assert.h"
#include "Common/SQLData.h"

#include "Exception/Unexpected.h"

#include "LogicalFile/FileID.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

#include "Trans/Transaction.h"
#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace {

// 索引ファイルの名前(SystemTable.cppと同じである必要あり)
const char* const _pszParentIDIndex	= "ParentID";
const char* const _pszIDIndex		= "ID"; // ID index can't be used because it is ID->parentID index
const char* const _pszAreaIDIndex	= "AreaID";
const char* const _pszObjectIDIndex	= "ObjectID";

// スレッド間排他制御するためのクリティカルセクション

Os::CriticalSection	_criticalSection;

// メタデータベース

ModAutoPointer<SystemDatabase> _database = 0;

//	TYPEDEF local
//	$$::_AddColumns -- 列オブジェクトを作る関数の型
//
//	NOTES

typedef void (*_AddColumns)(Trans::Transaction& cTrans_,
							Database& cDatabase_, Table& cTable_, File& cFile_,
							Schema::Object::ID::Value& iObjectID_);

//	TYPEDEF local
//	$$::_AddIndex -- 索引ファイルオブジェクトを作る関数の型
//
//	NOTES

typedef void (*_AddIndex)(Trans::Transaction& cTrans_,
						  Database& cDatabase_, Table& cTable_,
						  SystemTable::SystemFile& cSystemFile_,
						  Schema::Object::ID::Value& iObjectID_);

//	CONST local
//	$$::_XXXType -- AddColumnsの定義に使うデータ型の定数
//
//	NOTES

const Common::SQLData _IntType(Common::SQLData::Type::Int, Common::SQLData::Flag::Fixed, 4, 0);
const Common::SQLData _UIntType(Common::SQLData::Type::UInt, Common::SQLData::Flag::Fixed, 4, 0);
const Common::SQLData _TextType(Common::SQLData::Type::NText, Common::SQLData::Flag::Unlimited, 0, 0);
const Common::SQLData _IntArrayType(Common::SQLData::Type::Int, Common::SQLData::Flag::Fixed, 4, 0, -1);
const Common::SQLData _TextArrayType(Common::SQLData::Type::NText, Common::SQLData::Flag::Unlimited, 0, 0, -1);
const Common::SQLData _ImageType(Common::SQLData::Type::Image, Common::SQLData::Flag::Unlimited, 0, 0);
const Common::SQLData _BigIntType(Common::SQLData::Type::BigInt, Common::SQLData::Flag::Fixed, 8, 0);

namespace _addColumns
{
	// 各システム表の列オブジェクトを作る関数の宣言
	void _Database(Trans::Transaction& cTrans_,Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Table(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Column(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Constraint(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Index(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Key(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _File(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Field(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Area(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _AreaContent(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Privilege(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Cascade(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Partition(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
	void _Function(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, File& cFile_, Schema::Object::ID::Value& iObjectID_);
}

namespace _addIndex
{
	// 各システム表の索引ファイルオブジェクトを作る関数の宣言
	void _Database(Trans::Transaction& cTrans_,Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Table(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Column(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Constraint(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Index(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Key(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _File(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Field(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Area(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _AreaContent(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Privilege(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Cascade(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Partition(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
	void _Function(Trans::Transaction& cTrans_, Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_, Schema::Object::ID::Value& iObjectID_);
}

//	CONST local
//	$$::_AddColumnsTable -- カテゴリーと列オブジェクト追加関数を関係づける
//
//	NOTES

const _AddColumns _AddColumnsTable[] =
{
	0,									// Unknown
	&_addColumns::_Database,
	&_addColumns::_Table,
	&_addColumns::_Column,
	&_addColumns::_Constraint,
	&_addColumns::_Index,
	&_addColumns::_Key,
	&_addColumns::_File,
	&_addColumns::_Field,
	&_addColumns::_Area,
	&_addColumns::_AreaContent,
	&_addColumns::_Privilege,
	&_addColumns::_Cascade,
	&_addColumns::_Partition,
	&_addColumns::_Function,
	0
};

//	CONST local
//	$$::_AddIndexTable -- カテゴリーと索引ファイルオブジェクト追加関数を関係づける
//
//	NOTES

const _AddIndex _AddIndexTable[] =
{
	0,									// Unknown
	&_addIndex::_Database,
	&_addIndex::_Table,
	&_addIndex::_Column,
	&_addIndex::_Constraint,
	&_addIndex::_Index,
	&_addIndex::_Key,
	&_addIndex::_File,
	&_addIndex::_Field,
	&_addIndex::_Area,
	&_addIndex::_AreaContent,
	&_addIndex::_Privilege,
	&_addIndex::_Cascade,
	&_addIndex::_Partition,
	&_addIndex::_Function,
	0
};

namespace _Sub
{
	// システム表の列オブジェクト以下を作る
	void _addColumn(Trans::Transaction& cTrans_,
					Database& cDatabase_, Table& cTable_, File& cFile_,
					Column::Position iColPos_, const Object::Name& cName_,
					Column::Category::Value eCategory_, Common::SQLData cDataType_,
					int iPosition_, Common::DataType::Type eType_,
					Schema::Object::ID::Value& iObjectID_);

	//	STRUCT local
	//	$$::_Sub::_ColumnSpec -- AddColumnsの定義に使う列定義用の構造体
	//
	//	NOTES

	struct _ColumnSpec
	{
		const char* m_pszName;
		Column::Category::Value m_eCategory;
		const Common::SQLData* m_pDataType;
		int m_iMetaID;
	};

	// システム表の索引ファイルオブジェクト以下を作る
	void _addIndex(Trans::Transaction& cTrans_,
				   Database& cDatabase_, Table& cTable_,
				   SystemTable::SystemFile& cSystemFile_,
				   const char* pszName_,
				   Schema::Object::ID::Value& iObjectID_);
}

} // namespace

/////////////////////////////
// $$
/////////////////////////////

//	FUNCTION local
//	$$::_Sub::_addColumn -- システム表の列オブジェクト以下を作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Column::Position iColPos_
//			列の位置
//		const Object::Name& cName_
//			列の名前
//		Column::Category::Value eCategory_
//			列の種別
//		Common::SQLData cDataType_
//			列の型
//		int iPosition_
//			列のMetaData上での位置
//		Common::DataType::Type eType_
//			Fieldに格納されるCommon::Dataの型
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_Sub::_addColumn(Trans::Transaction& cTrans_,
				 Database& cDatabase_, Table& cTable_, File& cFile_,
				 Column::Position iColPos_,
				 const Object::Name& cName_,
				 Column::Category::Value eCategory_,
				 Common::SQLData cDataType_,
				 int iPosition_,
				 Common::DataType::Type eType_,
				 Schema::Object::ID::Value& iObjectID_)
{
	Column::Pointer pColumn =
		Column::createSystem(cTrans_,
							 cTable_, iColPos_, cName_, eCategory_,
							 cDataType_, Default(),
							 iObjectID_--);

	// 列を表とキャッシュに加える
	cDatabase_.addCache(cTable_.addColumn(pColumn, cTrans_));

	Field::Pointer pField =
		Field::createSystem(cTrans_,
							cFile_, iPosition_,
							Field::Category::Data,
							Field::Permission::Getable,
							*pColumn, eType_,
							iObjectID_--);

	// フィールドをファイルとキャッシュに加える
	cDatabase_.addCache(cFile_.addField(pField, cTrans_));

	pColumn->setField(*pField);
}

//	FUNCTION local
//	$$::_Sub::_addIndex -- システム表の索引ファイルオブジェクト以下を作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		SystemTable::SystemFile& cSystemFile_
//			システム表を表すクラス
//		const char* pszName_
//			索引ファイルの名前
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_Sub::_addIndex(Trans::Transaction& cTrans_,
				Database& cDatabase_, Table& cTable_,
				SystemTable::SystemFile& cSystemFile_,
				const char* pszName_,
				Schema::Object::ID::Value& iObjectID_)
{
#if 0 // plan is not support non-rowid index, so disable system file index temporarily
	SystemTable::IndexFile* pSystemIndexFile = cSystemFile_.getIndex(pszName_);
	; _SYDNEY_ASSERT(pSystemIndexFile);

	File::Pointer pFile;
	switch (pSystemIndexFile->getCategory()) {
	case SystemTable::IndexFile::Category::Btree:
		{
			pFile = BtreeFile::createSystem(cTrans_, cDatabase_, cTable_, cSystemFile_, pszName_, iObjectID_--);
			break;
		}
	case SystemTable::IndexFile::Category::Vector:
		{
			pFile = VectorFile::createSystem(cTrans_, cDatabase_, cTable_, cSystemFile_, pszName_, iObjectID_--);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

	; _SYDNEY_ASSERT(pFile.get());

	// add to cache
	cDatabase_.addCache(cTable_.addFile(pFile, cTrans_));

	//////////////////
	// create fields
	//////////////////

	// get record file (id = table's id)
	File* pRecordFile = cTable_.getFile(cTable_.getID(), cTrans_);
	; _SYDNEY_ASSERT(pRecordFile);

	// key/value source
	Field* pKeySource =
		pRecordFile->getFieldByPosition(pSystemIndexFile->getRecordKeyPosition(), cTrans_);
	Field* pValueSource =
		pRecordFile->getFieldByPosition(pSystemIndexFile->getRecordValuePosition(), cTrans_);
	; _SYDNEY_ASSERT(pKeySource);
	; _SYDNEY_ASSERT(pValueSource);

	// create key/value fields
	Field::Pointer pKeyField =
		Field::createSystem(cTrans_,
							*pFile, pSystemIndexFile->getKeyPosition(),
							Field::Category::Key,
							Field::Permission::Getable,
							*pKeySource,
							iObjectID_--);
	cDatabase_.addCache(pFile->addField(pKeyField, cTrans_));

	Field::Pointer pValueField =
		Field::createSystem(cTrans_,
							*pFile, pSystemIndexFile->getValuePosition(),
							Field::Category::Data,
							Field::Permission::Getable,
							*pValueSource,
							iObjectID_--);
	cDatabase_.addCache(pFile->addField(pValueField, cTrans_));
#endif
}

/////////////////////////////////////////////
// _addColumns::_XXX
/////////////////////////////////////////////

//	FUNCTION local
//	$$::_addColumns::_Database -- データベース表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Database(Trans::Transaction& cTrans_,
					 Database& cDatabase_, Table& cTable_, File& cFile_,
					 Schema::Object::ID::Value& iObjectID_)
{
	// RowID、Name、Flag、Path
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Database::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID), _Category(TupleID), _Type(UInt), 	   _ID(ID)},
		{_Name(Name),	 _Category(Normal),  _Type(Text),	   _ID(Name)},
		{_Name(Flag),	 _Category(Normal),  _Type(UInt),	   _ID(Flag)},
 		{_Name(Path),	 _Category(Function),  _Type(TextArray), _ID(Path)},
 		{_Name(MasterURL), _Category(Function),  _Type(Text),  _ID(MasterURL)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Database().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Table -- 表表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Table(Trans::Transaction& cTrans_,
				  Database& cDatabase_, Table& cTable_, File& cFile_,
				  Schema::Object::ID::Value& iObjectID_)
{
	// RowID、Name、AreaID
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Table::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID), _Category(TupleID), _Type(UInt), 	  _ID(ID)},
		{_Name(Name),	 _Category(Normal),  _Type(Text),	  _ID(Name)},
		{_Name(AreaID),	 _Category(Normal),  _Type(IntArray), _ID(AreaIDs)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Table().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Column -- 列表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Column(Trans::Transaction& cTrans_,
				   Database& cDatabase_, Table& cTable_, File& cFile_,
				   Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ParentID、Name、Category、Position、FieldID、Default、Flag
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Column::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt),	_ID(ID)},
		{_Name(ParentID), _Category(Normal),  _Type(UInt),  _ID(ParentID)},
		{_Name(Name),	  _Category(Normal),  _Type(Text),	_ID(Name)},
		{_Name(Category), _Category(Normal),  _Type(Int),	_ID(Category)},
		{_Name(Position), _Category(Normal),  _Type(Int),	_ID(Position)},
		{_Name(FieldID),  _Category(Normal),  _Type(UInt),  _ID(FieldID)},
		{_Name(Default),  _Category(Normal),  _Type(Image), _ID(Default)},
		{_Name(Flag),	  _Category(Normal),  _Type(Int),	_ID(Flag)},
		{_Name(MetaData), _Category(Function),_Type(TextArray),	_ID(MetaData)},
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Column().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Constraint -- 制約表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Constraint(Trans::Transaction& cTrans_,
					   Database& cDatabase_, Table& cTable_, File& cFile_,
					   Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ParentID、Name、Category、Position、ColumnID
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Constraint::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt), 	   _ID(ID)},
		{_Name(ParentID), _Category(Normal),  _Type(UInt),	   _ID(ParentID)},
		{_Name(Name),	  _Category(Normal),  _Type(Text),	   _ID(Name)},
		{_Name(Category), _Category(Normal),  _Type(Int),	   _ID(Category)},
		{_Name(Position), _Category(Normal),  _Type(Int),	   _ID(Position)},
		{_Name(ColumnID), _Category(Normal),  _Type(IntArray), _ID(ColumnIDs)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Constraint().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Index -- 索引表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Index(Trans::Transaction& cTrans_,
				  Database& cDatabase_, Table& cTable_, File& cFile_,
				  Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ParentID、Name、Category、Flag、FileID, AreaID
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Index::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt), 	   _ID(ID)},
		{_Name(ParentID), _Category(Normal),  _Type(UInt),	   _ID(ParentID)},
		{_Name(Name),	  _Category(Normal),  _Type(Text),	   _ID(Name)},
		{_Name(Category), _Category(Normal),  _Type(Int),	   _ID(Category)},
		{_Name(Flag),	  _Category(Normal),  _Type(Int),	   _ID(Flag)},
		{_Name(FileID),	  _Category(Normal),  _Type(UInt),	   _ID(FileID)},
		{_Name(AreaID),	  _Category(Normal),  _Type(IntArray), _ID(AreaIDs)},
		{_Name(HintString), _Category(Function),  _Type(Text),     _ID(HintString)},
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Index().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Key -- キー表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Key(Trans::Transaction& cTrans_,
				Database& cDatabase_, Table& cTable_, File& cFile_,
				Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ParentID、Position、ColumnID、FieldID
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Key::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt),	_ID(ID)},
		{_Name(ParentID), _Category(Normal),  _Type(UInt),	_ID(ParentID)},
		{_Name(Position), _Category(Normal),  _Type(Int),	_ID(Position)},
		{_Name(ColumnID), _Category(Normal),  _Type(UInt),	_ID(ColumnID)},
		{_Name(FieldID),  _Category(Normal),  _Type(UInt),	_ID(FieldID)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Key().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_File -- ファイル表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_File(Trans::Transaction& cTrans_,
				 Database& cDatabase_, Table& cTable_, File& cFile_,
				 Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ParentID、Name、Category、FileID、AreaID
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::File::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt), 	   _ID(ID)},
		{_Name(ParentID), _Category(Normal),  _Type(UInt),	   _ID(ParentID)},
		{_Name(Name),	  _Category(Normal),  _Type(Text),     _ID(Name)},
		{_Name(Category), _Category(Normal),  _Type(Int),	   _ID(Category)},
		{_Name(FileID),	  _Category(Normal),  _Type(Image),	   _ID(FileID)},
		{_Name(AreaID),	  _Category(Normal),  _Type(IntArray), _ID(AreaIDs)},
		{_Name(FileSize), _Category(Function),_Type(BigInt),   _ID(FileSize)},
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(File().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Field -- フィールド表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Field(Trans::Transaction& cTrans_,
				  Database& cDatabase_, Table& cTable_, File& cFile_,
				  Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ParentID、Category、Function、Position、Permission、SourceID、ColumnID、KeyID、Type
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Field::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt),	_ID(ID)},
		{_Name(ParentID), _Category(Normal),  _Type(UInt),	_ID(ParentID)},
		{_Name(Category), _Category(Normal),  _Type(Int),	_ID(Category)},
		{_Name(Function), _Category(Normal),  _Type(Int),	_ID(Function)},
		{_Name(Position), _Category(Normal),  _Type(Int),	_ID(Position)},
		{_Name(Permission), _Category(Normal),_Type(Int),	_ID(Permission)},
		{_Name(SourceID), _Category(Normal),  _Type(UInt),	_ID(SourceID)},
		{_Name(ColumnID), _Category(Normal),  _Type(UInt),	_ID(ColumnID)},
		{_Name(KeyID),	  _Category(Normal),  _Type(UInt),	_ID(KeyID)},
		{_Name(DataType), _Category(Normal),  _Type(Int),	_ID(Type)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Field().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Area -- エリア表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_Area(Trans::Transaction& cTrans_,
				 Database& cDatabase_, Table& cTable_, File& cFile_,
				 Schema::Object::ID::Value& iObjectID_)
{
	// RowID、Name、Path
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Area::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt),	   _ID(ID)},
		{_Name(Name),	  _Category(Normal),  _Type(Text),	   _ID(Name)},
		{_Name(Path),	  _Category(Normal),  _Type(TextArray),_ID(Path)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Area().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_AreaContent -- エリア格納関係表の列オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::File& cFile_
//			システム表を構成するレコードファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addColumns::_AreaContent(Trans::Transaction& cTrans_,
						Database& cDatabase_, Table& cTable_, File& cFile_,
						Schema::Object::ID::Value& iObjectID_)
{
	// RowID、AreaID、ObjectID
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::AreaContent::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),  _Category(TupleID), _Type(UInt), _ID(ID)},
		{_Name(AreaID),	  _Category(Normal),  _Type(UInt), _ID(AreaID)},
		{_Name(ObjectID), _Category(Normal),  _Type(UInt), _ID(ObjectID)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(AreaContent().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Privilege -- Create system table's columns for privilege
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::File& cFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addColumns::_Privilege(Trans::Transaction& cTrans_,
						Database& cDatabase_, Table& cTable_, File& cFile_,
						Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ID, Name、Type
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Privilege::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),	_Category(TupleID),	_Type(UInt),	_ID(ID)},
		{_Name(UserID),		_Category(Normal),	_Type(Int),		_ID(UserID)},
		{_Name(Privilege),	_Category(Function),_Type(TextArray),_ID(Flags)},
		{_Name(ObjectType),	_Category(Function),_Type(Text),	_ID(Type)},
		{_Name(ObjectID),	_Category(Function),_Type(Text),	_ID(ObjectIDs)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Privilege().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Cascade -- Create system table's columns for cascade
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::File& cFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addColumns::_Cascade(Trans::Transaction& cTrans_,
					  Database& cDatabase_, Table& cTable_, File& cFile_,
					  Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ID, Name、Type
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Cascade::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),	_Category(TupleID),	_Type(UInt),	_ID(ID)},
		{_Name(Name),		_Category(Normal),	_Type(Text),	_ID(Name)},
		{_Name(Target),		_Category(Normal),	_Type(TextArray),_ID(Target)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Cascade().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Partition -- Create system table's columns for partition
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::File& cFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addColumns::_Partition(Trans::Transaction& cTrans_,
						Database& cDatabase_, Table& cTable_, File& cFile_,
						Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ID, Name、Type
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Partition::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),	_Category(TupleID),	_Type(UInt),	_ID(ID)},
		{_Name(ParentID),	_Category(Normal),	_Type(UInt),	_ID(TableID)},
		{_Name(Category),	_Category(Function),_Type(Text),	_ID(Category)},
		{_Name(FunctionName),_Category(Normal),	_Type(Text),	_ID(FunctionName)},
		{_Name(ColumnID),	_Category(Normal),	_Type(IntArray),_ID(ColumnIDs)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Partition().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

//	FUNCTION local
//	$$::_addColumns::_Function -- Create system table's columns for function
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::File& cFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addColumns::_Function(Trans::Transaction& cTrans_,
					   Database& cDatabase_, Table& cTable_, File& cFile_,
					   Schema::Object::ID::Value& iObjectID_)
{
	// RowID、ID, Name、Type
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) &_##__str__##Type
#define _ID(__str__) Meta::Function::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),	_Category(TupleID),	_Type(UInt),	_ID(ID)},
		{_Name(Name),		_Category(Normal),	_Type(Text),	_ID(Name)},
		{_Name(Routine),	_Category(Function),_Type(Text),	_ID(Routine)}
	};
#undef _ID
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cDatabase_, cTable_, cFile_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 *(columnSpec[i].m_pDataType),
						 columnSpec[i].m_iMetaID,
						 Meta::getFieldType(Function().getMetaMemberType(columnSpec[i].m_iMetaID)),
						 iObjectID_);
	}
}

/////////////////////////////////////////////
// _addIndex::_XXX
/////////////////////////////////////////////

//	FUNCTION local
//	$$::_addIndex::_Database -- データベース表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Database(Trans::Transaction& cTrans_,
					 Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
					 Schema::Object::ID::Value& iObjectID_)
{
	// no index
}

//	FUNCTION local
//	$$::_addIndex::_Table -- 表表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Table(Trans::Transaction& cTrans_,
				  Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
				  Schema::Object::ID::Value& iObjectID_)
{
	// no index
}

//	FUNCTION local
//	$$::_addIndex::_Column -- 列表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Column(Trans::Transaction& cTrans_,
				   Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
				   Schema::Object::ID::Value& iObjectID_)
{
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszParentIDIndex, iObjectID_);
}

//	FUNCTION local
//	$$::_addIndex::_Constraint -- 制約表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Constraint(Trans::Transaction& cTrans_,
					   Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
					   Schema::Object::ID::Value& iObjectID_)
{
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszParentIDIndex, iObjectID_);
}

//	FUNCTION local
//	$$::_addIndex::_Index -- 索引表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Index(Trans::Transaction& cTrans_,
				  Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
				  Schema::Object::ID::Value& iObjectID_)
{
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszParentIDIndex, iObjectID_);
}

//	FUNCTION local
//	$$::_addIndex::_Key -- キー表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Key(Trans::Transaction& cTrans_,
				Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
				Schema::Object::ID::Value& iObjectID_)
{
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszParentIDIndex, iObjectID_);
}

//	FUNCTION local
//	$$::_addIndex::_File -- ファイル表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_File(Trans::Transaction& cTrans_,
				 Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
				 Schema::Object::ID::Value& iObjectID_)
{
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszParentIDIndex, iObjectID_);
}

//	FUNCTION local
//	$$::_addIndex::_Field -- フィールド表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Field(Trans::Transaction& cTrans_,
				  Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
				  Schema::Object::ID::Value& iObjectID_)
{
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszParentIDIndex, iObjectID_);
}

//	FUNCTION local
//	$$::_addIndex::_Area -- エリア表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_Area(Trans::Transaction& cTrans_,
				 Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
				 Schema::Object::ID::Value& iObjectID_)
{
	// no index
}

//	FUNCTION local
//	$$::_addIndex::_AreaContent -- エリア格納関係表の索引オブジェクトを作る
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		Schema::Database& cDatabase_
//			システム表が属するデータベース
//		Schema::Table& cTable_
//			システム表を表すオブジェクト
//		Schema::SystemFile& cSystemFile_
//			システム表を構成するシステムファイル
//		Schema::Object::ID::Value& iObjectID_
//			スキーマオブジェクトID
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
_addIndex::_AreaContent(Trans::Transaction& cTrans_,
						Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
						Schema::Object::ID::Value& iObjectID_)
{
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszAreaIDIndex, iObjectID_);
	_Sub::_addIndex(cTrans_, cDatabase_, cTable_, cSystemFile_,
					_pszObjectIDIndex, iObjectID_);
}

//	FUNCTION local
//	$$::_addIndex::_Privilege -- Create system table's index for privilege
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::SystemFile& cSystemFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addIndex::_Privilege(Trans::Transaction& cTrans_,
						Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
						Schema::Object::ID::Value& iObjectID_)
{
	// no index
}

//	FUNCTION local
//	$$::_addIndex::_Cascade -- Create system table's index for cascade
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::SystemFile& cSystemFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addIndex::_Cascade(Trans::Transaction& cTrans_,
					Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
					Schema::Object::ID::Value& iObjectID_)
{
	// no index
}

//	FUNCTION local
//	$$::_addIndex::_Partition -- Create system table's index for partition
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::SystemFile& cSystemFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addIndex::_Partition(Trans::Transaction& cTrans_,
					  Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
					  Schema::Object::ID::Value& iObjectID_)
{
	// no index
}

//	FUNCTION local
//	$$::_addIndex::_Function -- Create system table's index for function
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Database& cDatabase_
//		Schema::Table& cTable_
//		Schema::SystemFile& cSystemFile_
//		Schema::Object::ID::Value& iObjectID_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_addIndex::_Function(Trans::Transaction& cTrans_,
					 Database& cDatabase_, Table& cTable_, SystemTable::SystemFile& cSystemFile_,
					 Schema::Object::ID::Value& iObjectID_)
{
	// no index
}

/////////////////////////////
// SystemDatabase
/////////////////////////////

//	FUNCTION public
//	Schema::SystemDatabase::getInstance --
//		メタデータベースを表すクラスを得る
//
//	NOTES
//		Singletonである
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子(ダミー)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

// static
SystemDatabase*
SystemDatabase::getInstance(Trans::Transaction& cTrans_)
{
	if (!_database.get()) {
		_database = new SystemDatabase;
		_database->create(cTrans_);
	}
	return _database;
}

//	FUNCTION public
//	Schema::SystemDatabase::terminate --
//		メタデータベースを表すクラスを抹消する
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

// static
void
SystemDatabase::
terminate()
{
	// AutoPointerなので0を入れればdeleteされる
	_database = 0;
}

//	FUNCTION public
//	Schema::SystemDatabase::SystemDatabase --
//		メタデータベースを表すクラスのコンストラクター
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

SystemDatabase::
SystemDatabase()
	: Database(Name(), Scope::Meta)
{
	// 名前を設定する
	setName(ModCharString(NameParts::Database::System));

	// IDを設定する
	setID(ID::SystemTable);

	// メタデータベースは読み取り専用にする
	setReadOnly(true);

	// メタデータベースは永続化の必要はないので
	// この時点でPersistにする
	setStatus(Status::Persistent);
}

//	FUNCTION private
//	Schema::SystemDatabase::destruct --
//		データベースを表すクラスのデストラクター下位関数
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
SystemDatabase::
destruct()
{}

//	FUNCTION public
//	Schema::SystemDatabase::createSystemTable --
//		メタデータベースに属する表以下のオブジェクトを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子
//		const Database& cDatabase_
//			システム表が属するデータベースを表すオブジェクト
//		Object::Category::Value eCategory_
//			システム表のオブジェクト種
//		Object::ID::Value& iObjectID_
//			ファイル以下のオブジェクトに使うID
//
//	RETURN
//		作成した表オブジェクト
//
//	EXCEPTIONS

//static
Table*
SystemDatabase::
createSystemTable(Trans::Transaction& cTrans_,
				  Database& cDatabase_,
				  Object::Category::Value eCategory_,
				  Object::ID::Value& iObjectID_)
{
	// データベース表を表すオブジェクトを作成する
	Table::Pointer pTable =
		Table::createSystem(cTrans_, cDatabase_, eCategory_);

	// 表をデータベースに登録する
	// 表はキャッシュには加えない
	cDatabase_.addTable(pTable, cTrans_);

	// ファイルオブジェクトを作成する
	ModAutoPointer<SystemTable::SystemFile> pSystemFile =
		SystemTable::getSystemFile(eCategory_, &cDatabase_);

	pSystemFile->initialize();

	// ★注意★
	// システム表のファイルのIDは表と同じものになっている
	// 表はキャッシュに載らないのでIDがぶつかってもキャッシュが壊れることはない
	// また、表を構成するファイルは1つだけなのでファイルのIDが重複することもない
	File::Pointer pFile =
		RecordFile::createSystem(cTrans_, cDatabase_, *pTable,
								 *pSystemFile,
								 pTable->getID());

	// キャッシュに加える
	cDatabase_.addCache(pTable->addFile(pFile, cTrans_));

	// add oid field
	Field::Pointer pField =
		Field::createSystem(cTrans_,
							*pFile, 0,
							Field::Category::ObjectID,
							Field::Permission::Getable,
							LogicalFile::ObjectID().getType(),
							iObjectID_--);

	// フィールドをファイルとキャッシュに加える
	cDatabase_.addCache(pFile->addField(pField, cTrans_));


	(*_AddColumnsTable[eCategory_])(cTrans_, cDatabase_, *pTable, *pFile, iObjectID_);
	(*_AddIndexTable[eCategory_])(cTrans_, cDatabase_, *pTable, *pSystemFile, iObjectID_);

	return pTable.get();
}

//	FUNCTION private
//	Schema::SystemDatabase::create --
//		メタデータベースに属する表以下のオブジェクトを生成する
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//			操作を行うトランザクション記述子(ダミー)
//
//	RETURN
//		なし
//
//	EXCEPTIONS

void
SystemDatabase::create(Trans::Transaction& cTrans_)
{
	// スキーマオブジェクトのIDは
	// カテゴリーの最大値に対応するものから順に小さくしていく
	ID::Value iObjectID = Table::getSystemTableID(Object::Category::ValueNum);

	// データベース表を表すオブジェクトを作成する
	(void) createSystemTable(cTrans_, *this, Object::Category::Database, iObjectID);
}

//
// Copyright (c) 2001, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2012, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
