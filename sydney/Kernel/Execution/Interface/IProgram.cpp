// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/IProgram.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Interface";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Interface/Class.h"
#include "Execution/Interface/IAction.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Action/FileAccess.h"
#include "Execution/Action/Locator.h"
#include "Execution/Action/NoTypeData.h"
#include "Execution/Action/Timestamp.h"
#include "Execution/Action/Thread.h"
#include "Execution/Collection/Connection.h"
#include "Execution/Utility/Serialize.h"


#include "Common/Assert.h"
#include "Common/DataInstance.h"
#include "Common/DataArrayData.h"
#include "Common/Hasher.h"
#include "Common/InputArchive.h"
#include "Common/OutputArchive.h"

#include "Communication/Connection.h"

#include "DExecution/Action/Fulltext.h"

#include "Exception/DynamicParameterNotMatch.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/SchemaObject.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"
#include "Os/Event.h"

#include "Schema/Database.h"
#include "Server/Session.h"

#include "ModHashMap.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_INTERFACE_BEGIN

namespace Impl
{

	// CLASS local
	//	Execution::Interface::Impl::ProgramImpl -- implementation class of IProgram
	//
	// NOTES
	class ProgramImpl
		: public Interface::IProgram
	{
	public:
		typedef ProgramImpl This;
		typedef Interface::IProgram Super;
		typedef MAP<int, VECTOR<int>, LESS<int> > ArrayVariableMap;

		ProgramImpl()
			: Super(),
			  m_pDatabase(0),
			  m_iDatabaseID(Schema::ObjectID::Invalid),
			  m_pTrans(0),
			  m_vecExecuteIterator(),
			  m_iOutConnection(-1),
			  m_vecIterator(),
			  m_vecCollection(),
			  m_vecConnection(),
			  m_vecVariable(),
			  m_mapArrayVariable(),
			  m_vecPlaceHolder(),
			  m_vecFileAccess(),
			  m_vecLocker(),
			  m_vecLocator(),
			  m_vecThread(),
			  m_vecFulltext(),
			  m_iNextThreadID(-1),
			  m_pTimestamp(0),
			  m_vecSchemaTable(),
			  m_vecSchemaColumn(),
			  m_vecSchemaFile(),
			  m_bIsUpdate(false),
			  m_bBatchMode(false),
			  m_iExplain(Opt::Explain::Option::None),
			  m_cLatch()
		{}
		~ProgramImpl()
		{
			try {
				clear();
			} catch (...) {
				// ignore exceptions in destructor
				;
			}
		}

	/////////////////////////////
	// Interface::IProgram::
		virtual Schema::Database* getDatabase()
		{
			if (!m_pDatabase) {
				m_pDatabase = Schema::Database::get(m_iDatabaseID,
													*m_pTrans);
			}
			return m_pDatabase;
		}
		virtual Trans::Transaction* getTransaction() {return m_pTrans;}

		virtual void setDatabase(Schema::Database* pDatabase_)
		{
			if ((m_pDatabase = pDatabase_) != 0) {
				m_iDatabaseID = m_pDatabase->getID();
			}
		}
		virtual void setTransaction(Trans::Transaction& cTrans_)
		{
			m_pTrans = &cTrans_;
		}
		virtual void setParameter(const Common::DataArrayData* pParameter_);

		virtual int getExecuteIteratorCount()
		{
			return m_vecExecuteIterator.GETSIZE();
		}

		virtual Interface::IIterator* getExecuteIterator(int iStep_)
		{
			return getIterator(m_vecExecuteIterator[iStep_]);
		}
		virtual void addExecuteIterator(Interface::IIterator* pIterator_)
		{
			m_vecExecuteIterator.PUSHBACK(pIterator_->getID());
		}

		virtual int prepareInputConnection();
		virtual int prepareOutputConnection();

		virtual void setInputConnection(Communication::Connection* pConnection_);
		virtual void setOutputConnection(Communication::Connection* pConnection_);
		virtual Common::Data* getOutputVariable(const int iSessionID_,
												const STRING& cstrValName,
												int iTableId_);
		

		virtual bool isUpdate()
		{
			return m_bIsUpdate;
		}
		virtual void setIsUpdate(bool bFlag_)
		{
			m_bIsUpdate = bFlag_;
		}
		virtual bool isBatchMode()
		{
			return m_bBatchMode;
		}
		virtual void setBatchMode(bool bFlag_)
		{
			m_bBatchMode = bFlag_;
		}

		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual void explainVariable(Opt::Explain& cExplain_,
									 int iNumber_);

	//	void initialize(Interface::IIterator* pIterator_);
	// 	void terminate(Interface::IIterator* pIterator_);
	// 	Action::Status::Value startUp(Interface::IIterator* pIterator_);
	// 	bool next(Interface::IIterator* pIterator_);
	// 	void reset(Interface::IIterator* pIterator_);
	//	void finish(Interface::IIterator* pIterator_);

		virtual void clear();

		virtual Os::CriticalSection& getLatch() {return m_cLatch;}

		virtual int registerIterator(Interface::IIterator* pIterator_)
		{
			m_vecIterator.PUSHBACK(pIterator_);
			return m_vecIterator.GETSIZE() - 1;
		}
		virtual int registerCollection(Interface::ICollection* pCollection_)
		{
			m_vecCollection.PUSHBACK(pCollection_);
			return m_vecCollection.GETSIZE() - 1;
		}
		virtual int registerAction(Interface::IAction* pOperator_)
		{
			m_vecAction.PUSHBACK(pOperator_);
			return m_vecAction.GETSIZE() - 1;
		}
		virtual int reserveConnection()
		{
			m_vecConnection.PUSHBACK(0);
			return m_vecConnection.GETSIZE() - 1;
		}
		virtual int registerFileAccess(Action::FileAccess* pFileAccess_)
		{
			m_vecFileAccess.PUSHBACK(pFileAccess_);
			return m_vecFileAccess.GETSIZE() - 1;
		}
		virtual int registerLocker(Action::Locker* pLocker_)
		{
			m_vecLocker.PUSHBACK(pLocker_);
			return m_vecLocker.GETSIZE() - 1;
		}
		virtual int registerLocator(Action::Locator* pLocator_)
		{
			m_vecLocator.PUSHBACK(pLocator_);
			return m_vecLocator.GETSIZE() - 1;
		}
		virtual int registerThread(Action::Thread* pThread_)
		{
			m_vecThread.PUSHBACK(pThread_);
			m_iNextThreadID = m_vecThread.GETSIZE();
			return m_iNextThreadID - 1;
		}
		virtual void registerTimestamp(Action::Timestamp* pTimestamp_)
		{
			if (m_pTimestamp) {
				_SYDNEY_THROW0(Exception::Unexpected);
			}
			m_pTimestamp = pTimestamp_;
		}
		virtual int registerFulltext(DExecution::Action::Fulltext* pFulltext_)
		{
			
			m_vecFulltext.PUSHBACK(pFulltext_);
			return m_vecFulltext.GETSIZE() - 1;
		}

		virtual Interface::IIterator* getIterator(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecIterator.GETSIZE()) {
				return m_vecIterator[iNumber_];
			}
			return 0;
		}
		virtual Interface::ICollection* getCollection(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecCollection.GETSIZE()) {
				return m_vecCollection[iNumber_];
			}
			return 0;
		}
		virtual Interface::IAction* getAction(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecAction.GETSIZE()) {
				return m_vecAction[iNumber_];
			}
			return 0;
		}

		virtual Communication::Connection* getConnection(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecConnection.GETSIZE()) {
				return m_vecConnection[iNumber_];
			}
			return 0;
		}
		virtual Action::FileAccess* getFileAccess(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecFileAccess.GETSIZE()) {
				return m_vecFileAccess[iNumber_];
			}
			return 0;
		}
		virtual Action::Locker* getLocker(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecLocker.GETSIZE()) {
				return m_vecLocker[iNumber_];
			}
			return 0;
		}
		virtual Action::Locator* getLocator(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecLocator.GETSIZE()) {
				return m_vecLocator[iNumber_];
			}
			return 0;
		}
		virtual Action::Thread* getThread(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecThread.GETSIZE()) {
				return m_vecThread[iNumber_];
			}
			return 0;
		}

		virtual DExecution::Action::Fulltext* getFulltext(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecFulltext.GETSIZE()) {
				return m_vecFulltext[iNumber_];
			}
			return 0;
		}
		
		virtual Action::Timestamp* getTimestamp()
		{
			return m_pTimestamp;
		}

		virtual int getNextThreadID()
		{
			if (m_vecThread.ISEMPTY()) return -1;
			return (m_iNextThreadID++ % m_vecThread.GETSIZE());
		}

		virtual void addTable(const Schema::Table& cSchemaTable_);
		virtual void addColumn(const Schema::Column& cSchemaColumn_);
		virtual void addFile(const Schema::File& cSchemaFile_);
		virtual bool isObsoleteSchema();

		virtual int addVariable(const Common::Data::Pointer& pData_);
		virtual int addVariable(const VECTOR<int>& vecID_);
		virtual int copyVariable(int iNumber_);
		virtual Common::Data::Pointer getVariable(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecVariable.GETSIZE()) {
				return m_vecVariable[iNumber_];
			}
			return Common::Data::Pointer();
		}
		virtual bool isArray(int iNumber_);
		virtual const VECTOR<int>& getArray(int iNumber_);

		virtual int getPlaceHolder(int iNumber_)
		{
			if (iNumber_ >= 0 && iNumber_ < m_vecPlaceHolder.GETSIZE()) {
				return m_vecPlaceHolder[iNumber_];
			}
			return -1;
		}
		virtual void setPlaceHolder(int iNumber_, int iID_)
		{
			Opt::ExpandContainer(m_vecPlaceHolder, iNumber_ + 1, -1);
			m_vecPlaceHolder[iNumber_] = iID_;
		}

	////////////////////////////
		// set actual connection at specified connection entry
		void setConnection(int iNumber_,
						   Communication::Connection* pConnection_)
		{
			if (iNumber_ >= 0) {
				m_vecConnection[iNumber_] = pConnection_;
			}
		}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		void addSortedVector(const Schema::Object& cSchemaObject_,
							 VECTOR<Opt::SchemaObject>* pvecSchemaObject_);

		Schema::Database* m_pDatabase;
		Schema::Object::ID::Value m_iDatabaseID;
		Trans::Transaction* m_pTrans;

		// execute iterator
		VECTOR<int> m_vecExecuteIterator;
		// connection ID to client
		int m_iOutConnection;

		// object pool
		VECTOR<Interface::IIterator*> m_vecIterator;
		VECTOR<Interface::ICollection*> m_vecCollection;
		VECTOR<Interface::IAction*> m_vecAction;
		VECTOR<Communication::Connection*> m_vecConnection;
		VECTOR<Action::FileAccess*> m_vecFileAccess;
		VECTOR<Action::Locker*> m_vecLocker;
		VECTOR<Action::Locator*> m_vecLocator;
		VECTOR<Action::Thread*> m_vecThread;
		VECTOR<DExecution::Action::Fulltext*> m_vecFulltext;
		Action::Timestamp* m_pTimestamp; // singleton for a program
		int m_iNextThreadID;

		VECTOR<Common::Data::Pointer> m_vecVariable;
		ArrayVariableMap m_mapArrayVariable;
		VECTOR<int> m_vecPlaceHolder;

		VECTOR<Opt::SchemaObject> m_vecSchemaTable;
		VECTOR<Opt::SchemaObject> m_vecSchemaColumn;
		VECTOR<Opt::SchemaObject> m_vecSchemaFile;

		Os::CriticalSection m_cLatch;

		// is for update?
		bool m_bIsUpdate;
		// is batch mode?
		bool m_bBatchMode;
		// explain mode
		int m_iExplain;
	};
}

