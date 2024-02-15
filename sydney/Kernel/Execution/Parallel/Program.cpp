// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parallel/Program.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Parallel";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Parallel/Class.h"
#include "Execution/Parallel/Program.h"

#include "Execution/Action/ActionList.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/DataInstance.h"

#include "Exception/Unexpected.h"

#include "Os/AutoCriticalSection.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PARALLEL_BEGIN

// FUNCTION public
//	Parallel::Program::resetData -- reset data
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
Program::
resetData()
{
	Common::BitSet::ConstIterator iterator = m_cSetData.begin();
	const Common::BitSet::ConstIterator last = m_cSetData.end();
	for (; iterator != last; ++iterator) {
		Os::AutoCriticalSection l(m_pProgram->getLatch());
		Common::Data::Pointer pSourceData = m_pProgram->getVariable(*iterator);
		; _SYDNEY_ASSERT(pSourceData.get());

		Common::Data::Pointer pData = m_vecVariable[*iterator];
		if (pData->getType() == Common::DataType::Null) {
			if (pSourceData->isNull() == false) {
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		} else if (pData->getType() == Common::DataType::Array
				   && pData->getElementType() == Common::DataType::Data) {
			Common::Data::Pointer pNewData = pSourceData->copy();
			pData->assign(pNewData.get());
		} else {
			pData->assign(pSourceData.get());
		}
	}
}

// FUNCTION public
//	Parallel::Program::returnData -- return data
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
Program::
returnData()
{
	Common::BitSet::ConstIterator iterator = m_cReturnData.begin();
	const Common::BitSet::ConstIterator last = m_cReturnData.end();
	for (; iterator != last; ++iterator) {
		Os::AutoCriticalSection l(m_pProgram->getLatch());
		Common::Data::Pointer pTargetData = m_pProgram->getVariable(*iterator);
		; _SYDNEY_ASSERT(pTargetData.get());

		Common::Data::Pointer pData = m_vecVariable[*iterator];
		; _SYDNEY_ASSERT(pData.get());

		pTargetData->assign(pData.get());
	}
}

// FUNCTION public
//	Parallel::Program::setReturnData -- set returned data
//
// NOTES
//
// ARGUMENTS
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
setReturnData(int iDataID_)
{
	m_cReturnData.set(iDataID_);
}

// FUNCTION public
//	Parallel::Program::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Program*
//
// EXCEPTIONS

//static
Program*
Program::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Program);
	return new Program;
}

// FUNCTION public
//	Parallel::Program::getDatabase -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Schema::Database*
//
// EXCEPTIONS

//virtual
Schema::Database*
Program::
getDatabase()
{
	return m_pProgram->getDatabase();
}

// FUNCTION public
//	Parallel::Program::getTransaction -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Trans::Transaction*
//
// EXCEPTIONS

//virtual
Trans::Transaction*
Program::
getTransaction()
{
	return m_pProgram->getTransaction();
}

