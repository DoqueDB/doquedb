// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Server.cpp --
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
const char moduleName[] = "DExecution::Iterator";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DExecution/Iterator/Server.h"
#include "DExecution/Iterator/Class.h"
#include "DExecution/Action/ServerAccess.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerArrayData.h"
#include "Common/Message.h"

#include "DServer/ResultSet.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Plan/Sql/Query.h"

#include "Schema/Cascade.h"
#include "Schema/Database.h"

#include "ModTime.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_ITERATOR_BEGIN

namespace
{
	struct _Explain
	{
		enum {
			Query = 0,
			Execute,
			PreparedQuery
		};
	};
	const char* const _pszExplainName[] =
	{
		"server query",					// execute query
		"server execute",				// execute
		"server prepared query"
	};

#ifndef NO_TRACE
	class _AutoPrintTime
	{
	public:
		// コンストラクター
		_AutoPrintTime(const STRING& cstrCascadeName_)
			: _name(&cstrCascadeName_)
			{
				if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) 
					_start = ModTime::getCurrentTime();
			}

		void start();
		

		// デストラクター
		~_AutoPrintTime()
			{
				if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
					ModTimeSpan interval(ModTime::getCurrentTime() - _start);
					_OPT_EXECUTION_MESSAGE
						<< "Server [" << *_name
						<< "] first getting data Time: "
						<< ModCharString().format(
							"%02d:%02d:%02d.%03d",
							interval.getTotalHours(), interval.getMinutes(),
							interval.getSeconds(), interval.getMilliSeconds())
						<< ModEndl;
				}
			}

	private:
		// 実行時間を出力する処理の名前
		const STRING* _name;
		// 処理の開始時刻
		ModTime	_start;
	};

#endif
}

namespace ServerImpl
{
	// CLASS local
	//	Iterator::ServerImpl::Base -- base class for server iterator
	//
	// NOTES

	class Base
		: public Iterator::Server
	{
	public:
		typedef Iterator::Server Super;
		typedef Base This;

		// constructor
		Base()
			: Super(),
			  m_cAccess(),
			  m_pResultSet(0),
			  m_cOutData()
		{}
		Base(int iAccessID_)
			: Super(),
			  m_cAccess(iAccessID_),
			  m_pResultSet(0),
			  m_cOutData()
		{}
		// destructor
		virtual ~Base() {close();}

	///////////////////////////
	// Iterator::Server::

	///////////////////////////
	//Execution::Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);
	//	virtual Execution::Action::Status::Value startUp(Execution::Interface::IProgram& cProgram_);
		virtual void finish(Execution::Interface::IProgram& cProgram_)
		{
			Super::finish(cProgram_);
			close();
		}
		virtual bool next(Execution::Interface::IProgram& cProgram_);
		virtual void reset(Execution::Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Execution::Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Execution::Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Execution::Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Execution::Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		DExecution::Action::ServerAccessHolder& getServerAccess() {return m_cAccess;}
		DServer::ResultSet* getResultSet() {return m_pResultSet;}
		Execution::Action::ArrayDataHolder& getOutData() {return m_cOutData;}

		bool open(Execution::Interface::IProgram& cProgram_);
		virtual void close();

	//////////////////////////////
	// Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual void addOutData(Execution::Interface::IProgram& cProgram_,
								const Execution::Action::Argument& cAction_);

	private:
		virtual void explainMethod(Opt::Explain& cExplain_) = 0;
		virtual bool getData(Execution::Interface::IProgram& cProgram_) = 0;

		DExecution::Action::ServerAccessHolder m_cAccess;
		DServer::ResultSet* m_pResultSet;
		Execution::Action::ArrayDataHolder m_cOutData;
	};

	// CLASS local
	//	Iterator::ServerImpl::Query -- server iterator
	//
	// NOTES

