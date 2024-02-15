// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeTable.cpp -- Implementation of classes concerning with table reorganization
// 
// Copyright (c) 2006, 2007, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/Column.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/Partition.h"
#include "Schema/ReorganizeTable.h"
#include "Schema/ReorganizeColumn.h"
#include "Schema/ReorganizeConstraint.h"
#include "Schema/SystemTable_AreaContent.h"
#include "Schema/SystemTable_Column.h"
#include "Schema/SystemTable_Constraint.h"
#include "Schema/SystemTable_Field.h"
#include "Schema/SystemTable_File.h"
#include "Schema/SystemTable_Index.h"
#include "Schema/SystemTable_Key.h"
#include "Schema/SystemTable_Partition.h"
#include "Schema/SystemTable_Table.h"
#include "Schema/TableMap.h"
#ifdef DEBUG
#include "Schema/Debug.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/SystemTable.h"
#include "Exception/TableNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "Opt/Argument.h"

#include "Statement/TableDefinition.h"
#include "Statement/DropTableStatement.h"
#include "Statement/AlterTableStatement.h"
#include "Statement/AlterTableAction.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{
	namespace Progress
	{
		// ENUM
		// $$::Progress::Create::Value -- Progress value for create table
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Create
		{
			enum Value
			{
				None,					// initial value
				FileCreated,			// files are created
				Persisting,				// persisting has been started
				Persisted,				// all system tabels are persisted
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Drop::Value -- Progress value for drop table
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Drop
		{
			enum Value
			{
				None,					// initial value
				Dropped,				// drop flag is set
				Logged,					// log data is stored
				Stored,					// persisting is finished
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::AlterArea::Value -- Progress value for alter table set/drop area
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct AlterArea
		{
			enum Value
			{
				None,					// initial value
				Moved,					// files are moved
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::AlterName::Value -- Progress value for alter table rename
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct AlterName
		{
			enum Value
			{
				None,					// initial value
				Moved,					// files are moved
				Succeeded,				// process succeeded
				ValueNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Base::Base -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	bool bIsTemporary_
//	bool bRedo_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::Base::
Base(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 bool bIsTemporary_,
	 bool bRedo_,
	 bool bRollforward_)
	: ReorganizeExecutor(cTrans_),
	  m_pDatabase(pDatabase_),
	  m_bIsTemporary(bIsTemporary_),
	  m_bRedo(bRedo_),
	  m_bRollforward(bRollforward_)
{
	// open the database so that cache object are not freed
	m_pDatabase->open();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeTable::Base::
~Base()
{
	try {
		m_pDatabase->close(m_bRedo /* if redo, close database in volatile mode */);
	} catch (...) {
		// ignore all the exception in destructor
		;
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Base::lock -- lock operation
//
// NOTES
//
// ARGUMENTS
//	Schema::Hold::Operation::Value eOperation_
//	Schema::Table* pTable_  = 0
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

// lock operation
void
Manager::SystemTable::ReorganizeTable::Base::
lock(Schema::Hold::Operation::Value eOperation_, Schema::Table* pTable_ /* = 0 */)
{
	// Convert the lock for the tuple in database system table
	// from read-for-write into the operation

	convert(getTransaction(), Hold::Target::MetaTuple,
			Lock::Name::Category::Tuple, Hold::Operation::ReadForWrite,
			Lock::Name::Category::Tuple, eOperation_,
			getDatabase()->getID());

	// Lock the logical log file of the database in which the target table exists
	// for operating tuples in database system table

	hold(getTransaction(), Hold::Target::LogicalLog,
		 Lock::Name::Category::Tuple, eOperation_,
		 0, Trans::Log::File::Category::Database, getDatabase());

	if (pTable_) {
		// Lock the database and the table for operating the table
		hold(getTransaction(), Hold::Target::Database,
			 Lock::Name::Category::Table, eOperation_,
			 getDatabase()->getID());
		ObjectTree::Table::hold(
			getTransaction(), Lock::Name::Category::Table, eOperation_,
			getDatabase()->getID(), pTable_->getID());
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::TableElement
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::TableElement::TableElement -- constructor
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

Manager::SystemTable::ReorganizeTable::TableElement::
TableElement(Trans::Transaction& cTrans_,
			 Database* pDatabase_,
			 Table* pTable_,
			 bool bRedo_)
	: Base(cTrans_, pDatabase_, (pTable_?pTable_->isTemporary():false), bRedo_),
	  m_pTable(pTable_)
{
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::TableElement::~TableElement -- destructor
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
Manager::SystemTable::ReorganizeTable::TableElement::
~TableElement()
{
	// do nothing
	;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::TableElement::lock -- lock operation
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

// lock operation
void
Manager::SystemTable::ReorganizeTable::TableElement::
lock(Schema::Hold::Operation::Value eOperation_)
{
	// call superclass's method with Table object
	Base::lock(eOperation_, getTable());
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::TableElement::import -- importing
//
// NOTES
//
// ARGUMENTS
//	const ModVector<Schema::Field*>& vecSourceFields_
//	const ModVector<Schema::Field*>& vecTargetFields_
//	bool bCheckConstraint_
//	bool bRowIDOrder_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeTable::TableElement::
import(const ModVector<Schema::Field*>& vecSourceFields_,
	   const ModVector<Schema::Field*>& vecTargetFields_,
	   bool bCheckConstraint_,
	   bool bRowIDOrder_)
{
	// while importing, no log
	bool bPrevNoLog = getTransaction().setNoLog(true);

	// When return from this scope, revert the nolog, nolock flag
	Common::AutoCaller1<Trans::Transaction, bool>
		cAutoRevertNoLog(&getTransaction(),
						 syd_reinterpret_cast<Common::AutoCaller1<Trans::Transaction, bool>::FunctionPointer>(
							 &Trans::Transaction::setNoLog),
						 bPrevNoLog);

	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "Importing");
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "Importing");
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "StartImport");

	// create an analyzer to create import program
	Opt::ImportArgument cArgument(getTable(),
								  vecSourceFields_,
								  vecTargetFields_,
								  bCheckConstraint_,
								  bRowIDOrder_);

	// execute using the analyzer
	executeProgram(cArgument, getDatabase());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "Imported");
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "Imported");
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateIndex", "Import");
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::TableElement::getImportField --
//		create importing field pair using source-destination
//
// NOTES
//
// ARGUMENTS
//	const File* pFile_
//	ModVector<Schema::Field*>& vecSourceFields_
//	ModVector<Schema::Field*>& vecTargetFields_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeTable::TableElement::
getImportField(const File* pFile_,
			   ModVector<Schema::Field*>& vecSourceFields_,
			   ModVector<Schema::Field*>& vecTargetFields_)
{
	; _SYDNEY_ASSERT(pFile_);

	// create source and target field vector using source-destination connections
	const ModVector<Field*>& vecField = pFile_->getField(getTransaction());
	ModSize n = vecField.getSize();
	vecSourceFields_.reserve(n);
	vecTargetFields_.reserve(n);
	for (ModSize i = 0; i < n; ++i) {
		Field* pField = vecField[i];
		if (pField->isPutable()) {
			// if the field is putable, it should be imported
			vecTargetFields_.pushBack(pField);
			if (Field* pSource = pField->getSource(getTransaction())) {
				// In index importing, source data is always obtained from source field
				vecSourceFields_.pushBack(pSource);
			} else {
				// for now, only objectID holds this condition
				; _SYDNEY_ASSERT(pField->isObjectID());
				vecSourceFields_.pushBack(0);
			}
		}
	}
	; _SYDNEY_ASSERT(vecSourceFields_.getSize() == vecTargetFields_.getSize());
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::TableDefinition* pStatement_
//	bool bIsTemporary_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Statement::TableDefinition* pStatement_,
	   bool bIsTemporary_)
	: Base(cTrans_, pDatabase_, bIsTemporary_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pTable(),
	  m_vecReferencedTable()
{}

// for redoing
Manager::SystemTable::ReorganizeTable::Create::
Create(Trans::Transaction& cTrans_,
	   Database* pDatabase_,
	   const Schema::LogData* pLogData_,
	   bool bIsTemporary_)
	: Base(cTrans_, pDatabase_, bIsTemporary_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Create::Succeeded),
	  m_pTable(),
	  m_vecReferencedTable()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Create::~Create -- destructor
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

Manager::SystemTable::ReorganizeTable::Create::
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
//	Schema::Manager::SystemTable::ReorganizeTable::Create::execute -- execute
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
Manager::SystemTable::ReorganizeTable::Create::
execute()
{
	if (m_pStatement) { // normal case
		// prepare log data
		LogData cLogData(LogData::Category::CreateTable);

		// create new table object
		// log data is also filled
		m_pTable = Table::create(*getDatabase(), *m_pStatement, cLogData, getTransaction());
		; _SYDNEY_ASSERT(m_pTable.get() || Configuration::isCanceledWhenDuplicated());

		if (m_pTable.get() == 0) {
			// another object has same name
			return Result::None;
		}

		// Reset progress status
		m_iStatus = Progress::Create::None;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "Created");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock and log are needed only non-temporary tables
		if (!isTemporary()) {

			// lock meta-tuple and logical log
			lock(Hold::Operation::ReadWrite);

			// delay for test
			objectCheckWait();

			// store log data
			storeDatabaseLog(cLogData, getDatabase());
		}

	} else { // redoing
		; _SYDNEY_ASSERT(m_pLogData);
		// create new object 
		m_pTable = Table::create(getTransaction(), *getDatabase(), *m_pLogData);
		; _SYDNEY_ASSERT(m_pTable.get());
	}

	// create table files
	m_pTable->create(getTransaction());
	m_iStatus = Progress::Create::FileCreated;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "FileCreated");

	// record referenced tables
	if (m_pTable->hasReferencedTable(getTransaction())) {
		m_vecReferencedTable = m_pTable->getReferencedTable(getTransaction());
	}

	// persisting should be done from parent to children in creation
	m_iStatus = Progress::Create::Persisting;

	Database* pDatabase = getDatabase();
	Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "TableStored");

	Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "ConstraintStored");

	Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "IndexStored");

	Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "KeyStored");

	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "FileStored");

	Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "ColumnStored");

	Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "FieldStored");

	// persisting referenced tables
	if (ModSize n = m_vecReferencedTable.getSize()) {
		for (ModSize i = 0; i < n; ++i) {
			; _SYDNEY_ASSERT(m_vecReferencedTable[i]->getID() != m_pTable->getID());
			// persist table and constraint objects
			Schema::SystemTable::Table(*pDatabase).store(getTransaction(), *m_vecReferencedTable[i]);
			Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *m_vecReferencedTable[i]);
		}
	}

	// return value
	Result::Value iResult = Result::None;

	if (!isTemporary()) {

		// persist area-object connection
		Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "CreateTable", "AreaContentStored");

		// persist ObjectID value
		Schema::ObjectID::persist(getTransaction(), getDatabase());

		m_iStatus = Progress::Create::Persisted;

		if (m_pStatement) { // do only in normal cases
			// increment timestamp
			addTimestamp(getTransaction());

			// if parameter Schema_CauseCheckpoint is set, cause checkpoint after commit
			iResult = (isCauseCheckpoint()
					   ? (Result::NeedCheckpoint | Result::NeedReCache)
					   : Result::NeedReCache);
		}
	}

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pTable.get());

	m_iStatus = Progress::Create::Succeeded;
	return iResult;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Create::undo -- error recovery
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
Manager::SystemTable::ReorganizeTable::Create::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "create table failed." << ModEndl;
	}

	// withdraw name from object name reservation
	Manager::ObjectName::withdraw(m_pTable.get());

	// set drop flag
	m_pTable->drop(getTransaction());

	switch (m_iStatus) {
	case Progress::Create::Succeeded:
	case Progress::Create::Persisted:
	case Progress::Create::Persisting:
		{
			// Even if any kinds of system table has not been stored,
			// store should be called for all the system tables to clear object's status
			Database* pDatabase = getDatabase();
			if (!isTemporary()) {
				Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
			}

			// persisting referenced tables
			if (ModSize n = m_vecReferencedTable.getSize()) {
				for (ModSize i = 0; i < n; ++i) {
					; _SYDNEY_ASSERT(m_vecReferencedTable[i]->getID() != m_pTable->getID());
					// persist table and constraint objects
					Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(),
																		  *m_vecReferencedTable[i]);
					Schema::SystemTable::Table(*pDatabase).store(getTransaction(),
																	 *m_vecReferencedTable[i]);
				}
			}
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);

			if (!isTemporary()) {
				Schema::ObjectID::persist(getTransaction(), getDatabase());
			}
			break;
		}
	case Progress::Create::FileCreated:
		{
			m_pTable->destroy(getTransaction(), true, true);
			break;
		}
	case Progress::Create::None:
	default:
		break;
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::DropTableStatement* pStatement_
//	bool bIsTemporary_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Statement::DropTableStatement* pStatement_,
	 bool bIsTemporary_)
	: Base(cTrans_, pDatabase_, bIsTemporary_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pTable(),
	  m_vecReferencedTable()
{}

// for redoing
Manager::SystemTable::ReorganizeTable::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 const Schema::LogData* pLogData_,
	 bool bIsTemporary_,
	 bool bRollforward_)
	: Base(cTrans_, pDatabase_, bIsTemporary_, true /* redo */, bRollforward_),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded),
	  m_pTable(),
	  m_vecReferencedTable()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizeTable::Drop::
