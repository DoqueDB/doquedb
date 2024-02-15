// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeFunction.cpp -- Implementation of classes concerning with function reorganization
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

#include "Schema/ReorganizeFunction.h"

#include "Schema/Function.h"
#include "Schema/AutoLatch.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Partition.h"
#include "Schema/SystemTable_Function.h"
#include "Schema/Table.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/StoredFunctionUsed.h"
#include "Exception/StoredFunctionNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "Os/AutoCriticalSection.h"

#include "Statement/FunctionDefinition.h"
#include "Statement/DropFunctionStatement.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create function
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
		// $$::Progress::Drop::Value -- Progress value for drop function
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
// Manager::SystemTable::ReorganizeFunction::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeFunction::Base::Base -- constructor
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

Manager::SystemTable::ReorganizeFunction::Base::
Base(Trans::Transaction& cTrans_, Database* pDatabase_, bool bRedo_)
	: ReorganizeExecutor(cTrans_),
	  m_pDatabase(pDatabase_),
	  m_bRedo(bRedo_)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeFunction::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeFunction::Base::
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
//	Schema::Manager::SystemTable::ReorganizeFunction::Base::lock -- lock operation
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
Manager::SystemTable::ReorganizeFunction::Base::
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

	// Lock the logical log file of the database in which the target function exists
	// for operating tuples in database system function

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
// Manager::SystemTable::ReorganizeFunction::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeFunction::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::FunctionDefinition* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeFunction::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Statement::FunctionDefinition* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pFunction()
{}

// for redoing
Manager::SystemTable::ReorganizeFunction::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pFunction()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeFunction::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizeFunction::Create::
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
//	Schema::Manager::SystemTable::ReorganizeFunction::Create::execute -- execute
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
Manager::SystemTable::ReorganizeFunction::Create::
execute()
{
	if (m_pStatement) { // normal case
		// prepare log data
		LogData cLogData(LogData::Category::CreateFunction);

		// create new function object
		// log data is also filled
		m_pFunction = Function::create(*getDatabase(), *m_pStatement, cLogData, getTransaction());

		if (m_pFunction.get() == 0) {
			// another object with same name exists
			return Result::None;
		}

		// reset progress status
		// NOTE: reset here because function name should be withdrawn when error occurs
		m_iStatus = Progress::Create::None;

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::ReadWrite, true /* weaken */);

		// check delay setting for test
		objectCheckWait();

		// store log data to database log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateFunction", "Created");

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		// create function object from log data
		m_pFunction = Function::create(getTransaction(), *getDatabase(), *m_pLogData);
		; _SYDNEY_ASSERT(m_pFunction.get());
	}

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Function(*pDatabase).store(getTransaction(), m_pFunction);
	m_iStatus = Progress::Create::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateFunction", "Stored");

	// persist ObjectID value
	Schema::ObjectID::persist(getTransaction(), pDatabase);

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pFunction.get());
			
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
//	Schema::Manager::SystemTable::ReorganizeFunction::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizeFunction::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create function failed." << ModEndl;
	}

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pFunction.get());

	switch (m_iStatus) {
	case Progress::Create::Stored:
		{
			// drop object
			m_pFunction->drop(getTransaction(), false /* not in recovery */); 

			// store again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Function(*pDatabase).store(getTransaction(), m_pFunction);

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
// Manager::SystemTable::ReorganizeFunction::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeFunction::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::DropFunctionStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeFunction::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Statement::DropFunctionStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pFunction()
{}

// for redoing
Manager::SystemTable::ReorganizeFunction::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pFunction()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeFunction::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizeFunction::Drop::
~Drop()
{
	// drop statement's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeFunction::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeFunction::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.

	try {
		if (m_pStatement) { // normal case
			// get name of target function from the statement
			const Object::Name& cName = Function::getName(*m_pStatement);

			// obtain function object
			Database* pDatabase = getDatabase();
			m_pFunction = syd_reinterpret_cast<const Function*>(pDatabase->getFunction(cName, getTransaction()));
			if (!m_pFunction.get()) {
				if (m_pStatement->isIfExists()) {
					// With 'IF EXISTS', ignore this case
					return Result::None;
				} else {
					_SYDNEY_THROW2(Exception::StoredFunctionNotFound, cName, pDatabase->getName());
				}
			}

			// check cancellation
			Manager::checkCanceled(getTransaction());

			// lock database
			lock(Hold::Operation::ReadWrite);

			// reset progress status
			// NOTE: reset here because drop flag should be reverted when error occurs
			m_iStatus = Progress::Drop::None;

			// check whether function is used by partition
			checkPartition();

			// prepare log data
			LogData cLogData(LogData::Category::DropFunction);

			// set drop mark to the object
			// log data is also filled.
			Function::drop(*m_pFunction, cLogData, getTransaction());
			m_iStatus = Progress::Drop::Dropped;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropFunction", "Dropped");

			// store log data to database log file
			storeDatabaseLog(cLogData, pDatabase);
			m_iStatus = Progress::Drop::Logged;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropFunction", "Logged");

		} else { // redo
			; _SYDNEY_ASSERT(m_pLogData);

			m_pFunction = Function::drop(getTransaction(), *getDatabase(), *m_pLogData);

			if (m_pFunction.get() == 0) {
				// nothing to do
				return Result::None;
			}
		}

		// persist system table
		// [NOTES]
		//	function directry will be destroyed in Function::doBeforePersist
		Database* pDatabase = getDatabase();
		Schema::SystemTable::Function(*pDatabase).store(getTransaction(), m_pFunction);
		m_iStatus = Progress::Drop::Stored;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropFunction", "Stored");

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
//	Schema::Manager::SystemTable::ReorganizeFunction::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizeFunction::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop function failed." << ModEndl;
	}
	bool bResult = false;

	// error recovery of drop is retrying
	// unless the process has been failed before storing log data
	switch (m_iStatus) {
	case Progress::Drop::Logged:
		{
			// wait for a while before retrying
			retryWait();

			SydErrorMessage << "retry drop function." << ModEndl;
			// persist again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Function(*pDatabase).store(getTransaction(), m_pFunction);
			SydErrorMessage << "retry drop function succeeded." << ModEndl;
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
			m_pFunction->undoDrop();
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

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeFunction::Drop::checkPartition -- check usage in partition
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
Manager::SystemTable::ReorganizeFunction::Drop::
checkPartition()
{
	const Object::Name& cFunctionName = m_pFunction->getName();
	// check all the tables in the database
	Database* pDatabase = getDatabase();
	const ModVector<Table*>& vecTable = pDatabase->getTable(getTransaction(),
															true /* internal */);
	ModVector<Table*>::ConstIterator iterator = vecTable.begin();
	const ModVector<Table*>::ConstIterator last = vecTable.end();
	for (; iterator != last; ++iterator) {
		Partition* pPartition = (*iterator)->getPartition(getTransaction());
		if (pPartition
			&& pPartition->getTarget().m_cFunctionName == cFunctionName) {
			// used by a partition
			_SYDNEY_THROW2(Exception::StoredFunctionUsed,
						   cFunctionName,
						   (*iterator)->getName());
		}
	}
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
