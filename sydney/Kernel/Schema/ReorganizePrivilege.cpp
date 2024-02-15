// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizePrivilege.cpp -- Implementation of classes concerning with privilege reorganization
// 
// Copyright (c) 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
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
#include "Schema/AutoLatch.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/ReorganizePrivilege.h"
#include "Schema/SystemTable_Privilege.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"
#include "Exception/UserNotFound.h"

#include "Lock/Name.h"

#include "Os/AutoCriticalSection.h"

#include "Server/Session.h"

#include "Statement/GrantStatement.h"
#include "Statement/IdentifierList.h"
#include "Statement/RevokeStatement.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create privilege
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Create
		{
			enum Value
			{
				None,					// initial value
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Alter::Value -- Progress value for grant/revoke privilege
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Alter
		{
			enum Value
			{
				None,					// initial value
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Drop::Value -- Progress value for drop user
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Drop
		{
			enum Value
			{
				None,					// initial value
				Dropped,				// drop flag is set
				Logged,					// log data is stored
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizePrivilege::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Base::Base -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	bool bRedo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizePrivilege::Base::
Base(Trans::Transaction& cTrans_, Database* pDatabase_, bool bRedo_)
	: ReorganizeExecutor(cTrans_),
	  m_pDatabase(pDatabase_),
	  m_bRedo(bRedo_)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Base::~Base -- destructor
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

//virtual
Manager::SystemTable::ReorganizePrivilege::Base::
~Base()
{
	try {
		m_pDatabase->close(m_bRedo /* if redo, close in volatile mode */);
	} catch (...) {
		// ignore all the exception in destructor
		;
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Base::lock -- lock operation
//
// NOTES
//
// ARGUMENTS
//	Schema::Hold::Operation::Value eOperation_
//	bool bWeaken_ = false
//		If true, lock mode for meta table is weakened
//		Otherwise, lock mode for meta tuple is enstronged
//	bool bLockDatabase_ = false
//		If true, lock database object by the operation
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizePrivilege::Base::
lock(Schema::Hold::Operation::Value eOperation_,
	 bool bWeaken_ /* = false */, bool bLockDatabase_ /* = false */)
{
	// Convert the lock for the tuple in database system table
	// from read-for-write into the operation

	if (bWeaken_) {
		// Meta table has been locked by X, weaken it
		convert(getTransaction(), Hold::Target::MetaTable,
				Lock::Name::Category::Table, Hold::Operation::ReadWrite,
				Lock::Name::Category::Tuple, Hold::Operation::ReadWrite);
	} else {
		// Meta tuple has been locked by U, change it
		convert(getTransaction(), Hold::Target::MetaTuple,
				Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
				Lock::Name::Category::Tuple, eOperation_,
				getDatabase()->getID());
	}

	// Lock the logical log file of the database in which the target privilege exists
	// for operating tuples in database system privilege

	hold(getTransaction(), Hold::Target::LogicalLog,
		 Lock::Name::Category::Tuple, eOperation_,
		 0, Trans::Log::File::Category::Database, getDatabase());

	if (bLockDatabase_) {
		hold(getTransaction(), Hold::Target::Database,
			 Lock::Name::Category::Database, eOperation_,
			 getDatabase()->getID());
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizePrivilege::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::CreateStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizePrivilege::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   Server::Session* pSession_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pSession(pSession_), m_pLogData(0),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pPrivilege()
{}

// for redoing
Manager::SystemTable::ReorganizePrivilege::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pSession(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pPrivilege()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizePrivilege::Create::
~Create()
{
	if (m_pSession && (m_iStatus != Progress::Create::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Create::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizePrivilege::Create::
execute()
{
	if (m_pSession) { // normal case
		// prepare log data
		LogData cLogData;

		m_pPrivilege = Privilege::create(*getDatabase(),
										 m_pSession->getUserID(),
										 cLogData,
										 getTransaction());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreatePrivilege", "Created");

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreatePrivilege", "Logged");

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		m_pPrivilege = Privilege::create(getTransaction(),
										 *getDatabase(),
										 *m_pLogData);

		; _SYDNEY_ASSERT(m_pPrivilege.get());
	}

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Privilege(*pDatabase).store(getTransaction(), m_pPrivilege);
	m_iStatus = Progress::Create::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreatePrivilege", "Stored");

	// persist ObjectID value
	Schema::ObjectID::persist(getTransaction(), getDatabase());
			
	// return value
	Result::Value iResult = Result::None;
	if (m_pLogData == 0) { // do only for normal case
		// increment timestamp value
		addTimestamp(getTransaction());

		iResult = isCauseCheckpoint()
			? (Result::NeedCheckpoint | Result::NeedReCache)
			: Result::NeedReCache;
	}

	m_iStatus = Progress::Create::Succeeded;
	return iResult;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizePrivilege::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create privilege failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Create::Stored:
		{
			// drop object
			m_pPrivilege->Object::drop(false /* not in recovery */); 

			// store
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Privilege(*pDatabase).store(getTransaction(), m_pPrivilege);

			// persist objectID
			Schema::ObjectID::persist(getTransaction(), getDatabase());
			break;
		}
	case Progress::Create::None:
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizePrivilege::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::GrantStatement/RevokeStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizePrivilege::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Statement::GrantStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pRoles(pStatement_->getRole()),
	  m_pGrantee(pStatement_->getGrantee()),
	  m_bGrant(true),
	  m_pLogData(0),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pPrivilege(),
	  m_vecPrevValue(),
	  m_vecPostValue(Common::Privilege::Category::ValueNum,
					 Common::Privilege::None)
{}

Manager::SystemTable::ReorganizePrivilege::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Statement::RevokeStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pRoles(pStatement_->getRole()),
	  m_pGrantee(pStatement_->getGrantee()),
	  m_bGrant(false),
	  m_pLogData(0),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pPrivilege(),
	  m_vecPrevValue(),
	  m_vecPostValue(Common::Privilege::Category::ValueNum,
					 Common::Privilege::None)
{}

// for redoing
Manager::SystemTable::ReorganizePrivilege::Alter::
Alter(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pRoles(0), m_pGrantee(0), m_bGrant(true), m_pLogData(pLogData_),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pPrivilege(),
	  m_vecPrevValue(),
	  m_vecPostValue(Common::Privilege::Category::ValueNum,
					 Common::Privilege::None)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizePrivilege::Alter::
~Alter()
{
	if (m_pRoles && (m_iStatus != Progress::Alter::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Alter::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizePrivilege::Alter::
execute()
{
	if (m_pRoles) { // normal case
		// prepare log data
		LogData cLogData;

		m_pPrivilege = Privilege::alter(*getDatabase(),
										*m_pRoles,
										*m_pGrantee,
										m_bGrant,
										m_vecPrevValue,
										m_vecPostValue,
										cLogData,
										getTransaction());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterPrivilege", "Altered");

		if (m_pPrivilege.get() == 0) {
			// nothing to do
			return Result::None;
		}

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterPrivilege", "Logged");

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		m_pPrivilege = Privilege::alter(getTransaction(),
										*getDatabase(),
										*m_pLogData);
		if (m_pPrivilege.get() == 0) {
			// nothing to do
			return Result::None;
		}
	}

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Privilege(*pDatabase).store(getTransaction(), m_pPrivilege);
	m_iStatus = Progress::Alter::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterPrivilege", "Stored");

	// persist ObjectID value
	Schema::ObjectID::persist(getTransaction(), getDatabase());
			
	// return value
	Result::Value iResult = Result::None;
	if (m_pLogData == 0) { // do only for normal case
		// increment timestamp value
		addTimestamp(getTransaction());

		iResult = isCauseCheckpoint()
			? (Result::NeedCheckpoint | Result::NeedReCache)
			: Result::NeedReCache;
	}

	m_iStatus = Progress::Alter::Succeeded;
	return iResult;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Alter::undo -- error recovery
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
Manager::SystemTable::ReorganizePrivilege::Alter::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "grant/revoke privilege failed." << ModEndl;
	}

	if (m_vecPrevValue.isEmpty()) {
		// newly created -> drop it
		m_pPrivilege->Object::drop(false /* not in recovery */);
	} else {
		// recover original privilege
		m_pPrivilege->setValue(m_vecPrevValue);
	}

	switch (m_iStatus) {
	case Progress::Alter::Stored:
		{
			// make dirty again
			m_pPrivilege->touch();
			// store
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Privilege(*pDatabase).store(getTransaction(), m_pPrivilege);

			// persist objectID value because an object with the new Id has been persisted
			Schema::ObjectID::persist(getTransaction(), getDatabase());
			break;
		}
	case Progress::Alter::None:
		{
			// clear dirty flag
			m_pPrivilege->untouch();
			// thru.
		}
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizePrivilege::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	int iUserID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizePrivilege::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 int iUserID_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_iUserID(iUserID_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pPrivilege()
{}

// for redoing
Manager::SystemTable::ReorganizePrivilege::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_iUserID(-1), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pPrivilege()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizePrivilege::Drop::
~Drop()
{
	// drop's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Drop::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizePrivilege::Drop::
execute()
{
	// Drop's error recovery is retrying,
	// so error recovery is done in catch clause.

	try {
		if (m_pLogData == 0) { // normal case
			// obtain privilege object
			m_pPrivilege = static_cast<const Privilege*>(
							 getDatabase()->getPrivilege(getTransaction(), m_iUserID));
			if (m_pPrivilege.get() == 0) {
				// Do nothing
				return Result::None;
			}

			// lock database
			lock(Hold::Operation::Drop,
				 false /* not weaken */,
				 true /* lock database */);

			// reset progress status
			// NOTE: reset here because drop flag should be reverted when error occurs
			m_iStatus = Progress::Drop::None;

			// prepare log data
			LogData cLogData(LogData::Category::DropPrivilege);

			// set drop mark to the object
			// log data is also filled.
			(void) Privilege::drop(*getDatabase(), m_pPrivilege.get(), cLogData, getTransaction());
			m_iStatus = Progress::Drop::Dropped;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropPrivilege", "Dropped");

			// store log data to database log file
			storeDatabaseLog(cLogData, getDatabase());
			m_iStatus = Progress::Drop::Logged;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropPrivilege", "Logged");

		} else { // redo
			; _SYDNEY_ASSERT(m_pLogData);

			m_pPrivilege = Privilege::drop(getTransaction(), *getDatabase(), *m_pLogData);

			if (m_pPrivilege.get() == 0) {
				// nothing to do
				return Result::None;
			}
		}

		// persist system table
		Database* pDatabase = getDatabase();
		Schema::SystemTable::Privilege(*pDatabase).store(getTransaction(), m_pPrivilege);
		m_iStatus = Progress::Drop::Stored;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropPrivilege", "Stored");

		// return value
		Result::Value iResult = Result::None;
		if (m_pLogData == 0) { // do only for normal case
			// increment timestamp value
			addTimestamp(getTransaction());

			// if parameter Schema_CauseCheckpoint is set, cause checkpoint after commit
			iResult = isCauseCheckpoint()
				? (Result::NeedCheckpoint | Result::NeedReCache)
				: Result::NeedReCache;
		}

		m_iStatus = Progress::Drop::Succeeded;
		return iResult;

	} catch (...) {
		if (m_pLogData == 0 && (m_iStatus != Progress::Drop::Succeeded)) {
			bool bRetrySucceeded = false;
			bool bUndo_ = false; // used in the macro below

			_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

			bRetrySucceeded = retry();

			_END_REORGANIZE_RECOVERY(getDatabase()->getID());

			// if retry has been succeeded, treat as success case
			if (bRetrySucceeded) {
				Common::Thread::resetErrorCondition();
				m_iStatus = Progress::Drop::Succeeded;
				return isCauseCheckpoint()
					? (Result::NeedCheckpoint | Result::NeedReCache)
					: Result::NeedReCache;
			} else {
				_SYDNEY_RETHROW;
			}
		} else { // in redoing
			_SYDNEY_RETHROW;
		}
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePrivilege::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizePrivilege::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop privilege failed." << ModEndl;
	}
	bool bResult = false;

	// error recovery of drop is retrying
	// unless the process has been failed before storing log data
	switch (m_iStatus) {
	case Progress::Drop::Logged:
		{
			// wait for a while before retrying
			retryWait();

			SydErrorMessage << "retry drop privilege." << ModEndl;
			// persist again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Privilege(*pDatabase).store(getTransaction(), m_pPrivilege);
			SydErrorMessage << "retry drop privilege succeeded." << ModEndl;
			// thru.
		}
	case Progress::Drop::Stored:
		{
			// incremint timestamp value again
			addTimestamp(getTransaction());

			// retrying has been succeeded
			bResult = true;
			break;
		}
	case Progress::Drop::Dropped:
		{
			// just cancel drop flag
			m_pPrivilege->undoDrop();
			// thru.
		}
	case Progress::Drop::None:
	default:
		{
			break;
		}
	}
	return bResult;
}

//
// Copyright (c) 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