	class Query
		: public Base
	{
	public:
		typedef Base Super;
		typedef Query This;

		// constructor
		Query()
			: Super()
		{}
		Query(int iAccessID_)
			: Super(iAccessID_)
		{}
		// destructor
		virtual ~Query() {}

	///////////////////////////
	// Iterator::Server::

	///////////////////////////
	//Execution::Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Execution::Interface::IProgram& cProgram_);
	//	virtual void terminate(Execution::Interface::IProgram& cProgram_);
	//	virtual void finish(Execution::Interface::IProgram& cProgram_);
	//	virtual bool next(Execution::Interface::IProgram& cProgram_);
	//	virtual void reset(Execution::Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Execution::Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Execution::Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Execution::Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Execution::Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// Iterator::Base::

	private:
	//////////////////////////////
	// ServerImpl::Base::
		virtual void explainMethod(Opt::Explain& cExplain_);
		virtual bool getData(Execution::Interface::IProgram& cProgram_);
	};

		// CLASS local
	//	Iterator::ServerImpl::Query -- server iterator
	//
	// NOTES

	class PreparedQuery
		: public Iterator::Server
	{

	public:
		typedef Iterator::Server Super;
		typedef PreparedQuery This;

		// constructor
		PreparedQuery()
			: Super(),
			  m_cAccess(),
			  m_pResultSet(0),
			  m_cOutData()
		{}
		PreparedQuery(int iAccessID_, int iDataID_)
			: Super(),
			  m_cAccess(iAccessID_),
			  m_pResultSet(0),
			  m_cOutData(),
			  m_cExecuteData(iDataID_)
		{}
		// destructor
		virtual ~PreparedQuery() {}

	///////////////////////////
	// Iterator::Server::

	///////////////////////////
	//Execution::Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);
		virtual Execution::Action::Status::Value startUp(Execution::Interface::IProgram& cProgram_);
		virtual void finish(Execution::Interface::IProgram& cProgram_);
		virtual bool next(Execution::Interface::IProgram& cProgram_);
		virtual void reset(Execution::Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Execution::Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Execution::Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Execution::Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Execution::Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		DExecution::Action::ServerAccessHolder& getServerAccess() {return m_cAccess;}
		DServer::ResultSet* getResultSet() {return m_pResultSet;}
		Execution::Action::ArrayDataHolder& getOutData() {return m_cOutData;}

		bool open(Execution::Interface::IProgram& cProgram_);
		virtual void close();

	//////////////////////////////
	// Iterator::Base::		
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Execution::Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual void addOutData(Execution::Interface::IProgram& cProgram_,
								const Execution::Action::Argument& cAction_);

	private:

		virtual bool getData(Execution::Interface::IProgram& cProgram_);		

		DExecution::Action::ServerAccessHolder m_cAccess;
		DServer::ResultSet* m_pResultSet;
		Execution::Action::ArrayDataHolder m_cOutData;
		Execution::Action::ArrayDataHolder m_cExecuteData;
	};
	

	// CLASS local
	//	Iterator::ServerImpl::Execute -- server iterator
	//
	// NOTES

	class Execute
		: public Base
	{
	public:
		typedef Base Super;
		typedef Execute This;

		// constructor
		Execute()
			: Super()
		{}
		Execute(int iAccessID_)
			: Super(iAccessID_)
		{}
		// destructor
		virtual ~Execute() {}

	///////////////////////////
	// Iterator::Server::

	///////////////////////////
	//Execution::Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Execution::Interface::IProgram& cProgram_);
	//	virtual void terminate(Execution::Interface::IProgram& cProgram_);
	//	virtual void finish(Execution::Interface::IProgram& cProgram_);
	//	virtual bool next(Execution::Interface::IProgram& cProgram_);
	//	virtual void reset(Execution::Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Execution::Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Execution::Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Execution::Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Execution::Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	//////////////////////////////
	// Iterator::Base::
	//	virtual void explainThis(Opt::Environment* pEnvironment_,
	//							 Execution::Interface::IProgram& cProgram_,
	//							 Opt::Explain& cExplain_);

	private:
	//////////////////////////////
	// ServerImpl::Base::
		virtual void explainMethod(Opt::Explain& cExplain_);
		virtual bool getData(Execution::Interface::IProgram& cProgram_);
		
	};
}

/////////////////////////////////////////////
// DExecution::Iterator::ServerImpl::Base

// FUNCTION public
//	Iterator::ServerImpl::Base::initialize -- initialize
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::Base::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (m_cAccess.isInitialized() == 0) {
		initializeBase(cProgram_);
		m_cOutData.initialize(cProgram_);
		m_cAccess.initialize(cProgram_);
	}
}

