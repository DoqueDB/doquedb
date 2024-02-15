// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeConstraint.cpp -- Implementation of classes concerning with constraint reorganization
// 
// Copyright (c) 2006, 2007, 2008, 2011, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/Constraint.h"
#include "Schema/Database.h"
#include "Schema/ErrorRecovery.h"
#include "Schema/FakeError.h"
#include "Schema/Field.h"
#include "Schema/File.h"
#include "Schema/Hold.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/ReorganizeConstraint.h"
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

#include "Common/Assert.h"
#include "Common/AutoCaller.h"

#include "Exception/NotSupported.h"

#include "Lock/Name.h"

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
		// $$::Progress::Add::Value -- Progress value for alter table add constraint
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Add
		{
			enum Value
			{
				None,					// Initial value
				Prepared,				// constraint is created
				Storing,				// system table persisting are started
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Alter::Value -- Progress value for alter table alter constraint
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Drop
		{
			enum Value
			{
				None,					// initial value
				Dropped,				// drop flag is set
				Succeeded,				// process succeeded
				ValuNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeConstraint::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Base::Base -- constructor
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

Manager::SystemTable::ReorganizeConstraint::Base::
Base(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 Table* pTable_,
	 bool bRedo_)
	: Super(cTrans_, pDatabase_, pTable_, bRedo_)
{
	// Database should have been opened
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeConstraint::Base::
~Base()
{
	// nothing to do
	;
}

// FUNCTION protected
//	Schema::Manager::SystemTable::ReorganizeConstraint::Base::dropIndex -- drop corresponding index if needed
//
// NOTES
//
// ARGUMENTS
//	Constraint* pConstraint_						   
//	bool bRecovery_
//	bool bNoUnset_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeConstraint::Base::
dropIndex(Constraint* pConstraint_, bool bRecovery_, bool bNoUnset_)
{
	if ((pConstraint_->getCategory() != Constraint::Category::ReferedKey)
		&& (pConstraint_->getCategory() != Constraint::Category::OldPrimaryKey)
		&& (pConstraint_->getCategory() != Constraint::Category::OldUnique)) {
		Index* pIndex = pConstraint_->getIndex(getTransaction());
		if (pIndex) {
			pIndex->drop(getTransaction(), bRecovery_, bNoUnset_);
		}
	}
}

///////////////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeConstraint::Add
///////////////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Add::Add -- constructor
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

Manager::SystemTable::ReorganizeConstraint::Add::
Add(Trans::Transaction& cTrans_,
	Database* pDatabase_,
	Table* pTable_,
	const Statement::AlterTableAction* pStatement_)
	: Base(cTrans_, pDatabase_, pTable_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Add::Succeeded),
	  m_pConstraint(),
	  m_vecReferencedTable()
{}

// for redoing
Manager::SystemTable::ReorganizeConstraint::Add::
Add(Trans::Transaction& cTrans_,
	Database* pDatabase_,
	Table* pTable_,
	const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Add::Succeeded),
	  m_pConstraint(),
	  m_vecReferencedTable()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Add::~Add -- destructor
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

Manager::SystemTable::ReorganizeConstraint::Add::
~Add()
{
	if (m_pStatement && (m_iStatus != Progress::Add::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Add::execute -- execute
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
Manager::SystemTable::ReorganizeConstraint::Add::
execute()
{
	if (m_pStatement) { // normal case

		// Poling interrupt message from client
		Manager::checkCanceled(getTransaction());

		// lock meta-tuple, logical log, database and table
		lock(Hold::Operation::Drop);

		// declare log object
		LogData cLogData(LogData::Category::AddConstraint);

		// reset progress status
		// NOTE: reset here because add change the contents of Table
		m_iStatus = Progress::Add::None;

		// prepare altering and create log data
		if (!Table::alterAddConstraint(getTransaction(), *getTable(),
									   *m_pStatement, m_pConstraint, m_vecReferencedTable,
									   cLogData)) {
			// if no change, finish the method
			return Result::None;
		}
		m_iStatus = Progress::Add::Prepared;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "Prepared");

		// Store the log data into the logical log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "Logged");

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		// prepare altering using log data
		if (!Table::alterAddConstraint(getTransaction(), *getTable(),
									   m_pConstraint, m_vecReferencedTable,
									   *m_pLogData)) {
			// nothing to do
			return Result::None;
		}
	}

	// create file
	m_pConstraint->create(getTransaction());

	// import existing data
	; _SYDNEY_ASSERT(m_pConstraint->getIndex(getTransaction()));
	; _SYDNEY_ASSERT(m_pConstraint->getIndex(getTransaction())->getFile(getTransaction()));
	File* pFile = m_pConstraint->getIndex(getTransaction())->getFile(getTransaction());

	ModVector<Field*> vecSourceFields;
	ModVector<Field*> vecTargetFields;
	getImportField(pFile, vecSourceFields, vecTargetFields);
	import(vecSourceFields, vecTargetFields, !isRedo() /* check constraint if not redo */,
		   false /* not in rowid order */);

	// tell the table is dirty
	getTable()->touch();

	m_iStatus = Progress::Add::Storing;

	// store schema objects
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Table(*pDatabase).store(getTransaction(), getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "TableStored");
	Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "ConstraintStored");
	Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "IndexStored");
	Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "KeyStored");
	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pConstraint);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "FileStored");
	Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "ColumnStored");
	Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "FieldStored");

	// persisting referenced tables
	if (ModSize n = m_vecReferencedTable.getSize()) {
		for (ModSize i = 0; i < n; ++i) {
			// persist table and constraint objects
			Schema::SystemTable::Table(*pDatabase).store(getTransaction(), *m_vecReferencedTable[i]);
			Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *m_vecReferencedTable[i]);
		}
	}
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddConstraint", "ReferencedTableStored");

	// store area content
	Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

	// persist ObjectID value
	Schema::ObjectID::persist(getTransaction(), getDatabase());

	if (m_pStatement) { // do only for normal case
		// increment the timestamp
		addTimestamp(getTransaction());
	}

	m_iStatus = Progress::Add::Succeeded;

	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Add::undo -- error recovery
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
Manager::SystemTable::ReorganizeConstraint::Add::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter table failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Add::Prepared:
	case Progress::Add::Storing:
		{
			// drop constraint
			m_pConstraint->drop(getTransaction(), false /* no recovery */, true /* need persist */);
			// If the constraint is not refered key, drop corresponding index
			dropIndex(m_pConstraint.get(), false, true);

			getTable()->touch();

			// Even if any kinds of system table has not been stored,
			// store should be called for all the system tables to clear object's status

			// store area content
			Database* pDatabase = getDatabase();
			Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

			// persisting referenced tables
			if (ModSize n = m_vecReferencedTable.getSize()) {
				for (ModSize i = 0; i < n; ++i) {
					// persist table and constraint objects
					Schema::SystemTable::Table(*pDatabase).store(getTransaction(), *m_vecReferencedTable[i]);
					Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *m_vecReferencedTable[i]);
				}
			}
			Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pConstraint);
			Schema::SystemTable::Key(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Index(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Constraint(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Table(*pDatabase).store(getTransaction(), getTable());

			if (m_iStatus >= Progress::Add::Storing) {
				// persist ObjectID value
				Schema::ObjectID::persist(getTransaction(), getDatabase());
			}
			break;
		}
	case Progress::Add::None:
	default:
		{
			break;
		}
	}
}

///////////////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeConstraint::Drop
///////////////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Database* pDatabase_
//	Table* pTable_
//	const Statement::DropTableAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeConstraint::Drop::
Drop(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 Table* pTable_,
	 const Statement::AlterTableAction* pStatement_)
	: Base(cTrans_, pDatabase_, pTable_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Drop::Succeeded)
{}

// for redoing
Manager::SystemTable::ReorganizeConstraint::Drop::
Drop(Trans::Transaction& cTrans_,
	Database* pDatabase_,
	Table* pTable_,
	const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Drop::Succeeded)
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Drop::~Drop -- destructor
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

Manager::SystemTable::ReorganizeConstraint::Drop::
~Drop()
{
	if (m_pStatement && (m_iStatus != Progress::Drop::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeConstraint::Drop::
execute()
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeConstraint::Drop::undo -- error recovery
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
Manager::SystemTable::ReorganizeConstraint::Drop::
undo()
{
	//...
}

//
// Copyright (c) 2006, 2007, 2008, 2011, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