namespace
{
	// TEMPLATE FUNCTION local
	//	$$$::_Destruct -- destructor
	//
	// TEMPLATE ARGUMENTS
	//	class Object_
	//	
	// NOTES
	//
	// ARGUMENTS
	//	VECTOR<Object_*>& vecTarget_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS
	template <class Object_>
	void
	_DestructElement(Object_* p_)
	{
		delete p_;
	}
	template <class Object_>
	void
	_Destruct(VECTOR<Object_*>& vecTarget_)
	{
		FOREACH(vecTarget_.begin(),
				vecTarget_.end(),
				_DestructElement<Object_>);
		vecTarget_.erase(vecTarget_.begin(), vecTarget_.end());
	}

} // namespace

///////////////////////////////////////////////
// Execution::Interface::Impl::ProgramImpl

// FUNCTION public
//	Interface::Impl::ProgramImpl::setParameter -- 
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
Impl::ProgramImpl::
setParameter(const Common::DataArrayData* pParameter_)
{
	if (SIZE nParameter = m_vecPlaceHolder.GETSIZE()) {
		int iDataCount = pParameter_ ? pParameter_->getCount() : 0;
		if (iDataCount < nParameter) {
			// lack of parameter values
			_SYDNEY_THROW2(Exception::DynamicParameterNotMatch,
						   iDataCount, nParameter);
		}
		for (SIZE i = 0; i < nParameter; ++i) {
			int iVariableID = getPlaceHolder(i);
			if (iVariableID < 0) {
				// ? is not used
				continue;
			}
			Common::Data::Pointer pData = getVariable(iVariableID);
			; _SYDNEY_ASSERT(pData.get());
			pData->assign(pParameter_->getElement(i).get());
		}

		// create openoption if any parameters are used in index file.
		
	}
}   

