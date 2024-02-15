// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Iterator/Input.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Iterator";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Iterator/Input.h"
#include "Execution/Iterator/Class.h"

#include "Execution/Action/Collection.h"
#include "Execution/Action/Thread.h"
#include "Execution/Interface/ICollection.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "input";
}

namespace InputImpl
{
	// CLASS local
	//	Execution::Iterator::InputImpl::Base -- implementation class of Input
	//
	// NOTES
	class Base
		: public Iterator::Input
	{
	public:
		typedef Base This;
		typedef Iterator::Input Super;

		Base()
			: Super(),
			  m_cCollection()
		{}

		Base(int iCollectionID_)
			: Super(),
			  m_cCollection()
		{
			m_cCollection.setCollectionID(iCollectionID_);
		}
		
		virtual ~Base()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
		virtual bool next(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
		virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		Action::Collection& getCollection() {return m_cCollection;}

	///////////////////////////
	//Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);

		virtual void addInput(Interface::IProgram& cProgram_,
							  const Action::Argument& cAction_);

		virtual void addOutData(Interface::IProgram& cProgram_,
								const Action::Argument& cAction_);		

	private:
		// Collection access
		Action::Collection m_cCollection;
	};

	// CLASS local
	//	Execution::Iterator::InputImpl::Normal -- implementation class of Input
	//
	// NOTES
	class Normal
		: public Base
	{
	public:
		typedef Normal This;
		typedef Base Super;

		Normal()
			: Super()
		{}

		Normal(int iCollectionID_)
			: Super(iCollectionID_)
		{}
		
		~Normal()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual bool next(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);
	//	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	void serialize(ModArchive& archiver_);

	protected:
	private:
	};

	// CLASS local
	//	Execution::Iterator::InputImpl::Thread -- implementation class of Input
	//
	// NOTES
	class Thread
		: public Base
	{
	public:
		typedef Thread This;
		typedef Base Super;

		Thread()
			: Super(),
			  m_cThread()
		{}
		Thread(int iThreadID_)
			: Super(),
			  m_cThread(iThreadID_)
		{}
		~Thread()
		{}

	///////////////////////////
	//Interface::IIterator::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);
	//	virtual bool next(Interface::IProgram& cProgram_);
	//	virtual void reset(Interface::IProgram& cProgram_);
		virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
	//	virtual void finish(Interface::IProgram& cProgram_);
	//	virtual void setWasLast(Interface::IProgram& cProgram_);
	//	virtual void addStartUp(Interface::IProgram& cProgram_,
	//							const Action::Argument& cAction_);
	//	virtual void addAction(Interface::IProgram& cProgram_,
	//						   const Action::Argument& cAction_);
	//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
	//	virtual bool isEndOfData();

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	///////////////////////////
	//Iterator::Base::
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	private:
		Action::ThreadHolder m_cThread;
	};
}

/////////////////////////////////////////////
// Execution::Iterator::InputImpl::Base

// FUNCTION public
//	Iterator::InputImpl::Base::initialize -- 
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
InputImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	// convert collection ID and variable ID to objects
	if (m_cCollection.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cCollection.initialize(cProgram_);
		m_cCollection.prepareGetInterface();
	}
}

// FUNCTION public
//	Iterator::InputImpl::Base::terminate -- 
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
InputImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cCollection.isInitialized()) {
		m_cCollection.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Iterator::InputImpl::Base::next -- go to next tuple
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
InputImpl::Base::
next(Interface::IProgram& cProgram_)
{
	// read one tuple from collection and assign to result data
	if (hasNext()) {
		setHasNext(setHasData(m_cCollection.get(cProgram_)));
	} else {
		setHasData(false);
	}
	return hasData();
}

// FUNCTION public
//	Iterator::InputImpl::Base::reset -- 
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
InputImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	if (m_cCollection.isInitialized()) {
		m_cCollection.reset();
		resetBase(cProgram_);
	}
}

// FUNCTION public
//	Iterator::InputImpl::Base::finish -- 
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
InputImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	m_cCollection.finish(cProgram_);
	finishBase(cProgram_);
}

