// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Thread.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/Thread.h"
#include "Execution/Operator/Class.h"

#include "Execution/Action/Thread.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			Start,
			Join,
			ValueNum
		};
	};
	const char* const _pszExplainName[] =
	{
		"thread start",
		"thread join",
	};
}

namespace ThreadImpl
{
	// CLASS local
	//	Execution::Operator::ThreadImpl::Base -- base class of implementation classes of Thread
	//
	// NOTES
	class Base
		: public Operator::Thread
	{
	public:
		typedef Base This;
		typedef Operator::Thread Super;

		virtual ~Base() {}

	///////////////////////////
	// Operator::Thread::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		Base()
			: Super(),
			  m_cThread(),
			  m_bInitialized(false)
		{}
		Base(int iThreadID_)
			: Super(),
			  m_cThread(iThreadID_),
			  m_bInitialized(false)
		{}

		// accessor
		Action::ThreadHolder& getThread() {return m_cThread;}

	private:
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

		// thread object
		Action::ThreadHolder m_cThread;

		// runtime flag
		bool m_bInitialized;
	};

	// CLASS local
	//	Execution::Operator::ThreadImpl::Start -- implementation class of ThreadStart
	//
	// NOTES
	class Start
		: public Base
	{
	public:
		typedef Start This;
		typedef Base Super;

		Start()
			: Super()
		{}
		Start(int iThreadID_)
			: Super(iThreadID_)
		{}
		~Start() {}

	///////////////////////////
	// Operator::Thread::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Operator::ThreadImpl::Join -- implementation class of Thread
	//
	// NOTES
	class Join
		: public Base
	{
	public:
		typedef Join This;
		typedef Base Super;

		Join() : Super() {}
		Join(int iThreadID_)
			: Super(iThreadID_) {}
		~Join() {}

	///////////////////////////
	// Operator::Thread::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////
	// Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};
}

///////////////////////////////////////////////
// Execution::Operator::ThreadImpl::Base

// FUNCTION public
//	Operator::ThreadImpl::Base::explain -- 
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
ThreadImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainOperator(cProgram_, cExplain_);
	cExplain_.pushIndent();
	m_cThread.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent();
}

// FUNCTION public
//	Operator::ThreadImpl::Base::initialize -- 
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
ThreadImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	m_cThread.initialize(cProgram_);
}

// FUNCTION public
//	Operator::ThreadImpl::Base::terminate -- 
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
ThreadImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cThread.terminate(cProgram_);
	m_bInitialized = false;
}

// FUNCTION public
//	Operator::ThreadImpl::Base::finish -- 
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
ThreadImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::ThreadImpl::Base::reset -- 
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
ThreadImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	; // YET
}

// FUNCTION public
//	Operator::ThreadImpl::Base::undone -- 
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
ThreadImpl::Base::
undone(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::ThreadImpl::Base::serialize -- 
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
ThreadImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cThread.serialize(archiver_);
}

///////////////////////////////////////////////
// Execution::Operator::ThreadImpl::Start

// FUNCTION public
//	Operator::ThreadImpl::Start::execute -- 
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
ThreadImpl::Start::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	// thread only start
	if (isDone() == false) {
		getThread()->start(cProgram_);
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::ThreadImpl::Start::getClassID -- 
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
ThreadImpl::Start::
getClassID() const
{
	return Class::getClassID(Class::Category::ThreadStart);
}

// FUNCTION private
//	Operator::ThreadImpl::Start::explainOperator -- 
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
ThreadImpl::Start::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Start]);
}

///////////////////////////////////////////////
// Execution::Operator::ThreadImpl::Join

// FUNCTION public
//	Operator::ThreadImpl::Join::execute -- 
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
ThreadImpl::Join::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	// same as iteration loop of V2Impl::ExecutorImpl::execute 
	if (isDone() == false) {
		getThread()->join(cProgram_);
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::ThreadImpl::Join::getClassID -- 
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
ThreadImpl::Join::
getClassID() const
{
	return Class::getClassID(Class::Category::ThreadJoin);
}

// FUNCTION private
//	Operator::ThreadImpl::Join::explainOperator -- 
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
ThreadImpl::Join::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Join]);
}

/////////////////////////////////
// Operator::Thread::Start

// FUNCTION public
//	Operator::Thread::Start::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iThreadID_
//	
// RETURN
//	Thread*
//
// EXCEPTIONS

//static
Thread*
Thread::Start::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iThreadID_)
{
	AUTOPOINTER<This> pResult =
		new ThreadImpl::Start(iThreadID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Thread::Join

// FUNCTION public
//	Operator::Thread::Join::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iThreadID_
//	
// RETURN
//	Thread*
//
// EXCEPTIONS

//static
Thread*
Thread::Join::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iThreadID_)
{
	AUTOPOINTER<This> pResult =
		new ThreadImpl::Join(iThreadID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::Thread::

// FUNCTION public
//	Operator::Thread::getInstance -- for serialize
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
	case Class::Category::ThreadStart:
		{
			return new ThreadImpl::Start;
		}
	case Class::Category::ThreadJoin:
		{
			return new ThreadImpl::Join;
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
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// Join rights reserved.
//
