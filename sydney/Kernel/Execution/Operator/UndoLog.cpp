// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/UndoLog.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Operator/UndoLog.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/Argument.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"
#include "Opt/UndoLog.h"

#include "Os/AutoCriticalSection.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	const char* const _pszStoreExplainName = "store undolog";
	const char* const _pszPrepareExplainName = "prepare undolog";
	const char* const _pszResetExplainName = "reset undolog";
}

namespace UndoLogImpl
{
	// CLASS local
	//	Execution::Operator::UndoLogImpl::Base -- base class of implementation classes of UndoLog
	//
	// NOTES
	class Base
		: public Operator::UndoLog
	{
	public:
		typedef Base This;
		typedef Operator::UndoLog Super;

		virtual ~Base() {}

	///////////////////////////
	// Operator::UndoLog::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// ModSerializer
	//	virtual void serialize(ModArchive& archiver_);

	protected:
		Base()
			: Super(),
			  m_cUndoLog()
		{}
		Base(int iUndoLogID_)
			: Super(),
			  m_cUndoLog(iUndoLogID_)
		{}

		// base implementation
		void initializeBase(Interface::IProgram& cProgram_);
		void terminateBase(Interface::IProgram& cProgram_);
		void serializeBase(ModArchive& archiver_);

		// store undolog data
		void store(Interface::IProgram& cProgram_);

	private:
		Action::DataHolder m_cUndoLog;
	};

	// CLASS local
	//	Execution::Operator::UndoLogImpl::Single -- implementation class of undolog with single value
	//
	// NOTES
	class Single
		: public Base
	{
	public:
		typedef Single This;
		typedef Base Super;

		Single()
			: Super()
		{}
		Single(int iUndoLogID_,
			   int iDataID_,
			   int iLogDataID_)
			: Super(iUndoLogID_),
			  m_cData(iDataID_),
			  m_cLogData(iLogDataID_)
		{}
		~Single() {}

	///////////////////////////
	// Operator::UndoLog::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		Action::ArrayDataHolder m_cData;
		Action::ArrayDataHolder m_cLogData;
	};

	// CLASS local
	//	Execution::Operator::UndoLogImpl::Double -- implementation class of undolog with double values
	//
	// NOTES
	class Double
		: public Base
	{
	public:
		typedef Double This;
		typedef Base Super;

		Double()
			: Super()
		{}
		Double(int iUndoLogID_,
			   int iKeyDataID_,
			   int iKeyLogDataID_,
			   int iValueDataID_,
			   int iValueLogDataID_)
			: Super(iUndoLogID_),
			  m_cKeyData(iKeyDataID_),
			  m_cKeyLogData(iKeyLogDataID_),
			  m_cValueData(iValueDataID_),
			  m_cValueLogData(iValueLogDataID_)
		{}
		~Double() {}

	///////////////////////////
	// Operator::UndoLog::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		Action::ArrayDataHolder m_cKeyData;
		Action::ArrayDataHolder m_cKeyLogData;
		Action::ArrayDataHolder m_cValueData;
		Action::ArrayDataHolder m_cValueLogData;
	};

	// CLASS local
	//	Execution::Operator::UndoLogImpl::Prepare -- implementation class of prepare UndoLog
	//
	// NOTES
	class Prepare
		: public Operator::UndoLog
	{
	public:
		typedef Prepare This;
		typedef Operator::UndoLog Super;

		Prepare()
			: Super()
		{}
		~Prepare() {}

	///////////////////////////
	// Operator::UndoLog::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	};

	// CLASS local
	//	Execution::Operator::UndoLogImpl::Reset -- implementation class of reset UndoLog
	//
	// NOTES
	class Reset
		: public Operator::UndoLog
	{
	public:
		typedef Reset This;
		typedef Operator::UndoLog Super;

		Reset()
			: Super()
		{}
		Reset(int iLogTypeID_)
			: Super(),
			  m_cLogType(iLogTypeID_)
		{}
		~Reset() {}

	///////////////////////////
	// Operator::UndoLog::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
		Action::DataHolder m_cLogType;
	};
}

///////////////////////////////////////////////
// Execution::Operator::UndoLogImpl::Base

// FUNCTION public
//	Operator::UndoLogImpl::Base::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszStoreExplainName);
}