// FUNCTION public
//	Iterator::InputImpl::Base::serialize -- 
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
InputImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cCollection.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::InputImpl::Base::explainThis -- 
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
InputImpl::Base::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperatorName).put(" from ");
	cExplain_.popNoNewLine();
	cExplain_.pushIndent();
	m_cCollection.explain(pEnvironment_, cProgram_, cExplain_);
	m_cCollection.explainGetData(cProgram_, cExplain_);
	cExplain_.popIndent();
}

// FUNCTION protected
//	Iterator::InputImpl::Base::addInput -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InputImpl::Base::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	// collection and variable for input data is set through addAction(Input)
	m_cCollection.setCollectionID(cAction_.getInstanceID());
	m_cCollection.setDataID(cAction_.getArgumentID());
}

// FUNCTION protected
//	Iterator::InputImpl::Base::addInput -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Action::Argument& cAction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
InputImpl::Base::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	// collection and variable for input data is set through addAction(Input)
	m_cCollection.setDataID(cAction_.getInstanceID());
}


////////////////////////////////////////////
// Execution::Iterator::InputImpl::Normal

// FUNCTION public
//	Iterator::InputImpl::Normal::getClassID -- 
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
InputImpl::Normal::
getClassID() const
{
	return Class::getClassID(Class::Category::Input);
}

////////////////////////////////////////////
// Execution::Iterator::InputImpl::Thread

// FUNCTION public
//	Iterator::InputImpl::Thread::initialize -- 
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
InputImpl::Thread::
initialize(Interface::IProgram& cProgram_)
{
	Super::initialize(cProgram_);
	m_cThread.initialize(cProgram_);
}

// FUNCTION public
//	Iterator::InputImpl::Thread::terminate -- 
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
InputImpl::Thread::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cThread.isInitialized()) {
		getCollection().finish(cProgram_);
		m_cThread->join(cProgram_);
		m_cThread.terminate(cProgram_);
		Super::terminate(cProgram_);
	}
}

// FUNCTION public
//	Iterator::InputImpl::Thread::startUp -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
InputImpl::Thread::
startUp(Interface::IProgram& cProgram_)
{
	Action::Status::Value eResult = Action::Status::Success;
	if ((eResult = Super::startUp(cProgram_)) != Action::Status::Break) {
		m_cThread->start(cProgram_);
	}
	return eResult;
}

// FUNCTION public
//	Iterator::InputImpl::Thread::getClassID -- 
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
InputImpl::Thread::
getClassID() const
{
	return Class::getClassID(Class::Category::InputThread);
}

// FUNCTION public
//	Iterator::InputImpl::Thread::serialize -- 
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
InputImpl::Thread::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	m_cThread.serialize(archiver_);
}

// FUNCTION protected
//	Iterator::InputImpl::Thread::explainThis -- 
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
InputImpl::Thread::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	Super::explainThis(pEnvironment_,
					   cProgram_,
					   cExplain_);
	cExplain_.pushIndent();
	m_cThread.explain(pEnvironment_,
					  cProgram_,
					  cExplain_);
	cExplain_.popIndent();
}

//////////////////////////////
// Iterator::Input::Thread::

// FUNCTION public
//	Iterator::Input::Thread::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iThreadID_
//	
// RETURN
//	Input*
//
// EXCEPTIONS

//static
Input*
Input::Thread::
create(Interface::IProgram& cProgram_,
	   int iThreadID_)
{
	AUTOPOINTER<This> pResult = new InputImpl::Thread(iThreadID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

//////////////////////////////
// Iterator::Input::

// FUNCTION public
//	Iterator::Input::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Input*
//
// EXCEPTIONS

//static
Input*
Input::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new InputImpl::Normal;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}


// FUNCTION public
//	Iterator::Input::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Input*
//
// EXCEPTIONS

//static
Input*
Input::
create(Interface::IProgram& cProgram_,
	   int iCollectionID_)
{
	AUTOPOINTER<This> pResult = new InputImpl::Normal(iCollectionID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}


// FUNCTION public
//	Iterator::Input::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Input*
//
// EXCEPTIONS

//static
Input*
Input::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Input:
		{
			return new InputImpl::Normal;
		}
	case Class::Category::InputThread:
		{
			return new InputImpl::Thread;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
