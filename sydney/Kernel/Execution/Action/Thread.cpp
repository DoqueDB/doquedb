// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Action/Thread.cpp --
// 
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Action";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Action/Thread.h"
#include "Execution/Action/Class.h"

#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/ExceptionObject.h"
#include "Common/Message.h"
#include "Common/Thread.h"

#include "Exception/ModLibraryError.h"
#include "Exception/NumberCancel.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Os/AutoCriticalSection.h"
#include "Os/CriticalSection.h"

_SYDNEY_USING
_SYDNEY_EXECUTION_USING
_SYDNEY_EXECUTION_ACTION_USING



//////////////////////////////////////////
// Execution::Action::Thread

// FUNCTION public
//	Action::Thread::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iThreadID_
//	
// RETURN
//	Thread*
//
// EXCEPTIONS

//static
Thread*
Thread::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Thread;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Action::Thread::explain -- 
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

void
Thread::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("[thread]");
	m_cActionList.explain(pEnvironment_, cProgram_, cExplain_);
}

// FUNCTION public
//	Action::Thread::addActionID -- add ActionID to be executed
//
// NOTES
//
// ARGUMENTS
//	int iActionID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Thread::
addActionID(int iActionID_)
{
	m_cActionList.addID(iActionID_);
}

// FUNCTION public
//	Action::Thread::getList -- get action list to be executed
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Action::ActionList&
//
// EXCEPTIONS

Action::ActionList&
Thread::
getList()
{
	return m_cActionList;
}

// FUNCTION public
//	Action::Thread::initialize -- 
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
Thread::
initialize(Interface::IProgram& cProgram_)
{
	m_cProgram.setProgram(&cProgram_);
	m_cActionList.initialize(m_cProgram);
}

// FUNCTION public
//	Action::Thread::terminate -- 
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
Thread::
terminate(Interface::IProgram& cProgram_)
{
	m_cActionList.terminate(m_cProgram);
}

// FUNCTION public
//	Action::Thread::start -- 
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
Thread::
start(Interface::IProgram& cProgram_)
{
	// start thread
	create();
}

// FUNCTION public
//	Action::Thread::join -- 
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
Thread::
join(Interface::IProgram& cProgram_)
{
	// join thread
	switch (join()) {
	case Common::Thread::Normally:
		{
			break;
		}
	case Common::Thread::ThrowException:
		{
			switch (getException().getErrorNumber()) {
			case Exception::ErrorNumber::Cancel:
				{
					break;
				}
			default:
				{
					SydInfoMessage << "Exceptiion occurered in ThreadID: " << getMessageThreadID() << ModEndl;
					getException().throwClassInstance();
				}
			}
			break;
		}
	case Common::Thread::ThrowModException:
		{
			throw Exception::ModLibraryError(moduleName,
											 __FILE__,
											 __LINE__,
											 getModException());
		}
	case Common::Thread::ExitThread:
	case Common::Thread::KillThread:
	case Common::Thread::Unknown:
	default:
		{
			// YET
			break;
		}
	}
}

// FUNCTION public
//	Action::Thread::join -- 
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
Thread::
checkExitStatus(Interface::IProgram& cProgram_)
{
	// join thread
	switch (getExitStatus()) {
	case Common::Thread::Normally:
		{
			break;
		}
	case Common::Thread::ThrowException:
		{
			switch (getException().getErrorNumber()) {
			case Exception::ErrorNumber::Cancel:
				{
					break;
				}
			default:
				{
					SydInfoMessage << "Exceptiion occurered in ThreadID: " << getMessageThreadID() << ModEndl;
					getException().throwClassInstance();
				}
			}
			break;
		}
	case Common::Thread::ThrowModException:
		{
			throw Exception::ModLibraryError(moduleName,
											 __FILE__,
											 __LINE__,
											 getModException());
		}
	case Common::Thread::ExitThread:
	case Common::Thread::KillThread:
	case Common::Thread::Unknown:
	default:
		{
			// YET
			break;
		}
	}
}

// FUNCTION public
//	Action::Thread::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Thread*
//
// EXCEPTIONS

//static
Thread*
Thread::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Thread:
		{
			return new Thread;
		}
	default:
		{
			_SYDNEY_ASSERT(false);
			break;
		}
	}
	return 0;
}

// FUNCTION public
//	Action::Thread::getClassID -- 
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
Thread::
getClassID() const
{
	return Class::getClassID(Class::Category::Thread);
}

// FUNCTION public
//	Action::Thread::serialize -- 
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
Thread::
serialize(ModArchive& archiver_)
{
	m_cActionList.serialize(archiver_);
}

// FUNCTION public
//	Action::Thread::registerToProgram -- register to program
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
Thread::
registerToProgram(Interface::IProgram& cProgram_)
{
	setID(cProgram_.registerThread(this));
}

// FUNCTION protected
//	Action::Thread::runnable -- 
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
Thread::
runnable()
{

	try {

		m_cProgram.resetData();
		(void)m_cActionList.execute(m_cProgram);
		m_cActionList.finish(m_cProgram);

	} catch (...) {
		try {
			m_cActionList.terminate(m_cProgram);
		} catch (...) {; /* ignore */}
		_SYDNEY_RETHROW;
	}

}

// FUNCTION private
//	Action::Thread::destruct -- 
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
Thread::
destruct()
{

}

//////////////////////////////////////////
// Execution::Action::ThreadHolder

// FUNCTION public
//	Action::ThreadHolder::explain -- 
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

void
ThreadHolder::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (m_pThread) {
		m_pThread->explain(pEnvironment_, cProgram_, cExplain_);
	} else {
		cProgram_.getThread(m_iID)->explain(pEnvironment_, cProgram_, cExplain_);
	}
}

// FUNCTION public
//	Action::ThreadHolder::initialize -- initialize Thread instance
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
ThreadHolder::
initialize(Interface::IProgram& cProgram_)
{
	if (m_pThread == 0) {
		m_pThread = cProgram_.getThread(m_iID);
		if (m_pThread == 0) {
			_SYDNEY_THROW0(Exception::Unexpected);
		}
		m_pThread->initialize(cProgram_);
	}
}

// FUNCTION public
//	Action::ThreadHolder::terminate -- terminate Thread instance
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
ThreadHolder::
terminate(Interface::IProgram& cProgram_)
{
	if (m_pThread) {
		m_pThread->terminate(cProgram_);
		m_pThread = 0;
	}
}

// FUNCTION public
//	Action::ThreadHolder::serialize -- serializer
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
ThreadHolder::
serialize(ModArchive& archiver_)
{
	archiver_(m_iID);
}

//
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