~Drop()
{
	// drop statement's error recovery is retrying
	// so, retry is called in the catch clause in execute() method
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeTable::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.

	try {
		if (m_pStatement) { // normal case
			// get target table name from the statement
			const Object::Name& cName = Table::getName(*m_pStatement);

			// get table object
			m_pTable = syd_reinterpret_cast<const Table*>(getDatabase()->getTable(cName, getTransaction(),
																			  true /* internal */));
			if (!m_pTable.get()) {
				if (m_pStatement->isIfExists()) {
					// With 'IF EXISTS', ignore this case
					return Result::None;
				} else {
					_SYDNEY_THROW2(Exception::TableNotFound, cName, getDatabase()->getName());
				}
			}
			if (m_pTable->isSystem()) {
				// system table cannot be dropped
				_SYDNEY_THROW1(Exception::SystemTable, cName);
			}

			if (!isTemporary()) {
				// lock database and table
				lock(Hold::Operation::Drop, m_pTable.get());
			}

			// prepare log data
			LogData cLogData(LogData::Category::DropTable);

			// check cancellation
			Manager::checkCanceled(getTransaction());

			// set drop flag
			// log data is also filled
			Table::drop(*m_pTable, cLogData, getTransaction());
			m_iStatus = Progress::Drop::Dropped;
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "Dropped");

			if (!isTemporary()) {
				// store log data to database log file
				storeDatabaseLog(cLogData, getDatabase());
				m_iStatus = Progress::Drop::Logged;
			}
		} else { // redo

			// get table id from log data
			ObjectID::Value iID = Table::getObjectID(*m_pLogData);

			// get table object
			m_pTable = getDatabase()->loadTable(getTransaction()).get(iID);
			if (!m_pTable.get()) {
				// if target table already deleted, do nothing
				return Result::None;
			}

			// set drop flag
			m_pTable->drop(getTransaction(), isRollforward() ? false : true /* recovery */);
			m_iStatus = Progress::Drop::Dropped;
		}

		// get referenced tables
		if (m_pTable->hasReferencedTable(getTransaction())) {
			m_vecReferencedTable = m_pTable->getReferencedTable(getTransaction());
		}

		// persisting referenced tables
		Database* pDatabase = getDatabase();
		if (ModSize n = m_vecReferencedTable.getSize()) {
			for (ModSize i = 0; i < n; ++i) {
				if (m_vecReferencedTable[i]->getID() != m_pTable->getID()) {
					// persist table and constraint objects
					Schema::SystemTable::Table(*pDatabase).store(getTransaction(),
																	 *m_vecReferencedTable[i]);
					Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(),
																		  *m_vecReferencedTable[i]);
				}
			}
		}

		Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "FieldStored");

		Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "ColumnStored");

		Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "ConstraintStored");

		Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "KeyStored");

		Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "FileStored");

		Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "IndexStored");

		Schema::Partition* pPartition = m_pTable->getPartition(getTransaction());
		if (pPartition) {
			Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), *pPartition);
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "PartitionStored");
		}

		// [NOTES]
		//	m_pTable object will be deleted, m_pTable cannot be refered any more
		Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
		m_pTable = static_cast<Table*>(0);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "TableStored");

		if (!isTemporary()) {
			Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "DropTable", "AreaContentStored");
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
//	Schema::Manager::SystemTable::ReorganizeTable::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizeTable::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop table failed." << ModEndl;
	}
	bool bResult = false;

	// error recovery of drop is retrying
	// unless the process has been failed before storing log data
	switch (m_iStatus) {
	case Progress::Drop::Logged:
		{
			// wait for a while before retrying
			retryWait();

			SydErrorMessage << "retry drop table." << ModEndl;

			// persist again
			Database* pDatabase = getDatabase();
			if (ModSize n = m_vecReferencedTable.getSize()) {
				for (ModSize i = 0; i < n; ++i) {
					if (m_vecReferencedTable[i]->getID() != m_pTable->getID()) {
						Schema::SystemTable::Table(*pDatabase).store(getTransaction(),
																		 *m_vecReferencedTable[i]);
						Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(),
																			  *m_vecReferencedTable[i]);
					}
				}
			}
			if (m_pTable.get()) {
				Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *m_pTable);
				Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *m_pTable);
				Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *m_pTable);
				Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *m_pTable);
				Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
				Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
				Schema::Partition* pPartition = m_pTable->getPartition(getTransaction());
				if (pPartition) {
					Schema::SystemTable::Partition(*pDatabase).store(getTransaction(), *pPartition);
				}
				Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
			}
			if (!isTemporary()) {
				Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
			}
			// retry succeeded
			SydErrorMessage << "retry drop table succeeded." << ModEndl;

			// thru.
		}
	case Progress::Drop::Stored:
		{
			if (!isTemporary()) {
				// increment timestamp value
				addTimestamp(getTransaction());
			}
			bResult = true;
			break;
		}
	case Progress::Drop::Dropped:
		{
			// cancel drop flag
			m_pTable->undoDrop(getTransaction());
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
// Manager::SystemTable::ReorganizeTable::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	const Statement::AlterTableStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Statement::AlterTableStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0)
{}

// for redoing
Manager::SystemTable::ReorganizeTable::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  const Schema::LogData* pLogData_,
	  bool bRollforward_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, true /* redo */, bRollforward_),
	  m_pStatement(0), m_pLogData(pLogData_)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizeTable::Alter::