// FUNCTION public
//	Iterator::ServerImpl::Base::terminate -- terminate
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::Base::
terminate(Execution::Interface::IProgram& cProgram_)
{
	if (m_cAccess.isInitialized()) {
		terminateBase(cProgram_);
		close();
		m_cOutData.terminate(cProgram_);
		m_cAccess.terminate(cProgram_);
	}
}


// FUNCTION public
//	Iterator::ServerImpl::Base::next -- go to next tuple
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ServerImpl::Base::
next(Execution::Interface::IProgram& cProgram_)
{
	if (!hasNext()) {
		return setHasData(false);
	}
	
	
	if (!m_pResultSet) {
		
#ifndef NO_TRACE
		_AutoPrintTime tmsg(getServerAccess()->getCascadeName());
#endif
		if (open(cProgram_)) {
			setHasData(getData(cProgram_));
		} else {
			setHasData(false);
		}
		
	} else {
		setHasData(getData(cProgram_));
	}

	return setHasNext(hasData());

}





// FUNCTION public
//	Iterator::ServerImpl::Base::reset --
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::Base::
reset(Execution::Interface::IProgram& cProgram_)
{
	resetBase(cProgram_);
	close();
}

// FUNCTION public
//	Iterator::ServerImpl::Base::serialize -- 
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
ServerImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cAccess.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Iterator::ServerImpl::Base::open -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ServerImpl::Base::
open(Execution::Interface::IProgram& cProgram_)
{
	if (m_pResultSet == 0) {
		m_pResultSet = m_cAccess->getResultSet(cProgram_);
	}
	return m_pResultSet != 0;
}

// FUNCTION public
//	Iterator::ServerImpl::Base::close -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::Base::
close()
{
	if (m_pResultSet) {
		DServer::ResultSet::erase(m_pResultSet);
		m_pResultSet = 0;
		m_cAccess->resetResultSet();
	}
}

// FUNCTION protected
//	Iterator::ServerImpl::Base::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::Base::
explainThis(Opt::Environment* pEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	explainMethod(cExplain_);
	cExplain_.put(" to ");

	m_cAccess.explain(pEnvironment_,
					  cProgram_,
					  cExplain_);
	cExplain_.popNoNewLine();

	cExplain_.pushIndent();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.newLine();
		cExplain_.put("to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popIndent(true /* force new line */);
}

// FUNCTION protected
//	Iterator::ServerImpl::Base::addOutData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const Execution::Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::Base::
addOutData(Execution::Interface::IProgram& cProgram_,
		   const Execution::Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cOutData.isValid() == false);

	m_cOutData.setDataID(cAction_.getInstanceID());
}

/////////////////////////////////////////////
// DExecution::Iterator::ServerImpl::Query

// FUNCTION public
//	Iterator::ServerImpl::Query::getClassID -- 
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
ServerImpl::Query::
getClassID() const
{
	return Class::getClassID(Class::Category::ServerQuery);
}

// FUNCTION private
//	Iterator::ServerImpl::Query::explainMethod -- 
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
ServerImpl::Query::
explainMethod(Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::Query]);
}