// FUNCTION public
//	Parallel::Program::setDatabase -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setDatabase(Schema::Database* pDatabase_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::setTransaction -- 
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setTransaction(Trans::Transaction& cTrans_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::DataArrayData* pParameter_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setParameter(const Common::DataArrayData* pParameter_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::getExecuteIteratorCount -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
getExecuteIteratorCount()
{
	return m_pProgram->getExecuteIteratorCount();
}

// FUNCTION public
//	Parallel::Program::getExecuteIterator -- 
//
// NOTES
//
// ARGUMENTS
//	int iStep_
//	
// RETURN
//	Interface::IIterator*
//
// EXCEPTIONS

//virtual
Interface::IIterator*
Program::
getExecuteIterator(int iStep_)
{
	return m_pProgram->getExecuteIterator(iStep_);
}

// FUNCTION public
//	Parallel::Program::addExecuteIterator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
addExecuteIterator(Interface::IIterator* pIterator_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::prepareInputConnection -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
prepareInputConnection()
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::prepareOutputConnection -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
prepareOutputConnection()
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::setInputConnection -- 
//
// NOTES
//
// ARGUMENTS
//	Communication::Connection* pConnection_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setInputConnection(Communication::Connection* pConnection_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::setOutputConnection -- 
//
// NOTES
//
// ARGUMENTS
//	Communication::Connection* pConnection_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setOutputConnection(Communication::Connection* pConnection_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::getOutputVariable -- 
//
// NOTES
//
// ARGUMENTS
//	const int iSessionID
//	const STRING& cstrValName
//	int iTableId_
//	
// RETURN
//	Common::Data*
//
// EXCEPTIONS

//virtual
Common::Data*
Program::
getOutputVariable(const int iSessionID,
				  const STRING& cstrValName,
				  int iTableId_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}
	
// FUNCTION public
//	Parallel::Program::isUpdate -- 
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

//virtual
bool
Program::
isUpdate()
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::setIsUpdate -- 
//
// NOTES
//
// ARGUMENTS
//	bool bFlag_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setIsUpdate(bool bFlag_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::isBatchMode -- 
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

//virtual
bool
Program::
isBatchMode()
{
	return m_pProgram && m_pProgram->isBatchMode();
}

// FUNCTION public
//	Parallel::Program::setBatchMode -- 
//
// NOTES
//
// ARGUMENTS
//	bool bFlag_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setBatchMode(bool bFlag_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	m_pProgram->explain(pEnvironment_,
						cExplain_);
}

// FUNCTION public
//	Parallel::Program::explainVariable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	int iNumber_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
explainVariable(Opt::Explain& cExplain_,
				int iNumber_)
{
	m_pProgram->explainVariable(cExplain_,
								iNumber_);
}

// FUNCTION public
//	Parallel::Program::clear -- 
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
void
Program::
clear()
{
	m_pProgram->clear();
	m_vecVariable.clear();
	m_cSetData.clear();
	m_cReturnData.clear();
}

// FUNCTION public
//	Parallel::Program::getLatch -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Os::CriticalSection&
//
// EXCEPTIONS

//virtual
Os::CriticalSection&
Program::
getLatch()
{
	return m_pProgram->getLatch();
}

// FUNCTION public
//	Parallel::Program::registerIterator -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IIterator* pIterator_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerIterator(Interface::IIterator* pIterator_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::registerCollection -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::ICollection* pCollection_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerCollection(Interface::ICollection* pCollection_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::registerAction -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IAction* pOperator_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerAction(Interface::IAction* pOperator_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::reserveConnection -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
reserveConnection()
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::registerFileAccess -- 
//
// NOTES
//
// ARGUMENTS
//	Action::FileAccess* pFileAccess_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerFileAccess(Action::FileAccess* pFileAccess_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::registerLocker -- 
//
// NOTES
//
// ARGUMENTS
//	Action::Locker* pLocker_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerLocker(Action::Locker* pLocker_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::registerThread -- 
//
// NOTES
//
// ARGUMENTS
//	Action::Thread* pThread_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerThread(Action::Thread* pThread_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}



// FUNCTION public
//	Parallel::Program::registerFulltext -- 
//
// NOTES
//
// ARGUMENTS
//	Action::Thread* pThread_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerFulltext(DExecution::Action::Fulltext* pFulltext__)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}


// FUNCTION public
//	Parallel::Program::registerLocator -- 
//
// NOTES
//
// ARGUMENTS
//	Action::Locator* pLocator_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
registerLocator(Action::Locator* pLocator_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::registerTimestamp -- 
//
// NOTES
//
// ARGUMENTS
//	Action::Timestamp* pTimestamp_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
registerTimestamp(Action::Timestamp* pTimestamp_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::getIterator -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Interface::IIterator*
//
// EXCEPTIONS

//virtual
Interface::IIterator*
Program::
getIterator(int iNumber_)
{
	return m_pProgram->getIterator(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getCollection -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Interface::ICollection*
//
// EXCEPTIONS

//virtual
Interface::ICollection*
Program::
getCollection(int iNumber_)
{
	return m_pProgram->getCollection(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getAction -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Interface::IAction*
//
// EXCEPTIONS

//virtual
Interface::IAction*
Program::
getAction(int iNumber_)
{
	return m_pProgram->getAction(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getConnection -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Communication::Connection*
//
// EXCEPTIONS

//virtual
Communication::Connection*
Program::
getConnection(int iNumber_)
{
	return m_pProgram->getConnection(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getFileAccess -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Action::FileAccess*
//
// EXCEPTIONS

//virtual
Action::FileAccess*
Program::
getFileAccess(int iNumber_)
{
	return m_pProgram->getFileAccess(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getLocker -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Action::Locker*
//
// EXCEPTIONS

//virtual
Action::Locker*
Program::
getLocker(int iNumber_)
{
	return m_pProgram->getLocker(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getLocator -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Action::Locator*
//
// EXCEPTIONS

//virtual
Action::Locator*
Program::
getLocator(int iNumber_)
{
	return m_pProgram->getLocator(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getThread -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Action::Thread*
//
// EXCEPTIONS

//virtual
Action::Thread*
Program::
getThread(int iNumber_)
{
	return m_pProgram->getThread(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getFulltext -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Action::Thread*
//
// EXCEPTIONS

//virtual
DExecution::Action::Fulltext*
Program::
getFulltext(int iNumber_)
{
	return m_pProgram->getFulltext(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getTimestamp -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Action::Timestamp*
//
// EXCEPTIONS

//virtual
Action::Timestamp*
Program::
getTimestamp()
{
	return m_pProgram->getTimestamp();
}

// FUNCTION public
//	Parallel::Program::getNextThreadID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
getNextThreadID()
{
	return m_pProgram->getNextThreadID();
}

// FUNCTION public
//	Parallel::Program::addTable -- 
//
// NOTES
//
// ARGUMENTS
//	const Schema::Table& cSchemaTable_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
addTable(const Schema::Table& cSchemaTable_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::addColumn -- 
//
// NOTES
//
// ARGUMENTS
//	const Schema::Column& cSchemaColumn_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
addColumn(const Schema::Column& cSchemaColumn_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::addFile -- 
//
// NOTES
//
// ARGUMENTS
//	const Schema::File& cSchemaFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
addFile(const Schema::File& cSchemaFile_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::isObsoleteSchema -- 
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

//virtual
bool
Program::
isObsoleteSchema()
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::addVariable -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data::Pointer& pData_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
addVariable(const Common::Data::Pointer& pData_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::addVariable -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecData_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
addVariable(const VECTOR<int>& vecData_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::copyVariable -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
copyVariable(int iNumber_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::getVariable -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	Common::Data::Pointer
//
// EXCEPTIONS

//virtual
Common::Data::Pointer
Program::
getVariable(int iNumber_)
{
	if (iNumber_ >= m_vecVariable.GETSIZE()
		|| m_vecVariable[iNumber_].get() == 0) {

		while (m_vecVariable.GETSIZE() <= iNumber_) {
			m_vecVariable.PUSHBACK(Common::Data::Pointer());
		}

		Common::Data::Pointer pNewData;

		if (isArray(iNumber_)) {
			const VECTOR<int>& vecArray = getArray(iNumber_);
			ModVector<Common::Data::Pointer> vecData;
			Opt::MapContainer(vecArray,
							  vecData,
							  boost::bind(&Program::getVariable,
										  this,
										  _1));
			pNewData = new Common::DataArrayData(vecData);

		} else {
			Common::Data::Pointer pData = m_pProgram->getVariable(iNumber_);
			if (pData->isNull()
				&& pData->getType() != Common::DataType::Null) {
				pNewData = Common::DataInstance::create(pData->getType());
				pNewData->setNull();
			} else {
				pNewData = pData->copy();
			}

			// set flag only for non-array data
			m_cSetData.set(iNumber_);
		}
		m_vecVariable[iNumber_] = pNewData;
	}

	return m_vecVariable[iNumber_];
}

// FUNCTION public
//	Parallel::Program::isArray -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Program::
isArray(int iNumber_)
{
	return m_pProgram->isArray(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getArray -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	const VECTOR<int>&
//
// EXCEPTIONS

//virtual
const VECTOR<int>&
Program::
getArray(int iNumber_)
{
	return m_pProgram->getArray(iNumber_);
}

// FUNCTION public
//	Parallel::Program::getPlaceHolder -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Program::
getPlaceHolder(int iNumber_)
{
	return m_pProgram->getPlaceHolder(iNumber_);
}

// FUNCTION public
//	Parallel::Program::setPlaceHolder -- 
//
// NOTES
//
// ARGUMENTS
//	int iNumber_
//	int iID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Program::
setPlaceHolder(int iNumber_, int iID_)
{
	// never called
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Parallel::Program::getClassID -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Program::
getClassID() const
{
	return Class::getClassID(Class::Category::Program);
}

// FUNCTION public
//	Parallel::Program::serialize -- 
//
// NOTES
//
// ARGUMENTS
//	ModArchive& archiver_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Program::
serialize(ModArchive& archiver_)
{
	m_cReturnData.serialize(archiver_);
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
