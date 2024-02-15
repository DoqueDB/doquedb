// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Between.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Predicate/Between.h"
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
		"between",
		"not between"
	};
}
namespace BetweenImpl
{
	// CLASS local
	//	Execution::Predicate::Impl::BetweenImpl -- implementation class of Between
	//
	// NOTES
	class Base
		: public Predicate::Between
	{
	public:
		typedef Base This;
		typedef Predicate::Between Super;

		// constructor
		Base()
			: Super(),
			  m_cData0(),
			  m_cData1(),
			  m_cData2()
		{}
		Base(int iDataID0_, int iDataID1_, int iDataID2_)
			: Super(),
			  m_cData0(iDataID0_),
			  m_cData1(iDataID1_),
			  m_cData2(iDataID2_)
		{}

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Predicate::Between::

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
		const Common::Data* getData1() {return m_cData1.getData();}
		const Common::Data* getData2() {return m_cData2.getData();}

	private:
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// operands
		Action::DataHolder m_cData0;
		Action::DataHolder m_cData1;
		Action::DataHolder m_cData2;
	};

	// CLASS local
	//	Execution::Predicate::BetweenImpl::Between -- implementation class of Between
	//
	// NOTES
	class Between
		: public Base
	{
	public:
		typedef Between This;
		typedef Base Super;

		// constructor
		Between()
			: Super()
		{}
		Between(int iDataID0_, int iDataID1_, int iDataID2_)
			: Super(iDataID0_, iDataID1_, iDataID2_)
		{}

		// destructor
		virtual ~Between() {}

	///////////////////////////
	// Predicate::Between::

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
	// BetweenImpl::Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Predicate::BetweenImpl::NotBetween -- implementation class of Between
	//
	// NOTES
	class NotBetween
		: public Between
	{
	public:
		typedef NotBetween This;
		typedef Between Super;

		// constructor
		NotBetween()
			: Super()
		{}
		NotBetween(int iDataID0_, int iDataID1_, int iDataID2_)
			: Super(iDataID0_, iDataID1_, iDataID2_)
		{}

		// destructor
		virtual ~NotBetween() {}

	///////////////////////////
	// Predicate::Between::

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
	// BetweenImpl::Base::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_);
	};

	// CLASS local
	//	Execution::Predicate::BetweenImpl::BetweenAnyElement -- implementation class of Between
	//
	// NOTES
	class BetweenAnyElement
		: public ArrayCheck::AnyElement<Between>
	{
	public:
		typedef BetweenAnyElement This;
		typedef ArrayCheck::AnyElement<Between> Super;

		// constructor
		BetweenAnyElement()
			: Super()
		{}
		BetweenAnyElement(int iDataID0_, int iDataID1_, int iDataID2_)
			: Super(iDataID0_, iDataID1_, iDataID2_)
		{
			setArray(iDataID0_);
		}

		// destructor
		~BetweenAnyElement() {}

	///////////////////////////
	// Predicate::Between::

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
	//	Execution::Predicate::BetweenImpl::NotBetweenAnyElement -- implementation class of NotBetween
	//
	// NOTES
	class NotBetweenAnyElement
		: public ArrayCheck::AnyElement<NotBetween>
	{
	public:
		typedef NotBetweenAnyElement This;
		typedef ArrayCheck::AnyElement<NotBetween> Super;

		// constructor
		NotBetweenAnyElement()
			: Super()
		{}
		NotBetweenAnyElement(int iDataID0_, int iDataID1_, int iDataID2_)
			: Super(iDataID0_, iDataID1_, iDataID2_)
		{
			setArray(iDataID0_);
		}

		// destructor
		~NotBetweenAnyElement() {}

	///////////////////////////
	// Predicate::Between::

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
} // namespace BetweenImpl

namespace
{
	const Between::Boolean::Value _CompareTable[][3] =
	{
#define F Between::Boolean::False
#define T Between::Boolean::True
#define U Between::Boolean::Unknown
	  //-1  0  1
		{F, F, F},	// compare == -1
		{T, T, F},	// compare == 0
		{T, T, F},	// compare == 1
#undef F
#undef T
#undef U
	};

} // namespace $$$

///////////////////////////////////////
// Predicate::BetweenImpl::Base

// FUNCTION public
//	Predicate::BetweenImpl::Base::explain -- 
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
BetweenImpl::Base::
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
		m_cData1.explain(cProgram_, cExplain_);
		cExplain_.put(" and ");
		m_cData2.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::BetweenImpl::Base::initialize -- 
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
BetweenImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData0.initialize(cProgram_);
		m_cData1.initialize(cProgram_);
		m_cData2.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::BetweenImpl::Base::terminate -- 
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
BetweenImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized()) {
		m_cData0.terminate(cProgram_);
		m_cData1.terminate(cProgram_);
		m_cData2.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::BetweenImpl::Base::finish -- 
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
BetweenImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::BetweenImpl::Base::reset -- 
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
BetweenImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::BetweenImpl::Base::serialize -- for serialize
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
BetweenImpl::Base::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cData0.serialize(cArchive_);
	m_cData1.serialize(cArchive_);
	m_cData2.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::BetweenImpl::Base::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Between::Boolean::Value
//
// EXCEPTIONS

//virtual
Between::Boolean::Value
BetweenImpl::Base::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return checkByData(cProgram_,
					   getData0());
}

///////////////////////////////////////
// Predicate::BetweenImpl::Between

// FUNCTION public
//	Predicate::BetweenImpl::Between::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Between::Boolean::Value
//
// EXCEPTIONS

//virtual
Between::Boolean::Value
BetweenImpl::Between::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	if (getData1()->isNull() || getData2()->isNull() || pData_->isNull()) {
		return Boolean::Unknown;
	}

	int iResult0 = pData_->compareTo(getData1());
	; _SYDNEY_ASSERT(iResult0 >= -1 && iResult0 <= 1);
	int iResult1 = pData_->compareTo(getData2());
	; _SYDNEY_ASSERT(iResult1 >= -1 && iResult1 <= 1);

	return _CompareTable[iResult0+1][iResult1+1];
}

// FUNCTION public
//	Predicate::BetweenImpl::Between::getClassID -- 
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
BetweenImpl::Between::
getClassID() const
{
	return Class::getClassID(Class::Category::Between);
}

// FUNCTION private
//	Predicate::BetweenImpl::Between::explainOperator -- 
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
BetweenImpl::Between::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[0]);
}

