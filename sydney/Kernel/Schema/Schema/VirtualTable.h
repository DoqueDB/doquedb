// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// VirtualTable.h -- Virtual tables
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_VIRTUALTABLE_H
#define	__SYDNEY_SCHEMA_VIRTUALTABLE_H

#include "Schema/Module.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_SCHEMA_BEGIN

class Database;

//	CLASS
//	Schema::VirtualTable -- Class for virtual tables
//
//	NOTES

class VirtualTable
	: public	Table
{
public:
	typedef Table Super;

	//	ENUM
	//	Schema::VirtualTable::Category::Value -- category of virtual tables
	//
	//	NOTES

	struct Category {
		enum Value {
			Unknown,
			User,						// SYSTEM_USER
			Session,					// SYSTEM_SESSION
			ValueNum
		};
	};

	// constructor
	VirtualTable();
	// destructor
	~VirtualTable();

	// is this table virtual?
	virtual bool isVirtual() const {return true;}

	// create new object
	static TablePointer create(Trans::Transaction& cTrans_,
							   const Database& cDatabase_,
							   Category::Value eCategory_);

	// get virtual table category
	Category::Value getCategory() const {return m_eCategory;}

	// get virtual table category from a table name
	static Category::Value getCategory(const Name& cName_);

	// this class will not be serialized, so following virtualmethods are not overriden
//	virtual void			serialize(ModArchive& archiver);
//	virtual int				getClassID() const;

	// this class will not be packed, so getMetaFieldNumberserialize and getClassID is not overriden.
//	virtual int				getMetaFieldNumber() const;
//	virtual Meta::MemberType::Value
//							getMetaMemberType(int iMemberID_) const;
//	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
//	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

protected:
private:
	VirtualTable(const Database& cDatabase_,
				 Category::Value eCategory_);

	Category::Value m_eCategory;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_VIRTUALTABLE_H

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