// FUNCTION public
//	Operator::UndoLogImpl::Base::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::UndoLogImpl::Base::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION protected
//	Operator::UndoLogImpl::Base::initializeBase -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UndoLogImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	m_cUndoLog.initialize(cProgram_);
}

// FUNCTION protected
//	Operator::UndoLogImpl::Base::terminateBase -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UndoLogImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	m_cUndoLog.terminate(cProgram_);
}

// FUNCTION protected
//	Operator::UndoLogImpl::Base::serializeBase -- 
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
UndoLogImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cUndoLog.serialize(archiver_);
}

// FUNCTION protected
//	Operator::UndoLogImpl::Base::store -- store undolog data
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UndoLogImpl::Base::
store(Interface::IProgram& cProgram_)
{
	Common::Data::Pointer pUndoLog = m_cUndoLog.get();
	cProgram_.getTransaction()->storeUndoLog(pUndoLog);

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Log)) {
		_OPT_EXECUTION_MESSAGE
			<< "UndoLog: " << Opt::Trace::toString(*pUndoLog)
			<< ModEndl;
	}
#endif
}

///////////////////////////////////////////////
// Execution::Operator::UndoLogImpl::Single

// FUNCTION public
//	Operator::UndoLogImpl::Single::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Single::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
		m_cLogData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::UndoLogImpl::Single::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Single::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cData.terminate(cProgram_);
	m_cLogData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::UndoLogImpl::Single::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
UndoLogImpl::Single::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Os::AutoCriticalSection l(cProgram_.getLatch());
		Utility::DataType::assignElements(m_cLogData.get(),
										  m_cData.getData());
		store(cProgram_);
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::UndoLogImpl::Single::getClassID -- 
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
UndoLogImpl::Single::
getClassID() const
{
	return Class::getClassID(Class::Category::UndoLogSingle);
}

// FUNCTION public
//	Operator::UndoLogImpl::Single::serialize -- 
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
UndoLogImpl::Single::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cData.serialize(archiver_);
	m_cLogData.serialize(archiver_);
}

///////////////////////////////////////////////
// Execution::Operator::UndoLogImpl::Double

// FUNCTION public
//	Operator::UndoLogImpl::Double::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Double::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cKeyData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cKeyData.initialize(cProgram_);
		m_cKeyLogData.initialize(cProgram_);
		m_cValueData.initialize(cProgram_);
		m_cValueLogData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Operator::UndoLogImpl::Double::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Double::
terminate(Interface::IProgram& cProgram_)
{
	terminateBase(cProgram_);
	m_cKeyData.terminate(cProgram_);
	m_cKeyLogData.terminate(cProgram_);
	m_cValueData.terminate(cProgram_);
	m_cValueLogData.terminate(cProgram_);
}

// FUNCTION public
//	Operator::UndoLogImpl::Double::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
UndoLogImpl::Double::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Os::AutoCriticalSection l(cProgram_.getLatch());
		Utility::DataType::assignElements(m_cKeyLogData.get(),
										  m_cKeyData.getData());
		Utility::DataType::assignElements(m_cValueLogData.get(),
										  m_cValueData.getData());
		store(cProgram_);
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::UndoLogImpl::Double::getClassID -- 
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
UndoLogImpl::Double::
getClassID() const
{
	return Class::getClassID(Class::Category::UndoLogDouble);
}

// FUNCTION public
//	Operator::UndoLogImpl::Double::serialize -- 
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
UndoLogImpl::Double::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cKeyData.serialize(archiver_);
	m_cKeyLogData.serialize(archiver_);
	m_cValueData.serialize(archiver_);
	m_cValueLogData.serialize(archiver_);
}

