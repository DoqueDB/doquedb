// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Privilege.cpp -- Privilege object
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Privilege.h"
#include "Schema/AutoRWLock.h"
#include "Schema/Database.h"
#include "Schema/LogData.h"
#include "Schema/Recovery.h"
#include "Schema/SystemTable_Privilege.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"

#include "Server/Manager.h"

#include "Statement/Identifier.h"
#include "Statement/IdentifierList.h"

#include "ModAlgorithm.h"
#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	FUNCTION public
//	Schema::Privilege::Privilege -- constructor
//
//	NOTES
//
//	ARGUMENTS
//		Nothing
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

Privilege::
Privilege()
	: Object(Object::Category::Privilege),
	  m_iUserID(-1),
	  m_vecPrivilege(),
	  m_eObjectType(Common::Privilege::Object::Unknown),
	  m_vecObjectID()
{ }

// FUNCTION public
//	Schema::Privilege::Privilege -- constructor
//
// NOTES
//
// ARGUMENTS
//	Database& cDatabase_
//	int iUserID_
//	const ModVector<Common::Privilege::Value>& vecPrivilege_
//	Common::Privilege::Object::Value eObjectType_
//	const ModVector<ID::Value>& vecObjectID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Privilege::
Privilege(Database& cDatabase_,
		  int iUserID_,
		  const ModVector<Common::Privilege::Value>& vecPrivilege_)
	: Object(Object::Category::Privilege, cDatabase_.getScope(),
			 Object::Status::Unknown,
			 ID::Invalid, cDatabase_.getID(), cDatabase_.getID()),
	  m_iUserID(iUserID_),
	  m_vecPrivilege(vecPrivilege_),
	  m_eObjectType(Common::Privilege::Object::Unknown),
	  m_vecObjectID()
{
}

//	FUNCTION public
//	Schema::Privilege::~Privilege -- destructor
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

Privilege::
~Privilege()
{ }

// FUNCTION public
//	Schema::Privilege::getNewInstance -- create new instance from DataArrayData
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData& cData_
//	
// RETURN
//	Privilege*
//
// EXCEPTIONS

// static
Privilege*
Privilege::
getNewInstance(const Common::DataArrayData& cData_)
{
	ModAutoPointer<Privilege> pObject = new Privilege;
	pObject->unpack(cData_);
	return pObject.release();
}

//	FUNCTION public
//	Schema::Privilege::clear -- clear all members
//
//	NOTES
//
//	ARGUMENTS
//		Nothing
//
//	RETURN
//		Nothing
//
//	EXCEPTIONS

void
Privilege::
clear()
{
	m_iUserID = -1;
	m_vecPrivilege.clear();
	m_eObjectType = Common::Privilege::Object::Unknown;
	m_vecObjectID.clear();
}

// FUNCTION public
//	Schema::Privilege::alter -- create/alter/drop privilege object according to grant/revoke statement
//
// NOTES
//
// ARGUMENTS
//	Database& cDatabase_
//	const Statement::IdentifierList& cRoles_
//	const Statement::IdentifierList& cGrantee_
//	bool bIsGrant_
//	ModVector<Common::Privilege::Value>& vecPrevValue_
//	ModVector<Common::Privilege::Value>& vecPostValue_
//	LogData& cLogData_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Privilege::Pointer
//
// EXCEPTIONS

