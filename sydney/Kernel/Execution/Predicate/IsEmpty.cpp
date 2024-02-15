// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/IsEmpty.cpp --
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
const char moduleName[] = "Execution::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/IsEmpty.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Action/IteratorHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

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
	const char* const _pszExplainName = "is empty";
}
namespace IsEmptyImpl
{
	// CLASS local
	//	Execution::Predicate::IsEmptyImpl::BitSet -- implementation class of IsEmpty
	//
	// NOTES
	class BitSet
		: public IsEmpty
	{
	public:
		typedef BitSet This;
		typedef IsEmpty Super;

		// constructor
		BitSet()
			: Super(),
			  m_cBitSet()
		{}
		BitSet(int iDataID_)
			: Super(),
			  m_cBitSet(iDataID_)
		{}

		// destructor
		~BitSet() {}

	///////////////////////////
	// Predicate::IsEmpty::

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
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////////
	// Predicate::Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		Action::BitSetHolder m_cBitSet;
	};

	// CLASS local
	//	Execution::Predicate::IsEmptyImpl::Iterator -- implementation class of IsEmpty
	//
	// NOTES
	class Iterator
		: public IsEmpty
	{
	public:
		typedef Iterator This;
		typedef IsEmpty Super;

		// constructor
		Iterator()
			: Super(),
			  m_cIterator()
		{}
		Iterator(int iDataID_)
			: Super(),
			  m_cIterator(iDataID_)
		{}

		// destructor
		~Iterator() {}

	///////////////////////////
	// Predicate::IsEmpty::

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
		virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////////
	// Predicate::Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		Action::IteratorHolder m_cIterator;
	};
} // namespace IsEmptyImpl

///////////////////////////////////////
// Predicate::IsEmptyImpl::BitSet

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::explain -- 
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
IsEmptyImpl::BitSet::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cBitSet.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszExplainName);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::initialize -- 
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
IsEmptyImpl::BitSet::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cBitSet.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cBitSet.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::terminate -- 
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
IsEmptyImpl::BitSet::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cBitSet.isInitialized() == true) {
		m_cBitSet.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::finish -- 
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
IsEmptyImpl::BitSet::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::reset -- 
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
IsEmptyImpl::BitSet::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::getClassID -- 
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
IsEmptyImpl::BitSet::
getClassID() const
{
	return Class::getClassID(Class::Category::IsEmptyBitSet);
}

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::serialize -- for serialize
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
IsEmptyImpl::BitSet::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cBitSet.serialize(cArchive_);
}

// FUNCTION public
//	Predicate::IsEmptyImpl::BitSet::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	IsEmpty::Boolean::Value
//
// EXCEPTIONS

//virtual
IsEmpty::Boolean::Value
IsEmptyImpl::BitSet::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return m_cBitSet->none()
		? Boolean::True
		: Boolean::False;
}

///////////////////////////////////////
// Predicate::IsEmptyImpl::Iterator

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::explain -- 
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
IsEmptyImpl::Iterator::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName);
	cExplain_.pushIndent();
	m_cIterator.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent();
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::initialize -- 
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
IsEmptyImpl::Iterator::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cIterator.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cIterator.initialize(cProgram_);
		cProgram_.initialize(m_cIterator.getIterator());
	}
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::terminate -- 
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
IsEmptyImpl::Iterator::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cIterator.isInitialized() == true) {
		m_cIterator.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::finish -- 
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
IsEmptyImpl::Iterator::
finish(Interface::IProgram& cProgram_)
{
	cProgram_.finish(m_cIterator.getIterator());
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::reset -- 
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
IsEmptyImpl::Iterator::
reset(Interface::IProgram& cProgram_)
{
	cProgram_.reset(m_cIterator.getIterator());
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::undone -- 
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
IsEmptyImpl::Iterator::
undone(Interface::IProgram& cProgram_)
{
	Super::undone(cProgram_);
	cProgram_.reset(m_cIterator.getIterator());
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::getClassID -- 
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
IsEmptyImpl::Iterator::
getClassID() const
{
	return Class::getClassID(Class::Category::IsEmptyIterator);
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::serialize -- for serialize
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
IsEmptyImpl::Iterator::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cIterator.serialize(cArchive_);
}

// FUNCTION public
//	Predicate::IsEmptyImpl::Iterator::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	IsEmpty::Boolean::Value
//
// EXCEPTIONS

//virtual
IsEmpty::Boolean::Value
IsEmptyImpl::Iterator::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return cProgram_.next(m_cIterator.getIterator())
		? Boolean::False
		: Boolean::True;
}

///////////////////////////////////
// Predicate::IsEmpty::BitSet::

// FUNCTION public
//	Predicate::IsEmpty::BitSet::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	IsEmpty*
//
// EXCEPTIONS

//static
IsEmpty*
IsEmpty::BitSet::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new IsEmptyImpl::BitSet(iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

///////////////////////////////////
// Predicate::IsEmpty::Iterator::

// FUNCTION public
//	Predicate::IsEmpty::Iterator::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIteratorID_
//	
// RETURN
//	IsEmpty*
//
// EXCEPTIONS

//static
IsEmpty*
IsEmpty::Iterator::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIteratorID_)
{
	AUTOPOINTER<This> pResult = new IsEmptyImpl::Iterator(iIteratorID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Predicate::IsEmpty::

// FUNCTION public
//	Predicate::IsEmpty::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	IsEmpty*
//
// EXCEPTIONS

//static
IsEmpty*
IsEmpty::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::IsEmptyBitSet:
		{
			return new IsEmptyImpl::BitSet;
		}
	case Class::Category::IsEmptyIterator:
		{
			return new IsEmptyImpl::Iterator;
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
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
