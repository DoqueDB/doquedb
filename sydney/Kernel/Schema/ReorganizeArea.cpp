// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeArea.cpp -- Implementation of classes concerning with area reorganization
// 
// Copyright (c) 2006, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
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

#include "Schema/Area.h"
#include "Schema/AreaContent.h"
#include "Schema/AutoLatch.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/ReorganizeArea.h"
#include "Schema/SystemTable_Area.h"
#include "Schema/SystemTable_AreaContent.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/AreaNotFound.h"
#include "Exception/InvalidPath.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "Os/AutoCriticalSection.h"

#include "Statement/AreaDefinition.h"
#include "Statement/DropAreaStatement.h"
#include "Statement/AlterAreaAction.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create area
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Create
		{
			enum Value
			{
				None,					// initial value
				PathReserved,			// paths are reserved
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Drop::Value -- Progress value for drop area
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

		// ENUM
		// $$::Progress::Alter::Value -- Progress value for alter area
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Alter
		{
			enum Value
			{
				None,					// initial value
				PathReserved,			// path is reserved
				Moved,					// directry is moved
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeArea::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Base::Base -- constructor
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

Manager::SystemTable::ReorganizeArea::Base::
Base(Trans::Transaction& cTrans_, Database* pDatabase_, bool bRedo_)
	: ReorganizeExecutor(cTrans_),
	  m_pDatabase(pDatabase_),
	  m_bRedo(bRedo_)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeArea::Base::
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
//	Schema::Manager::SystemTable::ReorganizeArea::Base::lock -- lock operation
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
Manager::SystemTable::ReorganizeArea::Base::
lock(Schema::Hold::Operation::Value eOperation_,
	 bool bWeaken_ /* = false */, bool bLockDatabase_ /* = false */)
{
	// Convert the lock for the tuple in database system table
	// from read-for-write into the operation

	if (bWeaken_) {
		// Meta table has been locked by SIX, weaken it
		convert(getTransaction(), Hold::Target::MetaTable,
				Lock::Name::Category::Tuple, Hold::Operation::ReadWrite,
				Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	} else {
		// Meta tuple has been locked by U, change it
		convert(getTransaction(), Hold::Target::MetaTuple,
				Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
				Lock::Name::Category::Tuple, eOperation_,
				getDatabase()->getID());
	}

	// Lock the logical log file of the database in which the target area exists
	// for operating tuples in database system area

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
// Manager::SystemTable::ReorganizeArea::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::AreaDefinition* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeArea::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Statement::AreaDefinition* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pArea()
{}

// for redoing
Manager::SystemTable::ReorganizeArea::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pArea()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizeArea::Create::
~Create()
{
	if (m_pStatement && (m_iStatus != Progress::Create::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Create::execute -- execute
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
Manager::SystemTable::ReorganizeArea::Create::
execute()
{
	if (m_pStatement) { // normal case
		// prepare log data
		LogData cLogData(LogData::Category::CreateArea);

		// create new area object
		// log data is also filled
		m_pArea = Area::create(*getDatabase(), *m_pStatement, cLogData, getTransaction());

		if (m_pArea.get() == 0) {
			// another object with same name exists
			return Result::None;
		}

		// reset progress status
		// NOTE: reset here because area name should be withdrawn when error occurs
		m_iStatus = Progress::Create::None;

		// Check whether new path is already used by other objects
		if (m_pArea->checkPath(getTransaction())) {
			// Another object using the path
			_SYDNEY_THROW1(Exception::InvalidPath, m_pArea->getName());
		}
		m_iStatus = Progress::Create::PathReserved;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateArea", "CheckPath");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::ReadWrite, true /* weaken */);

		// check delay setting for test
		objectCheckWait();

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateArea", "Created");

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		// create area object from log data
		m_pArea = Area::create(getTransaction(), *getDatabase(), *m_pLogData);
		; _SYDNEY_ASSERT(m_pArea.get());
	}

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Area(*pDatabase).store(getTransaction(), m_pArea);
	m_iStatus = Progress::Create::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateArea", "Stored");

	// persist ObjectID value
	Schema::ObjectID::persist(getTransaction(), pDatabase);

	// withdraw path from object path reservation
	Manager::ObjectPath::withdraw(m_pArea.get());
	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pArea.get());
			
	// return value
	Result::Value iResult = Result::None;
	if (m_pStatement) { // do only for normal case
		// increment timestamp value
		addTimestamp(getTransaction());

		// create area need causing checkpoint in any case
		iResult = (Result::NeedCheckpoint | Result::NeedReCache);
	}

	m_iStatus = Progress::Create::Succeeded;
	return iResult;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizeArea::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create area failed." << ModEndl;
	}

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pArea.get());

	switch (m_iStatus) {
	case Progress::Create::Stored:
		{
			// withdraw path from object path reservation
			Manager::ObjectPath::withdraw(m_pArea.get());

			// drop object
			m_pArea->drop(getTransaction(), false /* not in recovery */, false /* no check */); 

			// store again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Area(*pDatabase).store(getTransaction(), m_pArea);

			// persist objectID value because an object with the new Id has been persisted
			Schema::ObjectID::persist(getTransaction(), pDatabase);
			break;
		}
	case Progress::Create::PathReserved:
		{
			// withdraw path from object path reservation
			Manager::ObjectPath::withdraw(m_pArea.get());
			// thru.
		}
	case Progress::Create::None:
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeArea::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::DropAreaStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeArea::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Statement::DropAreaStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pArea()
{}

// for redoing
Manager::SystemTable::ReorganizeArea::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pArea()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizeArea::Drop::
~Drop()
{
	// drop statement's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeArea::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.

	try {
		if (m_pStatement) { // normal case
			// get name of target area from the statement
			const Object::Name& cName = Area::getName(*m_pStatement);

			// obtain area object
			Database* pDatabase = getDatabase();
			m_pArea = syd_reinterpret_cast<const Area*>(pDatabase->getArea(cName, getTransaction()));
			if (!m_pArea.get()) {
				if (m_pStatement->isIfExists()) {
					// With 'IF EXISTS', ignore this case
					return Result::None;
				} else {
					_SYDNEY_THROW2(Exception::AreaNotFound, cName, pDatabase->getName());
				}
			}

			// check cancellation
			Manager::checkCanceled(getTransaction());

			// lock database
			lock(Hold::Operation::ReadWrite);

			// reset progress status
			// NOTE: reset here because drop flag should be reverted when error occurs
			m_iStatus = Progress::Drop::None;

			// prepare log data
			LogData cLogData(LogData::Category::DropArea);

			// set drop mark to the object
			// log data is also filled.
			Area::drop(*m_pArea, cLogData, getTransaction());
			m_iStatus = Progress::Drop::Dropped;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropArea", "Dropped");

			// store log data to database log file
			storeDatabaseLog(cLogData, pDatabase);
			m_iStatus = Progress::Drop::Logged;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropArea", "Logged");

		} else { // redo
			// drop does not use this class in redoing
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}

		// persist system table
		// [NOTES]
		//	area directry will be destroyed in Area::doBeforePersist
		Database* pDatabase = getDatabase();
		Schema::SystemTable::Area(*pDatabase).store(getTransaction(), m_pArea);
		m_iStatus = Progress::Drop::Stored;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropArea", "Stored");

		// return value
		Result::Value iResult = Result::None;
		if (m_pStatement) { // do only for normal case
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
		if (m_pStatement && (m_iStatus != Progress::Drop::Succeeded)) {
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
//	Schema::Manager::SystemTable::ReorganizeArea::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizeArea::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop area failed." << ModEndl;
	}
	bool bResult = false;

	// error recovery of drop is retrying
	// unless the process has been failed before storing log data
	switch (m_iStatus) {
	case Progress::Drop::Logged:
		{
			// wait for a while before retrying
			retryWait();

			SydErrorMessage << "retry drop area." << ModEndl;
			// persist again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Area(*pDatabase).store(getTransaction(), m_pArea);
			SydErrorMessage << "retry drop area succeeded." << ModEndl;
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
			m_pArea->undoDrop();
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

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeArea::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::AlterAreaStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeArea::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Statement::AlterAreaStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_vecPrevPath(), m_vecPostPath(),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pArea()
{}

// for redoing
Manager::SystemTable::ReorganizeArea::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_vecPrevPath(), m_vecPostPath(),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pArea()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizeArea::Alter::
~Alter()
{
	if (m_pStatement && (m_iStatus != Progress::Alter::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Alter::execute -- execute
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
Manager::SystemTable::ReorganizeArea::Alter::
execute()
{
	if (m_pStatement) { // normal case
		// get name of target area from the statement
		const Object::Name& cName = Area::getName(*m_pStatement);

		// obtain area object
		m_pArea = syd_reinterpret_cast<const Area*>(getDatabase()->getArea(cName, getTransaction()));
		if (!m_pArea.get()) {
			_SYDNEY_THROW2(Exception::AreaNotFound, cName, getDatabase()->getName());
		}

		// prepare log data
		LogData cLogData(LogData::Category::AlterArea);

		// check statement and set path string before/after alter
		// log data is also filled.
		if (!Area::alter(getTransaction(), *m_pArea, *m_pStatement, m_vecPrevPath, m_vecPostPath, cLogData)) {
			// nothing to do
			return Result::None;
		}
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterArea", "Alter");

		// check the target paths
		// If another object uses the same path, error
		if (m_pArea->checkPath(getTransaction(), &m_vecPostPath)) {
			_SYDNEY_THROW1(Exception::InvalidPath, m_pArea->getName());
		}
		m_iStatus = Progress::Alter::PathReserved;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterArea", "CheckPath");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::MoveDatabase,
			 false /* not weaken */, true /* lock database too */);

		// after database is locked, convert lock for meta table to drop mode
		convert(getTransaction(), Hold::Target::MetaTable,
				Lock::Name::Category::Table, Hold::Operation::MoveDatabase,
				Lock::Name::Category::Tuple, Hold::Operation::MoveDatabase);

		// check delay setting for test
		objectCheckWait();

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());

	} else { // redo
		// alter does not use this class in redoing
		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// move area directry
	m_pArea->move(getTransaction(), m_vecPrevPath, m_vecPostPath);
	// tell the object is changed
	m_pArea->touch();
	m_iStatus = Progress::Alter::Moved;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterArea", "Moved");
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterArea", "Touched");

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Area(*pDatabase).store(getTransaction(), m_pArea);
	m_iStatus = Progress::Alter::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterArea", "Stored");

	// withdraw path from object path reservation
	Manager::ObjectPath::withdraw(m_pArea.get());

	if (m_pStatement) {
		// increment timestamp value
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::Alter::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeArea::Alter::undo -- error recovery
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
Manager::SystemTable::ReorganizeArea::Alter::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter area failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Alter::Stored:
		{
			// cancel moving
			m_pArea->move(getTransaction(), m_vecPostPath, m_vecPrevPath, true /* undo */);

			// withdraw path reservation
			Manager::ObjectPath::withdraw(m_pArea.get());

			// persist system table
			m_pArea->touch(); // touch again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Area(*pDatabase).store(getTransaction(), m_pArea);
			break;
		}
	case Progress::Alter::Moved:
		{
			// cancel moving
			m_pArea->move(getTransaction(), m_vecPostPath, m_vecPrevPath, true /* undo */);

			// cancel dirty flag
			m_pArea->untouch();

			// thru.
		}
	case Progress::Alter::PathReserved:
		{
			// withdraw path reservation
			Manager::ObjectPath::withdraw(m_pArea.get());
			break;
		}
	case Progress::Alter::None:
	default:
		{
			break;
		}
	}
}

//
// Copyright (c) 2006, 2007, 2008, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
