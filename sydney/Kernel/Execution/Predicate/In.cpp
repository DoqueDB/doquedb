// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/In.cpp --
// 
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/In.h"
#include "Execution/Predicate/ArrayCheck.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszExplainName = "check ";
	const char* const _pszOperatorName[] =
	{
		"in",
		"not in"
	};
}
namespace InImpl
{
	// CLASS local
	//	Execution::Predicate::Impl::InImpl -- implementation class of In
	//
	// NOTES
	class Base
		: public Predicate::In
	{
	public:
		typedef Base This;
		typedef Predicate::In Super;

		// constructor
		Base()
			: Super(),
			  m_cData0(),
			  m_cArray1()
		{}
		Base(int iDataID0_, int iDataID1_)
			: Super(),
			  m_cData0(iDataID0_),
			  m_cArray1(iDataID1_)
		{}

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Predicate::In::

	/////////////////////////////
	// Interface::IPredicate::

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

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
		const Common::Data* getData0() {return m_cData0.getData();}
		const Common::DataArrayData* getArray1() {return m_cArray1.getData();}

	private:
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// operands
		Action::DataHolder m_cData0;
		Action::ArrayDataHolder m_cArray1;
	};

	// CLASS local
	//	Execution::Predicate::InImpl::In -- implementation class of In
	//
	// NOTES
	class In
		: public Base
	{
	public:
		typedef In This;
		typedef Base Super;

		// constructor
		In()
			: Super()
		{}
		In(int iDataID0_, int iDataID1_)
			: Super(iDataID0_, iDataID1_)
		{}

		// destructor
		virtual ~In() {}

	///////////////////////////
	// Predicate::In::

	/////////////////////////////
	// Interface::IPredicate::
		virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
										   const Common::Data* pData_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////
	// InImpl::Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Predicate::InImpl::NotIn -- implementation class of In
	//
	// NOTES
	class NotIn
		: public In
	{
	public:
		typedef NotIn This;
		typedef In Super;

		// constructor
		NotIn()
			: Super()
		{}
		NotIn(int iDataID0_, int iDataID1_)
			: Super(iDataID0_, iDataID1_)
		{}

		// destructor
		virtual ~NotIn() {}

	///////////////////////////
	// Predicate::In::

	/////////////////////////////
	// Interface::IPredicate::
		virtual Boolean::Value checkByData(Interface::IProgram& cProgram_,
										   const Common::Data* pData_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////
	// InImpl::Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Predicate::InImpl::InAnyElement -- implementation class of In
	//
	// NOTES
	class InAnyElement
		: public ArrayCheck::AnyElement<In>
	{
	public:
		typedef InAnyElement This;
		typedef ArrayCheck::AnyElement<In> Super;

		// constructor
		InAnyElement()
			: Super()
		{}
		InAnyElement(int iDataID0_, int iDataID1_)
			: Super(iDataID0_, iDataID1_)
		{
			setArray(iDataID0_);
		}

		// destructor
		~InAnyElement() {}

	///////////////////////////
	// Predicate::In::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	////////////////////////////
	// ArrayCheck::Base::
		virtual bool isUnknown();
	};

	// CLASS local
	//	Execution::Predicate::InImpl::NotInAnyElement -- implementation class of NotIn
	//
	// NOTES
	class NotInAnyElement
		: public ArrayCheck::AnyElement<NotIn>
	{
	public:
		typedef NotInAnyElement This;
		typedef ArrayCheck::AnyElement<NotIn> Super;

		// constructor
		NotInAnyElement()
			: Super()
		{}
		NotInAnyElement(int iDataID0_, int iDataID1_)
			: Super(iDataID0_, iDataID1_)
		{
			setArray(iDataID0_);
		}

		// destructor
		~NotInAnyElement() {}

	///////////////////////////
	// Predicate::In::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	////////////////////////////
	// ArrayCheck::Base::
		virtual bool isUnknown();
	};
} // namespace InImpl

///////////////////////////////////////
// Predicate::InImpl::Base

// FUNCTION public
//	Predicate::InImpl::Base::explain -- 
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
InImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cData0.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	explainOperator(cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cArray1.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::InImpl::Base::initialize -- 
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
InImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData0.initialize(cProgram_);
		m_cArray1.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::InImpl::Base::terminate -- 
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
InImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized()) {
		m_cData0.terminate(cProgram_);
		m_cArray1.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::InImpl::Base::finish -- 
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
InImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::InImpl::Base::reset -- 
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
InImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::InImpl::Base::serialize -- for serialize
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
InImpl::Base::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cData0.serialize(cArchive_);
	m_cArray1.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::InImpl::Base::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	In::Boolean::Value
//
// EXCEPTIONS

//virtual
In::Boolean::Value
InImpl::Base::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return checkByData(cProgram_,
					   getData0());
}

///////////////////////////////////////
// Predicate::InImpl::In

// FUNCTION public
//	Predicate::InImpl::In::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	In::Boolean::Value
//
// EXCEPTIONS

//virtual
In::Boolean::Value
InImpl::In::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	if (getArray1()->isNull() || pData_->isNull()) {
		return Boolean::Unknown;
	}

	Boolean::Value result = Boolean::Unknown;
	int n = getArray1()->getCount();
	for (int i = 0; i < n; ++i) {
		if (getArray1()->getElement(i)->compareTo(pData_) == 0) {
			result = Boolean::True;
			break;
		}
	}
	return result;
}

// FUNCTION public
//	Predicate::InImpl::In::getClassID -- 
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
InImpl::In::
getClassID() const
{
	return Class::getClassID(Class::Category::In);
}

// FUNCTION private
//	Predicate::InImpl::In::explainOperator -- 
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
InImpl::In::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[0]);
}