///////////////////////////////////////////////
// Execution::Operator::UndoLogImpl::Prepare::

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Prepare::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszPrepareExplainName);
}

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Prepare::
initialize(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Prepare::
terminate(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
UndoLogImpl::Prepare::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Os::AutoCriticalSection l(cProgram_.getLatch());
		cProgram_.getTransaction()->prepareUndoLog();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Prepare::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Prepare::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::getClassID -- 
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
UndoLogImpl::Prepare::
getClassID() const
{
	return Class::getClassID(Class::Category::UndoLogPrepare);
}

// FUNCTION public
//	Operator::UndoLogImpl::Prepare::serialize -- 
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
UndoLogImpl::Prepare::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
}

///////////////////////////////////////////////
// Execution::Operator::UndoLogImpl::Reset::

// FUNCTION public
//	Operator::UndoLogImpl::Reset::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
//	Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Reset::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszResetExplainName);
}

// FUNCTION public
//	Operator::UndoLogImpl::Reset::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Reset::
initialize(Interface::IProgram& cProgram_)
{
	m_cLogType.initialize(cProgram_);
}

// FUNCTION public
//	Operator::UndoLogImpl::Reset::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Reset::
terminate(Interface::IProgram& cProgram_)
{
	m_cLogType.terminate(cProgram_);
}

// FUNCTION public
//	Operator::UndoLogImpl::Reset::execute -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
UndoLogImpl::Reset::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		Os::AutoCriticalSection l(cProgram_.getLatch());
		Common::Data::Pointer pLogType = m_cLogType.get();
		cProgram_.getTransaction()->clearUndoLog();
		cProgram_.getTransaction()->storeUndoLog(pLogType);
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::UndoLogImpl::Reset::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Reset::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::UndoLogImpl::Reset::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
UndoLogImpl::Reset::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::UndoLogImpl::Reset::getClassID -- 
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
UndoLogImpl::Reset::
getClassID() const
{
	return Class::getClassID(Class::Category::UndoLogReset);
}

// FUNCTION public
//	Operator::UndoLogImpl::Reset::serialize -- 
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
UndoLogImpl::Reset::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cLogType.serialize(archiver_);
}

/////////////////////////////////
// Operator::UndoLog::Prepare

// FUNCTION public
//	Operator::UndoLog::Prepare::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	UndoLog*
//
// EXCEPTIONS

//static
UndoLog*
UndoLog::Prepare::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_)
{
	AUTOPOINTER<This> pResult = new UndoLogImpl::Prepare;
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::UndoLog::Reset

// FUNCTION public
//	Operator::UndoLog::Reset::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLogTypeID_
//	
// RETURN
//	UndoLog*
//
// EXCEPTIONS

//static
UndoLog*
UndoLog::Reset::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLogTypeID_)
{
	AUTOPOINTER<This> pResult = new UndoLogImpl::Reset(iLogTypeID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::UndoLog::

// FUNCTION public
//	Operator::UndoLog::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iUndoLogID_
//	int iDataID_
//	int iLogDataID_
//	
// RETURN
//	UndoLog*
//
// EXCEPTIONS

//static
UndoLog*
UndoLog::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iUndoLogID_,
	   int iDataID_,
	   int iLogDataID_)
{
	AUTOPOINTER<This> pResult =
		new UndoLogImpl::Single(iUndoLogID_, iDataID_, iLogDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::UndoLog::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iUndoLogID_
//	int iKeyDataID_
//	int iKeyLogDataID_
//	int iValueDataID_
//	int iValueLogDataID_
//	
// RETURN
//	UndoLog*
//
// EXCEPTIONS

//static
UndoLog*
UndoLog::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iUndoLogID_,
	   int iKeyDataID_,
	   int iKeyLogDataID_,
	   int iValueDataID_,
	   int iValueLogDataID_)
{
	AUTOPOINTER<This> pResult =
		new UndoLogImpl::Double(iUndoLogID_,
								iKeyDataID_, iKeyLogDataID_,
								iValueDataID_, iValueLogDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::UndoLog::

// FUNCTION public
//	Operator::UndoLog::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	UndoLog*
//
// EXCEPTIONS

//static
UndoLog*
UndoLog::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::UndoLogSingle:
		{
			return new UndoLogImpl::Single;
		}
	case Class::Category::UndoLogDouble:
		{
			return new UndoLogImpl::Double;
		}
	case Class::Category::UndoLogPrepare:
		{
			return new UndoLogImpl::Prepare;
		}
	case Class::Category::UndoLogReset:
		{
			return new UndoLogImpl::Reset;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
