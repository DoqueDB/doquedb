// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VirtualTable.cpp -- オブジェクト ID 関連の関数定義
// 
// Copyright (c) 2007, 2011, 2012, 2017, 2023 Ricoh Company, Ltd.
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

#include "Schema/VirtualTable.h"
#include "Schema/Database.h"
#include "Schema/File.h"
#include "Schema/NameParts.h"
#include "Schema/Table.h"

#include "Common/SQLData.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
//	CONST
//	$$$::_NameTable -- table name for each category of virtual table
//
//	NOTES
const Table::Name _NameTable[] =
{
	Table::Name(_TRMEISTER_U_STRING("")), // Unknown
	Table::Name(_TRMEISTER_U_STRING("System_User")), // User
	Table::Name(_TRMEISTER_U_STRING("System_Session")) // Session
};

//	TYPEDEF local
//	$$::_AddColumns -- Type of methods which is used to add a column
//
//	NOTES

typedef void (*_AddColumns)(Trans::Transaction& cTrans_, Table& cTable_);

namespace _addColumns
{
	// addcolumns functions for each virtual table type
	void _User(Trans::Transaction& cTrans_, Table& cTable_);
	void _Session(Trans::Transaction& cTrans_, Table& cTable_);
}

//	CONST local
//	$$::_AddColumnsTable -- Correspondence between virtual table type and addcolumns functions
//
//	NOTES

const _AddColumns _AddColumnsTable[] =
{
	0,								// Unknown
	&_addColumns::_User,			// User
	&_addColumns::_Session,			// Session
	0
};

namespace _Sub
{
	// Create column object
	void _addColumn(Trans::Transaction& cTrans_,
					Table& cTable_,
					Column::Position iPosition_, const Object::Name& cName_,
					Column::Category::Value eCategory_,
					Common::DataType::Type eType_,
					Schema::Object::ID::Value iObjectID_);

	//	STRUCT local
	//	$$::_Sub::_ColumnSpec -- column defitinion for AddColumns
	//
	//	NOTES

	struct _ColumnSpec
	{
		const char* m_pszName;		// column name
		Column::Category::Value m_eCategory; // column category
		Common::DataType::Type m_eType;	// column data type
	};
} // namespace _Sub

// FUNCTION local
//	$$$::_addColumns::_User -- addcolumns functions for each virtual table type
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_addColumns::_User(Trans::Transaction& cTrans_, Table& cTable_)
{
	// RowID、ID, Name、Type
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) Common::DataType::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),	_Category(TupleID),	_Type(UnsignedInteger)},
		{_Name(UserID),		_Category(Normal),	_Type(Integer)},
		{_Name(Name),		_Category(Normal),	_Type(String)},
		{_Name(Category),	_Category(Normal),	_Type(Integer)}
	};
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cTable_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 columnSpec[i].m_eType,
						 Object::ID::Invalid - i - 1);
	}
}

// FUNCTION local
//	$$$::_addColumns::_Session
//		-- addcolumns functions for each virtual table type
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Table& cTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_addColumns::_Session(Trans::Transaction& cTrans_, Table& cTable_)
{
	// RowID, ClientID, HostName, ConnectedTime, ProtocolVersion, CryptMode, SessionID, DatabaseName, UserName, StatementType, TransactionState
#define _Name(__str__) NameParts::Column::System::__str__
#define _Category(__str__) Column::Category::__str__
#define _Type(__str__) Common::DataType::__str__
	_Sub::_ColumnSpec columnSpec[] =
	{
		{_Name(TupleID),		_Category(TupleID),	_Type(UnsignedInteger)},
		{_Name(ClientID),		_Category(Normal),	_Type(Integer)},
		{_Name(HostName),		_Category(Normal),	_Type(String)},
		{_Name(ConnectedTime),	_Category(Normal),	_Type(DateTime)},
		{_Name(ProtocolVersion),_Category(Normal),	_Type(Integer)},
		{_Name(CryptMode),		_Category(Normal),	_Type(String)},
		{_Name(SessionID),		_Category(Normal),	_Type(Integer)},
		{_Name(DatabaseName),	_Category(Normal),	_Type(String)},
		{_Name(UserName),		_Category(Normal),	_Type(String)},
		{_Name(SessionStartTime),	_Category(Normal),	_Type(DateTime)},
		{_Name(StatementType),	_Category(Normal),	_Type(String)},
		{_Name(TransactionState),	_Category(Normal),	_Type(String)},
		{_Name(TransactionStartTime),	_Category(Normal),	_Type(DateTime)},
		{_Name(SqlStatement),	_Category(Normal),	_Type(String)}
	};
#undef _Type
#undef _Category
#undef _Name

	for (int i = 0; i < sizeof(columnSpec) / sizeof(columnSpec[0]); ++i) {
		_Sub::_addColumn(cTrans_, cTable_,
						 i, Schema::Object::Name(columnSpec[i].m_pszName),
						 columnSpec[i].m_eCategory,
						 columnSpec[i].m_eType,
						 Object::ID::Invalid - i - 1);
	}
}

