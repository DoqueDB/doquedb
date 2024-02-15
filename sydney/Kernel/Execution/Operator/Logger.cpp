// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Logger.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/Logger.h"
#include "Execution/Operator/Class.h"
#include "Execution/Action/Argument.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/LogData.h"
#include "Opt/Trace.h"

#include "Trans/AutoLatch.h"
#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	const char* const _pszExplainName = "write log";
}

namespace Impl
{
	// CLASS local
	//	Execution::Operator::Impl::LoggerImpl -- implementation classes of Logger
	//
	// NOTES
	class LoggerImpl
		: public Operator::Logger
	{
	public:
		typedef LoggerImpl This;
		typedef Operator::Logger Super;

		// constructor
		LoggerImpl()
			: Super(),
			  m_cLogData(),
			  m_iLogTypeID(-1),
			  m_pLogType()
		{}
		LoggerImpl(int iLogDataID_,
				   int iLogTypeID_)
			: Super(),
			  m_cLogData(iLogDataID_),
			  m_iLogTypeID(iLogTypeID_),
			  m_pLogType()
		{}
		// destrucotr
		~LoggerImpl() {}

	///////////////////////////
	// Operator::Logger::

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
		Action::ArrayDataHolder m_cLogData;
		int m_iLogTypeID;
		Common::Data::Pointer m_pLogType;
	};
}

///////////////////////////////////////////////
// Execution::Operator::Impl::LoggerImpl

// FUNCTION public
//	Operator::Impl::LoggerImpl::explain -- 
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
Impl::LoggerImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName);
}

// FUNCTION public
//	Operator::Impl::LoggerImpl::initialize -- 
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
Impl::LoggerImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cLogData.isInitialized() == false) {
		m_cLogData.initialize(cProgram_);
		m_pLogType = cProgram_.getVariable(m_iLogTypeID);
	}
}

// FUNCTION public
//	Operator::Impl::LoggerImpl::terminate -- 
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
Impl::LoggerImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cLogData.terminate(cProgram_);
	m_pLogType = Common::Data::Pointer();
}

// FUNCTION public
//	Operator::Impl::LoggerImpl::execute -- 
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
Impl::LoggerImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Log)) {
			_OPT_EXECUTION_MESSAGE
				<< "writeLog: " << Opt::Trace::toString(*m_cLogData)
				<< ModEndl;
		}
#endif
		Opt::LogData cLogData(Trans::Log::Data::Category::TupleModify, *m_cLogData.getData());
		Trans::Transaction& cTrans = *cProgram_.getTransaction();
		{
			Trans::AutoLatch latch(cTrans,
								   cTrans.getLogInfo(Trans::Log::File::Category::Database).getLockName());
			(void)cTrans.storeLog(Trans::Log::File::Category::Database, cLogData);

			// write logtype as undolog
			cTrans.storeUndoLog(m_pLogType);
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::LoggerImpl::finish -- 
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
Impl::LoggerImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::LoggerImpl::reset -- 
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
Impl::LoggerImpl::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::LoggerImpl::getClassID -- 
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
Impl::LoggerImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Logger);
}

// FUNCTION public
//	Operator::Impl::LoggerImpl::serialize -- 
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
Impl::LoggerImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cLogData.serialize(archiver_);
	archiver_(m_iLogTypeID);
}

/////////////////////////////////
// Operator::Logger::

// FUNCTION public
//	Operator::Logger::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iLogDataID_
//	int iLogTypeID_
//	
// RETURN
//	Logger*
//
// EXCEPTIONS

//static
Logger*
Logger::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iLogDataID_,
	   int iLogTypeID_)
{
	AUTOPOINTER<This> pResult = new Impl::LoggerImpl(iLogDataID_, iLogTypeID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Logger::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Logger*
//
// EXCEPTIONS

//static
Logger*
Logger::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Logger);
	return new Impl::LoggerImpl;
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
