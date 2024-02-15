// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Interface/IProgram.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_INTERFACE_IPROGRAM_H
#define __SYDNEY_EXECUTION_INTERFACE_IPROGRAM_H

#include "Execution/Interface/Module.h"

#include "Execution/Action/Status.h"
#include "Execution/Declaration.h"


#include "Common/AutoCaller.h"
#include "Common/Data.h"
#include "Common/Externalizable.h"

#include "Opt/Algorithm.h"
#include "Opt/Declaration.h"

#include "Plan/Declaration.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
	class SQLData;
	class Status;
}
namespace Communication
{
	class Connection;
}
namespace Exception
{
	class Object;
}
namespace Os
{
	class CriticalSection;
}
namespace Schema
{
	class Column;
	class Database;
	class File;
	class Table;
}
namespace Trans
{
	class Transaction;
}

namespace DExecution
{
	namespace Action
	{
		class Fulltext;
	}
}




_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Interface::Program -- Base class for the classes which represents interface data information
//
//	NOTES
//		This class is not constructed directly
class IProgram
	: public Common::Externalizable
{
public:
	typedef Common::Externalizable Super;
	typedef IProgram This;

	// constructor
	static This* create();

	// destructor
	virtual ~IProgram() {}

	////////////////
	// accessors
	////////////////

	// get database object
	virtual Schema::Database* getDatabase() = 0;
	// get transaction object
	virtual Trans::Transaction* getTransaction() = 0;

	// set database object
	virtual void setDatabase(Schema::Database* pDatabase_) = 0;
	// set transaction object
	virtual void setTransaction(Trans::Transaction& cTrans_) = 0;

	// set parameter for prepared program
	virtual void setParameter(const Common::DataArrayData* pParameter_) = 0;

	// get the number of executed iterators
	virtual int getExecuteIteratorCount() = 0;
	// get executed iterators
	virtual Interface::IIterator* getExecuteIterator(int iStep_) = 0;
	// add executed iterator
	virtual void addExecuteIterator(Interface::IIterator* pIterator_) = 0;

	// prepare input/output connection
	virtual int prepareInputConnection() = 0;
	virtual int prepareOutputConnection() = 0;

	// set input/output connection
	virtual void setInputConnection(Communication::Connection* pConnection_) = 0;
	virtual void setOutputConnection(Communication::Connection* pConnection_) = 0;

	virtual Common::Data* getOutputVariable(const int iSessionID,
											const STRING& cstrValName,
											int iTableId_) = 0;
	
	// is for update?
	virtual bool isUpdate() = 0;
	virtual void setIsUpdate(bool bFlag_) = 0;

	// is batch mode?
	virtual bool isBatchMode() = 0;
	virtual void setBatchMode(bool bFlag_) = 0;

	// explain
	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_) = 0;

	// explain array->element correspondence for explain
	virtual void explainVariable(Opt::Explain& cExplain_,
								 int iNumber_) = 0;

	////////////////
	// execution
	////////////////

	// initialize
	void initialize(Interface::IIterator* pIterator_);
	// terminate
	void terminate(Interface::IIterator* pIterator_);
	// do before main loop
	Action::Status::Value startUp(Interface::IIterator* pIterator_);
	// main loop
	bool next(Interface::IIterator* pIterator_);
	// reset iteration to first tuple
	void reset(Interface::IIterator* pIterator_);
	// do after main loop
	void finish(Interface::IIterator* pIterator_);

	// execute loop
	void execute(Interface::IIterator* pIterator_);

	// clear program execution status
	virtual void clear() = 0;

	// critical section to protect members
	virtual Os::CriticalSection& getLatch() = 0;

	////////////////////////
	// resource management
	////////////////////////

	// register iterator object
	virtual int registerIterator(Interface::IIterator* pIterator_) = 0;
	// register collection object
	virtual int registerCollection(Interface::ICollection* pCollection_) = 0;
	// register action object
	virtual int registerAction(Interface::IAction* pOperator_) = 0;
	// reserve connection object room
	virtual int reserveConnection() = 0;
	// register fileaccess object
	virtual int registerFileAccess(Action::FileAccess* pFileAccess_) = 0;
	// register locker object
	virtual int registerLocker(Action::Locker* pLocker_) = 0;
	// register locator object
	virtual int registerLocator(Action::Locator* pLocator_) = 0;
	// register thread object
	virtual int registerThread(Action::Thread* pThread_) = 0;
	// register timestamp object
	virtual void registerTimestamp(Action::Timestamp* pTimestamp_) = 0;
	// register term object
	virtual int registerFulltext(DExecution::Action::Fulltext* pFulltext_) = 0;

	// get iterator
	virtual Interface::IIterator* getIterator(int iNumber_) = 0;
	// get collection
	virtual Interface::ICollection* getCollection(int iNumber_) = 0;
	// get action
	virtual Interface::IAction* getAction(int iNumber_) = 0;
	// get connection
	virtual Communication::Connection* getConnection(int iNumber_) = 0;
	// get fileaccess
	virtual Action::FileAccess* getFileAccess(int iNumber_) = 0;
	// get locker object
	virtual Action::Locker* getLocker(int iNumber_) = 0;
	// get locator object
	virtual Action::Locator* getLocator(int iNumber_) = 0;
	// get thread object
	virtual Action::Thread* getThread(int iNumber_) = 0;
	// get timestamp
	virtual Action::Timestamp* getTimestamp() = 0;
	// get term
	virtual DExecution::Action::Fulltext* getFulltext(int iNumber_) = 0;
	

	// get thread ID without register
	virtual int getNextThreadID() = 0;

	// add schema object
	virtual void addTable(const Schema::Table& cSchemaTable_) = 0;
	virtual void addColumn(const Schema::Column& cSchemaColumn_) = 0;
	virtual void addFile(const Schema::File& cSchemaFile_) = 0;
	// check schema object validity
	virtual bool isObsoleteSchema() = 0;

	// add variable
	virtual int addVariable(const Common::Data::Pointer& pData_) = 0;
	virtual int addVariable(const VECTOR<int>& vecData_) = 0;
	// copy variable
	virtual int copyVariable(int iNumber_) = 0;
	// get variable
	virtual Common::Data::Pointer getVariable(int iNumber_) = 0;
	// is array variable?
	virtual bool isArray(int iNumber_) = 0;
	// get array correspondence
	virtual const VECTOR<int>& getArray(int iNumber_) = 0;

	// placeholders
	virtual int getPlaceHolder(int iNumber_) = 0;
	virtual void setPlaceHolder(int iNumber_, int iID_) = 0;

protected:
	// constructor
	IProgram() : Super() {}

private:
};

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_INTERFACE_IPROGRAM_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