//	FUNCTION local
//	$$::_Sub::_addColumn -- create column object
//
//	NOTES
//
//	ARGUMENTS
//		Trans::Transaction& cTrans_
//		Schema::Table& cTable_
//		Column::Position iPosition_
//		const Object::Name& cName_
//		Column::Category::Value eCategory_
//		Common::DataType::Type eType_
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
_Sub::_addColumn(Trans::Transaction& cTrans_,
				 Table& cTable_,
				 Column::Position iPosition_,
				 const Object::Name& cName_,
				 Column::Category::Value eCategory_,
				 Common::DataType::Type eType_,
				 Schema::Object::ID::Value iObjectID_)
{
	// create column object as system column
	Column::Pointer pColumn =
		Column::createSystem(cTrans_,
							 cTable_, iPosition_, cName_, eCategory_,
							 Common::SQLData::create(eType_), Default(),
							 iObjectID_);

	// 列を表に加える
	cTable_.addColumn(pColumn, cTrans_);
}
} // namespace

// FUNCTION public
//	Schema::VirtualTable::VirtualTable -- constructor
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

VirtualTable::
VirtualTable()
	: Table(),
	  m_eCategory(Category::Unknown)
{}

// FUNCTION private
//	Schema::VirtualTable::VirtualTable -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Database& cDatabase_
//	Category::Value eCategory_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// constructor
VirtualTable::
VirtualTable(const Database& cDatabase_,
			 Category::Value eCategory_)
	: Table(cDatabase_, _NameTable[eCategory_]),
	  m_eCategory(eCategory_)
{}

// FUNCTION public
//	Schema::VirtualTable::~VirtualTable -- destructor
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

VirtualTable::
~VirtualTable()
{}

// FUNCTION public
//	Schema::VirtualTable::create -- create new object
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Database& cDatabase_
//	Category::Value eCategory_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
TablePointer
VirtualTable::
create(Trans::Transaction& cTrans_,
	   const Database& cDatabase_,
	   Category::Value eCategory_)
{
	TablePointer pResult = new VirtualTable(cDatabase_, eCategory_);
	// set as system table
	pResult->setStatus(Status::Persistent);
	pResult->setSystem(true);

	// create column objects
	(*_AddColumnsTable[eCategory_])(cTrans_, *pResult);

	return pResult;
}

// FUNCTION public
//	Schema::VirtualTable::getCategory -- get virtual table category from a table name
//
// NOTES
//
// ARGUMENTS
//	const Name& cName_
//	
// RETURN
//	VirtualTable::Category::Value
//
// EXCEPTIONS

//static
VirtualTable::Category::Value
VirtualTable::
getCategory(const Name& cName_)
{
	int i = static_cast<int>(Category::Unknown);
	for (++i; i < Category::ValueNum; ++i) {
		if (cName_ == _NameTable[i]) return static_cast<Category::Value>(i);
	}
	return Category::Unknown;
}

//
// Copyright (c) 2007, 2011, 2012, 2017, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
