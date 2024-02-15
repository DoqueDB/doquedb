// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizePartition.cpp -- Implementation of classes concerning with partition reorganization
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

#include "Schema/ReorganizePartition.h"

#include "Schema/Partition.h"
#include "Schema/AutoLatch.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/PartitionMap.h"
#include "Schema/SystemTable_Partition.h"
#include "Schema/Table.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "DSchema/Table.h"

#include "Exception/PartitionNotFound.h"
#include "Exception/TableNotEmpty.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "Os/AutoCriticalSection.h"

#include "Statement/AlterPartitionStatement.h"
#include "Statement/DropPartitionStatement.h"
#include "Statement/PartitionDefinition.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create partition
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
		// $$::Progress::Drop::Value -- Progress value for drop partition
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Drop
		{
			enum Value
			{
				None,					// Initial value
				Dropped,				// object are marked as dropped
				Logged,					// logical log is stored
				Storing,				// persisting started
				Stored,					// persisting finished
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Alter::Value -- Progress value for alter partition
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
// Manager::SystemTable::ReorganizePartition::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Base::Base -- constructor
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

Manager::SystemTable::ReorganizePartition::Base::
Base(Trans::Transaction& cTrans_, Database* pDatabase_, bool bRedo_)
	: ReorganizeExecutor(cTrans_),
	  m_pDatabase(pDatabase_),
	  m_bRedo(bRedo_)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizePartition::Base::
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
//	Schema::Manager::SystemTable::ReorganizePartition::Base::lock -- lock operation
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
Manager::SystemTable::ReorganizePartition::Base::
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

	// Lock the logical log file of the database in which the target partition exists
	// for operating tuples in database system partition

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
// Manager::SystemTable::ReorganizePartition::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::PartitionDefinition* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizePartition::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Statement::PartitionDefinition* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pPartition()
{}

// for redoing
Manager::SystemTable::ReorganizePartition::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pPartition()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizePartition::Create::
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
//	Schema::Manager::SystemTable::ReorganizePartition::Create::execute -- execute
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
Manager::SystemTable::ReorganizePartition::Create::
execute()
{
	if (m_pStatement) { // normal case
		// prepare log data
		LogData cLogData(LogData::Category::CreatePartition);

		// reset progress status
		// NOTE: reset here because partition name should be withdrawn when error occurs
		m_iStatus = Progress::Create::None;

		// create new partition object
		// log data is also filled
		m_pPartition = Partition::create(*getDatabase(), *m_pStatement, cLogData, getTransaction());

		if (m_pPartition.get() == 0) {
			// another object with same name exists
			return Result::None;
		}

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::ReadWrite, true /* weaken */);

		// check delay setting for test
		objectCheckWait();

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreatePartition", "Created");

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		// create partition object from log data
		m_pPartition = Partition::create(getTransaction(), *getDatabase(), *m_pLogData);
		; _SYDNEY_ASSERT(m_pPartition.get());
	}

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), m_pPartition);
	m_iStatus = Progress::Create::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreatePartition", "Stored");

	// persist ObjectID value
	Schema::ObjectID::persist(getTransaction(), pDatabase);

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pPartition.get());
			
	// return value
	Result::Value iResult = Result::None;
	if (m_pStatement) { // do only for normal case
		// increment timestamp value
		addTimestamp(getTransaction());

		// if parameter Schema_CauseCheckpoint is set, cause checkpoint after commit
		iResult = (isCauseCheckpoint()
				   ? (Result::NeedCheckpoint | Result::NeedReCache)
				   : Result::NeedReCache);
	}

	m_iStatus = Progress::Create::Succeeded;
	return iResult;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizePartition::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create partition failed." << ModEndl;
	}

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pPartition.get());

	switch (m_iStatus) {
	case Progress::Create::Stored:
		{
			// drop object
			m_pPartition->drop(getTransaction(), false /* not in recovery */); 

			// store again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), m_pPartition);

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
// Manager::SystemTable::ReorganizePartition::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::DropPartitionStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizePartition::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Statement::DropPartitionStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pPartition()
{}

// for redoing
Manager::SystemTable::ReorganizePartition::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pPartition()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizePartition::Drop::
~Drop()
{
	// drop statement's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Drop::execute -- execute
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
Manager::SystemTable::ReorganizePartition::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.

	try {
		if (m_pStatement) { // normal case
			// get target name from the statement
			const Object::Name& cTableName = Partition::getName(*m_pStatement);
			Table* pTable = getDatabase()->getTable(cTableName, getTransaction());
			if (pTable == 0) {
				_SYDNEY_THROW2(Exception::TableNotFound, cTableName, getDatabase()->getName());
			}
			// obtain partition object
			m_pPartition = syd_reinterpret_cast<const Partition*>(pTable->getPartition(getTransaction()));
			if (!m_pPartition.get()) {
				if (m_pStatement->isIfExists()) {
					// With 'IF EXISTS', ignore this case
					return Result::None;
				} else {
					_SYDNEY_THROW2(Exception::PartitionNotFound, cTableName, getDatabase()->getName());
				}
			}
			if (m_pPartition->getCategory() != Partition::Category::ReadOnly
				&& DSchema::Table::isEmpty(getTransaction(), *pTable) == false) {
				// Table is not empty
				_SYDNEY_THROW1(Exception::TableNotEmpty, cTableName);
			}

			// check cancellation
			Manager::checkCanceled(getTransaction());

			// lock database
			lock(Hold::Operation::Drop);

			// prepare log data
			LogData cLogData(LogData::Category::DropPartition);

			// reset progress status
			// NOTE: reset here because partition name should be withdrawn when error occurs
			m_iStatus = Progress::Drop::None;

			// set drop flag to the object and fill log data
			Partition::drop(getTransaction(), *m_pPartition, cLogData);
			m_iStatus = Progress::Drop::Dropped;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropPartition", "Dropped");

			// store log data
			storeDatabaseLog(cLogData, getDatabase());
			m_iStatus = Progress::Drop::Logged;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropPartition", "Logged");

		} else { // redo
			// get partition id from log data
			ObjectID::Value iID = Partition::getObjectID(*m_pLogData);
			// get partition object
			m_pPartition = getDatabase()->loadPartition(getTransaction()).get(iID);
			if (!m_pPartition.get()) {
				// if no object, do nothing;
				return Result::None;
			}
			// set drop flag
			m_pPartition->drop(getTransaction(), true /* recovery */);
		}

		// start persisting
		m_iStatus = Progress::Drop::Storing;

		Database* pDatabase = getDatabase();
		// persist drop partition
		// [NOTES]
		//	object pointed by m_pPartition become invalid
		Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), m_pPartition);
		m_pPartition = static_cast<Partition*>(0);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropPartition", "PartitionStored");
		m_iStatus = Progress::Drop::Stored;

		// return value
		Result::Value iResult = Result::None;

		// increment timestamp value
		addTimestamp(getTransaction());

		// if parameter Schema_CauseCheckpoint is set, cause checkpoint after commit
		iResult = (isCauseCheckpoint()
				   ? (Result::NeedCheckpoint | Result::NeedReCache)
				   : Result::NeedReCache);

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
				return (isCauseCheckpoint()
					   ? (Result::NeedCheckpoint | Result::NeedReCache)
					   : Result::NeedReCache);
			} else {
				_SYDNEY_RETHROW;
			}
		} else { // in redoing
			_SYDNEY_RETHROW;
		}
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizePartition::Drop::
retry()
{
	bool bResult = false;

	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop partition failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Drop::Logged:
	case Progress::Drop::Storing:
		{
			// wait for a while before retrying
			retryWait();

			SydErrorMessage << "retry drop partition." << ModEndl;

			Database* pDatabase = getDatabase();
			if (m_pPartition.get()) {
				Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), m_pPartition);
			}

			SydErrorMessage << "retry drop partition succeeded." << ModEndl;
			// thru.
		}
	case Progress::Drop::Stored:
		{
			addTimestamp(getTransaction());
			bResult = true;
			break;
		}
	case Progress::Drop::Dropped:
		{
			// cancel drop flag
			m_pPartition->undoDrop();
			break;
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
// Manager::SystemTable::ReorganizePartition::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::AlterPartitionStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizePartition::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Statement::AlterPartitionStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_cPrevTarget(), m_cPostTarget(),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pPartition()
{}

// for redoing
Manager::SystemTable::ReorganizePartition::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_cPrevTarget(), m_cPostTarget(),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pPartition()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizePartition::Alter::
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
//	Schema::Manager::SystemTable::ReorganizePartition::Alter::execute -- execute
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
Manager::SystemTable::ReorganizePartition::Alter::
execute()
{
	if (m_pStatement) { // normal case
		// get name of target partition from the statement
		const Object::Name& cTableName = Partition::getName(*m_pStatement);
		Table* pTable = getDatabase()->getTable(cTableName, getTransaction());
		if (pTable == 0) {
			_SYDNEY_THROW2(Exception::TableNotFound, cTableName, getDatabase()->getName());
		}
		// obtain partition object
		m_pPartition = syd_reinterpret_cast<const Partition*>(pTable->getPartition(getTransaction()));
		if (!m_pPartition.get()) {
			_SYDNEY_THROW2(Exception::PartitionNotFound, cTableName, getDatabase()->getName());
		}

		// prepare log data
		LogData cLogData(LogData::Category::AlterPartition);

		// check statement and set path string before/after alter
		// log data is also filled.
		if (!Partition::alter(getTransaction(), *m_pPartition, *m_pStatement,
							m_cPrevTarget, m_cPostTarget, cLogData)) {
			// nothing to do
			return Result::None;
		}

		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterPartition", "Alter");

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

		m_iStatus = Progress::Alter::None;

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());

	} else { // redo
		// get partition id from log data
		ObjectID::Value iID = Partition::getObjectID(*m_pLogData);
		// obtain partition object
		m_pPartition = syd_reinterpret_cast<const Partition*>(getDatabase()->getPartition(iID, getTransaction()));
		; _SYDNEY_ASSERT(m_pPartition.get());

		if (!Partition::alter(getTransaction(), *m_pLogData,
							m_cPrevTarget, m_cPostTarget)) {
			// never happens
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

	// change target
	m_pPartition->setTarget(m_cPostTarget);
	m_iStatus = Progress::Alter::Changed;

	// tell the object is changed
	m_pPartition->touch();
	m_iStatus = Progress::Alter::Touched;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterPartition", "Touched");

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), m_pPartition);
	m_iStatus = Progress::Alter::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterPartition", "Stored");

	if (m_pStatement) {
		// increment timestamp value
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::Alter::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizePartition::Alter::undo -- error recovery
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
Manager::SystemTable::ReorganizePartition::Alter::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter partition failed." << ModEndl;
	}

	if (m_iStatus >= Progress::Alter::Changed) {
		// cancel changing
		m_pPartition->setTarget(m_cPrevTarget);
	}

	switch (m_iStatus) {
	case Progress::Alter::Stored:
		{
			// persist system table
			m_pPartition->touch(); // touch again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), m_pPartition);
			break;
		}
	case Progress::Alter::Touched:
		{
			// cancel dirty flag
			m_pPartition->untouch();

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
