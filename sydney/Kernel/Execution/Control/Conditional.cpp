// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Control/Conditional.cpp --
// 
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Control";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Control/Conditional.h"
#include "Execution/Control/Class.h"

#include "Execution/Action/ActionList.h"
#include "Execution/Action/PredicateHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IPredicate.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_CONTROL_BEGIN

namespace
{
	struct _Explain
	{
		enum Value
		{
			If,
			Unless,
			ValueNum
		};
	};
	const char* const _pszExplainName[] = {
		"if ",
		"unless ",
		0
	};
}
namespace ConditionalImpl
{
	// CLASS local
	//	Execution::Control::ConditionalImpl::Base -- base class of Conditional control implementation classes
	//
	// NOTES
	class Base
		: public Control::Conditional
	{
	public:
		typedef Base This;
		typedef Control::Conditional Super;

		// constructor
		Base()
			: Super(),
			  m_cPredicate()
		{}
		Base(int iPredicateID_)
			: Super(),
			  m_cPredicate(iPredicateID_)
		{}

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Control::Conditional::

	/////////////////////////////
	// Interface::IControl::

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

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:

	private:
		virtual const char* getOperatorName() = 0;
		virtual bool check(Interface::IProgram& cProgram_,
						   Interface::IPredicate::Boolean::Value eResult_) = 0;

		Action::PredicateHolder m_cPredicate;
	};

	// CLASS local
	//	Execution::Control::ConditionalImpl::If -- implementation class of Conditional control
	//
	// NOTES
	class If
		: public Base
	{
	public:
		typedef If This;
		typedef Base Super;

		// constructor
		If()
			: Super()
		{}
		If(int iPredicateID_)
			: Super(iPredicateID_)
		{}

		// destructor
		virtual ~If() {}

	///////////////////////////
	// Control::Conditional::

	/////////////////////////////
	// Interface::IControl::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	virtual void serialize(ModArchive& archiver_);

	protected:
	private:
		virtual const char* getOperatorName();
		virtual bool check(Interface::IProgram& cProgram_,
						   Interface::IPredicate::Boolean::Value eResult_);
	};

	// CLASS local
	//	Execution::Control::ConditionalImpl::Unless -- implementation class of Conditional control
	//
	// NOTES
	class Unless
		: public Base
	{
	public:
		typedef Unless This;
		typedef Base Super;

		// constructor
		Unless()
			: Super()
		{}
		Unless(int iPredicateID_)
			: Super(iPredicateID_)
		{}

		// destructor
		virtual ~Unless() {}

	///////////////////////////
	// Control::Conditional::

	/////////////////////////////
	// Interface::IControl::

	/////////////////////////////
	// Interface::IAction::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Interface::IProgram& cProgram_,
	//						 Opt::Explain& cExplain_);
	//	virtual void initialize(Interface::IProgram& cProgram_);
	//	virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
	//	virtual void finish(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
	//	virtual void serialize(ModArchive& archiver_);

	protected:
	private:
		virtual const char* getOperatorName();
		virtual bool check(Interface::IProgram& cProgram_,
						   Interface::IPredicate::Boolean::Value eResult_);
	};

} // namespace ConditionalImpl

///////////////////////////////////////
// Control::ConditionalImpl::Base

// FUNCTION public
//	Control::ConditionalImpl::Base::explain -- 
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
ConditionalImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put("do until line ").put(getElse());
	cExplain_.popNoNewLine();
	cExplain_.pushIndent();
	cExplain_.put(" ").put(getOperatorName());
	m_cPredicate.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent();
}

// FUNCTION public
//	Control::ConditionalImpl::Base::initialize -- 
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
ConditionalImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cPredicate.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cPredicate.initialize(cProgram_);
	}
}

// FUNCTION public
//	Control::ConditionalImpl::Base::terminate -- 
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
ConditionalImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cPredicate.isInitialized()) {
		m_cPredicate.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Control::ConditionalImpl::Base::execute -- 
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
ConditionalImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	m_cPredicate->undone(cProgram_);
	Action::Status::Value eResult = m_cPredicate->execute(cProgram_, cActionList_);
	switch (eResult) {
	case Action::Status::Success:
	case Action::Status::False:
		{
			if (check(cProgram_, m_cPredicate->check(cProgram_)) == false) {
				cActionList_.setPointer(getElse());
			}
			break;
		}
	default:
		{
			break;
		}
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Control::ConditionalImpl::Base::finish -- 
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
ConditionalImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	m_cPredicate->finish(cProgram_);
}

// FUNCTION public
//	Control::ConditionalImpl::Base::reset -- 
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
ConditionalImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	m_cPredicate->reset(cProgram_);
}

// FUNCTION public
//	Control::ConditionalImpl::Base::serialize -- for serialize
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
ConditionalImpl::Base::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	serializeBase(cArchive_);
	m_cPredicate.serialize(cArchive_);
}

//////////////////////////////
// Control::ConditionalImpl::If

// FUNCTION public
//	Control::ConditionalImpl::If::getClassID -- 
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
ConditionalImpl::If::
getClassID() const
{
	return Class::getClassID(Class::Category::If);
}

// FUNCTION private
//	Control::ConditionalImpl::If::getOperatorName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
ConditionalImpl::If::
getOperatorName()
{
	return _pszExplainName[_Explain::If];
}

// FUNCTION private
//	Control::ConditionalImpl::If::check -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IPredicate::Boolean::Value eResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ConditionalImpl::If::
check(Interface::IProgram& cProgram_,
	  Interface::IPredicate::Boolean::Value eResult_)
{
	return eResult_ == Interface::IPredicate::Boolean::True;
}

//////////////////////////////
// Control::ConditionalImpl::Unless

// FUNCTION public
//	Control::ConditionalImpl::Unless::getClassID -- 
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
ConditionalImpl::Unless::
getClassID() const
{
	return Class::getClassID(Class::Category::Unless);
}

// FUNCTION private
//	Control::ConditionalImpl::Unless::getOperatorName -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const char*
//
// EXCEPTIONS

//virtual
const char*
ConditionalImpl::Unless::
getOperatorName()
{
	return _pszExplainName[_Explain::Unless];
}

// FUNCTION private
//	Control::ConditionalImpl::Unless::check -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IPredicate::Boolean::Value eResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
ConditionalImpl::Unless::
check(Interface::IProgram& cProgram_,
	  Interface::IPredicate::Boolean::Value eResult_)
{
	return eResult_ != Interface::IPredicate::Boolean::True;
}

//////////////////////////////
// Control::Conditional::If

// FUNCTION public
//	Control::Conditional::If::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iPredicateID_
//	
// RETURN
//	Conditional*
//
// EXCEPTIONS

//static
Conditional*
Conditional::If::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iPredicateID_)
{
	AUTOPOINTER<This> pResult = new ConditionalImpl::If(iPredicateID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////////
// Control::Conditional::Unless

// FUNCTION public
//	Control::Conditional::Unless::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iPredicateID_
//	
// RETURN
//	Conditional*
//
// EXCEPTIONS

//static
Conditional*
Conditional::Unless::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iPredicateID_)
{
	AUTOPOINTER<This> pResult = new ConditionalImpl::Unless(iPredicateID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Control::Conditional::

// FUNCTION public
//	Control::Conditional::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Conditional*
//
// EXCEPTIONS

//static
Conditional*
Conditional::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::If:
		{
			return new ConditionalImpl::If;
		}
	case Class::Category::Unless:
		{
			return new ConditionalImpl::Unless;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_CONTROL_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