//static
Privilege::Pointer
Privilege::
alter(Database& cDatabase_,
	  const Statement::IdentifierList& cRoles_,
	  const Statement::IdentifierList& cGrantee_,
	  bool bIsGrant_,
	  ModVector<Common::Privilege::Value>& vecPrevValue_,
	  ModVector<Common::Privilege::Value>& vecPostValue_,
	  LogData& cLogData_,
	  Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(cRoles_.getCount() > 0);
	; _SYDNEY_ASSERT(cGrantee_.getCount() > 0);

	// for now only one grantee is supported
	if (cGrantee_.getCount() > 1) {
		SydInfoMessage << "GRANT/REVOKE for more than one users is not supported." << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// create privilege value from the roles
	ModVector<Common::Privilege::Value> vecRoleValue(Common::Privilege::Category::ValueNum,
													 Common::Privilege::None);
	for (int i = 0; i < cRoles_.getCount(); ++i) {
		; _SYDNEY_ASSERT(cRoles_.getIdentifierAt(i));
		const ModUnicodeString* pRoleName = cRoles_.getIdentifierAt(i)->getIdentifier();
		; _SYDNEY_ASSERT(pRoleName);

		cDatabase_.getRolePrivilege(cTrans_, *pRoleName, vecRoleValue);
	}

	; _SYDNEY_ASSERT(cGrantee_.getCount() == 1);

	// get grantee id by name
	; _SYDNEY_ASSERT(cGrantee_.getIdentifierAt(0));
	const ModUnicodeString* pUserName = cGrantee_.getIdentifierAt(0)->getIdentifier();
	; _SYDNEY_ASSERT(pUserName);

	if (Server::Manager::isSuperUser(*pUserName)) {
		// if specified user is superuser, grant/revoke is not applicable
		SydInfoMessage << "Grant/Revoke for " << *pUserName << " is not allowed." << ModEndl;
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	int iUserID = Server::Manager::getUserID(*pUserName);

	// get privilege object if exists
	Privilege* pPrivilege = cDatabase_.getPrivilege(cTrans_, iUserID);
	if (pPrivilege) {
		// modify privilege value
		vecPrevValue_ = pPrivilege->getValue();
		vecPostValue_ = pPrivilege->getValue();
		if (bIsGrant_) {
			// create merged privilege value
			Privilege::addValue(vecRoleValue, vecPostValue_);
		} else {
			// create removed privilege value
			if (Privilege::removeValue(vecRoleValue, vecPostValue_)) {
				// all the privilege is disabled -> drop object
				return Privilege::drop(cDatabase_, pPrivilege, cLogData_, cTrans_);
			}
		}
		return Privilege::alter(cDatabase_, pPrivilege,
								vecPostValue_,
								cLogData_,
								cTrans_);
	} else {
		if (bIsGrant_) {
			// create new privilege
			vecPostValue_ = vecRoleValue;
			return Privilege::create(cDatabase_, iUserID,
									 vecPostValue_,
									 cLogData_,
									 cTrans_);
		} else {
			// do nothing
			return Pointer();
		}
	}
}

// FUNCTION public
//	Schema::Privilege::alter -- create/alter privilege object according to grant statement(recovery)
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	const LogData& cLogData_
//	
// RETURN
//	Privilege::Pointer
//
// EXCEPTIONS

//static
Privilege::Pointer
Privilege::
alter(Trans::Transaction& cTrans_,
	  Database& cDatabase_,
	  const LogData& cLogData_)
{
	// get schema object id from log data
	ObjectID::Value id = getID(cLogData_);

	// get privilege object from the id
	Privilege* pPrivilege = cDatabase_.getPrivilege(cTrans_, id);

	// get final effective privilege value
	ModVector<Common::Privilege::Value> vecValue;
	Manager::RecoveryUtility::PrivilegeValue::getEffectiveValue(cLogData_,
																Log::Alter::PostPrivilege,
																id,
																cDatabase_.getName(),
																vecValue);

	if (pPrivilege->alter(vecValue)) {
		return Pointer(syd_reinterpret_cast<const Privilege*>(pPrivilege));
	} else {
		return Pointer();
	}
}

// FUNCTION public
//	Schema::Privilege::create -- create privilege object according to create database
//
// NOTES
//
// ARGUMENTS
//	Database& cDatabase_
//	int iUserID_
//	LogData& cLogData_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Privilege::Pointer
//
// EXCEPTIONS

//static
Privilege::Pointer
Privilege::
create(Database& cDatabase_,
	   int iUserID_,
	   LogData& cLogData_,
	   Trans::Transaction& cTrans_)
{
	// create new privilege with full-privilege
	ModVector<Common::Privilege::Value> vecValue(Common::Privilege::Category::ValueNum,
												 Common::Privilege::All);
	return Privilege::create(cDatabase_, iUserID_,
							 vecValue,
							 cLogData_,
							 cTrans_);
}

// FUNCTION public
//	Schema::Privilege::create -- create privilege object according to create database(recovery)
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database& cDatabase_
//	const LogData& cLogData_
//	
// RETURN
//	Pointer
//
// EXCEPTIONS

//static
Privilege::Pointer
Privilege::
create(Trans::Transaction& cTrans_,
	   Database& cDatabase_,
	   const LogData& cLogData_)
{
	// get schema object id from log data
	ObjectID::Value id = getID(cLogData_);

	// get final effective privilege value
	ModVector<Common::Privilege::Value> vecValue;
	Manager::RecoveryUtility::PrivilegeValue::getEffectiveValue(cLogData_, Log::Privilege, id,
																cDatabase_.getName(),
																vecValue);
	// create new object
	Pointer pResult = new Privilege(cDatabase_,
									getUserID(cLogData_),
									vecValue);
	// take consistency of object ID and set it
	pResult->Object::create(cTrans_, id);

	return pResult;
}

// FUNCTION private
//	Schema::Privilege::drop -- delete privilege object according to revoke statement
//
// NOTES
//
// ARGUMENTS
//	Database& cDatabase_
//	Privilege* pPrivilege_
//	LogData& cLogData_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Privilege::Pointer
//
// EXCEPTIONS

//static
Privilege::Pointer
Privilege::
drop(Database& cDatabase_,
	 Privilege* pPrivilege_,
	 LogData& cLogData_,
	 Trans::Transaction& cTrans_)
{
	// set subcategory of log data
	cLogData_.setSubCategory(LogData::Category::DropPrivilege);
	// create log data
	pPrivilege_->makeLogData(cLogData_);

	// set drop flag
	pPrivilege_->Object::drop();

	return Pointer(syd_reinterpret_cast<const Privilege*>(pPrivilege_));
}

// drop privilege object according to drop user(recovery)
//static
Privilege::Pointer
Privilege::
drop(Trans::Transaction& cTrans_,
	 Database& cDatabase_,
	 const LogData& cLogData_)
{
	// get schema object id from log data
	ObjectID::Value id = getID(cLogData_);

	// get privilege object from the id
	Privilege* pPrivilege = cDatabase_.getPrivilege(cTrans_, id);

	// set drop flag
	pPrivilege->Object::drop();

	return Pointer(syd_reinterpret_cast<const Privilege*>(pPrivilege));
}

// FUNCTION public
//	Schema::Privilege::get -- Get privilege object
//
// NOTES
//
// ARGUMENTS
//	ID::Value iID_
//	Database* pDatabase_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Privilege*
//
// EXCEPTIONS

//static
Privilege*
Privilege::
get(ID::Value iID_, Database* pDatabase_,
	Trans::Transaction& cTrans_)
{
	return pDatabase_->getPrivilege(cTrans_, iID_);
}

//static
Privilege*
Privilege::
get(ID::Value iID_, ID::Value iDatabaseID_,
	Trans::Transaction& cTrans_)
{
	return get(iID_, Database::get(iDatabaseID_, cTrans_), cTrans_);
}

// Operation done before persisting object
//static
void
Privilege::
doBeforePersist(const Pointer& pPrivilege_,
				Status::Value eStatus_,
				bool bNeedToErase_,
				Trans::Transaction& cTrans_)
{
	; // do nothing
}

// Operation done after persisting object
//static
void
Privilege::
doAfterPersist(const Pointer& pPrivilege_,
			   Status::Value eStatus_,
			   bool bNeedToErase_,
			   Trans::Transaction& cTrans_)
{
	; _SYDNEY_ASSERT(pPrivilege_.get());

	// Save database ID here because pPrivilege_ might be deleted
	const ObjectID::Value iDBID = pPrivilege_->getDatabaseID();

	switch (eStatus_) {
	case Status::Created:
	case Status::Mounted:
	case Status::DeleteCanceled:
	{
		// Add this object to database
		Database* pDatabase = pPrivilege_->getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);
		// Privilege is not stored in cache
		(void) pDatabase->addPrivilege(pPrivilege_, cTrans_);
		break;
	}
	case Status::Changed:
	case Status::CreateCanceled:
		break;

	case Status::Deleted:
	case Status::DeletedInRecovery:
	{
		// Change status to 'really deleted'

		pPrivilege_->setStatus(Status::ReallyDeleted);

		// Erase from database
		Database* pDatabase = pPrivilege_->getDatabase(cTrans_);
		; _SYDNEY_ASSERT(pDatabase);
		(void) pDatabase->erasePrivilege(pPrivilege_->getID());
		break;
	}
	default:

		// 何もしない

		return;
	}

	// システム表の状態を変える
	SystemTable::unsetStatus(iDBID, Object::Category::Privilege);
}

// Operation done after reading object from system table
//static
void
Privilege::
doAfterLoad(const Pointer& pPrivilege_,
			Database& cDatabase_,
			Trans::Transaction& cTrans_)
{
	// Add this object to database
	(void) cDatabase_.addPrivilege(pPrivilege_, cTrans_);
}

// FUNCTION public
//	Schema::Privilege::getUserID -- User ID
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Privilege::
getUserID() const
{
	return m_iUserID;
}

// FUNCTION public
//	Schema::Privilege::setUserID -- 
//
// NOTES
//
// ARGUMENTS
//	int iUserID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Privilege::
setUserID(int iUserID_)
{
	m_iUserID = iUserID_;
}

// FUNCTION public
//	Schema::Privilege::getValue -- Privilege values
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const ModVector<Common::Privilege::Value>&
//
// EXCEPTIONS

const ModVector<Common::Privilege::Value>&
Privilege::
getValue() const
{
	return m_vecPrivilege;
}

// FUNCTION public
//	Schema::Privilege::setValue -- 
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::Privilege::Value>& vecValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Privilege::
setValue(const ModVector<Common::Privilege::Value>& vecValue_)
{
	m_vecPrivilege = vecValue_;
}

// FUNCTION public
//	Schema::Privilege::addValue -- merge privilege values
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::Privilege::Value>& vecValue_
//	ModVector<Common::Privilege::Value>& vecResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Privilege::
addValue(const ModVector<Common::Privilege::Value>& vecValue_,
		 ModVector<Common::Privilege::Value>& vecResult_)
{
	ModSize n = vecResult_.getSize();
	ModSize m = vecValue_.getSize();
	ModSize i = 0;
	for (; i < n; ++i) {
		vecResult_[i] |= vecValue_[i];
	}
	for (; i < m; ++i) {
		vecResult_.pushBack(vecValue_[i]);
	}
}

// FUNCTION public
//	Schema::Privilege::removeValue -- remove privilege values
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::Privilege::Value>& vecValue_
//	ModVector<Common::Privilege::Value>& vecResult_
//	
// RETURN
//	bool true if all the privileges become none
//
// EXCEPTIONS

//static
bool
Privilege::
removeValue(const ModVector<Common::Privilege::Value>& vecValue_,
			ModVector<Common::Privilege::Value>& vecResult_)
{
	bool bResult = true;
	ModSize n = vecResult_.getSize();
	ModSize i = 0;
	for (; i < n; ++i) {
		if (vecValue_[i] != Common::Privilege::None) {
			vecResult_[i] &= ~(vecValue_[i]);
		}
		if (bResult && vecResult_[i] != Common::Privilege::None) {
			bResult = false;
		}
	}
	return bResult;
}

// FUNCTION public
//	Schema::Privilege::getObjectType -- Object type
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

Common::Privilege::Object::Value
Privilege::
getObjectType() const
{
	return m_eObjectType;
}

// FUNCTION public
//	Schema::Privilege::setObjectType -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Privilege::Object::Value eType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Privilege::
setObjectType(Common::Privilege::Object::Value eType_)
{
	m_eObjectType = eType_;
}

// FUNCTION public
//	Schema::Privilege::getObjectID -- Object IDs
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

const ModVector<ObjectID::Value>&
Privilege::
getObjectID() const
{
	return m_vecObjectID;
}

// FUNCTION public
//	Schema::Privilege::setObjectID -- 
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
Privilege::
setObjectID(const ModVector<ID::Value>& vecIDs_)
{
	m_vecObjectID = vecIDs_;
}

// FUNCTION public
//	Schema::Privilege::serialize -- serialize	
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Privilege::
serialize(ModArchive& cArchiver_)
{
	// never serialized
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Schema::Privilege::getClassID -- get class id for privilege class
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Privilege::
getClassID() const
{
	return Externalizable::Category::Privilege +
		Common::Externalizable::SchemaClasses;
}

// FUNCTION public
//	Schema::Privilege::makeLogData -- Create logical log data
//
// NOTES
//
// ARGUMENTS
//	LogData& cLogData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Privilege::
makeLogData(LogData& cLogData_) const
{
	// Common log data
	// 1. ID
	// 2. UserID
	// 3. ObjectType
	// 4. ObjectID
	// 5. Privilege values
	cLogData_.addID(getID());
	cLogData_.addInteger(getUserID());
	cLogData_.addInteger(static_cast<int>(getObjectType()));
	cLogData_.addIDs(getObjectID());
	cLogData_.addUnsignedIntegers(getValue());
}

// FUNCTION public
//	Schema::Privilege::getID -- get ID from log data
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	
// RETURN
//	Privilege::ID::Value
//
// EXCEPTIONS

//static
Privilege::ID::Value
Privilege::
getID(const LogData& cLogData_)
{
	return cLogData_.getID(Log::ID);
}

// FUNCTION public
//	Schema::Privilege::getUserID -- get user id from log data
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	
// RETURN
//	int
//
// EXCEPTIONS

//static
int
Privilege::
getUserID(const LogData& cLogData_)
{
	return cLogData_.getInteger(Log::UserID);
}

// FUNCTION public
//	Schema::Privilege::getValue -- get privilege values from log data
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	int iElement_ /* = Log::Privilege */
//	
// RETURN
//	const ModVector<Common::Privilege::Value>&
//
// EXCEPTIONS

//static
const ModVector<Common::Privilege::Value>&
Privilege::
getValue(const LogData& cLogData_,
		 int iElement_ /* = Log::Privilege */)
{
	return cLogData_.getUnsignedIntegers(iElement_);	
}

// FUNCTION public
//	Schema::Privilege::getObjectType -- get object type from log data
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	
// RETURN
//	Common::Privilege::Object::Value
//
// EXCEPTIONS

//static
Common::Privilege::Object::Value
Privilege::
getObjectType(const LogData& cLogData_)
{
	int iValue = cLogData_.getInteger(Log::ObjectType);
	if (iValue < 0 || iValue > Common::Privilege::Object::ValueNum) {
		_SYDNEY_THROW0(Exception::LogItemCorrupted);
	}		
	return static_cast<Common::Privilege::Object::Value>(iValue);
}

// FUNCTION public
//	Schema::Privilege::getObjectID -- get object id from log data
//
// NOTES
//
// ARGUMENTS
//	const LogData& cLogData_
//	
// RETURN
//	const ModVector<Object::ID::Value>&
//
// EXCEPTIONS

//static
const ModVector<Object::ID::Value>&
Privilege::
getObjectID(const LogData& cLogData_)
{
	return cLogData_.getIDs(Log::ObjectID);
}

////////////////////////////////////////////////////////////
// Definition for system table
////////////////////////////////////////////////////////////

// System table schema for privilege is as follows;
// create table Privilege (
//		ID		id,
//		userID	int,
//		value	<unsigned int array>, -- UnsignedIntegerArrayData
//		type	int,
//		objectID <id array>,
//		time	timestamp
// )

namespace
{
#define _DEFINE0(_type_) \
	Meta::Definition<Privilege>(Meta::MemberType::_type_)
#define _DEFINE2(_type_, _get_, _set_) \
	Meta::Definition<Privilege>(Meta::MemberType::_type_, &Privilege::_get_, &Privilege::_set_)

	Meta::Definition<Privilege> _vecDefinition[] =
	{
		_DEFINE0(FileOID),		// FileOID
		_DEFINE0(ObjectID),		// ID
		_DEFINE2(Integer, getUserID, setUserID), // UserID
		_DEFINE2(UnsignedIntegerArray, getValue, setValue), // Privilege values
		_DEFINE2(Integer, getObjectType, setObjectType), // Type
		_DEFINE2(IDArray, getObjectID, setObjectID), // objectID
		_DEFINE0(Timestamp),	// Timestamp
	};
#undef _DEFINE0
#undef _DEFINE2
}

// FUNCTION public
//	Schema::Privilege::getMetaFieldNumber -- get number of fields in system table file
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Privilege::
getMetaFieldNumber() const
{
	return static_cast<int>(Meta::Privilege::MemberNum);
}

// FUNCTION public
//	Schema::Privilege::getMetaMemberType -- get member type for specified position
//
// NOTES
//
// ARGUMENTS
//	int iMemberID_
//	
// RETURN
//	Meta::MemberType::Value
//
// EXCEPTIONS

//virtual
Meta::MemberType::Value
Privilege::
getMetaMemberType(int iMemberID_) const
{
	return _vecDefinition[iMemberID_].m_eType;
}

// FUNCTION public
//	Schema::Privilege::packMetaField -- convert member value into common::data in storing to systme table
//
// NOTES
//
// ARGUMENTS
//	int iMemberID_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//virtual
Common::Data::Pointer
Privilege::
packMetaField(int iMemberID_) const
{
	Meta::Definition<Privilege>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Use Object's method
		return Object::packMetaField(iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			return pack((this->*(cDef.m_funcGet._integer))());
		}
	case Meta::MemberType::UnsignedIntegerArray:
		{
			return pack((this->*(cDef.m_funcGet._unsignedIntegers))());
		}
	case Meta::MemberType::IDArray:
		{
			return pack((this->*(cDef.m_funcGet._ids))());
		}
	default:
		break;
	}
	; _SYDNEY_ASSERT(false);
	return Common::Data::Pointer();
}

// FUNCTION public
//	Schema::Privilege::unpackMetaField -- convert common::data into member value in loading from systme table
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	int iMemberID_
//	
// RETURN
//	bool
//
// EXCEPTIONS

///virtual
bool
Privilege::
unpackMetaField(const Common::Data* pData_, int iMemberID_)
{
	Meta::Definition<Privilege>& cDef = _vecDefinition[iMemberID_];

	if (cDef.m_eType < Meta::MemberType::UseObjectMax) {
		// Use Object's method
		return Object::unpackMetaField(pData_, iMemberID_);
	}

	switch (cDef.m_eType) {
	case Meta::MemberType::Integer:
		{
			int iValue;
			if (unpack(pData_, iValue)) {
				(this->*(cDef.m_funcSet._integer))(iValue);
				return true;
			}
			break;
		}
	case Meta::MemberType::UnsignedIntegerArray:
		{
			ModVector<unsigned int> vecValue;
			if (unpack(pData_, vecValue)) {
				(this->*(cDef.m_funcSet._unsignedIntegers))(vecValue);
				return true;
			}
			break;
		}
	case Meta::MemberType::IDArray:
		{
			ModVector<ID::Value> vecValue;
			if (unpack(pData_, vecValue)) {
				(this->*(cDef.m_funcSet._ids))(vecValue);
				return true;
			}
			break;
		}
	default:
		break;
	}
	return false;
}

// FUNCTION private
//	Schema::Privilege::create -- create privilege object according to grant statement
//
// NOTES
//
// ARGUMENTS
//	Database& cDatabase_
//	int iUserID_
//	const ModVector<Common::Privilege::Value>& vecPrivilege_
//	Common::Privilege::Object::Value eObjectType_
//	const ModVector<ID::Value>& vecObjectID_
//	LogData& cLogData_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Privilege::Pointer
//
// EXCEPTIONS

//static
Privilege::Pointer
Privilege::
create(Database& cDatabase_,
	   int iUserID_,
	   const ModVector<Common::Privilege::Value>& vecPrivilege_,
	   LogData& cLogData_,
	   Trans::Transaction& cTrans_)
{
	// create new object
	Pointer pResult = new Privilege(cDatabase_, iUserID_, vecPrivilege_);
	// assign new ID
	pResult->Object::create(cTrans_);

	// set subcategory of log data
	cLogData_.setSubCategory(LogData::Category::CreatePrivilege);

	// create log data
	pResult->makeLogData(cLogData_);

	return pResult;
}

// FUNCTION private
//	Schema::Privilege::alter -- modify privilege object according to grant/revoke statement
//
// NOTES
//
// ARGUMENTS
//	Database& cDatabase_
//	const Privilege* pPrivilege_
//	const ModVector<Common::Privilege::Value>& vecPostValue_
//	LogData& cLogData_
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Privilege::Pointer
//
// EXCEPTIONS

//static
Privilege::Pointer
Privilege::
alter(Database& cDatabase_,
	  Privilege* pPrivilege_,
	  const ModVector<Common::Privilege::Value>& vecPostValue_,
	  LogData& cLogData_,
	  Trans::Transaction& cTrans_)
{
	// set subcategory of log data
	cLogData_.setSubCategory(LogData::Category::AlterPrivilege);
	// create log data
	pPrivilege_->makeLogData(cLogData_);
	// add post privilege value
	cLogData_.addUnsignedIntegers(vecPostValue_);

	if (pPrivilege_->alter(vecPostValue_)) {
		return Pointer(syd_reinterpret_cast<const Privilege*>(pPrivilege_));
	} else {
		return Pointer();
	}
}

// FUNCTION private
//	Schema::Privilege::alter -- check change of privilege value and set to the object
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Common::Privilege::Value>& vecValue_
//	
// RETURN
//	bool	true if value is changed
//
// EXCEPTIONS

bool
Privilege::
alter(const ModVector<Common::Privilege::Value>& vecValue_)
{
	// compare current value and specified value
	const ModVector<Common::Privilege::Value>& vecValue0 = getValue();

	ModSize n0 = vecValue0.getSize();
	ModSize n1 = vecValue_.getSize();
	ModSize n = ModMin(n0, n1);

	bool bDiff = false;
	ModSize i = 0;
	for (; i < n; ++i) {
		if (vecValue0[i] != vecValue_[i]) {
			bDiff = true;
			break;
		}
	}
	if (!bDiff && n < n0) {
		for (; i < n0; ++i) {
			if (vecValue0[i] != Common::Privilege::None) {
				bDiff = true;
				break;
			}
		}
	}
	if (!bDiff && n < n1) {
		for (; i < n1; ++i) {
			if (vecValue_[i] != Common::Privilege::None) {
				bDiff = true;
				break;
			}
		}
	}
	if (bDiff) {
		// value is changed
		setValue(vecValue_);
		touch();
	}
	return bDiff;
}

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
