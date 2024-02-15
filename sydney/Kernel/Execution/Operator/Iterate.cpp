// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Iterate.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
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

#include "Execution/Operator/Iterate.h"
#include "Execution/Operator/Class.h"

#include "Execution/Action/IteratorHolder.h"
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
			Once,
			All,
			RuntimeStartup,
			NestedAll,
			ValueNum
		};
	};
	const char* const _pszExplainName[] =
	{
		"iterate once",
		"iterate all",
		"iterate all with runtime startup",
		"nested all"
	};
}

namespace IterateImpl
{
	// CLASS local
	//	Execution::Operator::IterateImpl::Base -- base class of implementation classes of Iterate
	//
	// NOTES
	class Base
		: public Operator::Iterate
	{
	public:
		typedef Base This;
		typedef Operator::Iterate Super;

		virtual ~Base() {}

	///////////////////////////
	// Operator::Iterate::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					startUp(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
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
			  m_cIterator(),
			  m_bNoUndone(false),
			  m_bInitialized(false),
			  m_eStatus(Action::Status::Success)
		{}
		Base(int iIteratorID_, bool bNoUndone_)
			: Super(),
			  m_cIterator(iIteratorID_),
			  m_bNoUndone(bNoUndone_),
			  m_bInitialized(false),
			  m_eStatus(Action::Status::Success)
		{}

		// accessor
		Interface::IIterator* getIterator() {return m_cIterator.getIterator();}
		void initializeIterator(Interface::IProgram& cProgram_);
		Action::Status::Value getStatus() {return m_eStatus;}

	private:
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

		// iterator object
		Action::IteratorHolder m_cIterator;
		// undone flag
		bool m_bNoUndone;

		// runtime flag
		bool m_bInitialized;
		Action::Status::Value m_eStatus;
	};

	// CLASS local
	//	Execution::Operator::IterateImpl::Once -- implementation class of IterateOnce
	//
	// NOTES
	class Once
		: public Base
	{
	public:
		typedef Once This;
		typedef Base Super;

		Once()
			: Super()
		{}
		Once(int iIteratorID_, bool bNoUndone_)
			: Super(iIteratorID_, bNoUndone_)
		{}
		~Once() {}

	///////////////////////////
	// Operator::Iterate::

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
	//	Execution::Operator::IterateImpl::All -- implementation class of Iterate
	//
	// NOTES
	class All
		: public Base
	{
	public:
		typedef All This;
		typedef Base Super;

		All() : Super() {}
		All(int iIteratorID_, bool bNoUndone_)
			: Super(iIteratorID_, bNoUndone_) {}
		~All() {}

	///////////////////////////
	// Operator::Iterate::

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
	//	Execution::Operator::IterateImpl::RunTimeStartup -- implementation class of Iterate
	//
	// NOTES
	class RuntimeStartup
		: public All
	{
	public:
		typedef RuntimeStartup This;
		typedef All Super;

		RuntimeStartup() : Super() {}
		RuntimeStartup(int iIteratorID_, bool bNoUndone_)
			: Super(iIteratorID_, bNoUndone_) {}
		~RuntimeStartup() {}

	///////////////////////////
	// Operator::Iterate::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					startUp(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		
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
	//	Execution::Operator::IterateImpl::NestedAll ---
	//
	//
	// NOTES
	class NestedAll
		: public All
	{
	public:
		typedef NestedAll This;
		typedef All Super;

		NestedAll() : Super() {}
		NestedAll(int iIteratorID_, bool bNoUndone_)
			: Super(iIteratorID_, bNoUndone_) {}
		~NestedAll() {}

	///////////////////////////
	// Operator::Iterate::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				startUp(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		
	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);

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
// Execution::Operator::IterateImpl::Base

// FUNCTION public
//	Operator::IterateImpl::Base::explain -- 
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
IterateImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	explainOperator(cProgram_, cExplain_);
	cExplain_.pushIndent();
	m_cIterator.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent();
}

// FUNCTION public
//	Operator::IterateImpl::Base::initialize -- 
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
IterateImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	m_cIterator.initialize(cProgram_);
	initializeIterator(cProgram_);
}

// FUNCTION public
//	Operator::IterateImpl::Base::terminate -- 
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
IterateImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cIterator.terminate(cProgram_);
	m_bInitialized = false;
}

// FUNCTION public
//	Operator::IterateImpl::Base::startUp -- 
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
IterateImpl::Base::
startUp(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		return m_eStatus = cProgram_.startUp(getIterator());
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::IterateImpl::Base::finish -- 
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
IterateImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	m_eStatus = Action::Status::Success;
}

// FUNCTION public
//	Operator::IterateImpl::Base::reset -- 
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
IterateImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	Interface::IIterator* pIterator = getIterator();
	if (pIterator == 0)
		_SYDNEY_THROW0(Exception::Unexpected);
	
