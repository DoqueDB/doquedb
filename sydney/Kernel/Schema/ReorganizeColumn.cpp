// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeColumn.cpp -- Implementation of classes concerning with column reorganization
// 
// Copyright (c) 2006, 2007, 2009, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
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
#include "Schema/ReorganizeColumn.h"
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

#ifdef USE_OLDER_VERSION
#include "Analysis/Reorganize_Import.h"
#endif

#include "Common/Assert.h"
#include "Common/AutoCaller.h"

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
		// $$::Progress::Add::Value -- Progress value for alter table add column
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Add
		{
			enum Value
			{
				None,					// Initial value
				Replacing,				// replace started
				PrevRenamed,			// old record file is renamed
				AllPrevRenamed,			// all old files are renamed
				PrevDropped,			// old record file is dropped
				AllPrevDropped,			// all old files are dropped
				Touched,				// Table object told as changed
				Succeeded,				// process succeeded
				ValueNum
			};
		};

		// ENUM
		// $$::Progress::Alter::Value -- Progress value for alter table alter column
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct Alter
		{
			enum Value
			{
				None,					// initial value
				Altered,				// the column has been replaced
				Stored,					// system table has been persisted
				CacheReplaced,			// Column object has been replaced in cache
				Succeeded,				// process succeeded
				ValuNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeColumn::Base
///////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Base::Base -- constructor
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

Manager::SystemTable::ReorganizeColumn::Base::
Base(Trans::Transaction& cTrans_,
	 Database* pDatabase_,
	 Table* pTable_,
	 bool bRedo_)
	: Super(cTrans_, pDatabase_, pTable_, bRedo_)
{
	// Database should have been opened
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Base::~Base -- destructor
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
Manager::SystemTable::ReorganizeColumn::Base::
~Base()
{
	// nothing to do
	;
}

///////////////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeColumn::Add
///////////////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::Add -- constructor
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

Manager::SystemTable::ReorganizeColumn::Add::
Add(Trans::Transaction& cTrans_,
	Database* pDatabase_,
	Table* pTable_,
	const Statement::AlterTableAction* pStatement_)
	: Base(cTrans_, pDatabase_, pTable_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Add::Succeeded),
	  m_iImportedFile(0), m_iReplacedFile(0), m_iDroppedFile(0), m_iStoredFile(0),
	  m_vecPrevFiles(), m_vecPostFiles(), m_vecNewColumns(),
	  m_vecPrevName(), m_vecPostName(), m_vecTmpName()
{}

// for redoing
Manager::SystemTable::ReorganizeColumn::Add::
Add(Trans::Transaction& cTrans_,
	Database* pDatabase_,
	Table* pTable_,
	const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Add::Succeeded),
	  m_iImportedFile(0), m_iReplacedFile(0), m_iDroppedFile(0), m_iStoredFile(0),
	  m_vecPrevFiles(), m_vecPostFiles(), m_vecNewColumns(),
	  m_vecPrevName(), m_vecPostName(), m_vecTmpName()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::~Add -- destructor
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

Manager::SystemTable::ReorganizeColumn::Add::
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
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::execute -- execute
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
Manager::SystemTable::ReorganizeColumn::Add::
execute()
{
	ModUnicodeString cSuffix;
	ModVector<Field*> vecSourceField;
	ModVector<Field*> vecTargetField;

	if (m_pStatement) { // normal case

		// Poling interrupt message from client
		Manager::checkCanceled(getTransaction());

		// lock meta-tuple, logical log, database and table
		lock(Hold::Operation::Drop);

		// declare log object
		LogData cLogData(LogData::Category::AddColumn);

		// reset progress status
		// NOTE: reset here because add change the contents of Table
		m_iStatus = Progress::Add::None;

		// prepare altering and create log data
		if (!Table::alterAddColumn(getTransaction(), *getTable(), *m_pStatement, m_vecPrevFiles, m_vecPostFiles,
								   m_vecNewColumns, vecSourceField, vecTargetField, cLogData))
			// if no change, finish the method
			return Result::None;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "Prepared");

		// Store the log data into the logical log file
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "Logged");

		// set suffix string from log data
		cSuffix = cLogData.getString(Schema::Table::Log::AlterAddColumn::TempSuffix);

	} else { // redo
		; _SYDNEY_ASSERT(m_pLogData);

		// prepare altering using log data
		if (!Table::alterAddColumn(getTransaction(), *getTable(), *m_pLogData, m_vecPrevFiles, m_vecPostFiles,
								   m_vecNewColumns, vecSourceField, vecTargetField))
			// if no change, finish the method
			return Result::None;

		// set suffix string from log data
		cSuffix = m_pLogData->getString(Schema::Table::Log::AlterAddColumn::TempSuffix);
	}

	// In m_vecPrevFiles[]                     | In m_vecPostFiles[]
	//    old record file RCD_TBL              |    new record file $$RCD_TBL
	//    old vector file VCT_TBL_$$Conversion |    new vector file $$VCT_TBL_$$Conversion
	//                                         |    new heap/lob files

	int nCenterFile = m_vecPrevFiles.getSize();
	int nFile = m_vecPostFiles.getSize();
	; _SYDNEY_ASSERT(nCenterFile);
	; _SYDNEY_ASSERT(nFile);

	// create new files
	for (int i = 0; i < nFile; ++i) {
		// create physical objects for the files
#ifdef SYD_FAKE_ERROR
		if (i > 0) {
			// cause fake error only when at least one center file has been processed
			SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "FileCreated1");
		}
#endif
		m_vecPostFiles[i]->create(getTransaction());
	}
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "FileCreated");

	// create names
	m_vecPrevName.reserve(nCenterFile);
	m_vecPostName.reserve(nCenterFile);
	m_vecTmpName.reserve(nCenterFile);
	for (int i = 0; i < nCenterFile; ++i) {
		m_vecPrevName.pushBack(m_vecPrevFiles[i]->getName());
		m_vecPostName.pushBack(m_vecPostFiles[i]->getName());
		m_vecTmpName.pushBack(m_vecPrevFiles[i]->getName());
		m_vecTmpName.getBack().append(cSuffix);
	}

	// import data from original files
	import(vecSourceField, vecTargetField, false /* no check */, true /* in rowid order */);

	// replace original center files
	for (int i = 0; i < nCenterFile; m_iReplacedFile = ++i) {
		const File::Pointer& pPostFile = m_vecPostFiles[i];
		const File::Pointer& pPrevFile = m_vecPrevFiles[i];

		// replace the source of centerred file
		m_iStatus = Progress::Add::Replacing;
		replace(pPrevFile, pPostFile);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "ChangeSource");

		// rename the old file to temporary name
		rename(pPrevFile, m_vecPrevName[i], m_vecTmpName[i]);
		m_iStatus = Progress::Add::PrevRenamed;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "OldFileRenamed");
	}
	m_iStatus = Progress::Add::AllPrevRenamed;

	for (int i = 0; i < nCenterFile; m_iDroppedFile = ++i) {
		// rename the new file
		const File::Pointer& pPostFile = m_vecPostFiles[i];
		const File::Pointer& pPrevFile = m_vecPrevFiles[i];
		rename(pPostFile, m_vecPostName[i], m_vecPrevName[i]);
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "Renamed");

		// remove the old file
		drop(pPrevFile);
		m_iStatus = Progress::Add::PrevDropped;
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "PrevDropped");
	}
	m_iStatus = Progress::Add::AllPrevDropped;

	// tell the table is dirty
	getTable()->touch();
	m_iStatus = Progress::Add::Touched;

	// store schema objects
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Table(*pDatabase).store(getTransaction(), getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "TableStored");
	Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "ColumnStored");
	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "FileStored");
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "FileStored_Fatal");
	Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "FieldStored");

	// store area content
	Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

	// ObjectIDを永続化する
	Schema::ObjectID::persist(getTransaction(), getDatabase());

	if (m_pStatement) { // do only for normal case
		// increment the timestamp
		addTimestamp(getTransaction());
	}

	m_iStatus = Progress::Add::Succeeded;

	// add column needs additional process after transaction committed
	return (Result::NeedCheckpoint | Result::NeedReCache);
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::undo -- error recovery
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
Manager::SystemTable::ReorganizeColumn::Add::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter table failed." << ModEndl;
	}

	int nCenterFile = m_vecPrevFiles.getSize();
	int nFile = m_vecPostFiles.getSize();

	// cancel intermediate processes
	switch (m_iStatus) {
	case Progress::Add::PrevRenamed:
		{
			// cancel the last target file
			; _SYDNEY_ASSERT(m_iReplacedFile < nCenterFile);
			cancelRename(m_vecPrevFiles[m_iReplacedFile],
						 m_vecTmpName[m_iReplacedFile], m_vecPrevName[m_iReplacedFile],
						 false /* not stored */);
			// thru.
		}
	case Progress::Add::Replacing:
		{
			// cancel the last target file
			; _SYDNEY_ASSERT(m_iReplacedFile < nCenterFile);
			cancelReplace(m_vecPrevFiles[m_iReplacedFile], m_vecPostFiles[m_iReplacedFile],
						  false /* not stored */);
			break;
		}
	case Progress::Add::PrevDropped:
		{
			// cancel the last target file
			cancelDrop(m_vecPrevFiles[m_iDroppedFile], false /* not stored */);
			break;
		}
	default:
		{
			break;
		}
	}

	// If status is larger than Touched, that means any of schema objects are stored
	bool bStored = (m_iStatus >= Progress::Add::Touched);

	// cancel drop files
	for (int i = 0; i < m_iDroppedFile; ++i) {
		const File::Pointer& pPrevFile = m_vecPrevFiles[i];
		cancelDrop(pPrevFile, bStored);
	}
	// cancel replace source-destination connections
	for (int i = 0; i < m_iReplacedFile; ++i) {
		const File::Pointer& pPrevFile = m_vecPrevFiles[i];
		const File::Pointer& pPostFile = m_vecPostFiles[i];
		cancelReplace(pPrevFile, pPostFile, bStored);
	}
	// destroy all the new files
	Database* pDatabase = getDatabase();
	for (int i = 0; i < nFile; ++i) {
		const File::Pointer& pPostFile = m_vecPostFiles[i];
		pPostFile->drop(getTransaction(), false, true /* need persist */);

		// Persist system table
		// Even no persisting has not executed,
		// persisting is needed to cancel the creation
		Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *pPostFile);
		Schema::SystemTable::File(*pDatabase).store(getTransaction(), pPostFile);
	}

	// Then, rename prev files to original name
	for (int i = 0; i < m_iReplacedFile; ++i) {
		const File::Pointer& pPrevFile = m_vecPrevFiles[i];
		cancelRename(pPrevFile, m_vecTmpName[i], m_vecPrevName[i], bStored);
	}

	// drop all the new columns
	int nColumn = m_vecNewColumns.getSize();
	for (int i = 0; i < nColumn; ++i) {
		m_vecNewColumns[i]->drop(getTransaction(), false, true /* need persist */);
	}

	// if old files have been dropped, persist individually
	for (int i = 0; i < m_iDroppedFile; ++i) {
		const File::Pointer& pPrevFile = m_vecPrevFiles[i];
		Schema::SystemTable::File(*pDatabase).store(getTransaction(), pPrevFile);
		Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *pPrevFile);
	}
	// persist rest schema objects
	Schema::SystemTable::Table(*pDatabase).store(getTransaction(), getTable());
	Schema::SystemTable::Column(*pDatabase).store(getTransaction(), *getTable());
	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *getTable());
	Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
	Schema::SystemTable::AreaContent(*pDatabase).store(getTransaction());

	if (bStored) {
		// New object with new objectID has been persisted, so ObjectID has to be persisted
		Schema::ObjectID::persist(getTransaction(), getDatabase());
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::drop -- drop a file
//
// NOTES
//
// ARGUMENTS
//	const File::Pointer& pFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeColumn::Add::
drop(const File::Pointer& pFile_)
{
	pFile_->drop(getTransaction(), false, false, false /* destroy not immediately */);
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::cancelDrop -- revert drop operation of prev files
//
// NOTES
//
// ARGUMENTS
//	const File::Pointer& pFile_
//	bool bStore_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeColumn::Add::
cancelDrop(const File::Pointer& pFile_, bool bStore_)
{
	// cancel drop
	pFile_->undoDrop(getTransaction());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "FileStored_Fatal");

	if (bStore_) {
		// if deletion has been stored, creating should be stored
		Database* pDatabase = getDatabase();
		Schema::SystemTable::File(*pDatabase).store(getTransaction(), pFile_);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::rename -- rename prev/post files
//
// NOTES
//
// ARGUMENTS
//	const File::Pointer& pFile_
//	const Object::Name& cPrevName_
//	const Object::Name& cPostName_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeColumn::Add::
rename(const File::Pointer& pFile_, const Object::Name& cPrevName_, const Object::Name& cPostName_)
{
	pFile_->moveRename(getTransaction(), getTable()->getName(), getTable()->getName(),
					   cPrevName_, cPostName_);
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::cancelRename -- revert rename operation of prev/post files
//
// NOTES
//
// ARGUMENTS
//	const File::Pointer& pFile_
//	const Object::Name& cPrevName_
//	const Object::Name& cPostName_
//	bool bStored_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeColumn::Add::
cancelRename(const File::Pointer& pFile_, const Object::Name& cPrevName_, const Object::Name& cPostName_,
			 bool bStored_)
{
	pFile_->moveRename(getTransaction(), getTable()->getName(), getTable()->getName(),
					   cPrevName_, cPostName_, true /* undo */);
	if (bStored_) {
		// store again
		Database* pDatabase = getDatabase();
		Schema::SystemTable::File(*pDatabase).store(getTransaction(), pFile_);
	} else {
		// delete the flag 'touch'
		pFile_->untouch();
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::replace -- replace source-destination connection of center files
//
// NOTES
//
// ARGUMENTS
//	const File::Pointer& pPrevFile_
//	const File::Pointer& pPostFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeColumn::Add::
replace(const File::Pointer& pPrevFile_, const File::Pointer& pPostFile_)
{
	const ModVector<Field*>& vecPrevFields = pPrevFile_->getField(getTransaction());
	const ModVector<Field*>& vecPostFields = pPostFile_->getField(getTransaction());
	ModSize n = vecPrevFields.getSize();
	for (ModSize i = 0; i < n; ++i) {
		Field* pPrevField = vecPrevFields[i];
		Field* pPostField = vecPostFields[i];

		if (pPrevField->isFunction()) {
			// skip function fields
			continue;
		}

		// change a source-destination connection
		// from prev -(source)-> field to post -(source)-> field
		if (Field* pPrevSource = pPrevField->getSource(getTransaction())) {
			pPostField->setSourceID(pPrevSource->getID());
			// modify destination vector here so that persisting can be delayed
			pPrevSource->eraseDestination(*pPrevField);
			pPrevSource->addDestination(getTransaction(), *pPostField);
		}

		// change source-destination connection
		// from fields -(source)-> prev to fields -(source)-> post 
		const ModVector<Field*>& vecDestination = pPrevField->getDestination(getTransaction());
		ModSize m = vecDestination.getSize();
		for (ModSize j = 0; j < m; ++j) {
			Field* pDestination = vecDestination[j];
			pDestination->setSourceID(pPostField->getID());
			// ** remain the destination of pPrevField for error recovery **
			//pPrevField->eraseDestination...
			pPostField->addDestination(getTransaction(), *pDestination);
			pDestination->touch();
		}

		// column object corresponding to i-th field
		// if the field does not correspond to any column, result is 0
		Column* pPrevColumn = getTable()->getColumnByID(pPrevField->getColumnID(), getTransaction());

		// change corresponding field for the column from prev to post
		if (pPrevColumn) {
			// modify field object here so that persisting can be delayed
			pPrevColumn->setField(*pPostField);
			pPostField->setColumnID(pPrevColumn->getID());
			pPrevColumn->touch();
		}
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AddColumn", "ChangeSource1");
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Add::cancelReplace
//		-- cancel replacing source-destination connection of center files
//
// NOTES
//
// ARGUMENTS
//	const File::Pointer& pPrevFile_
//	const File::Pointer& pPostFile_
//	bool bStored_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeColumn::Add::
cancelReplace(const File::Pointer& pPrevFile_, const File::Pointer& pPostFile_, bool bStored_)
{
	const ModVector<Field*>& vecPrevFields = pPrevFile_->getField(getTransaction());
	const ModVector<Field*>& vecPostFields = pPostFile_->getField(getTransaction());
	ModSize n = vecPrevFields.getSize();
	for (ModSize i = 0; i < n; ++i) {
		Field* pPrevField = vecPrevFields[i];
		Field* pPostField = vecPostFields[i];

		if (pPrevField->isFunction()) {
			// skip function fields
			continue;
		}

		// revert a source-destination connection
		// from post -(source)-> field to prev -(source)-> field
		if (Field* pPrevSource = pPrevField->getSource(getTransaction())) {
			// destination can be reverted only clearing because field system table will be reverted
			pPrevSource->clearDestination();
		}

		// revert source-destination connection
		// from fields -(source)-> post to fields -(source)-> prev
		const ModVector<Field*>& vecDestination = pPrevField->getDestination(getTransaction());
		ModSize m = vecDestination.getSize();
		for (ModSize j = 0; j < m; ++j) {
			Field* pDestination = vecDestination[j];
			pDestination->setSourceID(pPrevField->getID());
			if (bStored_) {
				pDestination->touch();
			} else {
				pDestination->untouch();
			}
		}

		// column object corresponding to i-th field
		// if the field does not correspond to any column, result is 0
		Column* pPrevColumn = getTable()->getColumnByID(pPrevField->getColumnID(), getTransaction());

		if (pPrevColumn) {
			pPrevColumn->setField(*pPrevField);
			if (bStored_) {
				pPrevColumn->touch();
			} else {
				pPrevColumn->untouch();
			}
		}

		// clear related info of post field
		pPostField->setSourceID(ObjectID::Invalid);
		pPostField->clearDestination();
		pPostField->setColumnID(ObjectID::Invalid);
	}
}

///////////////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeColumn::Alter
///////////////////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Alter::Alter -- constructor
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

Manager::SystemTable::ReorganizeColumn::Alter::
Alter(Trans::Transaction& cTrans_,
	  Database* pDatabase_,
	  Table* pTable_,
	  const Statement::AlterTableAction* pStatement_)
	: Base(cTrans_, pDatabase_, pTable_, false /* not redo */),
	  m_pStatement(pStatement_), m_pLogData(0),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pPrevColumn(),
	  m_pPostColumn()
{}

// for redoing
Manager::SystemTable::ReorganizeColumn::Alter::
Alter(Trans::Transaction& cTrans_,
	Database* pDatabase_,
	Table* pTable_,
	const Schema::LogData* pLogData_)
	: Base(cTrans_, pDatabase_, pTable_, true /* redo */),
	  m_pStatement(0), m_pLogData(pLogData_),
	  m_iStatus(Progress::Alter::Succeeded),
	  m_pPrevColumn(),
	  m_pPostColumn()
{}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Alter::~Alter -- destructor
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

Manager::SystemTable::ReorganizeColumn::Alter::
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
//	Schema::Manager::SystemTable::ReorganizeColumn::Alter::execute -- execute
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
Manager::SystemTable::ReorganizeColumn::Alter::
execute()
{
	if (m_pStatement) { // normal case
		// Prepare a log data object
		LogData cLogData(LogData::Category::AlterColumn);
		// Check the statement and create new column and log data
		if (!Table::alterAlterColumn(getTransaction(), *getTable(), *m_pStatement,
									 m_pPrevColumn, m_pPostColumn, cLogData)) {
			// if altering is not needed, finish the method
			return Result::None;
		}
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "Prepared");

		// Poling interrupt message from client
		Manager::checkCanceled(getTransaction());

		// lock database
		lock(Hold::Operation::Drop);

		// store log data
		storeDatabaseLog(cLogData, getDatabase());
		SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "Logged");

	} else { // redo
		// Check log data and create new column
		if (!Table::alterAlterColumn(getTransaction(), *getTable(), *m_pLogData,
									 m_pPrevColumn, m_pPostColumn)) {
			// if altering is not needed, finish the method
			return Result::None;
		}
	}
	// reset progress status
	m_iStatus = Progress::Alter::None;

	// actually alter the column
	getTable()->alterColumn(getTransaction(), m_pPrevColumn, m_pPostColumn);
	// set the flag indicating the object has been changed
	getTable()->touch();
	m_iStatus = Progress::Alter::Altered;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "Altered");

	// Persist related system tables
	Database* pDatabase = getDatabase();
	Schema::SystemTable::Table(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "TableStored");
	Schema::SystemTable::Column(*pDatabase).store(getTransaction(), m_pPostColumn);
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "ColumnStored");
	Schema::SystemTable::File(*pDatabase).store(getTransaction(), *getTable());
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "FileStored");
	Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());
	m_iStatus = Progress::Alter::Stored;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "FieldStored");

	// replace the cache in the database object
	getDatabase()->eraseCache(m_pPostColumn->getID());
	getDatabase()->addCache(m_pPostColumn);
	m_iStatus = Progress::Alter::CacheReplaced;
	SCHEMA_FAKE_ERROR("Schema::Reorganize", "AlterColumn", "CacheReplaced");

	// persist objectID value
	Schema::ObjectID::persist(getTransaction(), getDatabase());

	if (m_pStatement) {
		// increment the timestamp
		addTimestamp(getTransaction());
	}
	m_iStatus = Progress::Alter::Succeeded;
	return Result::NeedReCache;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeColumn::Alter::undo -- error recovery
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
Manager::SystemTable::ReorganizeColumn::Alter::
undo()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "alter table failed." << ModEndl;
	}

	switch (m_iStatus) {
	case Progress::Alter::CacheReplaced:
	case Progress::Alter::Stored:
	case Progress::Alter::Altered:
		{
			// take back the altered column
			getTable()->alterColumn(getTransaction(), m_pPostColumn, m_pPrevColumn, true /* undo */);
			// Persist again
			getTable()->touch();
			Database* pDatabase = getDatabase();
			Schema::SystemTable::Table(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Column(*pDatabase).store(getTransaction(), m_pPrevColumn);
			Schema::SystemTable::File(*pDatabase).store(getTransaction(), *getTable());
			Schema::SystemTable::Field(*pDatabase).store(getTransaction(), *getTable());

			if (m_iStatus >= Progress::Alter::CacheReplaced) {
				// get back cache
				getDatabase()->eraseCache(m_pPrevColumn->getID());
				getDatabase()->addCache(m_pPrevColumn);
			}

			// persist objectID value
			Schema::ObjectID::persist(getTransaction(), getDatabase());
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
// Copyright (c) 2006, 2007, 2009, 2011, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