// FUNCTION public
//	Interface::Impl::ProgramImpl::prepareInputConnection -- 
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
Impl::ProgramImpl::
prepareInputConnection()
{
	// input from client is never specified
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::prepareOutputConnection -- 
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
Impl::ProgramImpl::
prepareOutputConnection()
{
	if (m_iOutConnection < 0) {
		m_iOutConnection = reserveConnection();
	}
	return m_iOutConnection;
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::setInputConnection -- 
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
Impl::ProgramImpl::
setInputConnection(Communication::Connection* pConnection_)
{
	// input from client is never specified
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::setOutputConnection -- 
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
Impl::ProgramImpl::
setOutputConnection(Communication::Connection* pConnection_)
{
	if (m_iOutConnection >= 0) {
		setConnection(m_iOutConnection,
					  pConnection_);
	}
}



// FUNCTION public
//	Interface::Impl::ProgramImpl::setOutputConnection -- 
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
Common::Data*
Impl::ProgramImpl::
getOutputVariable(const int iSessionID_, const STRING& cstrName_, int iTableId_)
{
	Server::Session* pSession = Server::Session::getSession(iSessionID_);
	Server::Session::BitSetVariable* pValue = pSession->getBitSetVariable(cstrName_);
	pValue->setSchemaTableID(iTableId_);
	return &pValue->getValue();
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ProgramImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	int n = getExecuteIteratorCount();
		cExplain_.put("BEGIN PLAN");
	cExplain_.flush();
	for (int i = 0; i < n; ++i) {
		getExecuteIterator(i)->explain(pEnvironment_, *this, cExplain_);
		cExplain_.flush();
	}
	cExplain_.put("END PLAN");
	cExplain_.flush();
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::explainVariable -- 
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
Impl::ProgramImpl::
explainVariable(Opt::Explain& cExplain_,
				int iNumber_)
{
	ArrayVariableMap::ITERATOR found = m_mapArrayVariable.find(iNumber_);
	if (found != m_mapArrayVariable.end()) {
		cExplain_.put("{");
		Opt::Join((*found).second,
				  boost::bind(&This::explainVariable,
							  this,
							  boost::ref(cExplain_),
							  _1),
				  boost::bind(&Opt::Explain::putChar,
							  &cExplain_,
							  ','));
		cExplain_.put("}");

	} else {
		cExplain_.put("#").put(iNumber_);
	}
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::clear -- 
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
Impl::ProgramImpl::
clear()
{
	// destruct all the objects pooled by ProgramImpl
	_Destruct(m_vecIterator);
	_Destruct(m_vecCollection);
	_Destruct(m_vecAction);
	_Destruct(m_vecFileAccess);
	_Destruct(m_vecLocker);
	_Destruct(m_vecLocator);
	_Destruct(m_vecThread);
	_Destruct(m_vecFulltext);
	m_vecVariable.clear();
	m_mapArrayVariable.clear();
	m_vecPlaceHolder.clear();
	m_vecExecuteIterator.clear();
	delete m_pTimestamp, m_pTimestamp = 0;
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::addTable -- add schema object
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
Impl::ProgramImpl::
addTable(const Schema::Table& cSchemaTable_)
{
	addSortedVector(cSchemaTable_,
					&m_vecSchemaTable);
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::addColumn -- add schema object
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
Impl::ProgramImpl::
addColumn(const Schema::Column& cSchemaColumn_)
{
	addSortedVector(cSchemaColumn_,
					&m_vecSchemaColumn);
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::addFile -- add schema object
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
Impl::ProgramImpl::
addFile(const Schema::File& cSchemaFile_)
{
	addSortedVector(cSchemaFile_,
					&m_vecSchemaFile);
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::isObsoleteSchema -- check schema object validity
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
Impl::ProgramImpl::
isObsoleteSchema()
{
	return Opt::IsAll(m_vecSchemaTable,
					  boost::bind(&Opt::SchemaObject::isValidTable,
								  _1,
								  getTransaction())) == false
		|| Opt::IsAll(m_vecSchemaColumn,
					  boost::bind(&Opt::SchemaObject::isValidColumn,
								  _1,
								  getTransaction())) == false
		|| Opt::IsAll(m_vecSchemaFile,
					  boost::bind(&Opt::SchemaObject::isValidFile,
								  _1,
								  getTransaction())) == false;
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::addVariable -- 
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
Impl::ProgramImpl::
addVariable(const Common::Data::Pointer& pData_)
{
	; _SYDNEY_ASSERT(pData_.get())
	// store Common::Data to ProgramImpl and return data ID
	m_vecVariable.PUSHBACK(pData_);
	return m_vecVariable.GETSIZE() - 1;
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::addVariable -- 
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
Impl::ProgramImpl::
addVariable(const VECTOR<int>& vecData_)
{
	// store Common::DataArrayData to ProgramImpl and return data ID
	// elements of the DataArrayData is Common::Data objects
	//   which are stored by addVariable previously.
	AUTOPOINTER<Common::DataArrayData> pArrayData = new Common::DataArrayData;
	int n = vecData_.GETSIZE();
	if (n > 0) {
		pArrayData->reserve(n);
		int i = 0;
		do {
			if (vecData_[i] < 0) {
				// illegal data -> return invalid number
				return -1;
			}
			; _SYDNEY_ASSERT(getVariable(vecData_[i]).get());
			pArrayData->pushBack(getVariable(vecData_[i]));
		} while (++i < n);
	}
	int iResult =  addVariable(Common::Data::Pointer(pArrayData.release()));

	// record int array
	m_mapArrayVariable.insert(iResult, vecData_);

	return iResult;
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::copyVariable -- 
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
Impl::ProgramImpl::
copyVariable(int iNumber_)
{
	Common::Data::Pointer pVariable = getVariable(iNumber_);
	if (pVariable.get()) {
		return addVariable(pVariable->copy());
	}
	return -1;
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::isArray -- 
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
Impl::ProgramImpl::
isArray(int iNumber_)
{
	return m_mapArrayVariable.find(iNumber_) != m_mapArrayVariable.end();
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::getArray -- 
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
Impl::ProgramImpl::
getArray(int iNumber_)
{
	ArrayVariableMap::Iterator found = m_mapArrayVariable.find(iNumber_);
	if (found != m_mapArrayVariable.end()) {
		return (*found).second;
	}
	_SYDNEY_THROW0(Exception::Unexpected);
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::getClassID -- 
//
// NOTES
//	for serialization
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	int
//
// EXCEPTIONS

int
Impl::ProgramImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::IProgram);
}

// FUNCTION public
//	Interface::Impl::ProgramImpl::serialize -- 
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
Impl::ProgramImpl::
serialize(ModArchive& archiver_)
{
	// serialize all the objects pooled by ProgramImpl
	archiver_(m_iDatabaseID);
	Utility::SerializeValue(archiver_, m_vecExecuteIterator);
	archiver_(m_iOutConnection);
	Utility::Serialize(archiver_, m_vecIterator);
	Utility::Serialize(archiver_, m_vecCollection);
	Utility::Serialize(archiver_, m_vecAction);
	Utility::Serialize(archiver_, m_vecFileAccess);
	Utility::Serialize(archiver_, m_vecLocker);
	Utility::Serialize(archiver_, m_vecLocator);
	Utility::Serialize(archiver_, m_vecThread);
	Utility::Serialize(archiver_, m_vecFulltext);
	Utility::SerializeVariable(archiver_, m_vecVariable);
	Utility::SerializeArrayVariable(archiver_, m_mapArrayVariable);
	Utility::SerializeValue(archiver_, m_vecPlaceHolder);
	Utility::SerializeConnection(archiver_, m_vecConnection);
	Utility::SerializeObject(archiver_, m_vecSchemaTable);
	Utility::SerializeObject(archiver_, m_vecSchemaColumn);
	Utility::SerializeObject(archiver_, m_vecSchemaFile);
	archiver_(m_bBatchMode);

	// create correct dataarraydata from arrayvariable
	Utility::AssignArrayVariable(m_mapArrayVariable, m_vecVariable);
}

// FUNCTION private
//	Interface::Impl::ProgramImpl::addSortedVector -- 
//
// NOTES
//
// ARGUMENTS
//	const Schema::Object& cSchemaObject_
//	VECTOR<Opt::SchemaObject>* pvecSchemaObject_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::ProgramImpl::
addSortedVector(const Schema::Object& cSchemaObject_,
				VECTOR<Opt::SchemaObject>* pvecSchemaObject_)
{
	Opt::SchemaObject cObject(cSchemaObject_);
	VECTOR<Opt::SchemaObject>::ITERATOR found = LOWERBOUND((*pvecSchemaObject_).begin(),
														   (*pvecSchemaObject_).end(),
														   cObject,
														   LESS<Opt::SchemaObject>());
	if (found == (*pvecSchemaObject_).end()
		|| LESS<Opt::SchemaObject>()(cObject, *found)) {
		(*pvecSchemaObject_).insert(found, cObject);
	}
}

//////////////////////////////
// Interface::IProgram::

// FUNCTION public
//	Interface::IProgram::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	IProgram*
//
// EXCEPTIONS

//static
IProgram*
IProgram::
create()
{
	return new Impl::ProgramImpl;
}

// FUNCTION public
//	Interface::IProgram::initialize -- 
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
IProgram::
initialize(Interface::IIterator* pIterator_)
{
	pIterator_->initialize(*this);

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)
		|| _OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Thread)) {
		Opt::Explain cExplain(Opt::Explain::Option::Explain
							  | Opt::Explain::Option::File
							  | Opt::Explain::Option::Data,
							  0);
		cExplain.initialize();
		pIterator_->explain(0, *this, cExplain);
		_OPT_EXECUTION_MESSAGE
			<< "Start execution:\n" << cExplain.getString()
			<< ModEndl;
		cExplain.terminate();
	}
#endif
}

// FUNCTION public
//	Interface::IProgram::terminate -- 
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
IProgram::
terminate(Interface::IIterator* pIterator_)
{
	if (pIterator_) {
		pIterator_->terminate(*this);
	}
}

// FUNCTION public
//	Interface::IProgram::startUp -- do before main loop
//
// NOTES
//
// ARGUMENTS
//	Interface::IIterator* pIterator_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
IProgram::
startUp(Interface::IIterator* pIterator_)
{
	return pIterator_->startUp(*this);
}

// FUNCTION public
//	Interface::IProgram::next -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IIterator* pIterator_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
IProgram::
next(Interface::IIterator* pIterator_)
{
	// for each iteration step, actions recorded in the iterator is executed
	pIterator_->undoneAction(*this);
	while (pIterator_->next(*this)) {
		switch (pIterator_->doAction(*this)) {
		case Action::Status::Success:
		case Action::Status::False:
			{
				break;
			}
		case Action::Status::Continue:
			{
				pIterator_->undoneAction(*this);
				continue;
			}
		case Action::Status::Break:
			{
				return false;
			}
		}
		return !pIterator_->isEndOfData();
	}
	return false;
}

// FUNCTION public
//	Interface::IProgram::reset -- 
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
IProgram::
reset(Interface::IIterator* pIterator_)
{
	// reset iteration
	pIterator_->reset(*this);
}

// FUNCTION public
//	Interface::IProgram::finish -- 
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
IProgram::
finish(Interface::IIterator* pIterator_)
{
	pIterator_->finish(*this);
}

// FUNCTION public
//	Interface::IProgram::execute -- execute loop
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

void
IProgram::
execute(Interface::IIterator* pIterator_)
{
	try {
		// do actions which are done once before iteration loop
		if (startUp(pIterator_) != Action::Status::Break) {

			// main iteration loop
			while (next(pIterator_)) {}
		}

		// do actions which are done once after iteration loop
		finish(pIterator_);
	} catch (...) {
		try {
			// clear objects converted by initialize
			terminate(pIterator_);
		} catch (...) {
			// ignore
		}
		_SYDNEY_RETHROW;
	}
}

_SYDNEY_EXECUTION_INTERFACE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