	cProgram_.reset(pIterator);
}

// FUNCTION public
//	Operator::IterateImpl::Base::undone -- 
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
IterateImpl::Base::
undone(Interface::IProgram& cProgram_)
{
	if (m_bNoUndone == false) {
		cProgram_.reset(getIterator());
		Super::undone(cProgram_);
	}
}

// FUNCTION public
//	Operator::IterateImpl::Base::serialize -- 
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
IterateImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cIterator.serialize(archiver_);
	archiver_(m_bNoUndone);
}

// FUNCTION protected
//	Operator::IterateImpl::Base::initializeIterator -- 
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
IterateImpl::Base::
initializeIterator(Interface::IProgram& cProgram_)
{
	if (m_bInitialized == false) {
		cProgram_.initialize(getIterator());
		m_bInitialized = true;
	}
}

///////////////////////////////////////////////
// Execution::Operator::IterateImpl::Once

// FUNCTION public
//	Operator::IterateImpl::Once::execute -- 
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
IterateImpl::Once::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	// iterate only once
	if (isDone() == false) {
		try {
			switch (getStatus()) {
			case Action::Status::Success:
			case Action::Status::False:
			case Action::Status::Continue:
				{
					cProgram_.next(getIterator());
					break;
				}
			case Action::Status::Break:
				{
					break;
				}
			default:
				{
					_SYDNEY_THROW0(Exception::Unexpected);
				}
			}
			cProgram_.finish(getIterator());
		} catch (...) {
			try {
				cProgram_.terminate(getIterator());
			} catch (...) {
				// ignore
			}
			_SYDNEY_RETHROW;
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::IterateImpl::Once::getClassID -- 
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
IterateImpl::Once::
getClassID() const
{
	return Class::getClassID(Class::Category::IterateOnce);
}

// FUNCTION private
//	Operator::IterateImpl::Once::explainOperator -- 
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
IterateImpl::Once::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Once]);
}

///////////////////////////////////////////////
// Execution::Operator::IterateImpl::All

// FUNCTION public
//	Operator::IterateImpl::All::execute -- 
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
IterateImpl::All::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	// same as iteration loop of V2Impl::ExecutorImpl::execute 
	if (isDone() == false) {
		try {
			if (getStatus() != Action::Status::Break) {

				// main iteration loop
				while (cProgram_.next(getIterator())) {}
			}
			cProgram_.finish(getIterator());
		} catch (...) {
			try {
				// clear objects converted by initialize
				cProgram_.terminate(getIterator());
			} catch (...) {
				// ignore
			}
			_SYDNEY_RETHROW;
		}

		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::IterateImpl::All::getClassID -- 
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
IterateImpl::All::
getClassID() const
{
	return Class::getClassID(Class::Category::IterateAll);
}

// FUNCTION private
//	Operator::IterateImpl::All::explainOperator -- 
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
IterateImpl::All::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::All]);
}

///////////////////////////////////////////////
// Execution::Operator::IterateImpl::All

