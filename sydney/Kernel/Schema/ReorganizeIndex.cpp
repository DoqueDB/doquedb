// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeIndex.cpp -- Implementation of classes concerning with index reorganization
// 
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/Index.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Hold.h"
#include "Schema/IndexMap.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/ReorganizeIndex.h"
#include "Schema/SystemTable_AreaContent.h"
#include "Schema/SystemTable_Column.h"
#include "Schema/SystemTable_Constraint.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/SystemTable_Table.h"
#include "Schema/Table.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Admin/File.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/Reorganize_Import.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/IndexNotFound.h"

#include "Lock/Name.h"

#include "Statement/IndexDefinition.h"
#include "Statement/DropIndexStatement.h"
#include "Statement/AlterIndexAction.h"
#include "Statement/AlterIndexStatement.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create index
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Create
		{
			enum Value
			{
				None,					// Initial value
				Created,				// new object is created
				FileCreated,			// file is created
				Importing,				// import process is started
				Imported,				// import process is finished
				Persisting,				// system table is started to be persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Drop::Value -- Progress value for drop index
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Drop
		{
			enum Value
			{
				None,					// Initial value
				Logged,					// logical log is stored
				Dropped,				// files are dropped
				Storing,				// persisting started
				Stored,					// persisting finished
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::AlterArea::Value -- Progress value for alter index set/drop area
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct AlterArea
		{
			enum Value
			{
				None,					// Initial value
				Moved,					// files are moved
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::AlterName::Value -- Progress value for alter index rename
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct AlterName
		{
			enum Value
			{
				None,					// Initial value
				Renamed,				// name is changed and files are moved
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::AlterUsage::Value -- Progress value for alter index online/offline
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct AlterUsage
		{
			enum Value
			{
				None,					// Initial value
				Changed,				// status is changed
				Succeeded,				// process succeeded
				ValueNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeIndex::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Base::Base -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//	bool bRedo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::Base::
Base(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 Table* pTable_,
	 bool bRedo_)
	: Super(cTrans_, pDatabase_, pTable_, bRedo_)
{
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeIndex::Base::
~Base()
{
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Base::lockLogicalLog -- lock operation
//
// NOTES
//
// ARGUMENTS
//	Schema::Hold::Operation::Value eOperation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeIndex::Base::
lockLogicalLog(Schema::Hold::Operation::Value eOperation_)
{
	// Lock the logical log file of the database in which the target table exists
	// for operating tuples in database system table

	hold(getTransaction(), Hold::Target::LogicalLog,
		 Lock::Name::Category::Tuple, eOperation_,
		 0, Trans::Log::File::Category::Database, getDatabase());
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Base::lockDatabase -- lock database
//
// NOTES
//
// ARGUMENTS
//	Schema::Hold::Operation::Value eOperation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeIndex::Base::
lockDatabase(Schema::Hold::Operation::Value eOperation_)
{
	// lock database object
	ObjectTree::Database::hold(getTransaction(),
							   Lock::Name::Category::Tuple,
							   eOperation_,
							   getDatabase()->getID());
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Base::lockTable -- lock table
//
// NOTES
//
// ARGUMENTS
//	Schema::Hold::Operation::Value eOperation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeIndex::Base::
lockTable(Schema::Hold::Operation::Value eOperation_)
{
	ObjectTree::Table::hold(getTransaction(),
							Lock::Name::Category::Tuple, eOperation_,
							getDatabase()->getID(), getTable()->getID());
}

// FUNCTION protected
//	Schema::Manager::SystemTable::ReorganizeIndex::Base::isTemporary -- 
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
Manager::SystemTable::ReorganizeIndex::Base::
isTemporary()
{
	return getDatabase()->getScope() == Database::Scope::SessionTemporary;
}

///////////////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeIndex::Create
///////////////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//	const Statement::IndexDefinition* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Statement::IndexDefinition* pStatement_)
	: Base(cTrans_, pDatabase_, 0, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_pIndex(),
	  m_pFile(0),
	  m_iStatus(Progress::Create::Succeeded)
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, 0, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_pIndex(),
	  m_pFile(0),
	  m_iStatus(Progress::Create::Succeeded)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizeIndex::Create::
~Create()
{
	if (!isRedo() && (m_iStatus != Progress::Create::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::execute -- execute
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
Manager::SystemTable::ReorganizeIndex::Create::
execute()
{
	if (isRedo()) {
		// separate implementation for redoing
		return redo();
	}

	; _SYDNEY_ASSERT(m_pStatement);

	if (!isTemporary()) {
		// lock logical log file
		lockLogicalLog(Hold::Operation::ReadForWrite);

		// lock database for importing
		lockDatabase(Hold::Operation::ReadForImport);
	}

	// Prepare log data
	LogData cLogData(LogData::Category::CreateIndex);
	// create new index object
	// log data is also filled
	m_pIndex = Index::create(getTransaction(), *getDatabase(), *m_pStatement, cLogData);

	if (!m_pIndex.get()) {
		// another schema object has the same name, create canceled
		return Result::None;
	}

	m_iStatus = Progress::Create::Created;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "Created");

	// check cancellation
	Manager::checkCanceled(getTransaction());

	objectCheckWait();

	// set table object for following process
	setTable(m_pIndex->getTable(getTransaction()));

	// get file object
	m_pFile = m_pIndex->getFile(getTransaction());

	if (isTemporary()) {
		// create file
		m_pFile->create(getTransaction());
		m_iStatus = Progress::Create::FileCreated;

		// get field correspondence
		ModVector<Field*> vecSourceField;
		ModVector<Field*> vecTargetField;
		getImportField(m_pFile, vecSourceField, vecTargetField);

		// import data from table
		import(vecSourceField, vecTargetField, false /* no check */, false /* not in rowid order */);

	} else {
		// block update operations by locking logical log file
		blockUpdateOperation();

		// add index file to reorganized file
		getTable()->addReorganizedFile(getTransaction(), m_pFile);

		// When process goes out of this scope, call erase automatically
		Common::AutoCaller2<Table, Trans::Transaction&, File*>
			cAutoEraser(getTable(), &Table::eraseReorganizedFile,
						getTransaction(), m_pFile);

		// After registering reorganized file, allow update operations
		allowUpdateOperation();

		// store log data to database log
		// record the last LSN before importing table data
		Trans::Log::LSN cLastLSN = storeDatabaseLog(cLogData, getDatabase());

		// create file
		m_pFile->create(getTransaction());
		m_iStatus = Progress::Create::FileCreated;

		// lock table for importing
		lockTable(Hold::Operation::ReadForImport);

		// get field correspondence
		ModVector<Field*> vecSourceField;
		ModVector<Field*> vecTargetField;
		getImportField(m_pFile, vecSourceField, vecTargetField);

		// import data from table
		import(vecSourceField, vecTargetField, false /* no check */, false /* not in rowid order */);

		if (m_pIndex->isClustered()) {
			// !!! YET !!!
			// (new btree file holding large objects and null should be prepared)
			// create new vector file mapping rowid and OID of the index file
			// import all the record data
			//
			// (after entering single user mode)
			// delete record file
		}

		// delay for testing
		fileReflectWait();

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// start reflecting
		m_pFile->startImport(getTransaction());
		m_iStatus = Progress::Create::Importing;

		// Reflect update operation using logical log data
		// which recorded while importing in above process.
		// At this time, reflecting is done without locking
		reflect(cLastLSN);

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// block update operations by locking logical log file
		blockUpdateOperation();

		// Reflect update operation using logical log data
		// which recorded while reflecting without blocking.
		// At this time, reflecting is done with blocking update operations
		reflect(cLastLSN);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "Reflected");

		// try to lock database with Operation::ReadWrite
		// if locking has succeeded, it is guaranteed that no other transactions are accessing the database
		while (!tryLockDatabase()) {

			// check cancellation
			Manager::checkCanceled(getTransaction());

			// if locking has failed, allow update operations temporarily,
			// then try again
			allowUpdateOperation();
			waitLockDatabase();

			// Reflect update operation using logical log data
			// which recorded while waiting for lock.
			// At this time, reflecting is done without locking
			reflect(cLastLSN);

			// block update operation again
			blockUpdateOperation();

			// Reflect update operation using logical log data
			// which recorded while reflecting without blocking.
			// At this time, reflecting is done with blocking update operations
			reflect(cLastLSN);
		}

		// Reflect update operation using logical log data
		// which recorded while reflecting until Database has not been locked.
		// At this time, no other transactions are accessing the database
		reflect(cLastLSN);

		// reflecting finished
		m_pFile->endImport(getTransaction());
		m_iStatus = Progress::Create::Imported;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "EndImport");
	}

	// system tables are persisted
	m_iStatus = Progress::Create::Persisting;
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "IndexStored");

	Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pIndex);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "KeyStored");

	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "FileStored");

	// tell columns and fields become obsolete
	Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "ColumnStored");
	Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "FieldStored");

	// return value
	Result::Value iResult = Result::None;

	if (!isTemporary()) {
		Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "AreaContentStored");

		// persist objectId
		Schema::ObjectID::persist(getTransaction(), getDatabase());

		// increment timestamp value
		addTimestamp(getTransaction());

		// if parameter Schema_CauseCheckpoint is set, cause checkpoint after commit
		iResult = (isCauseCheckpoint()
				   ? (Result::NeedCheckpoint | Result::NeedReCache)
				   : Result::NeedReCache);
	}

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pIndex.get());
	m_iStatus = Progress::Create::Succeeded;

	return iResult;
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::redo -- execute in redo
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
Manager::SystemTable::ReorganizeIndex::Create::
redo()
{
	; _SYDNEY_ASSERT(m_pLogData);
	Database* pDatabase = getDatabase();
	m_pIndex = Index::create(getTransaction(), *pDatabase, *m_pLogData);
	; _SYDNEY_ASSERT(m_pIndex.get());

	// set table object for following process
	setTable(m_pIndex->getTable(getTransaction()));

	// add index file to reorganized file
	m_pFile = m_pIndex->getFile(getTransaction());

	// create file
	m_pFile->create(getTransaction());

	// get field correspondence
	ModVector<Field*> vecSourceField;
	ModVector<Field*> vecTargetField;
	getImportField(m_pFile, vecSourceField, vecTargetField);

	// import data from table
	import(vecSourceField, vecTargetField, false /* no check */, false /* not in rowid order */);

	if (m_pIndex->isClustered()) {
		// !!! YET !!!
		// (new btree file holding large objects and null should be prepared)
		// create new vector file mapping rowid and OID of the index file
		// import all the record data
		//
		// (after entering single user mode)
		// delete record file
	}

	// system tables are persisted
	Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
	Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pIndex);
	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
	// tell columns and fields become obsolete
	Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *getTable());
	Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
	Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

	// persist objectId
	Schema::ObjectID::persist(getTransaction(), getDatabase());

	return Result::None;
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizeIndex::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create index failed." << ModEndl;
	}

	Manager::ObjectName::withdraw(m_pIndex.get());

	switch (m_iStatus) {
	case Progress::Create::Persisting:
		{
			// set drop flag to the object
			m_pIndex->drop(getTransaction());

			// presist all the system tables again
			Database* pDatabase = getDatabase();
			if (!isTemporary()) {
				Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
			}
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
			Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pIndex);
			Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);

			if (!isTemporary()) {
				// persist objectID because new objectid has been persisted
				Schema::ObjectID::persist(getTransaction(), getDatabase());
			}
			break;
		}
	case Progress::Create::Importing:
		{
			// finish importing
			m_pFile->endImport(getTransaction());
			// thru.
		}
	case Progress::Create::Imported:
	case Progress::Create::FileCreated:
	case Progress::Create::Created:
		{
			// cancel creation
			m_pIndex->drop(getTransaction());
			break;
		}
	case Progress::Create::None:
		{
			break;
		}
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::reflect -- reflect data from logical log data
//
// NOTES
//
// ARGUMENTS
//	Trans::Log::LSN& cLSN_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeIndex::Create::
reflect(Trans::Log::LSN& cLSN_)
{
	if (cLSN_ != Trans::Log::IllegalLSN) {
		cLSN_ = Admin::Reorganization::File::reflect(
					getTransaction(), *getDatabase(), *m_pFile, cLSN_);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::blockUpdateOperation -- block update operations
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
Manager::SystemTable::ReorganizeIndex::Create::
blockUpdateOperation()
{
	// block update operations by locking logical log file by ReadWrite operation
	convert(getTransaction(), Hold::Target::LogicalLog,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::LogicalLog, Hold::Operation::ReadWrite,
			0, Trans::Log::File::Category::Database, getDatabase());
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::tryLockDatabase -- wait for all the updating transactions for the database end
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
Manager::SystemTable::ReorganizeIndex::Create::
tryLockDatabase()
{
	return convert(getTransaction(), Hold::Target::MetaTuple,
				   Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
				   Lock::Name::Category::Tuple, Hold::Operation::ReadWrite,
				   getDatabase()->getID(),
				   Trans::Log::File::Category::System, 0,
				   0 /* timeout = 0 */);
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::waitLockDatabase -- wait for all the updating transactions for the database end
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
Manager::SystemTable::ReorganizeIndex::Create::
waitLockDatabase()
{
	// wait for the update operation finish
	convert(getTransaction(), Hold::Target::MetaTuple,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadWrite,
			getDatabase()->getID());

	// if locked, release the database, and try again
	convert(getTransaction(), Hold::Target::MetaTuple,
			Lock::Name::Category::Tuple, Hold::Operation::ReadWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			getDatabase()->getID());
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Create::allowUpdateOperation -- allow update operations
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
Manager::SystemTable::ReorganizeIndex::Create::
allowUpdateOperation()
{
	// release update operations by locking logical log file by readforwrite operation
	convert(getTransaction(), Hold::Target::LogicalLog,
			Lock::Name::Category::LogicalLog, Hold::Operation::ReadWrite,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			0, Trans::Log::File::Category::Database, getDatabase());
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeIndex::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::DropIndexStatement* pStatement_
//	bool bIsTemporary_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Statement::DropIndexStatement* pStatement_)
	: Base(cTrans_, pDatabase_, 0, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pIndex()
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 Table* pTable_,
	 const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pIndex()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizeIndex::Drop::
~Drop()
{
	// drop statement's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeIndex::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.

	try {
		if (m_pStatement) { // normal case
			// get target name from the statement
			const Object::Name& cName = Index::getName(*m_pStatement);
			// obtain index object
			m_pIndex = syd_reinterpret_cast<const Index*>(Index::get(cName, getDatabase(), getTransaction()));
			if (!m_pIndex.get()) {
				if (m_pStatement->isIfExists()) {
					// With 'IF EXISTS', ignore this case
					return Result::None;
				} else {
					_SYDNEY_THROW2(Exception::IndexNotFound, cName, getDatabase()->getName());
				}
			}

			// get table object
			setTable(m_pIndex->getTable(getTransaction()));

			// check cancellation
			Manager::checkCanceled(getTransaction());

			if (!isTemporary()) {
				// lock database
				lock(Hold::Operation::Drop);
			}

			// prepare log data
			LogData cLogData(LogData::Category::DropIndex);

			// set drop flag to the object and fill log data
			Index::drop(getTransaction(), *m_pIndex, cLogData);
			m_iStatus = Progress::Drop::Dropped;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "Dropped");

			if (!isTemporary()) {
				// store log data
				storeDatabaseLog(cLogData, getDatabase());
				m_iStatus = Progress::Drop::Logged;
				SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "Logged");
			}

		} else { // redo
			// get index id from log data
			ObjectID::Value iID = Index::getObjectID(*m_pLogData);
			// get index object
			m_pIndex = getTable()->loadIndex(getTransaction()).get(iID);
			if (!m_pIndex.get()) {
				// if no object, do nothing;
				return Result::None;
			}
			// set drop flag
			m_pIndex->drop(getTransaction(), true /* recovery */);
		}

		// start persisting
		m_iStatus = Progress::Drop::Storing;

		Database* pDatabase = getDatabase();
		Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *m_pIndex);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "FieldStored");

		Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pIndex);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "KeyStored");

		Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "FileStored");

		// Field and Column objects are touched in persisting File 
		Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
		Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *m_pIndex);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "ColumnStored");

		// persist drop index
		// [NOTES]
		//	object pointed by m_pIndex become invalid
		Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
		m_pIndex = static_cast<Index*>(0);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "IndexStored");

		if (!isTemporary()) {
			Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropIndex", "AreaContentStored");
		}
		m_iStatus = Progress::Drop::Stored;

		// return value
		Result::Value iResult = Result::None;

		if (m_pStatement && !isTemporary()) {
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
				return isTemporary() ? Result::None
					: (isCauseCheckpoint()
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
//	Schema::Manager::SystemTable::ReorganizeIndex::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizeIndex::Drop::
retry()
{
	bool bResult = false;

	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop index failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Drop::Logged:
	case Progress::Drop::Storing:
		{
			// wait for a while before retrying
			retryWait();

			SydErrorMessage << "retry drop index." << ModEndl;

			Database* pDatabase = getDatabase();
			if (m_pIndex.get()) {
				Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *m_pIndex);
				Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pIndex);
				Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
				Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
				Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *m_pIndex);
				Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
			}
			if (!isTemporary()) {
				Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
			}

			SydErrorMessage << "retry drop index succeeded." << ModEndl;
			// thru.
		}
	case Progress::Drop::Stored:
		{
			if (!isTemporary()) {
				addTimestamp(getTransaction());
			}
			bResult = true;
			break;
		}
	case Progress::Drop::Dropped:
		{
			// cancel drop flag
			m_pIndex->undoDrop(getTransaction());
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
// Manager::SystemTable::ReorganizeIndex::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::AlterIndexStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Statement::AlterIndexStatement* pStatement_)
	: Base(cTrans_, pDatabase_, 0, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0)
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::Alter::
Alter(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, 0, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizeIndex::Alter::
~Alter()
{
	// do nothing
	;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::Alter::execute -- execute
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
Manager::SystemTable::ReorganizeIndex::Alter::
execute()
{
	if (m_pStatement) { // normal case
		// get target name from the statement
		const Object::Name& cName = Index::getName(*m_pStatement);
		// obtain index object
		Index* pIndex = Index::get(cName, getDatabase(), getTransaction());
		if (!pIndex) {
			_SYDNEY_THROW2(Exception::IndexNotFound, cName, getDatabase()->getName());
		}
		Table* pTable = pIndex->getTable(getTransaction());

		// use different object according to action of alter index
		Statement::AlterIndexAction* pAction = m_pStatement->getAlterIndexAction();
		; _SYDNEY_ASSERT(pAction);

		switch (pAction->getActionType()) {
		case Statement::AlterIndexAction::SetArea:
		case Statement::AlterIndexAction::DropArea:
			{
				return ReorganizeIndex::AlterArea(getTransaction(), getDatabase(),
												  pTable, pIndex, pAction)
					.execute();
			}
		case Statement::AlterIndexAction::Rename:
			{
				return ReorganizeIndex::AlterName(getTransaction(), getDatabase(),
												  pTable, pIndex, pAction)
					.execute();
			}
		case Statement::AlterIndexAction::Online:
		case Statement::AlterIndexAction::Offline:
			{
				return ReorganizeIndex::AlterUsage(getTransaction(), getDatabase(),
												   pTable, pIndex, pAction)
					.execute();
			}
		default:
			{
				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
	} else { // redoing
		// redo never call this
		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	// never reach
	return Result::None;
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeIndex::AlterArea
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterArea::AlterArea -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//  Index* pIndex_
//	const Statement::AlterIndexAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::AlterArea::
AlterArea(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  Index* pIndex_,
		  const Statement::AlterIndexAction* pStatement_)
	: Base(cTrans_, pDatabase_, pTable_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::AlterArea::Succeeded),
	  m_pIndex(syd_reinterpret_cast<const Index*>(pIndex_)),
	  m_vecPrevAreaID(),
	  m_vecPostAreaID()
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::AlterArea::
AlterArea(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  Index* pIndex_,
		  const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::AlterArea::Succeeded),
	  m_pIndex(syd_reinterpret_cast<const Index*>(pIndex_)),
	  m_vecPrevAreaID(),
	  m_vecPostAreaID()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterArea::~AlterArea -- destructor
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

Manager::SystemTable::ReorganizeIndex::AlterArea::
~AlterArea()
{
	if (m_pStatement && (m_iStatus != Progress::AlterArea::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterArea::execute -- execute
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
Manager::SystemTable::ReorganizeIndex::AlterArea::
execute()
{
	if (m_pStatement) { // normal case
			
		// prepare log data
		LogData cLogData(LogData::Category::AlterIndex);
		// check statement and fill log data
		if (!Index::alterArea(getTransaction(), *m_pIndex, *m_pStatement,
							  m_vecPrevAreaID, m_vecPostAreaID, cLogData)) {
			// nothing changed
			return Result::None;
		}
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterIndex", "Altered");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::Drop);

		// store log data
		storeDatabaseLog(cLogData, getDatabase());
		m_iStatus = Progress::AlterArea::None;

		// move files
		m_pIndex->moveArea(getTransaction(), m_vecPrevAreaID, m_vecPostAreaID);
		m_pIndex->touch();
		m_iStatus = Progress::AlterArea::Moved;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterIndex", "Moved");

		// persist system tables
		Database* pDatabase = getDatabase();
		Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterIndex", "IndexStored");

		Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterIndex", "FileStored");

		Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterIndex", "AreaContentStored");

		// persist objectID
		Schema::ObjectID::persist(getTransaction(), getDatabase());

		// increment timestamp value
		addTimestamp(getTransaction());
		m_iStatus = Progress::AlterArea::Succeeded;

	} else { // redoing
		// redo never call this
		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterArea::undo -- error recovery
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
Manager::SystemTable::ReorganizeIndex::AlterArea::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter index failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::AlterArea::Moved:
		{
			// move back files
			m_pIndex->moveArea(getTransaction(), m_vecPostAreaID, m_vecPrevAreaID, true /* undo */);
			// tell the object is dirty again
			m_pIndex->touch();

			// persist
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
			Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

			// persist ObjectID
			Schema::ObjectID::persist(getTransaction(), getDatabase());
			break;
		}
	case Progress::AlterArea::None:
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeIndex::AlterName
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterName::AlterName -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//	Index* pIndex_
//	const Statement::AlterIndexAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::AlterName::
AlterName(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  Index* pIndex_,
		  const Statement::AlterIndexAction* pStatement_)
	: Base(cTrans_, pDatabase_, pTable_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::AlterName::Succeeded),
	  m_pIndex(syd_reinterpret_cast<const Index*>(pIndex_)),
	  m_cPrevName(),
	  m_cPostName(),
	  m_bRollforward(false)
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::AlterName::
AlterName(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  Index* pIndex_,
		  const Schema::LogData* pLogData_,
		  bool bRollforward_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::AlterName::Succeeded),
	  m_pIndex(syd_reinterpret_cast<const Index*>(pIndex_)),
	  m_cPrevName(),
	  m_cPostName(),
	  m_bRollforward(bRollforward_)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterName::~AlterName -- destructor
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

Manager::SystemTable::ReorganizeIndex::AlterName::
~AlterName()
{
	if (m_pStatement && (m_iStatus != Progress::AlterName::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterName::execute -- execute
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
Manager::SystemTable::ReorganizeIndex::AlterName::
execute()
{
	if (m_pStatement) { // normal case
		// prepare log data
		LogData cLogData(LogData::Category::RenameIndex);
		// check statement and fill log data
		if (!Index::alterName(getTransaction(), *m_pIndex, *m_pStatement, m_cPostName, cLogData)) {
			// Nothing to do
			return Result::None;
		}
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameIndex", "Altered");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::Drop);

		// store log data
		storeDatabaseLog(cLogData, getDatabase());

	} else { // redoing
		// get post name from log data
		m_cPostName = m_pLogData->getString(Index::Log::Rename::PostName);
	}

	// set original name
	m_cPrevName = m_pIndex->getName();

	bool redo = isRedo();
	if (m_bRollforward == true) redo = false;

	// rename index object and move files
	m_pIndex->moveRename(getTransaction(), getTable()->getName(), getTable()->getName(),
						 m_cPrevName, m_cPostName,
						 false /* not undo */, redo);
	m_pIndex->touch();
	m_iStatus = Progress::AlterName::Renamed;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameIndex", "Moved");

	// persist system table
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameIndex", "IndexStored");

	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameIndex", "FileStored");

	if (m_pStatement) {
		// increment timestamp value
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::AlterName::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterName::undo -- error recovery
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
Manager::SystemTable::ReorganizeIndex::AlterName::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter index failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::AlterName::Renamed:
		{
			// rename back to original
			m_pIndex->moveRename(getTransaction(), getTable()->getName(), getTable()->getName(),
								 m_cPostName, m_cPrevName, true /* undo */);
			m_pIndex->touch();

			// persist again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pIndex);
			break;
		}
	case Progress::AlterName::None:
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeIndex::AlterUsage
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterUsage::AlterUsage -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//	Index* pIndex_
//	const Statement::AlterIndexAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::AlterUsage::
AlterUsage(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  Index* pIndex_,
		  const Statement::AlterIndexAction* pStatement_)
	: Base(cTrans_, pDatabase_, pTable_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::AlterUsage::Succeeded),
	  m_pIndex(syd_reinterpret_cast<const Index*>(pIndex_)),
	  m_bPrevOffline(),
	  m_bPostOffline()
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::AlterUsage::
AlterUsage(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  Index* pIndex_,
		  const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::AlterUsage::Succeeded),
	  m_pIndex(syd_reinterpret_cast<const Index*>(pIndex_)),
	  m_bPrevOffline(),
	  m_bPostOffline()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterUsage::~AlterUsage -- destructor
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

Manager::SystemTable::ReorganizeIndex::AlterUsage::
~AlterUsage()
{
	if (m_pStatement && (m_iStatus != Progress::AlterUsage::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterUsage::execute -- execute
//
// NOTES
//	alter index online/offline is hidden command.
//	no logical log data will be stored.
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeIndex::AlterUsage::
execute()
{
	if (m_pStatement) { // normal case
		// change to offline?
		m_bPostOffline = (m_pStatement->getActionType() == Statement::AlterIndexAction::Offline);
		// original status
		m_bPrevOffline = m_pIndex->isOffline();

		// do only when status will be changed.
		if (m_bPostOffline == m_bPrevOffline) {
			return Result::None;
		}

		// lock database
		lock(Hold::Operation::Drop);

		m_iStatus = Progress::AlterUsage::None;

		// change status
		m_pIndex->setOffline(m_bPostOffline);
		m_pIndex->touch();
		m_iStatus = Progress::AlterUsage::Changed;
		// persist
		Database* pDatabase = getDatabase();
		Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);

		; _SYDNEY_ASSERT(m_pIndex->isOffline() == m_bPostOffline);

		// increment timestamp value
		addTimestamp(getTransaction());
		m_iStatus = Progress::AlterUsage::Succeeded;

	} else { // redo
		// redo never call this
		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeIndex::AlterUsage::undo -- error recovery
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
Manager::SystemTable::ReorganizeIndex::AlterUsage::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter index failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::AlterUsage::Changed:
		{
			// change back to original
			m_pIndex->setOffline(m_bPrevOffline);
			m_pIndex->touch();

			// persist again
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Index(*pDatabase).store(getTransaction(), m_pIndex);
			break;
		}
	case Progress::AlterUsage::None:
	default:
		{
			break;
		}
	}
}

//
// Copyright (c) 2006, 2007, 2008, 2009, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