~Alter()
{
	// do nothing
	;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::Alter::execute -- execute
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
Manager::SystemTable::ReorganizeTable::Alter::
execute()
{
	if (m_pStatement) { // normal case
		// get target name from the statement
		const Object::Name& cName = Table::getName(*m_pStatement);
		// obtain table object
		Table* pTable = getDatabase()->getTable(cName, getTransaction(), true /* internal */);
		if (!pTable) {
			_SYDNEY_THROW2(Exception::TableNotFound, cName, getDatabase()->getName());
		}
		if (pTable->isSystem()) {
			// alter cannot applied to system table
			_SYDNEY_THROW1(Exception::SystemTable, cName);
		}

		// use different object according to action of alter table
		Statement::AlterTableAction* pAction = m_pStatement->getAlterTableAction();
		; _SYDNEY_ASSERT(pAction);

		switch (pAction->getActionType()) {
		case Statement::AlterTableAction::SetArea:
		case Statement::AlterTableAction::DropArea:
			{
				return ReorganizeTable::AlterArea(getTransaction(), getDatabase(), pTable, pAction).execute();
			}
		case Statement::AlterTableAction::Rename:
			{
				return ReorganizeTable::AlterName(getTransaction(), getDatabase(), pTable, pAction).execute();
			}
		case Statement::AlterTableAction::AddColumn:
			{
				return ReorganizeColumn::Add(getTransaction(), getDatabase(), pTable, pAction).execute();
			}
		case Statement::AlterTableAction::AlterColumn:
			{
				return ReorganizeColumn::Alter(getTransaction(), getDatabase(), pTable, pAction).execute();
			}
#ifndef SYD_COVERAGE
		case Statement::AlterTableAction::DropColumn:
			{
				// yet
				_SYDNEY_THROW0(Exception::NotSupported);
				//return ReorganizeColumn::Drop(getTransaction(), getDatabase(), pTable, pAction).execute();
			}
#endif
		case Statement::AlterTableAction::AddTableConstraint:
			{
				return ReorganizeConstraint::Add(getTransaction(), getDatabase(), pTable, pAction).execute();
			}
		case Statement::AlterTableAction::DropTableConstraint:
			{
				return ReorganizeConstraint::Drop(getTransaction(), getDatabase(), pTable, pAction).execute();
			}
		default:
			{
				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
	} else { // redoing
		// get table id from log data
		ObjectID::Value iID = Table::getObjectID(*m_pLogData);
		// obtain table object
		Table* pTable = getDatabase()->getTable(iID, getTransaction(), true /* internal */);
		; _SYDNEY_ASSERT(pTable);

		switch (m_pLogData->getSubCategory()) {
		case LogData::Category::AlterTable:
			{
				return ReorganizeTable::AlterArea(getTransaction(), getDatabase(), pTable, m_pLogData).execute();
			}
		case LogData::Category::RenameTable:
			{
				return ReorganizeTable::AlterName(getTransaction(), getDatabase(), pTable, m_pLogData, isRollforward()).execute();
			}
		case LogData::Category::AddColumn:
			{
				return ReorganizeColumn::Add(getTransaction(), getDatabase(), pTable, m_pLogData).execute();
			}
		case LogData::Category::AlterColumn:
			{
				return ReorganizeColumn::Alter(getTransaction(), getDatabase(), pTable, m_pLogData).execute();
			}
#ifndef SYD_COVERAGE
		case LogData::Category::DropColumn:
			{
				// yet
				_SYDNEY_THROW0(Exception::NotSupported);
				//return ReorganizeColumn::Drop(getTransaction(), getDatabase(), pTable, m_pLogData).execute();
			}
#endif
		case LogData::Category::AddConstraint:
			{
				return ReorganizeConstraint::Add(getTransaction(), getDatabase(), pTable, m_pLogData).execute();
			}
		case LogData::Category::DropConstraint:
			{
				return ReorganizeConstraint::Drop(getTransaction(), getDatabase(), pTable, m_pLogData).execute();
			}
		default:
			{
				; _SYDNEY_ASSERT(false);
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
	}
	// never reach
	return Result::None;
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::AlterArea
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::AlterArea::AlterArea -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//	const Statement::AlterTableAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::AlterArea::
AlterArea(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  const Statement::AlterTableAction* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::AlterArea::Succeeded),
	  m_pTable(syd_reinterpret_cast<const Table*>(pTable_)),
	  m_vecPrevAreaID(AreaCategory::ValueNum, Object::ID::Invalid),
	  m_vecPostAreaID(AreaCategory::ValueNum, Object::ID::Invalid)
{}

// for redoing
Manager::SystemTable::ReorganizeTable::AlterArea::
AlterArea(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::AlterArea::Succeeded),
	  m_pTable(syd_reinterpret_cast<const Table*>(pTable_)),
	  m_vecPrevAreaID(AreaCategory::ValueNum, Object::ID::Invalid),
	  m_vecPostAreaID(AreaCategory::ValueNum, Object::ID::Invalid)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::AlterArea::~AlterArea -- destructor
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

Manager::SystemTable::ReorganizeTable::AlterArea::
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
//	Schema::Manager::SystemTable::ReorganizeTable::AlterArea::execute -- execute
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
Manager::SystemTable::ReorganizeTable::AlterArea::
execute()
{
	if (m_pStatement) { // normal case
		// Prepare log data
		LogData cLogData(LogData::Category::AlterTable);

		// check statement
		// log data is also filled
		if (!Table::alterArea(getTransaction(), *m_pTable, *m_pStatement,
							  m_vecPrevAreaID, m_vecPostAreaID, cLogData)) {
			// Nothing changed
			return Result::None;
		}
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterTable", "Altered");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::Drop, m_pTable.get());

		// store log data
		storeDatabaseLog(cLogData, getDatabase());

		// Reset progress status
		m_iStatus = Progress::AlterArea::None;

		// change area specifications and move files 
		m_pTable->move(getTransaction(), m_vecPrevAreaID, m_vecPostAreaID);
		// tell object is dirty
		m_pTable->touch();
		m_iStatus = Progress::AlterArea::Moved;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterTable", "Moved");

		// persist system tables
		Database* pDatabase = getDatabase();
		Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterTable", "TableStored");

		Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterTable", "FileStored");

		Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterTable", "AreaContentStored");

		// persist ObjectID
		Schema::ObjectID::persist(getTransaction(), getDatabase());

		// increment timestamp value
		addTimestamp(getTransaction());
		m_iStatus = Progress::AlterArea::Succeeded;

	} else { // redo
		// alter table set/drop area does not use this class in redoing
		; _SYDNEY_ASSERT(false);
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::AlterArea::undo -- error recovery
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
Manager::SystemTable::ReorganizeTable::AlterArea::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter table failed." << ModEndl;
	}
	switch (m_iStatus) {
	case Progress::AlterArea::Moved:
		{
			// move files back to original position
			m_pTable->move(getTransaction(), m_vecPostAreaID, m_vecPrevAreaID, true /* undo */);
			// tell the object dirty again
			m_pTable->touch();
			// Even if any kinds of system table has not been stored,
			// store should be called for all the system tables to clear object's status
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

			// persist objectID because new objectID might have been persisted
			Schema::ObjectID::persist(getTransaction(), getDatabase());
			break;
		}
	case Progress::AlterArea::None:
	default:
		{
			// do nothing
			break;
		}
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::AlterName
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::AlterName::AlterName -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//	const Statement::AlterTableAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::AlterName::
AlterName(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  const Statement::AlterTableAction* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::AlterName::Succeeded),
	  m_pTable(syd_reinterpret_cast<const Table*>(pTable_)),
	  m_cPrevName(),
	  m_cPostName()
{}

// for redoing
Manager::SystemTable::ReorganizeTable::AlterName::
AlterName(Trans::Transaction& cTrans_,
		  Database* pDatabase_,
		  Table* pTable_,
		  const Schema::LogData* pLogData_,
		  bool bRollforward_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, true /* redo */, bRollforward_),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::AlterName::Succeeded),
	  m_pTable(syd_reinterpret_cast<const Table*>(pTable_)),
	  m_cPrevName(),
	  m_cPostName()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::AlterName::~AlterName -- destructor
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

Manager::SystemTable::ReorganizeTable::AlterName::
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
//	Schema::Manager::SystemTable::ReorganizeTable::AlterName::execute -- execute
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
Manager::SystemTable::ReorganizeTable::AlterName::
execute()
{
	if (m_pStatement) { // normal case
		// prepare log data
		LogData cLogData(LogData::Category::RenameTable);

		// check statement and fill log data
		if (!Table::alterName(getTransaction(), *m_pTable, *m_pStatement, m_cPostName, cLogData)) {
			// nothing changed
			return Result::None;
		}
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameTable", "Altered");

		// check cancellation
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::Drop, m_pTable.get());

		// store log data
		storeDatabaseLog(cLogData, getDatabase());

	} else { // redoing
		// get post name from log data
		m_cPostName = m_pLogData->getString(Table::Log::Rename::PostName);
	}

	// get current name
	m_cPrevName = m_pTable->getName();

	// Reset progress status
	m_iStatus = Progress::AlterArea::None;

	// change table's name and move files
	bool redo = isRedo();
	if (isRollforward() == true) redo = false;
	m_pTable->moveRename(getTransaction(), m_cPrevName, m_cPostName,
						 false /* not undo */, redo);
	// tell the object is dirty
	m_pTable->touch();
	m_iStatus = Progress::AlterName::Moved;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameTable", "Moved");

	Database* pDatabase = getDatabase();
	Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameTable", "TableStored");

	Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameTable", "IndexStored");

	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "RenameTable", "FileStored");

	if (m_pStatement) {
		// increment timestamp value
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::AlterName::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeTable::AlterName::undo -- error recovery
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
Manager::SystemTable::ReorganizeTable::AlterName::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter table failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::AlterName::Moved:
		{
			// move files back to original position and modify table's name
			m_pTable->moveRename(getTransaction(), m_cPostName, m_cPrevName, true /* undo */);
			// tell the object is dirty again
			m_pTable->touch();
			// persist system tables
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
			Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
			break;
		}
	case Progress::AlterName::None:
	default:
		{
			break;
		}
	}
}

//
// Copyright (c) 2006, 2007, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
