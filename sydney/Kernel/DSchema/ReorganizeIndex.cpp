// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeIndex.cpp -- Implementation of classes concerning with index reorganization
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
const char moduleName[] = "DSchema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "DSchema/ReorganizeIndex.h"

#include "Schema/ErrorRecovery.h"

#include "Common/Assert.h"
#include "Common/AutoCaller.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/LogItemCorrupted.h"
#include "Exception/NotSupported.h"
#include "Exception/SystemTable.h"
#include "Exception/IndexNotFound.h"
#include "Exception/Unexpected.h"

#include "Lock/Name.h"

#include "Opt/Argument.h"

#include "Statement/IndexDefinition.h"
#include "Statement/DropIndexStatement.h"
#include "Statement/AlterIndexStatement.h"
#include "Statement/AlterIndexAction.h"
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
		// $$::Progress::Create::Value -- Progress value for create index
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
		// $$::Progress::Drop::Value -- Progress value for drop index
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
		// $$::Progress::AlterArea::Value -- Progress value for alter index set/drop area
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct AlterArea
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
		// $$::Progress::AlterName::Value -- Progress value for alter index rename
		// NOTES
		//	The value is defined here so that changes of values do not affect on the header

		struct AlterName
		{
			enum Value
			{
				None,					// initial value
				LocalSucceeded,			// local schema is persisted
				Succeeded,				// whole process succeeded
				ValueNum
			};
		};
	}
}

///////////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeIndex::Create
///////////////////////////////////////////////////////////

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeIndex::Create::Create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	const Statement::IndexDefinition* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::Create::
Create(Trans::Transaction& cTrans_,
	   Schema::Database* pDatabase_,
	   const Statement::IndexDefinition* pStatement_)
	: Super(cTrans_, pDatabase_, pStatement_),
	  m_iStatus(Progress::Create::Succeeded)
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::Create::
Create(Trans::Transaction& cTrans_,
	   Schema::Database* pDatabase_,
	   const Schema::LogData* pLogData_)
	: Super(cTrans_, pDatabase_, pLogData_),
	  m_iStatus(Progress::Create::Succeeded)
{}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeIndex::Create::~Create -- destructor
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
	if (getStatement() && (m_iStatus != Progress::Create::Succeeded)) {
		bool bUndo_ = false; // used in the macro below
		_BEGIN_REORGANIZE_RECOVERY(getDatabase()->getID());

		undo();

		_END_REORGANIZE_RECOVERY(getDatabase()->getID());
	}
	close();
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeIndex::Create::execute -- execute
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
Manager::SystemTable::ReorganizeIndex::Create::
execute()
{
	m_iStatus = Progress::Create::None;

	// first of all, create local index using super class
	Schema::Manager::SystemTable::Result::Value iResult = Super::execute();

	m_iStatus = Progress::Create::LocalSucceeded;

	// execute same statement on cascade servers
	open(getDatabase(), getTransaction());
	executeStatement(getSQL());

	m_iStatus = Progress::Create::Succeeded;

	return iResult;
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeIndex::Create::undo -- error recovery
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
	if (m_iStatus >= Progress::Create::LocalSucceeded) {
		if (!getTransaction().isCanceledStatement()) {
			SydErrorMessage << "create index failed in cascade." << ModEndl;
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
//	DSchema::Manager::SystemTable::ReorganizeIndex::Create::getSQL -- get sql for execution
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
Manager::SystemTable::ReorganizeIndex::Create::
getSQL()
{
	if (getStatement()) {
		return getStatement()->toSQLStatement();
	} else {
		// YET
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeIndex::Create::getCancelSQL -- get sql for cancelation
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
Manager::SystemTable::ReorganizeIndex::Create::
getCancelSQL()
{
	ModUnicodeOstrStream cStream;
	cStream << "drop index ";
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
// Manager::SystemTable::ReorganizeIndex::Drop
///////////////////////////////////////////////////////////

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeIndex::Drop::Drop -- constructor
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::Database* pDatabase_
//	const Statement::DropIndexStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Manager::SystemTable::ReorganizeIndex::Drop::
Drop(Trans::Transaction& cTrans_,
	 Schema::Database* pDatabase_,
	 const Statement::DropIndexStatement* pStatement_)
	: Super(cTrans_, pDatabase_, pStatement_),
	  m_iStatus(Progress::Drop::Succeeded)
{}

// for redoing
Manager::SystemTable::ReorganizeIndex::Drop::
Drop(Trans::Transaction& cTrans_,
	 Schema::Database* pDatabase_,
	 Schema::Table* pTable_,
	 const Schema::LogData* pLogData_)
	: Super(cTrans_, pDatabase_, pTable_, pLogData_),
	  m_iStatus(Progress::Drop::Succeeded)
{}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeIndex::Drop::~Drop -- destructor
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
	close();
}

// FUNCTION public
//	DSchema::Manager::SystemTable::ReorganizeIndex::Drop::execute -- execute
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
Manager::SystemTable::ReorganizeIndex::Drop::
execute()
{
	// Drop statement's error recovery is retrying,
	// so error recovery is done in catch clause.
	Schema::Manager::SystemTable::Result::Value iResult = Schema::Manager::SystemTable::Result::None;

	try {
		m_iStatus = Progress::Drop::None;

		// first of all, create local index using super class
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
//	DSchema::Manager::SystemTable::ReorganizeIndex::Drop::retry -- error recovery
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
Manager::SystemTable::ReorganizeIndex::Drop::
retry()
{
	if (!getTransaction().isCanceledStatement()) {
		SydErrorMessage << "drop index failed." << ModEndl;
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
//	DSchema::Manager::SystemTable::ReorganizeIndex::Drop::getSQL -- get sql for execution
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
Manager::SystemTable::ReorganizeIndex::Drop::
getSQL()
{
	if (getStatement()) {
		return getStatement()->toSQLStatement();
	} else {
		// YET
		_SYDNEY_THROW0(Exception::Unexpected);
	}
}

// FUNCTION private
//	DSchema::Manager::SystemTable::ReorganizeIndex::Drop::getCancelSQL -- get sql for cancelation
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
Manager::SystemTable::ReorganizeIndex::Drop::
getCancelSQL()
{
	// redo with adding 'if exists'
	ModUnicodeOstrStream cStream;
	cStream << "drop index ";
	if (getStatement()) {
		cStream << getStatement()->getIndexName()->toSQLStatement();
	} else {
		// YET
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	cStream << " if exists";
	return cStream.getString();
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
