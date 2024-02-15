// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Privilege.h -- Privilege object
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

#ifndef	__SYDNEY_SCHEMA_PRIVILEGE_H
#define	__SYDNEY_SCHEMA_PRIVILEGE_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/IntegerData.h"
#include "Common/Privilege.h"
#include "Common/UnicodeString.h"

_SYDNEY_BEGIN

namespace Statement
{
	class IdentifierList;
}
namespace Trans
{
	class Transaction;
	class TimeStamp;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class LogData;

//	CLASS
//	Schema::Privilege -- Privilege object
//
//	NOTES

class Privilege
	: public Schema::Object
{
public:
	friend class SystemTable::Privilege;
	using Object::getID;

	//	TYPEDEF public
	//	Schema::Privilege::Pointer -- ObjectPointer holding Privilege
	//
	//	NOTES
	typedef PrivilegePointer Pointer;

	struct Log {

		//	STRUCT
		//		Schema::Log::Value -- Common part of logical log data format for privilege operations
		//
		//	NOTES
		enum Value {
			ID = 0,							// ID as schema object
			UserID,							// User ID
			ObjectType,						// Category of target object (not used yet)
			ObjectID,						// identifier of target object (not used yet)
			Privilege,						// Privilege values
			ValueNum
		};

		//	STRUCT
		//		Schema::Log::Create -- Logical log format of Create privilege
		//
		//	NOTES
		struct Create {
			enum Value {
				Num = ValueNum
			};
		};

		//	STRUCT
		//		Schema::Log::Drop -- Logical log format of Drop privilege
		//
		//	NOTES
		struct Drop {
			enum Value {
				Num = ValueNum
			};
		};

		//	STRUCT
		//		Schema::Log::Alter -- Logical log format of Alter privilege
		//
		//	NOTES
		struct Alter {
			enum Value {
				PrevPrivilege = Privilege, // Privilege value before update
				PostPrivilege,			// Privilege value after update
				Num
			};
		};
	};


	///////////////////////////
	// Methods for Privilege //
	///////////////////////////

	// constructor
	Privilege();
	// constructor
	Privilege(Database& cDatabase_,
			  int iUserID_,
			  const ModVector<Common::Privilege::Value>& vecPrivilege_);
	// destructor
	SYD_SCHEMA_FUNCTION
	~Privilege();

	// create new instance from DataArrayData
	static Privilege*		getNewInstance(const Common::DataArrayData& cData_);

	// clear all the members
	void					clear();

	// alter privilege object according to grant/revoke statement
	static Pointer			alter(Database& cDatabase_,
								  const Statement::IdentifierList& cRoles_,
								  const Statement::IdentifierList& cGrantee_,
								  bool bIsGrant_,
								  ModVector<Common::Privilege::Value>& vecPrevValue_,
								  ModVector<Common::Privilege::Value>& vecPostValue_,
								  LogData& cLogData_,
								  Trans::Transaction& cTrans_);
	// create/alter privilege object according to grant statement(recovery)
	static Pointer			alter(Trans::Transaction& cTrans_,
								  Database& cDatabase_,
								  const LogData& cLogData_);

	// create privilege object according to create database
	static Pointer			create(Database& cDatabase_,
								   int iUserID_,
								   LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	// create privilege object according to create database(recovery)
	static Pointer			create(Trans::Transaction& cTrans_,
								   Database& cDatabase_,
								   const LogData& cLogData_);

	// delete privilege object according to revoke statement/drop user
	static Pointer			drop(Database& cDatabase_,
								 Privilege* pPrivilege_,
								 LogData& cLogData_,
								 Trans::Transaction& cTrans_);
	// drop privilege object according to drop user(recovery)
	static Pointer			drop(Trans::Transaction& cTrans_,
								 Database& cDatabase_,
								 const LogData& cLogData_);

	// Get privilege object
	SYD_SCHEMA_FUNCTION
	static Privilege*		get(ID::Value iID_, Database* pDatabase_,
								Trans::Transaction& cTrans_);
	SYD_SCHEMA_FUNCTION
	static Privilege*		get(ID::Value iID_, ID::Value iDatabaseID_,
								Trans::Transaction& cTrans_);

	// Operation done before persisting object
	static void				doBeforePersist(const Pointer& pPrivilege_,
											Status::Value eStatus_,
											bool bNeedToErase_,
											Trans::Transaction& cTrans_);

	// Operation done after persisting object
	static void				doAfterPersist(const Pointer& pPrivilege_,
										   Status::Value eStatus_,
										   bool bNeedToErase_,
										   Trans::Transaction& cTrans_);

	// Operation done after reading object from system table
	static void				doAfterLoad(const Pointer& pPrivilege_,
										Database& cDatabase_,
										Trans::Transaction& cTrans_);

	// accessors

	// User ID
	int						getUserID() const;
	void					setUserID(int iUserID_);

	// Privilege values
	const ModVector<Common::Privilege::Value>& getValue() const;
	void					setValue(const ModVector<Common::Privilege::Value>& vecValue_);
	static void				addValue(const ModVector<Common::Privilege::Value>& vecValue_,
									 ModVector<Common::Privilege::Value>& vecResult_);
	static bool				removeValue(const ModVector<Common::Privilege::Value>& vecValue_,
										ModVector<Common::Privilege::Value>& vecResult_);

	// Object type
	Common::Privilege::Object::Value
							getObjectType() const;
	void					setObjectType(Common::Privilege::Object::Value eType_);

	// Object IDs
	const ModVector<ID::Value>& getObjectID() const;
	void					setObjectID(const ModVector<ID::Value>& vecIDs_);

//	Object::
//	Timestamp				getTimestamp() const; // get timestamp
//	ID::Value				getID() const;		// get schema object id
//	ID::Value				getParentID() const;
//												// get parent object's id
//	const Name&				getName() const;	// get object name
//	Category::Value			getCategory() const;
//												// get object category
//
//	void					reset(Database& cDatabase_);
//	void					reset();
//												// reset composing objects
	
	SYD_SCHEMA_FUNCTION
	virtual void			serialize(ModArchive& archiver);
	virtual int				getClassID() const;

	// Create logical log data
	void					makeLogData(LogData& cLogData_) const;

	//////////////////////
	// get from logdata
	static ID::Value		getID(const LogData& cLogData_);
	static int				getUserID(const LogData& cLogData_);
	static const ModVector<Common::Privilege::Value>&
							getValue(const LogData& cLogData_,
									 int iElement_ = Log::Privilege);
	static Common::Privilege::Object::Value
							getObjectType(const LogData& cLogData_);
	static const ModVector<ID::Value>&
							getObjectID(const LogData& cLogData_);

	// Pack/unpack members
	virtual int				getMetaFieldNumber() const;
	virtual Meta::MemberType::Value
							getMetaMemberType(int iMemberID_) const;

	virtual Common::Data::Pointer packMetaField(int iMemberID_) const;
	virtual bool			unpackMetaField(const Common::Data* pData_, int iMemberID_);

protected:
private:
	// create privilege object according to grant statement
	static Pointer			create(Database& cDatabase_,
								   int iUserID_,
								   const ModVector<Common::Privilege::Value>& vecPrivilege_,
								   LogData& cLogData_,
								   Trans::Transaction& cTrans_);
	// modify privilege object according to grant/revoke statement
	static Pointer			alter(Database& cDatabase_,
								  Privilege* pPrivilege_,
								  const ModVector<Common::Privilege::Value>& vecPostValue_,
								  LogData& cLogData_,
								  Trans::Transaction& cTrans_);

	// check change of privilege value and set to the object
	bool					alter(const ModVector<Common::Privilege::Value>& vecValue_);

//	Object::
//	void					addTimestamp();		// add timestamp

	int		m_iUserID;
	ModVector<Common::Privilege::Value> m_vecPrivilege;
	Common::Privilege::Object::Value m_eObjectType;
	ModVector<ID::Value> m_vecObjectID;
};

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_PRIVILEGE_H

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