// FUNCTION public
//	Iterator::ServerImpl::Query::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ServerImpl::Query::
getData(Execution::Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(getResultSet());
	try {
		for (;;) {
			DServer::ResultSet::Status::Value eStatus =
				getResultSet()->getNextTuple(getOutData().get());
			switch (eStatus) {
			case DServer::ResultSet::Status::Data:
				{
#ifndef NO_TRACE
					if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
						_OPT_EXECUTION_MESSAGE
							<< "Server [" << getServerAccess()->getCascadeName()
							<< "] getData = "
							<< Opt::Trace::toString(*getOutData())
							<< ModEndl;
					}
#endif
					return true;
				}
			case DServer::ResultSet::Status::EndOfData:
				{
#ifndef NO_TRACE
					if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
						_OPT_EXECUTION_MESSAGE
							<< "Server [" << getServerAccess()->getCascadeName()
							<< "] EndOfData"
							<< ModEndl;
					}
#endif
					break;
				}
			case DServer::ResultSet::Status::Success:
				{
					return false;
				}
			case DServer::ResultSet::Status::Canceled:
				{
					// cancelation in sub-server should be ignored
					return false;
				}
			case DServer::ResultSet::Status::MetaData:
			case DServer::ResultSet::Status::HasMoreData:
				{
					// metadata/has-more-data from sub-server is abandoned
					break;
				}
			default:
				{
					break;
				}
			}
		}
	} catch (...) {
		SydInfoMessage << "Exceptiion from " << getServerAccess()->getCascadeName() << ModEndl;
		_SYDNEY_RETHROW;
	}
}


/////////////////////////////////////////////
// DExecution::Iterator::ServerImpl::PreparedQuery

/////////////////////////////////////////////
// DExecution::Iterator::ServerImpl::Query

// FUNCTION public
//	Iterator::ServerImpl::Query::getClassID -- 
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
ServerImpl::PreparedQuery::
getClassID() const
{
	return Class::getClassID(Class::Category::PreparedQuery);
}



// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::initialize -- initialize
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::PreparedQuery::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (m_cAccess.isInitialized() == 0) {
		initializeBase(cProgram_);
		m_cOutData.initialize(cProgram_);
		m_cExecuteData.initialize(cProgram_);
		m_cAccess.initialize(cProgram_);
	}
}


// FUNCTION protected
//	Iterator::ServerImpl::PreparedQuery::open -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ServerImpl::PreparedQuery::
open(Execution::Interface::IProgram& cProgram_)
{
	try {
		m_cAccess->open(cProgram_);
		return m_cAccess->prepare(cProgram_);
	} catch (...) {
		SydInfoMessage << "Exceptiion from " << getServerAccess()->getCascadeName() << ModEndl;
		_SYDNEY_RETHROW;
	}
}

// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::terminate -- terminate
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::PreparedQuery::
terminate(Execution::Interface::IProgram& cProgram_)
{
	if (m_cAccess.isInitialized()) {
		terminateBase(cProgram_);
		close();
		m_cOutData.terminate(cProgram_);
		m_cExecuteData.terminate(cProgram_);
		m_cAccess.terminate(cProgram_);
	}
}

// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::startUp -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Execution::Action::Status::Value
//
// EXCEPTIONS

//virtual
Execution::Action::Status::Value
ServerImpl::PreparedQuery::
startUp(Execution::Interface::IProgram& cProgram_)
{
	if (open(cProgram_) == false) {
		setHasNext(false);
		return Execution::Action::Status::Break;
	}

	return Super::startUp(cProgram_);
}


// FUNCTION public
//	Iterator::ServerImpl::Base::terminate -- finish
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::PreparedQuery::
finish(Execution::Interface::IProgram& cProgram_)
{
	Super::finish(cProgram_);
	close();
}



// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::next -- go to next tuple
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ServerImpl::PreparedQuery::
next(Execution::Interface::IProgram& cProgram_)
{
	if (m_pResultSet == 0) {
		m_pResultSet = m_cAccess->executePrepare(cProgram_, *m_cExecuteData);
	}
	
	if (hasNext() == false) {
		setHasData(false);
	} else {
		setHasData(getData(cProgram_));
	}
	return setHasNext(hasData());
}

// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::reset --
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::PreparedQuery::
reset(Execution::Interface::IProgram& cProgram_)
{
	resetBase(cProgram_);
	close();
}

// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::serialize -- 
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
ServerImpl::PreparedQuery::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cAccess.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}



// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::close -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::PreparedQuery::
close()
{
	if (m_pResultSet) {
		DServer::ResultSet::erase(m_pResultSet);
		m_pResultSet = 0;
	}
}

// FUNCTION protected
//	Iterator::ServerImpl::PreparedQuery::explainThis -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::PreparedQuery::
explainThis(Opt::Environment* pEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName[_Explain::PreparedQuery]);
	cExplain_.put(" to ");
	m_cAccess.explain(pEnvironment_,
					  cProgram_,
					  cExplain_);
	cExplain_.popNoNewLine();

	cExplain_.pushIndent();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.newLine();
		cExplain_.put("to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popIndent(true /* force new line */);
}

// FUNCTION protected
//	Iterator::ServerImpl::PreparedQuery::addOutData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const Execution::Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ServerImpl::PreparedQuery::
addOutData(Execution::Interface::IProgram& cProgram_,
		   const Execution::Action::Argument& cAction_)
{
	; _SYDNEY_ASSERT(m_cOutData.isValid() == false);

	m_cOutData.setDataID(cAction_.getInstanceID());
}


// FUNCTION public
//	Iterator::ServerImpl::PreparedQuery::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ServerImpl::PreparedQuery::
getData(Execution::Interface::IProgram& cProgram_)
{

	try {
		for (;;) {
			DServer::ResultSet::Status::Value eStatus =
				getResultSet()->getNextTuple(getOutData().get());
			switch (eStatus) {
			case DServer::ResultSet::Status::Data:
				{
#ifndef NO_TRACE
					if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
						_OPT_EXECUTION_MESSAGE
							<< "Server [" << getServerAccess()->getCascadeName()
							<< "] getData = "
							<< Opt::Trace::toString(*getOutData())
							<< ModEndl;
					}
#endif
					return true;
				}
			case DServer::ResultSet::Status::EndOfData:
				{
#ifndef NO_TRACE
					if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
						_OPT_EXECUTION_MESSAGE
							<< "Server [" << getServerAccess()->getCascadeName()
							<< "] EndOfData"
							<< ModEndl;
					}
#endif
					break;
				}
			case DServer::ResultSet::Status::Success:
				{
					return false;
				}
			case DServer::ResultSet::Status::Canceled:
				{
					// cancelation in sub-server should be ignored
					return false;
				}
			case DServer::ResultSet::Status::MetaData:
			case DServer::ResultSet::Status::HasMoreData:
				{
					// metadata/has-more-data from sub-server is abandoned
					break;
				}
			default:
				{
					break;
				}
			}
		}
	} catch (...) {
		SydInfoMessage << "Exceptiion from " << getServerAccess()->getCascadeName() << ModEndl;
		_SYDNEY_RETHROW;
	}
	
	; _SYDNEY_ASSERT(getResultSet());
}



/////////////////////////////////////////////
// DExecution::Iterator::ServerImpl::Execute

// FUNCTION public
//	Iterator::ServerImpl::Execute::getClassID -- 
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
ServerImpl::Execute::
getClassID() const
{
	return Class::getClassID(Class::Category::ServerExecute);
}

// FUNCTION private
//	Iterator::ServerImpl::Execute::explainMethod -- 
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
ServerImpl::Execute::
explainMethod(Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Explain::Execute]);
}

// FUNCTION public
//	Iterator::ServerImpl::Execute::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ServerImpl::Execute::
getData(Execution::Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(getResultSet());

	for (;;) {
		DServer::ResultSet::Status::Value eStatus =
			getResultSet()->getStatus();
		switch (eStatus) {
		case DServer::ResultSet::Status::Success:
			{
				return false;
			}
		case DServer::ResultSet::Status::Canceled:
			{
				// cancelation in sub-server should be ignored
				return false;
			}
		default:
			{
				break;
			}
		}
	}
}