/////////////////////////////////////////////////
// Predicate::BetweenImpl::NotBetween

// FUNCTION public
//	Predicate::BetweenImpl::NotBetween::checkByData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Between::Boolean::Value
//
// EXCEPTIONS

//virtual
Between::Boolean::Value
BetweenImpl::NotBetween::
checkByData(Interface::IProgram& cProgram_,
			const Common::Data* pData_)
{
	return Boolean::boolNot(Super::checkByData(cProgram_, pData_));
}

// FUNCTION private
//	Predicate::BetweenImpl::NotBetween::explainOperator -- 
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
BetweenImpl::NotBetween::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName[1]);
}

// FUNCTION public
//	Predicate::BetweenImpl::NotBetween::getClassID -- 
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
BetweenImpl::NotBetween::
getClassID() const
{
	return Class::getClassID(Class::Category::NotBetween);
}

/////////////////////////////////////////////////
// Predicate::BetweenImpl::BetweenAnyElement

// FUNCTION public
//	Predicate::BetweenImpl::BetweenAnyElement::getClassID -- 
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
BetweenImpl::BetweenAnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::BetweenAnyElement);
}

// FUNCTION private
//	Predicate::BetweenImpl::BetweenAnyElement::isUnknown -- 
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
BetweenImpl::BetweenAnyElement::
isUnknown()
{
	return getData0()->isNull() || getArray()->getCount() == 0;
}

/////////////////////////////////////////////////
// Predicate::BetweenImpl::NotBetweenAnyElement

// FUNCTION public
//	Predicate::BetweenImpl::NotBetweenAnyElement::getClassID -- 
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
BetweenImpl::NotBetweenAnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::NotBetweenAnyElement);
}

// FUNCTION private
//	Predicate::BetweenImpl::NotBetweenAnyElement::isUnknown -- 
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
BetweenImpl::NotBetweenAnyElement::
isUnknown()
{
	return getData0()->isNull() || getArray()->getCount() == 0;
}

///////////////////////////////////////////
// Predicate::Between::AnyElement::

// FUNCTION public
//	Predicate::Between::AnyElement::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	int iDataID2_
//	bool bIsNot_
//	
// RETURN
//	Between*
//
// EXCEPTIONS

//static
Between*
Between::AnyElement::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   int iDataID2_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	if (bIsNot_) {
		pResult = new BetweenImpl::NotBetweenAnyElement(iDataID0_, iDataID1_, iDataID2_);
	} else {
		pResult = new BetweenImpl::BetweenAnyElement(iDataID0_, iDataID1_, iDataID2_);
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Predicate::Between::

// FUNCTION public
//	Predicate::Between::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	int iDataID2_
//	bool bIsNot_
//	
// RETURN
//	Between*
//
// EXCEPTIONS

//static
Between*
Between::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   int iDataID2_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	if (bIsNot_) {
		pResult = new BetweenImpl::NotBetween(iDataID0_, iDataID1_, iDataID2_);
	} else {
		pResult = new BetweenImpl::Between(iDataID0_, iDataID1_, iDataID2_);
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Between::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Between*
//
// EXCEPTIONS

//static
Between*
Between::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Between:
		{
			return new BetweenImpl::Between;
		}
	case Class::Category::NotBetween:
		{
			return new BetweenImpl::NotBetween;
		}
	case Class::Category::BetweenAnyElement:
		{
			return new BetweenImpl::BetweenAnyElement;
		}
	case Class::Category::NotBetweenAnyElement:
		{
			return new BetweenImpl::NotBetweenAnyElement;
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
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
