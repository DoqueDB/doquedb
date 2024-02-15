// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeCascade.cpp -- Implementation of classes concerning with cascade reorganization
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#include "Schema/ReorganizeCascade.h"

#include "Schema/Cascade.h"
#include "Schema/AutoLatch.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/SystemTable_Cascade.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/CascadeNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "Os/AutoCriticalSection.h"

#include "Statement/AlterCascadeStatement.h"
#include "Statement/CascadeDefinition.h"
#include "Statement/DropCascadeStatement.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create cascade
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
		// $$::Progress::Drop::Value -- Progress value for drop cascade
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
		// $$::Progress::Alter::Value -- Progress value for alter cascade
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Alter
		{
			enum Value
			{
				None,					// initial value
				Changed,				// schema object changed
				Touched,				// schema object touched
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeCascade::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Base::Base -- constructor
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

Manager::SystemTable::ReorganizeCascade::Base::
Base(Trans::Transaction& cTrans_, Database* pDatabase_, bool bRedo_)
	: ReorganizeExecutor(cTrans_),
	  m_pDatabase(pDatabase_),
	  m_bRedo(bRedo_)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeCascade::Base::
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
//	Schema::Manager::SystemTable::ReorganizeCascade::Base::lock -- lock operation
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
Manager::SystemTable::ReorganizeCascade::Base::
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

	// Lock the logical log file of the database in which the target cascade exists
	// for operating tuples in database system cascade

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
// Manager::SystemTable::ReorganizeCascade::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::CascadeDefinition* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeCascade::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Statement::CascadeDefinition* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pCascade()
{}

// for redoing
Manager::SystemTable::ReorganizeCascade::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pCascade()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizeCascade::Create::
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
//	Schema::Manager::SystemTable::ReorganizeCascade::Create::execute -- execute
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
Manager::SystemTable::ReorganizeCascade::Create::
execute()
{
	if (m_pStatement) { // normal case
		// prepare log data
		LogData cLogData(LogData::Category::CreateCascade);

		// create new cascade object
		// log data is also filled
		m_pCascade = Cascade::create(*getDatabase(), *m_pStatement, cLogData, getTransaction());

		if (m_pCascade.get() == 0) {
			// another object with same name exists
			return Result::None;
		}

		// reset progress status
		// NOTE: reset here because cascade name should be withdrawn when error occurs
		m_iStatus = Progress::Create::None;

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::ReadWrite, true /* weaken */);

		// check delay setting for test
		objectCheckWait();

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateCascade", "Created");

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		// create cascade object from log data
		m_pCascade = Cascade::create(getTransaction(), *getDatabase(), *m_pLogData);
		; _SYDNEY_ASSERT(m_pCascade.get());
	}

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Cascade(*pDatabase).store(getTransaction(), m_pCascade);
	m_iStatus = Progress::Create::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateCascade", "Stored");

	// persist ObjectID value
	Schema::ObjectID::persist(getTransaction(), pDatabase);

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pCascade.get());
			
	// return value
	Result::Value iResult = Result::None;
	if (m_pStatement) { // do only for normal case
		// increment timestamp value
		addTimestamp(getTransaction());

		// create cascade need causing checkpoint in any case
		iResult = isCauseCheckpoint()
			? (Result::NeedCheckpoint | Result::NeedReCache)
			: Result::NeedReCache;
	}

	m_iStatus = Progress::Create::Succeeded;
	return iResult;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizeCascade::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create cascade failed." << ModEndl;
	}

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pCascade.get());

	switch (m_iStatus) {
	case Progress::Create::Stored:
		{
			// drop object
			m_pCascade->drop(getTransaction(), false /* not in recovery */); 

			// store again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Cascade(*pDatabase).store(getTransaction(), m_pCascade);

			// persist objectID value because an object with the new Id has been persisted
			Schema::ObjectID::persist(getTransaction(), pDatabase);
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
// Manager::SystemTable::ReorganizeCascade::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::DropCascadeStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeCascade::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Statement::DropCascadeStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pCascade()
{}

// for redoing
Manager::SystemTable::ReorganizeCascade::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pCascade()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizeCascade::Drop::
~Drop()
{
	// drop statement's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeCascade::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.

	try {
		if (m_pStatement) { // normal case
			// get name of target cascade from the statement
			const Object::Name& cName = Cascade::getName(*m_pStatement);

			// obtain cascade object
			Database* pDatabase = getDatabase();
			m_pCascade = syd_reinterpret_cast<const Cascade*>(pDatabase->getCascade(cName, getTransaction()));
			if (!m_pCascade.get()) {
				if (m_pStatement->isIfExists()) {
					// With 'IF EXISTS', ignore this case
					return Result::None;
				} else {
					_SYDNEY_THROW2(Exception::CascadeNotFound, cName, pDatabase->getName());
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
			LogData cLogData(LogData::Category::DropCascade);

			// set drop mark to the object
			// log data is also filled.
			Cascade::drop(*m_pCascade, cLogData, getTransaction());
			m_iStatus = Progress::Drop::Dropped;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropCascade", "Dropped");

			// store log data to database log file
			storeDatabaseLog(cLogData, pDatabase);
			m_iStatus = Progress::Drop::Logged;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropCascade", "Logged");

		} else { // redo
			; _SYDNEY_ASSERT(m_pLogData);

			m_pCascade = Cascade::drop(getTransaction(), *getDatabase(), *m_pLogData);

			if (m_pCascade.get() == 0) {
				// nothing to do
				return Result::None;
			}
		}

		// persist system table
		// [NOTES]
		//	cascade directry will be destroyed in Cascade::doBeforePersist
		Database* pDatabase = getDatabase();
		Schema::SystemTable::Cascade(*pDatabase).store(getTransaction(), m_pCascade);
		m_iStatus = Progress::Drop::Stored;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropCascade", "Stored");

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
//	Schema::Manager::SystemTable::ReorganizeCascade::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizeCascade::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop cascade failed." << ModEndl;
	}
	bool bResult = false;

	// error recovery of drop is retrying
	// unless the process has been failed before storing log data
	switch (m_iStatus) {
	case Progress::Drop::Logged:
		{
			// wait for a while before retrying
			retryWait();

			SydErrorMessage << "retry drop cascade." << ModEndl;
			// persist again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Cascade(*pDatabase).store(getTransaction(), m_pCascade);
			SydErrorMessage << "retry drop cascade succeeded." << ModEndl;
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
			m_pCascade->undoDrop();
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
// Manager::SystemTable::ReorganizeCascade::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::AlterCascadeStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeCascade::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Statement::AlterCascadeStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_cPrevTarget(), m_cPostTarget(),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pCascade()
{}

// for redoing
Manager::SystemTable::ReorganizeCascade::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_cPrevTarget(), m_cPostTarget(),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pCascade()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizeCascade::Alter::
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
//	Schema::Manager::SystemTable::ReorganizeCascade::Alter::execute -- execute
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
Manager::SystemTable::ReorganizeCascade::Alter::
execute()
{
	if (m_pStatement) { // normal case
		// get name of target cascade from the statement
		const Object::Name& cName = Cascade::getName(*m_pStatement);

		// obtain cascade object
		m_pCascade = syd_reinterpret_cast<const Cascade*>(getDatabase()->getCascade(cName, getTransaction()));
		if (!m_pCascade.get()) {
			_SYDNEY_THROW2(Exception::CascadeNotFound, cName, getDatabase()->getName());
		}

		// prepare log data
		LogData cLogData(LogData::Category::AlterCascade);

		// check statement and set path string before/after alter
		// log data is also filled.
		if (!Cascade::alter(getTransaction(), *m_pCascade, *m_pStatement,
							m_cPrevTarget, m_cPostTarget, cLogData)) {
			// nothing to do
			return Result::None;
		}

		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterCascade", "Alter");

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
		// get cascade id from log data
		ObjectID::Value iID = Cascade::getObjectID(*m_pLogData);
		// obtain cascade object
		m_pCascade = syd_reinterpret_cast<const Cascade*>(getDatabase()->getCascade(iID, getTransaction()));
		; _SYDNEY_ASSERT(m_pCascade.get());

		if (!Cascade::alter(getTransaction(), *m_pLogData,
							m_cPrevTarget, m_cPostTarget)) {
			// never happens
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

	// change target
	m_pCascade->setTarget(m_cPostTarget);
	m_iStatus = Progress::Alter::Changed;

	// tell the object is changed
	m_pCascade->touch();
	m_iStatus = Progress::Alter::Touched;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterCascade", "Touched");

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Cascade(*pDatabase).store(getTransaction(), m_pCascade);
	m_iStatus = Progress::Alter::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterCascade", "Stored");

	if (m_pStatement) {
		// increment timestamp value
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::Alter::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeCascade::Alter::undo -- error recovery
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
Manager::SystemTable::ReorganizeCascade::Alter::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter cascade failed." << ModEndl;
	}

	if (m_iStatus >= Progress::Alter::Changed) {
		// cancel changing
		m_pCascade->setTarget(m_cPrevTarget);
	}

	switch (m_iStatus) {
	case Progress::Alter::Stored:
		{
			// persist system table
			m_pCascade->touch(); // touch again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Cascade(*pDatabase).store(getTransaction(), m_pCascade);
			break;
		}
	case Progress::Alter::Touched:
		{
			// cancel dirty flag
			m_pCascade->untouch();

			// thru.
		}
	case Progress::Alter::Changed:
	case Progress::Alter::None:
	default:
		{
			break;
		}
	}
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
