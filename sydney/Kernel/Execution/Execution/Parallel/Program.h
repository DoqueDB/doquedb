// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Parallel/Program.h --
// 
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_PARALLEL_PROGRAM_H
#define __SYDNEY_EXECUTION_PARALLEL_PROGRAM_H

#include "Execution/Parallel/Module.h"

#include "Execution/Interface/IParallel.h"
#include "Execution/Interface/IProgram.h"

#include "Common/BitSet.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PARALLEL_BEGIN

////////////////////////////////////
//	CLASS
//	Execution::Parallel::Program -- program for parallel execution
//
//	NOTES

class Program
	: public Interface::IProgram
{
public:
	typedef Interface::IProgram Super;
	typedef Program This;

	// constructor
	Program()
		: Super(),
		  m_pProgram(0),
		  m_vecVariable(),
		  m_cSetData(),
		  m_cReturnData()
	{}
	// destructor
	~Program() {}

	// accessor
	void setProgram(Interface::IProgram* pProgram_)
	{m_pProgram = pProgram_;}

	// reset data
	void resetData();

	// return data
	void returnData();

	// set returned data
	void setReturnData(int iDataID_);

	// for serialize
	static This* getInstance(int iCategory_);

///////////////////////////
// Interface::IProgram::
	virtual Schema::Database* getDatabase();
	virtual Trans::Transaction* getTransaction();

	virtual void setDatabase(Schema::Database* pDatabase_);
	virtual void setTransaction(Trans::Transaction& cTrans_);

	virtual void setParameter(const Common::DataArrayData* pParameter_);

	virtual int getExecuteIteratorCount();
	virtual Interface::IIterator* getExecuteIterator(int iStep_);
	virtual void addExecuteIterator(Interface::IIterator* pIterator_);

	virtual int prepareInputConnection();
	virtual int prepareOutputConnection();

	virtual void setInputConnection(Communication::Connection* pConnection_);
	virtual void setOutputConnection(Communication::Connection* pConnection_);

	virtual Common::Data* getOutputVariable(const int iSessionID,
											const STRING& cstrValName,
											int iTableId_);
	
	virtual bool isUpdate();
	virtual void setIsUpdate(bool bFlag_);

	virtual bool isBatchMode();
	virtual void setBatchMode(bool bFlag_);

	virtual void explain(Opt::Environment* pEnvironment_,
						 Opt::Explain& cExplain_);

	virtual void explainVariable(Opt::Explain& cExplain_,
								 int iNumber_);

	virtual void clear();
	virtual Os::CriticalSection& getLatch();

	virtual int registerIterator(Interface::IIterator* pIterator_);
	virtual int registerCollection(Interface::ICollection* pCollection_);
	virtual int registerAction(Interface::IAction* pOperator_);
	virtual int reserveConnection();
	virtual int registerFileAccess(Action::FileAccess* pFileAccess_);
	virtual int registerLocker(Action::Locker* pLocker_);
	virtual int registerLocator(Action::Locator* pLocator_);
	virtual int registerThread(Action::Thread* pThread_);
	virtual int registerFulltext(DExecution::Action::Fulltext* pFulltext__);	
	virtual void registerTimestamp(Action::Timestamp* pTimestamp_);

	virtual Interface::IIterator* getIterator(int iNumber_);
	virtual Interface::ICollection* getCollection(int iNumber_);
	virtual Interface::IAction* getAction(int iNumber_);
	virtual Communication::Connection* getConnection(int iNumber_);
	virtual Action::FileAccess* getFileAccess(int iNumber_);
	virtual Action::Locker* getLocker(int iNumber_);
	virtual Action::Locator* getLocator(int iNumber_);
	virtual Action::Thread* getThread(int iNumber_);
	virtual DExecution::Action::Fulltext* getFulltext(int iNumber_);
	virtual Action::Timestamp* getTimestamp();

	virtual int getNextThreadID();

	virtual void addTable(const Schema::Table& cSchemaTable_);
	virtual void addColumn(const Schema::Column& cSchemaColumn_);
	virtual void addFile(const Schema::File& cSchemaFile_);
	virtual bool isObsoleteSchema();

	virtual int addVariable(const Common::Data::Pointer& pData_);
	virtual int addVariable(const VECTOR<int>& vecData_);
	virtual int copyVariable(int iNumber_);

	virtual Common::Data::Pointer getVariable(int iNumber_);
	virtual bool isArray(int iNumber_);
	virtual const VECTOR<int>& getArray(int iNumber_);

	virtual int getPlaceHolder(int iNumber_);
	virtual void setPlaceHolder(int iNumber_, int iID_);

///////////////////////////////
// Common::Externalizable
	int getClassID() const;

///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_);

protected:
private:
	Interface::IProgram* m_pProgram;

	VECTOR<Common::Data::Pointer> m_vecVariable;
	Common::BitSet m_cSetData;
	Common::BitSet m_cReturnData;
};


_SYDNEY_EXECUTION_PARALLEL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_PARALLEL_PROGRAM_H

//
//	Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