/////////////////////////////////////////////////
// Predicate::InImpl::NotIn

// FUNCTION public
//	Predicate::InImpl::NotIn::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	In::Boolean::Value
//
// EXCEPTIONS

//virtual
In::Boolean::Value
InImpl::NotIn::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	return Boolean::boolNot(Super::checkByData(cProgram_, pData_));
}

// FUNCTION private
//	Predicate::InImpl::NotIn::explainOperator -- 
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
InImpl::NotIn::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[1]);
}

// FUNCTION public
//	Predicate::InImpl::NotIn::getClassID -- 
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
InImpl::NotIn::
getClassID() const
{
	return Class::getClassID(Class::Category::NotIn);
}

/////////////////////////////////////////////////
// Predicate::InImpl::InAnyElement

// FUNCTION public
//	Predicate::InImpl::InAnyElement::getClassID -- 
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
InImpl::InAnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::InAnyElement);
}

// FUNCTION private
//	Predicate::InImpl::InAnyElement::isUnknown -- 
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

bool
InImpl::InAnyElement::
isUnknown()
{
	return getData0()->isNull() || getArray()->getCount() == 0;
}

/////////////////////////////////////////////////
// Predicate::InImpl::NotInAnyElement

// FUNCTION public
//	Predicate::InImpl::NotInAnyElement::getClassID -- 
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
InImpl::NotInAnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::NotInAnyElement);
}

// FUNCTION private
//	Predicate::InImpl::NotInAnyElement::isUnknown -- 
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

bool
InImpl::NotInAnyElement::
isUnknown()
{
	return getData0()->isNull() || getArray()->getCount() == 0;
}

///////////////////////////////////////////
// Predicate::In::AnyElement::

// FUNCTION public
//	Predicate::In::AnyElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	bool bIsNot_
//	
// RETURN
//	In*
//
// EXCEPTIONS

//static
In*
In::AnyElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	if (bIsNot_) {
		pResult = new InImpl::NotInAnyElement(iDataID0_, iDataID1_);
	} else {
		pResult = new InImpl::InAnyElement(iDataID0_, iDataID1_);
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Predicate::In::

// FUNCTION public
//	Predicate::In::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	bool bIsNot_
//	
// RETURN
//	In*
//
// EXCEPTIONS

//static
In*
In::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	if (bIsNot_) {
		pResult = new InImpl::NotIn(iDataID0_, iDataID1_);
	} else {
		pResult = new InImpl::In(iDataID0_, iDataID1_);
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::In::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	In*
//
// EXCEPTIONS

//static
In*
In::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::In:
		{
			return new InImpl::In;
		}
	case Class::Category::NotIn:
		{
			return new InImpl::NotIn;
		}
	case Class::Category::InAnyElement:
		{
			return new InImpl::InAnyElement;
		}
	case Class::Category::NotInAnyElement:
		{
			return new InImpl::NotInAnyElement;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2016, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
