// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeTable.cpp -- Implementation of classes concerning with table reorganization
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DSchema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "DSchema/ReorganizeTable.h"

#include "Schema/ErrorRecovery.h"

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
#include "Statement/Identifier.h"

#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

#include "ModUnicodeOstrStream.h"

_SYDNEY_USING
_SYDNEY_DSCHEMA_USING

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
				LocalSucceeded,			// local schema is persisted
				Succeeded,				// whole process succeeded
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
				LocalSucceeded,			// local schema is persisted
				Succeeded,				// whole process succeeded
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
// Manager::SystemTable::ReorganizeTable::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	const Statement::TableDefinition* pStatement_
//	bool bIsTemporary_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::Create::
Create(Trans::Transaction& cTrans_,
	   Schema::Database* pDatabase_,
	   const Statement::TableDefinition* pStatement_,
	   bool bIsTemporary_)
	: Super(cTrans_, pDatabase_, pStatement_, bIsTemporary_),
	  m_iStatus(Progress::Create::Succeeded)
{}

// for redoing
Manager::SystemTable::ReorganizeTable::Create::
Create(Trans::Transaction& cTrans_,
	   Schema::Database* pDatabase_,
	   const Schema::LogData* pLogData_,
	   bool bIsTemporary_)
	: Super(cTrans_, pDatabase_, pLogData_, bIsTemporary_),
	  m_iStatus(Progress::Create::Succeeded)
{}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Create::~Create -- destructor
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
	if (getStatement() && (m_iStatus != Progress::Create::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
	close();
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Create::execute -- execute
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

Schema::Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeTable::Create::
execute()
{
	m_iStatus = Progress::Create::None;

	// first of all, create local table using super class
	Schema::Manager::SystemTable::Result::Value iResult = Super::execute();

	m_iStatus = Progress::Create::LocalSucceeded;

	// execute same statement on cascade servers
	open(getDatabase(), getTransaction());
	executeStatement(getSQL());

	m_iStatus = Progress::Create::Succeeded;

	return iResult;
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Create::undo -- error recovery
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
	if (m_iStatus >= Progress::Create::LocalSucceeded) {
		if (!getTransaction().isCanceledStatement()) {
			SydErrorMessage << "create table failed in cascade." << ModEndl;
		}
		// cancel all cascade servers
		cancelStatement(getCancelSQL(),
						getDatabase(),
						getTransaction());

		// cancel local
		Super::undo();
	}
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeTable::Create::getSQL -- get sql for execution
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
Manager::SystemTable::ReorganizeTable::Create::
getSQL()
{
	if (getStatement()) {
		return getStatement()->toSQLStatement(true /* for cascade */);
	} else {
		// YET
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeTable::Create::getCancelSQL -- get sql for cancelation
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
Manager::SystemTable::ReorganizeTable::Create::
getCancelSQL()
{
	ModUnicodeOstrStream cStream;
	cStream << "drop table ";
	if (getStatement()) {
		cStream << getStatement()->getName()->toSQLStatement();
	} else {
		// YET
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	cStream << " if exists";
	return cStream.getString();
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	const Statement::DropTableStatement* pStatement_
//	bool bIsTemporary_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::Drop::
Drop(Trans::Transaction& cTrans_,
	 Schema::Database* pDatabase_,
	 const Statement::DropTableStatement* pStatement_,
	 bool bIsTemporary_)
	: Super(cTrans_, pDatabase_, pStatement_, bIsTemporary_),
	  m_iStatus(Progress::Drop::Succeeded)
{}

// for redoing
Manager::SystemTable::ReorganizeTable::Drop::
Drop(Trans::Transaction& cTrans_,
	 Schema::Database* pDatabase_,
	 const Schema::LogData* pLogData_,
	 bool bIsTemporary_,
	 bool bRollforward_)
	: Super(cTrans_, pDatabase_, pLogData_, bIsTemporary_, bRollforward_),
	  m_iStatus(Progress::Drop::Succeeded)
{}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Drop::~Drop -- destructor
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
	close();
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Drop::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Schema::Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeTable::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.
	Schema::Manager::SystemTable::Result::Value iResult = Schema::Manager::SystemTable::Result::None;

	try {
		m_iStatus = Progress::Drop::None;

		// first of all, create local table using super class
		iResult = Super::execute();

		m_iStatus = Progress::Drop::LocalSucceeded;

		// execute same statement on cascade servers
		open(getDatabase(), getTransaction());
		executeStatement(getSQL());

		m_iStatus = Progress::Drop::Succeeded;

		return iResult;

	} catch (...) {
		if (getStatement() && (m_iStatus != Progress::Drop::Succeeded)) {
			bool bUndo_ = false; // used in the macro below

			_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

			iResult = retry();

			_END_REORGANIZE_RECOVERY(getDatabase()->getID());

			// if retry has been succeeded, treat as success case
			if (m_iStatus == Progress::Drop::Succeeded) {
				Common::Thread::resetErrorCondition();
				return iResult;
			} else {
				_SYDNEY_RETHROW;
			}
		} else { // in redoing
			_SYDNEY_RETHROW;
		}
	}
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Drop::retry -- error recovery
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Schema::Manager::SystemTable::Result::Value
Manager::SystemTable::ReorganizeTable::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop table failed." << ModEndl;
	}

	Schema::Manager::SystemTable::Result::Value iResult = Schema::Manager::SystemTable::Result::None;

	// error recovery of drop is retrying
	// unless the process has been failed before storing log data
	switch (m_iStatus) {
	case Progress::Drop::None:
		{
			// do nothing
			break;
		}
	case Progress::Drop::LocalSucceeded:
		{
			// execute statement adding 'if exists' on cascade servers
			open(getDatabase(), getTransaction());
			executeStatement(getCancelSQL());

			m_iStatus = Progress::Drop::Succeeded;
			break;
		}
	default:
		{
			break;
		}
	}
	return iResult;
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeTable::Drop::getSQL -- get sql for execution
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
Manager::SystemTable::ReorganizeTable::Drop::
getSQL()
{
	if (getStatement()) {
		return getStatement()->toSQLStatement(true /* for cascade */);
	} else {
		// YET
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeTable::Drop::getCancelSQL -- get sql for cancelation
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	ModUnicodeString
//
// EXCEPTIONS

ModUnicodeString
Manager::SystemTable::ReorganizeTable::Drop::
getCancelSQL()
{
	// redo with adding 'if exists'
	ModUnicodeOstrStream cStream;
	cStream << "drop table ";
	if (getStatement()) {
		cStream << getStatement()->getName()->toSQLStatement();
	} else {
		// YET
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	cStream << " if exists";
	return cStream.getString();
}

#ifdef YET
///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeTable::Alter
///////////////////////////////////////////////////////////

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Alter::Alter -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	const Statement::AlterTableStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::Alter::
Alter(Trans::Transaction& cTrans_,
	  Schema::Database* pDatabase_,
	  const Statement::AlterTableStatement* pStatement_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0)
{}

// for redoing
Manager::SystemTable::ReorganizeTable::Alter::
Alter(Trans::Transaction& cTrans_,
	  Schema::Database* pDatabase_,
	  const Schema::LogData* pLogData_,
	  bool bRollforward_)
	: Base(cTrans_, pDatabase_, false /* not temporary */, true /* redo */, bRollforward_),
	  m_pStatement(0), m_pLogData(pLogData_)
{}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::Alter::~Alter -- destructor
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
//	DSchema::Manager::SystemTable::ReorganizeTable::Alter::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Schema::Manager::SystemTable::Result::Value
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
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterArea::AlterArea -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Table* pTable_
//	const Statement::AlterTableAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::AlterArea::
AlterArea(Trans::Transaction& cTrans_,
		  Schema::Database* pDatabase_,
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
		  Schema::Database* pDatabase_,
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
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterArea::~AlterArea -- destructor
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
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterArea::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Schema::Manager::SystemTable::Result::Value
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
		DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "AlterTable", "Altered");

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
		DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "AlterTable", "Moved");

		// persist system tables
		Schema::Database* pDatabase = getDatabase();
		DSchema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
		DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "AlterTable", "TableStored");

		DSchema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
		DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "AlterTable", "FileStored");

		DSchema::SystemTable::AreaContent(*pDatabase).store(getTransaction());
		DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "AlterTable", "AreaContentStored");

		// persist ObjectID
		DSchema::ObjectID::persist(getTransaction(), getDatabase());

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
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterArea::undo -- error recovery
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
			Schema::Database* pDatabase = getDatabase();
			DSchema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
			DSchema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
			DSchema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

			// persist objectID because new objectID might have been persisted
			DSchema::ObjectID::persist(getTransaction(), getDatabase());
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
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterName::AlterName -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	Table* pTable_
//	const Statement::AlterTableAction* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeTable::AlterName::
AlterName(Trans::Transaction& cTrans_,
		  Schema::Database* pDatabase_,
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
		  Schema::Database* pDatabase_,
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
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterName::~AlterName -- destructor
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
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterName::execute -- execute
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Manager::SystemTable::Result::Value
//
// EXCEPTIONS

Schema::Manager::SystemTable::Result::Value
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
		DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "RenameTable", "Altered");

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
	DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "RenameTable", "Moved");

	Schema::Database* pDatabase = getDatabase();
	DSchema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
	DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "RenameTable", "TableStored");

	DSchema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
	DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "RenameTable", "IndexStored");

	DSchema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
	DSCHEMA_FAKE_ERROR("DSchema::Reorganize", "RenameTable", "FileStored");

	if (m_pStatement) {
		// increment timestamp value
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::AlterName::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeTable::AlterName::undo -- error recovery
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
			Schema::Database* pDatabase = getDatabase();
			DSchema::SystemTable::Table(*pDatabase).store(getTransaction(), m_pTable);
			DSchema::SystemTable::Index(*pDatabase).store(getTransaction(), *m_pTable);
			DSchema::SystemTable::File(*pDatabase).store(getTransaction(), *m_pTable);
			break;
		}
	case Progress::AlterName::None:
	default:
		{
			break;
		}
	}
}
#endif

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