////////////////////////////////////////////
// DExecution::Iterator::Server::Query

// FUNCTION public
//	Iterator::Server::Query::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	const STRING& cstrSQL_
//	
// RETURN
//	Server*
//
// EXCEPTIONS

//static
Server*
Server::Query::
create(Execution::Interface::IProgram& cProgram_,
	   Schema::Cascade* pSchemaCascade_,
	   const STRING& cstrSQL_)
{

	Action::ServerAccess* pAccess =
		Action::ServerAccess::create(cProgram_,
									 pSchemaCascade_,
									 cstrSQL_);
	AUTOPOINTER<Server> pServer = new ServerImpl::Query(pAccess->getID());
	pServer->registerToProgram(cProgram_);
	return pServer.release();
}


// FUNCTION public
//	Iterator::Server::Query::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	int iDataID_
//	
// RETURN
//	Server*
//
// EXCEPTIONS

//static
Server*
Server::Query::
create(Execution::Interface::IProgram& cProgram_,
	   Schema::Cascade* pSchemaCascade_,
	   int iDataID_)
{

	Action::ServerAccess* pAccess =
		Action::ServerAccess::create(cProgram_,
									 pSchemaCascade_,
									 iDataID_);
	AUTOPOINTER<Server> pServer = new ServerImpl::Query(pAccess->getID());
	pServer->registerToProgram(cProgram_);
	return pServer.release();
}

// FUNCTION public
//	Iterator::Server::Query::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	int iDataID_
//	
// RETURN
//	Server*
//
// EXCEPTIONS

//static
Server*
Server::Query::
create(Execution::Interface::IProgram& cProgram_,
	   int iAccessID_)
{
	AUTOPOINTER<Server> pServer = new ServerImpl::Query(iAccessID_);
	pServer->registerToProgram(cProgram_);
	return pServer.release();
}



////////////////////////////////////////////
// DExecution::Iterator::Server::Query

// FUNCTION public
//	Iterator::Server::Query::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	const STRING& cstrSQL_
//	
// RETURN
//	Server*
//
// EXCEPTIONS

//static
Server*
Server::PreparedQuery::
create(Execution::Interface::IProgram& cProgram_,
	   Schema::Cascade* pSchemaCascade_,
	   const STRING& cstrSQL_,
	   int iDataID_)
{
	Action::ServerAccess* pAccess =
		Action::ServerAccess::create(cProgram_,
									 pSchemaCascade_,
									 cstrSQL_);

	AUTOPOINTER<Server> pServer = new ServerImpl::PreparedQuery(pAccess->getID(), iDataID_);
	pServer->registerToProgram(cProgram_);
	return pServer.release();
}


////////////////////////////////////////////
// DExecution::Iterator::Server::Execute

// FUNCTION public
//	Iterator::Server::Execute::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Schema::Cascade* pSchemaCascade_
//	const STRING& cstrSQL_
//	
// RETURN
//	Server*
//
// EXCEPTIONS

//static
Server*
Server::Execute::
create(Execution::Interface::IProgram& cProgram_,
	   Schema::Cascade* pSchemaCascade_,
	   const STRING& cstrSQL_)
{
	Action::ServerAccess* pAccess =
		Action::ServerAccess::create(cProgram_,
									 pSchemaCascade_,
									 cstrSQL_);

	AUTOPOINTER<Server> pServer = new ServerImpl::Execute(pAccess->getID());
	pServer->registerToProgram(cProgram_);
	return pServer.release();
}

//////////////////////////////////////
// DExecution::Iterator::Server

// FUNCTION public
//	Iterator::Server::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Server*
//
// EXCEPTIONS

//static
Server*
Server::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::ServerQuery:
		{
			return new ServerImpl::Query;
		}
	case Class::Category::ServerExecute:
		{
			return new ServerImpl::Execute;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_DEXECUTION_ITERATOR_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