// FUNCTION public
//	Operator::IterateImpl::RuntimeStartup::startUp -- 
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
IterateImpl::RuntimeStartup::
startUp(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{

	return Action::Status::Success;
}


// FUNCTION public
//	Operator::IterateImpl::All::execute -- 
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
IterateImpl::RuntimeStartup::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	// same as iteration loop of V2Impl::ExecutorImpl::execute 
	if (isDone() == false) {
		try {
			if (cProgram_.startUp(getIterator()) != Action::Status::Break) {

				// main iteration loop
				while (cProgram_.next(getIterator())) {}
				cProgram_.finish(getIterator());
			}

		} catch (...) {
			try {
				// clear objects converted by initialize
				cProgram_.terminate(getIterator());
			} catch (...) {
				// ignore
			}
			_SYDNEY_RETHROW;
		}

		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::IterateImpl::RuntimeStartup::getClassID -- 
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
IterateImpl::RuntimeStartup::
getClassID() const
{
	return Class::getClassID(Class::Category::IterateRuntimeStartup);
}

// FUNCTION private
//	Operator::IterateImpl::All::explainOperator -- 
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
IterateImpl::RuntimeStartup::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::RuntimeStartup]);
}


// FUNCTION public
//	Operator::IterateImpl::NestedAll::finish -- 
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
IterateImpl::NestedAll::
finish(Interface::IProgram& cProgram_)
{
	Super::finish(cProgram_);
	cProgram_.reset(getIterator());
	Super::Super::Super::undone(cProgram_); // m_bNoUndoneを無視してundoneする
}

// FUNCTION public
//	Operator::IterateImpl::NestedAll::getClassID -- 
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
IterateImpl::NestedAll::
getClassID() const
{
	return Class::getClassID(Class::Category::IterateNestedAll);
}

// FUNCTION private
//	Operator::IterateImpl::All::explainOperator -- 
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
IterateImpl::NestedAll::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::NestedAll]);
}

/////////////////////////////////
// Operator::Iterate::Once

// FUNCTION public
//	Operator::Iterate::Once::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	bool bNoUndone_ = false
//	
// RETURN
//	Iterate*
//
// EXCEPTIONS

//static
Iterate*
Iterate::Once::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   bool bNoUndone_)
{
	AUTOPOINTER<This> pResult =
		new IterateImpl::Once(iIteratorID_, bNoUndone_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////
// Operator::Iterate::All

// FUNCTION public
//	Operator::Iterate::All::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	bool bNoUndone_ = false
//	
// RETURN
//	Iterate*
//
// EXCEPTIONS

//static
Iterate*
Iterate::All::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   bool bNoUndone_)
{
	AUTOPOINTER<This> pResult =
		new IterateImpl::All(iIteratorID_, bNoUndone_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}


/////////////////////////////////
// Operator::Iterate::All

// FUNCTION public
//	Operator::Iterate::All::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	bool bNoUndone_ = false
//	
// RETURN
//	Iterate*
//
// EXCEPTIONS

//static
Iterate*
Iterate::RuntimeStartup::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   bool bNoUndone_)
{
	AUTOPOINTER<This> pResult =
		new IterateImpl::RuntimeStartup(iIteratorID_, bNoUndone_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}


// FUNCTION public
//	Operator::Iterate::NestedAll::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	bool bNoUndone_ = false
//	
// RETURN
//	Iterate*
//
// EXCEPTIONS

//static
Iterate*
Iterate::NestedAll::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_,
	   bool bNoUndone_)
{
	AUTOPOINTER<This> pResult =
		new IterateImpl::NestedAll(iIteratorID_, bNoUndone_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Operator::Iterate::

// FUNCTION public
//	Operator::Iterate::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Iterate*
//
// EXCEPTIONS

//static
Iterate*
Iterate::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::IterateOnce:
		{
			return new IterateImpl::Once;
		}
	case Class::Category::IterateAll:
		{
			return new IterateImpl::All;
		}

	case Class::Category::IterateRuntimeStartup:
		{
			return new IterateImpl::RuntimeStartup;
		}
	case Class::Category::IterateNestedAll:
		{
			return new IterateImpl::NestedAll;
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
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
