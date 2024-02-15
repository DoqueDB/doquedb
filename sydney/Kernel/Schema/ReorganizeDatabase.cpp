// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeDatabase.cpp -- Implementation of classes concerning with database reorganization
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#include "Schema/AutoLatch.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/ReorganizeDatabase.h"
#include "Schema/ReorganizePrivilege.h"
#include "Schema/SystemTable_Database.h"
#include "Schema/SystemTable_File.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Admin/Replicator.h"

#include "Checkpoint/Executor.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/MetaDatabaseCorrupted.h"
#include "Exception/InvalidPath.h"

#include "Lock/Name.h"

#include "Server/Session.h"

#include "Statement/AlterDatabaseStatement.h"
#include "Statement/DatabaseDefinition.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

//	MACRO
//		_BEGIN_DB_REORGANIZE_RECOVERY -- Beginning of error recovery modifying database system table
//		_END_DB_REORGANIZE_RECOVERY	  -- End of error recovery modifying database system table
//
//	NOTES
//		These two macros should be placed before beginning and after end of error recovery
//		If any errors occur between these two macros, systemtable will become not-available.

#define _BEGIN_DB_REORGANIZE_RECOVERY(_database_) \
									if (Schema::Database::isAvailable((_database_).getID())) { \
										Schema::Object::ID::Value i_database_ID = (_database_).getID(); \
										try {
#define _END_DB_REORGANIZE_RECOVERY	\
										} catch (Exception::Object& e) { \
											SydErrorMessage << "Error recovery failed. FATAL. " << e << ModEndl; \
											if (m_bFatal) { \
												/* All the system become not-available */ \
												Schema::SystemTable::setAvailability(false); \
											} else { \
												/* A database becomes not-available */ \
												Schema::Database::setAvailability(i_database_ID, false); \
											} \
											/* No exceptions are rethrown which occur during error recovery */ \
											/* thru. */ \
										} catch (...) { \
											SydErrorMessage << "Error recovery failed. FATAL." << ModEndl; \
											if (m_bFatal) { \
												/* All the system become not-available */ \
												Schema::SystemTable::setAvailability(false); \
											} else { \
												/* A database becomes not-available */ \
												Schema::Database::setAvailability(i_database_ID, false); \
											} \
											/* No exceptions are rethrown which occur during error recovery */ \
											/* thru. */ \
										} \
									}

#define _BEGIN_DB_ALL_FATAL_IF_FAIL \
										{m_bFatal = true;}
