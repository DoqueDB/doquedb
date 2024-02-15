// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Parallel/Thread.cpp --
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
const char moduleName[] = "Execution::Parallel";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Parallel/Class.h"
#include "Execution/Parallel/Thread.h"

#include "Execution/Action/ActionList.h"
#include "Execution/Action/Thread.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PARALLEL_BEGIN

namespace Impl
{
	// CLASS local
	//	$$$::Impl::ThreadImpl --
	//
	// NOTES

	class ThreadImpl
		: public Parallel::Thread
	{
	public:
		typedef Parallel::Thread Super;
		typedef ThreadImpl This;

		// constructor
		ThreadImpl()
			: Super(),
			  m_vecThread()
		{}
		// descructor
		~ThreadImpl() {}

	///////////////////////////////
	// Interface::IParallel::
		virtual int addList(Interface::IProgram& cProgram_);
		virtual Action::ActionList& getList(Interface::IProgram& cProgram_);
		virtual void setReturnData(Interface::IProgram& cProgram_,
								   int iDataID_);

	///////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual Action::Status::Value execute(Interface::IProgram& cProgram_,
											  Action::ActionList& cActionList_);
	//	virtual void accumulate(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		VECTOR<Action::ThreadHolder> m_vecThread;
	};

} // namespace Impl

///////////////////////////////
// $$$::Impl::ThreadImpl
///////////////////////////////

// FUNCTION public
//	Parallel::Impl::ThreadImpl::addList -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::ThreadImpl::
addList(Interface::IProgram& cProgram_)
{
	Action::Thread* pThread = Action::Thread::create(cProgram_);
	if (pThread->getList().getSize() > 0) {
		// reused thread -> search for existing entry
		int n = m_vecThread.GETSIZE();
		for (int i = 0; i < n; ++i) {
			if (m_vecThread[i].getID() == pThread->getID()) {
				return i;
			}
		}
	}
	m_vecThread.PUSHBACK(Action::ThreadHolder(pThread->getID()));
	return m_vecThread.GETSIZE() - 1;
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::getList -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::ActionList&
//
// EXCEPTIONS

//virtual
Action::ActionList&
Impl::ThreadImpl::
getList(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_vecThread.ISEMPTY() == false);
	Action::ThreadHolder& cThread = m_vecThread.GETBACK();
	Action::Thread* pThread = cThread.get();
	if (pThread == 0) {
		pThread = cProgram_.getThread(cThread.getID());
	}

	return pThread->getList();
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::setReturnData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ThreadImpl::
setReturnData(Interface::IProgram& cProgram_,
			  int iDataID_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::explain -- 
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
Impl::ThreadImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("parallel");
	int n = m_vecThread.GETSIZE();
	for (int i = 0; i < n; ++i) {
		cExplain_.newLine(true).put("@").put(i);
		m_vecThread[i].explain(pEnvironment_,
							   cProgram_,
							   cExplain_);
	}
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::initialize -- 
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
Impl::ThreadImpl::
initialize(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecThread,
			boost::bind(&Action::ThreadHolder::initialize,
						_1,
						boost::ref(cProgram_)));
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::terminate -- 
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
Impl::ThreadImpl::
terminate(Interface::IProgram& cProgram_)
{
	undone(cProgram_);
	FOREACH(m_vecThread,
			boost::bind(&Action::ThreadHolder::terminate,
						_1,
						boost::ref(cProgram_)));
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::execute -- 
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
Impl::ThreadImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (!isDone()) {
		FOREACH(m_vecThread,
				boost::bind(&Action::Thread::start,
							boost::bind(&Action::ThreadHolder::get,
										_1),
							boost::ref(cProgram_)));
		done();
	}

	
	return Action::Status::Success;
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::finish -- 
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
Impl::ThreadImpl::
finish(Interface::IProgram& cProgram_)
{
	undone(cProgram_);
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::reset -- 
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
Impl::ThreadImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::undone -- 
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
Impl::ThreadImpl::
undone(Interface::IProgram& cProgram_)
{
	if (isDone()) {
		FOREACH(m_vecThread,
				boost::bind(&Common::Thread::join,
							boost::bind(&Action::ThreadHolder::get,
										_1)));
		
		FOREACH(m_vecThread,
				boost::bind(&Action::Thread::checkExitStatus,
							boost::bind(&Action::ThreadHolder::get,
										_1),
							boost::ref(cProgram_)));
		Super::undone(cProgram_);
	}
}



// FUNCTION public
//	Parallel::Impl::ThreadImpl::getClassID -- 
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
Impl::ThreadImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Thread);
}

// FUNCTION public
//	Parallel::Impl::ThreadImpl::serialize -- 
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
Impl::ThreadImpl::
serialize(ModArchive& archiver_)
{
	Execution::Utility::SerializeObject(archiver_,
										m_vecThread);
}

////////////////////////////////////
// Execution::Parallel::Thread

// FUNCTION public
//	Parallel::Thread::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	Thread*
//
// EXCEPTIONS

//static
Thread*
Thread::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_)
{
	AUTOPOINTER<This> pResult = new Impl::ThreadImpl;
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Parallel::Thread::getInstance -- 
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
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Thread);
	return new Impl::ThreadImpl;
}

_SYDNEY_EXECUTION_ACTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