#define _END_DB_ALL_FATAL_IF_FAIL \
										{m_bFatal = false;}

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create database
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Create
		{
			enum Value
			{
				None,					// initial value
				PathReserved,			// paths are reserved
				Created,				// files are created
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Drop::Value -- Progress value for drop database
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Drop
		{
			enum Value
			{
				None,					// initial value
				Dropped,				// drop flag is set
				Logged,					// logical log data is written
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Move::Value -- Progress value for move database
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Move
		{
			enum Value
			{
				None,					// initial value
				PathReserved,			// moved paths are reserved
				Moved,					// files are moved
				Storing,				// system table persisting
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Alter::Value -- Progress value for alter database
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Alter
		{
			enum Value
			{
				None,					// initial value
				SetAttribute,			// database attribute has been changed
				Touched,				// database object is touched
				Stored,					// system table persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeDatabase::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Base::Base -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	bool bRedo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeDatabase::Base::
Base(Trans::Transaction& cTrans_, bool bRedo_)
	: ReorganizeExecutor(cTrans_),
	  m_bRedo(bRedo_)
{
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeDatabase::Base::
~Base()
{
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Base::lock -- lock operation
//
// NOTES
//
// ARGUMENTS
//	Schema::Hold::Operation::Value eOperation_
//		operation for meta tuple
//	Schema::ObjectID::Value iID_
//		id of meta tuple (= id of the target database)
//	bool bCreating_ = false
//		true if operation is creating
//	bool bLockDatabase_ = true
//		true if database should be locked
//	Schema::Database* pDatabase_ = 0
//		non-zero if database log file should be locked
//
// RETURN
//	Nothing
//
// EXCEPTIONS

// lock operation
void
Manager::SystemTable::ReorganizeDatabase::Base::
lock(Schema::Hold::Operation::Value eOperation_, Schema::ObjectID::Value iID_,
	 bool bCreating_ /* = false */, bool bLockDatabase_ /* = true */,
	 Schema::Database* pDatabase_ /* = 0 */)
{
	if (!bCreating_) {
		// convert lock on meta tuple to specified mode
		convert(getTransaction(), Hold::Target::MetaTuple,
				Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
				Lock::Name::Category::Tuple, eOperation_,
				iID_);
	} else {
		// MetaDatabase, MetaTable has been locked
		hold(getTransaction(), Hold::Target::MetaTuple,
			 Lock::Name::Category::Tuple, eOperation_,
			 iID_);
		// after meta tuple has been locked, weaken the lock mode for meta table
		convert(getTransaction(), Hold::Target::MetaTable,
				Lock::Name::Category::Tuple, Hold::Operation::ReadWrite,
				Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite);
	}

	// Lock the system log file
	hold(getTransaction(), Hold::Target::LogicalLog,
		 Lock::Name::Category::Tuple, eOperation_);

	if (pDatabase_) {
		// Lock the database log file
		hold(getTransaction(), Hold::Target::LogicalLog,
			 Lock::Name::Category::Tuple, eOperation_,
			 0, Trans::Log::File::Category::Database, pDatabase_);
	}

	if (bLockDatabase_) {
		// Lock the database for operation to database
		hold(getTransaction(), Hold::Target::Database,
			 Lock::Name::Category::Database, eOperation_,
			 iID_);
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeDatabase::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Statement::DatabaseDefinition* pStatement_
//	Server::Session* pSession_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeDatabase::Create::
Create(Trans::Transaction& cTrans_,
	   const Statement::DatabaseDefinition* pStatement_,
	   Server::Session* pSession_)
	: Base(cTrans_, false /* not redo */),
	  m_pStatement(pStatement_),
	  m_pSession(pSession_),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pDatabase(),
	  m_bFatal(false)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizeDatabase::Create::
~Create()
{
	if (m_pStatement && (m_iStatus != Progress::Create::Succeeded)) {

		undoAlways();

		_BEGIN_DB_REORGANIZE_RECOVERY(*m_pDatabase);

		undo();

		_END_DB_REORGANIZE_RECOVERY;
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Create::execute -- execute
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
Manager::SystemTable::ReorganizeDatabase::Create::
execute()
{
	// prepare logdata object
	LogData cLogData(LogData::Category::CreateDatabase);

	// create new database object according to the SQL statement
	// logdata is also filled
	m_pDatabase = Database::create(*m_pStatement, cLogData, getTransaction());

	// return value
	Result::Value iResult = Result::None;

	// When there exists another database with the same name, 0 can be returned
	if (m_pDatabase.get()) {

		// reset progress status
		// NOTE: reset here because database name should be withdrawn when error occurs
		m_iStatus = Progress::Create::None;

		// Check whether new path is already used by other objects
		if (m_pDatabase->checkPath(getTransaction())) {
			// Another object using the path
			_SYDNEY_THROW1(Exception::InvalidPath, m_pDatabase->getName());
		}
		m_iStatus = Progress::Create::PathReserved;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateDatabase", "CheckPath");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock meta tuple
		lock(Hold::Operation::ReadWrite, m_pDatabase.get()->getID(),
			 true /* for creating */,
			 false /* database is not locked */);

		// Check metadatabase availability after locking
		if (!Schema::SystemTable::isAvailable()) {
			_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
		}

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// check delay setting for test
		objectCheckWait();

		// write log data to system log
		storeSystemLog(cLogData);

		// create database files
		m_pDatabase->create(getTransaction());
		m_iStatus = Progress::Create::Created;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateDatabase", "Created");

		// write log data to database log
		storeDatabaseLog(cLogData, m_pDatabase.get());

		// persist system table
		Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);
		m_iStatus = Progress::Create::Stored;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateDatabase", "Stored");

		// persist ObjectID value
		Schema::ObjectID::persist(getTransaction(), 0);

		// withdraw name from object name reservation
		Manager::ObjectName::withdraw(m_pDatabase.get());

		// withdraw path from object path reservation
		Manager::ObjectPath::withdraw(m_pDatabase.get());

		// create default privilege using creator
		if (m_pSession->getUserID() >= 0) {
			ReorganizePrivilege::Create cExecutor(getTransaction(), m_pDatabase.get(), m_pSession);
			cExecutor.execute();
		}

		// increment the timestamp
		addTimestamp(getTransaction());

		// cause checkpoint to the created database
		Checkpoint::Executor::cause(getTransaction(), *m_pDatabase);
		m_iStatus = Progress::Create::Succeeded;

		iResult = Result::NeedReCache;
	}
	return iResult;
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeDatabase::Create::undoAlways -- 
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
Manager::SystemTable::ReorganizeDatabase::Create::
undoAlways()
{
	switch (m_iStatus) {
	case Progress::Create::Stored:
	case Progress::Create::Created:
	case Progress::Create::PathReserved:
		{
			// withdraw path from object path reservation
			Manager::ObjectPath::withdraw(m_pDatabase.get());
			// thru.
		}
	case Progress::Create::None:
		{
			// withdraw name from object name reservation
			Manager::ObjectName::withdraw(m_pDatabase.get());
			break;
		}
	default:
		{
			break;
		}
	}
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeDatabase::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizeDatabase::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create database failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Create::Stored:
		{
			// If any errors occor, all the system become not-available
			_BEGIN_DB_ALL_FATAL_IF_FAIL;

			// drop the created object
			m_pDatabase->drop(getTransaction());

			// persist drop
			Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);

			// persist objectId value because the object with the new id has been persisted
			Schema::ObjectID::persist(getTransaction(), 0);

			_END_DB_ALL_FATAL_IF_FAIL;

			break;
		}
	case Progress::Create::Created:
	case Progress::Create::PathReserved:
		{
			// drop should be called only when path checking has been succeeded(No.1043)
			// cancel creation of the object
			m_pDatabase->drop(getTransaction());
			break;
		}
	case Progress::Create::None:
		{
			break;
		}
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeDatabase::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Statement::DropDatabaseStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeDatabase::Drop::
Drop(Trans::Transaction& cTrans_,
	 Schema::Database* pDatabase_,
	 const Statement::DropDatabaseStatement* pStatement_)
	: Base(cTrans_, false /* not redo */),
	  m_pStatement(pStatement_),
	  m_pDatabase(syd_reinterpret_cast<const Database*>(pDatabase_)),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_bFatal(false)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizeDatabase::Drop::
~Drop()
{
	// drop statement's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeDatabase::Drop::
execute()
{
	if (m_pDatabase->isSlaveStarted()) {
		// stop replicator
		Admin::Replicator::stop(getTransaction(), *m_pDatabase);
	}

	// lock database
	lock(Hold::Operation::Drop, m_pDatabase->getID());

	// Check metadatabase availability after locking
	if (!Schema::SystemTable::isAvailable()) {
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	// tell the database is refered
	// [Note]
	// close will be called in persisting
	m_pDatabase->open();

	try {
		// prepare log data
		LogData cLogData(LogData::Category::DropDatabase);
		m_iStatus = Progress::Drop::None;

		// set drop flag to the database object
		// log data is filled
		Database::drop(*m_pDatabase, cLogData, getTransaction());
		m_iStatus = Progress::Drop::Dropped;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropDatabase", "Dropped");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// setLog has to be called here
		// so that Transaction can get logical log file of the database
		getTransaction().setLog(*m_pDatabase);

		// store in the 'system' log
		storeSystemLog(cLogData);
		m_iStatus = Progress::Drop::Logged;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropDatabase", "Logged");

		// persist system table
		Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);
		m_iStatus = Progress::Drop::Stored;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropDatabase", "Stored");

		Result::Value iResult = Result::None;

		if (m_pStatement) { // do only in normal cases
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
		if (m_pStatement) { // do only in normal cases
			bool bRetrySucceeded = false;

			_BEGIN_DB_REORGANIZE_RECOVERY(*m_pDatabase);

			bRetrySucceeded = retry();

			_END_DB_REORGANIZE_RECOVERY;

			// If retry has been succeeded, treat as success case
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
//	Schema::Manager::SystemTable::ReorganizeDatabase::Drop::retry -- error recovery
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//		true if retry succeeded
//
// EXCEPTIONS

bool
Manager::SystemTable::ReorganizeDatabase::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop database failed." << ModEndl;
	}
	bool bResult = false;

	// error recovery of drop is retrying
	// unless the process has been failed before storing log data
	switch (m_iStatus) {
	case Progress::Drop::Logged:
		{
			// wait for a while before retrying
			retryWait();

			// If any errors occor, all the system become not-available
			_BEGIN_DB_ALL_FATAL_IF_FAIL;

			SydErrorMessage << "retry drop database." << ModEndl;
			// persist again
			Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);
			SydErrorMessage << "retry drop database succeeded." << ModEndl;

			_END_DB_ALL_FATAL_IF_FAIL;

			// thru.
		}
	case Progress::Drop::Stored:
		{
			// If any errors occor, all the system become not-available
			_BEGIN_DB_ALL_FATAL_IF_FAIL;

			// incrument timestamp value again
			addTimestamp(getTransaction());

			_END_DB_ALL_FATAL_IF_FAIL;

			// retrying has been succeeded
			bResult = true;
			break;
		}
	case Progress::Drop::Dropped:
		{
			// just cancel drop flag
			m_pDatabase->undoDrop(getTransaction());
			// thru.
		}
	case Progress::Drop::None:
	default:
		{
			// close the database
			m_pDatabase->close(true /* volatile */);
			break;
		}
	}
	return bResult;
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeDatabase::Move
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Move::Move -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Statement::MoveDatabaseStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeDatabase::Move::
Move(Trans::Transaction& cTrans_,
	 Schema::Database* pDatabase_,
	 const Statement::MoveDatabaseStatement* pStatement_)
	: Base(cTrans_, false /* not redo */),
	  m_pStatement(pStatement_),
	  m_pDatabase(syd_reinterpret_cast<const Database*>(pDatabase_)),
	  m_vecPrevPath(),
	  m_vecPostPath(),
	  m_iStatus(Progress::Move::Succeeded),
	  m_bFatal(false)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Move::~Move -- destructor
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

Manager::SystemTable::ReorganizeDatabase::Move::
~Move()
{
	if (m_pStatement && (m_iStatus != Progress::Move::Succeeded)) {

		_BEGIN_DB_REORGANIZE_RECOVERY(*m_pDatabase);

		undo();

		_END_DB_REORGANIZE_RECOVERY;

		undoAlways();
	}
	try {
		m_pDatabase->close(isRedo() /* if redo, close database in volatile mode */);
	} catch (...) {
		// ignore all the exceptions in destructor
		;
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Move::execute -- execute
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
Manager::SystemTable::ReorganizeDatabase::Move::
execute()
{
	// prepare log data
	LogData cLogData(LogData::Category::MoveDatabase);
	ModVector<bool> vecChanged;		// true for moved element

	// set above variables and fill the log data
	if (!Database::alter(*m_pDatabase, *m_pStatement,
						 m_vecPrevPath, m_vecPostPath, vecChanged,
						 cLogData, getTransaction())) {
		// no path specifications are changed
		return Result::None;
	}
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "MoveDatabase", "Alter");

	// check the target paths
	// If another object uses the same path, error
	if (m_pDatabase->checkPath(getTransaction(), &vecChanged, &m_vecPostPath)) {
		_SYDNEY_THROW1(Exception::InvalidPath, m_pDatabase->getName());
	}
	m_iStatus = Progress::Move::PathReserved;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "MoveDatabase", "CheckPath");

	// check cancellation
	Manager::checkCanceled(getTransaction());

	// lock database
	lock(Hold::Operation::MoveDatabase, m_pDatabase->getID());

	// after locking, lock mode for metatable should be changed from the mode for moving to for dropping
	convert(getTransaction(), Hold::Target::MetaTable,
			Lock::Name::Category::Table, Hold::Operation::MoveDatabase,
			Lock::Name::Category::Tuple, Hold::Operation::MoveDatabase);

	// check delay setting for test
	objectCheckWait();

	// Check metadatabase availability after locking
	if (!Schema::SystemTable::isAvailable()) {
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	// store log data to system log and database log
	storeSystemLog(cLogData);
	storeDatabaseLog(cLogData, m_pDatabase.get()); // for auto-recovery after FATAL error in moving

	// move the database files
	m_pDatabase->move(getTransaction(), m_vecPrevPath, m_vecPostPath);
	// tell changing attribute
	m_pDatabase->touch();
	m_iStatus = Progress::Move::Moved;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "MoveDatabase", "Moved");

	// persist file system table because path name has been changed
	m_iStatus = Progress::Move::Storing;
	Schema::SystemTable::File(*m_pDatabase).store(getTransaction());
	// persist database system table
	Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "MoveDatabase", "Stored");

	// withdraw path from object path reservation
	Manager::ObjectPath::withdraw(m_pDatabase.get());

	if (m_pStatement) { // do in normal cases
		// increment timestamp value
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::Move::Succeeded;

	return Result::NeedReCache;
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeDatabase::Move::undoAlways -- 
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
Manager::SystemTable::ReorganizeDatabase::Move::
undoAlways()
{
	// withdraw path from object path reservation
	Manager::ObjectPath::withdraw(m_pDatabase.get());
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeDatabase::Move::undo -- error recovery
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
Manager::SystemTable::ReorganizeDatabase::Move::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter database failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Move::Storing:
		{
			// If any errors occor, all the system become not-available
			_BEGIN_DB_ALL_FATAL_IF_FAIL;

			// cancel moving
			m_pDatabase->undoMove(getTransaction(), m_vecPrevPath, m_vecPostPath);

			// persist system table
			// even if some system tables has not been persisted,
			// store should be called so that object status are canceled
			Schema::SystemTable::File(*m_pDatabase).store(getTransaction());
			Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);

			_END_DB_ALL_FATAL_IF_FAIL;
			break;
		}
	case Progress::Move::Moved:
		{
			// cancel moving
			m_pDatabase->undoMove(getTransaction(), m_vecPrevPath, m_vecPostPath);

			// cancel dirty flag
			m_pDatabase->untouch();

			break;
		}
	case Progress::Move::PathReserved:
	case Progress::Move::None:
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeDatabase::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	const Statement::AlterDatabaseStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeDatabase::Alter::
Alter(Trans::Transaction& cTrans_,
	  Schema::Database* pDatabase_,
	  const Statement::AlterDatabaseStatement* pStatement_)
	: Base(cTrans_, false /* not redo */),
	  m_pStatement(pStatement_),
	  m_pDatabase(syd_reinterpret_cast<const Database*>(pDatabase_)),
	  m_cPrevAttribute(),
	  m_bPropagate(false),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_bFatal(false),
	  m_bRecoveryChange(false)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizeDatabase::Alter::
~Alter()
{
	if (m_pStatement && (m_iStatus != Progress::Alter::Succeeded)) {

		_BEGIN_DB_REORGANIZE_RECOVERY(*m_pDatabase);

		undo();

		_END_DB_REORGANIZE_RECOVERY;

		undoAlways();
	}
	try {
		m_pDatabase->close(isRedo() /* if redo, close database in volatile mode */);
	} catch (...) {
		// ignore all the exceptions in destructor
		;
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeDatabase::Alter::execute -- execute
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
Manager::SystemTable::ReorganizeDatabase::Alter::
execute()
{
	// database attribute after altering
	// (the attribute before altering is set to a data member)
	Database::Attribute cPostAttribute;

	// prepare log data
	LogData cLogData(LogData::Category::AlterDatabase);

	// get database attributes before and after altering
	// log data is also filled
	if (!Database::alter(*m_pDatabase, *m_pStatement,
						 m_cPrevAttribute, cPostAttribute, cLogData,
						 getTransaction())) {
		// no attributes are changed
		return Result::None;
	}
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterDatabase", "Alter");

	// check cancellation
	Manager::checkCanceled(getTransaction());

	// changing is read only -> read write?
	bool bReadOnlyToWrite = (m_cPrevAttribute.m_bReadOnly && !cPostAttribute.m_bReadOnly);
	// changing is read write -> read only?
	bool bReadWriteToOnly = (!m_cPrevAttribute.m_bReadOnly && cPostAttribute.m_bReadOnly);
	// changing is online -> offline?
	bool bOnlineToOffline = (m_cPrevAttribute.m_bOnline && !cPostAttribute.m_bOnline);
	// changing is offline -> online?
	bool bOfflineToOnline = (!m_cPrevAttribute.m_bOnline && cPostAttribute.m_bOnline);
	// changing is recovery full -> recovery checkpoint?
	bool bFullToCheckpoint = (m_cPrevAttribute.m_bRecoveryFull && !cPostAttribute.m_bRecoveryFull);
	// changing is recovery checkpoint -> recovery full?
	bool bCheckpointToFull = (!m_cPrevAttribute.m_bRecoveryFull && cPostAttribute.m_bRecoveryFull);

	// changing is user all -> super?
	bool bUserAllToSuper = (!m_cPrevAttribute.m_bSuperUserMode && cPostAttribute.m_bSuperUserMode);
	// changing is user super -> all?
	bool bUserSuperToAll = (m_cPrevAttribute.m_bSuperUserMode && !cPostAttribute.m_bSuperUserMode);

	// changing is slave stopped -> started?
	bool bSlaveStoppedToStarted = (!m_cPrevAttribute.m_bSlaveStarted && cPostAttribute.m_bSlaveStarted);
	// changing is slave started -> stopped?
	bool bSlaveStartedToStopped = (m_cPrevAttribute.m_bSlaveStarted && !cPostAttribute.m_bSlaveStarted);

	// changing is slave -> master?
	bool bSlaveToMaster = (m_cPrevAttribute.m_cstrMasterURL.getLength() != 0
						   && cPostAttribute.m_cstrMasterURL.getLength() == 0);

	if (bSlaveStartedToStopped)
	{
		// set to master では drop でロックする必要があるので、
		// ここで replicator を停止する必要がある
		
		Admin::Replicator::stop(getTransaction(), *m_pDatabase);
	}

	// read write mode is changed or online -> offline or set to master
	// then do alter in single access
	bool bSingleAccess = (bReadOnlyToWrite || bReadWriteToOnly || bOnlineToOffline
						  || bFullToCheckpoint || bCheckpointToFull || bUserAllToSuper || bUserSuperToAll
						  || bSlaveToMaster);

	const Hold::Operation::Value eOperation =
		bSingleAccess ? Hold::Operation::Drop : Hold::Operation::ReadWrite;

	// lock database
	lock(eOperation, m_pDatabase->getID(),
		 false /* not for creating */,
		 bSingleAccess /* lock database only in single mode */,
		 ((bReadOnlyToWrite || bReadWriteToOnly ||
		   bFullToCheckpoint || bCheckpointToFull) ? m_pDatabase.get() : 0)
			/* database log is locked if readwrite or recovery is changed */);

	// Check metadatabase availability after locking
	if (!Schema::SystemTable::isAvailable()) {
		_SYDNEY_THROW0(Exception::MetaDatabaseCorrupted);
	}

	// store log data to system log file
	storeSystemLog(cLogData);

	// store log data to database log file when changing is read write -> read only
	if (bReadWriteToOnly) {
		storeDatabaseLog(cLogData, m_pDatabase.get());
	}

	m_iStatus = Progress::Alter::SetAttribute;

	// set schema object flag
	// changing should be propagated if read write mode is changed
	if (bReadOnlyToWrite || bReadWriteToOnly) {
		m_pDatabase->propagateAttribute(getTransaction(), cPostAttribute);
		m_bPropagate = true;
	}
	m_pDatabase->setAttribute(cPostAttribute);
	if (bFullToCheckpoint || bCheckpointToFull) {
		m_pDatabase->getLogFile()->setRecoveryFull(
			cPostAttribute.m_bRecoveryFull);
		m_bRecoveryChange = true;
	}

	if (bSlaveStoppedToStarted)
	{
		// start replicator
		Admin::Replicator::start(getTransaction(), *m_pDatabase);
	}

	// tell the object become dirty
	m_pDatabase->touch();
	m_iStatus = Progress::Alter::Touched;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterDatabase", "Touched");

	if (bOfflineToOnline && m_pStatement->isDiscardLogicalLog())
	{
		// 論理ログの不要な部分を削除する
		// 実際には、トランザクションのコミット時に削除される
		
		getTransaction().setLog(*m_pDatabase);
		getTransaction().discardLog(true);
		storeDatabaseLog(cLogData, m_pDatabase.get());
	}

	// If changing is read only -> read write or slave -> master,
	// write log data to database here
	if (bReadOnlyToWrite || bSlaveToMaster) {
		storeDatabaseLog(cLogData, m_pDatabase.get());
	}

	objectCheckWait();

	// persist system table
	Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);
	m_iStatus = Progress::Alter::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterDatabase", "Stored");

	if (m_pStatement) {
		// increment timestamp value
		addTimestamp(getTransaction());
	}

	// If changing is read only -> read write, cause checkpoint for the database
	if (bReadOnlyToWrite || bOnlineToOffline) {
		Checkpoint::Executor::cause(getTransaction(), *m_pDatabase);
	}
	m_iStatus = Progress::Alter::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeDatabase::Alter::undoAlways -- 
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
Manager::SystemTable::ReorganizeDatabase::Alter::
undoAlways()
{
	;
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeDatabase::Alter::undo -- error recovery
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
Manager::SystemTable::ReorganizeDatabase::Alter::
undo()
{
	SydErrorMessage << "alter database failed." << ModEndl;

	if (m_iStatus >= Progress::Alter::SetAttribute) {
		// Set the attribute before altering
		if (m_bPropagate) {
			m_pDatabase->propagateAttribute(getTransaction(), m_cPrevAttribute);
		}
		m_pDatabase->setAttribute(m_cPrevAttribute);
		if (m_bRecoveryChange) {
			m_pDatabase->getLogFile()->setRecoveryFull(
				m_cPrevAttribute.m_bRecoveryFull);
		}
	}

	switch (m_iStatus) {
	case Progress::Alter::SetAttribute:
	case Progress::Alter::Touched:
		{
			// before persisted -> just cancel the flag
			m_pDatabase->untouch();
			break;
		}
	case Progress::Alter::Stored:
		{
			// If any errors occor, all the system become not-available
			_BEGIN_DB_ALL_FATAL_IF_FAIL;

			// after persisted -> persist again
			m_pDatabase->touch();
			Schema::SystemTable::Database().store(getTransaction(), m_pDatabase);

			_END_DB_ALL_FATAL_IF_FAIL;

			break;
		}
	default:
		{
			break;
		}
	}
}

//
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2012, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
