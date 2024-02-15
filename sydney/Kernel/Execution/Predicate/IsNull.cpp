// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/IsNull.cpp --
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/IsNull.h"
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
	const char* const _pszExplainName = "is null";
}
namespace IsNullImpl
{
	// CLASS local
	//	Execution::Predicate::IsNullImpl::Nadic -- implementation class of IsNull
	//
	// NOTES
	class Nadic
		: public IsNull
	{
	public:
		typedef Nadic This;
		typedef IsNull Super;

		// constructor
		Nadic()
			: Super(),
			  m_vecData()
		{}
		Nadic(const VECTOR<int>& vecDataID_);

		// destructor
		virtual ~Nadic() {}

	///////////////////////////
	// Predicate::IsNull::

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
	//	int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
		// accessor
		VECTOR<Action::DataHolder>& getData() {return m_vecData;}

	private:
	///////////////////////////////
	// Predicate::Base::
	//	virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
	//									Action::ActionList& cActionList_);

		VECTOR<Action::DataHolder> m_vecData;
	};

	// CLASS local
	//	Execution::Predicate::IsNullImpl::All -- implementation class of IsNull
	//
	// NOTES
	class All
		: public Nadic
	{
	public:
		typedef All This;
		typedef Nadic Super;

		// constructor
		All()
			: Super()
		{}
		All(const VECTOR<int>& vecDataID_)
			: Super(vecDataID_)
		{}

		// destructor
		~All() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// Predicate::Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);
	};

	// CLASS local
	//	Execution::Predicate::IsNullImpl::Any -- implementation class of IsNull
	//
	// NOTES
	class Any
		: public Nadic
	{
	public:
		typedef Any This;
		typedef Nadic Super;

		// constructor
		Any()
			: Super()
		{}
		Any(const VECTOR<int>& vecDataID_)
			: Super(vecDataID_)
		{}

		// destructor
		~Any() {}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	protected:
	private:
	///////////////////////////////
	// Predicate::Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);
	};

	// CLASS local
	//	Execution::Predicate::IsNullImpl::Monadic -- implementation class of IsNull
	//
	// NOTES
	class Monadic
		: public IsNull
	{
	public:
		typedef Monadic This;
		typedef IsNull Super;

		// constructor
		Monadic()
			: Super(),
			  m_cData()
		{}
		Monadic(int iDataID_)
			: Super(),
			  m_cData(iDataID_)
		{}

		// destructor
		virtual ~Monadic() {}

	///////////////////////////
	// Predicate::IsNull::

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

		Action::DataHolder m_cData;
	};
} // namespace IsNullImpl

///////////////////////////////////////
// Predicate::IsNullImpl::Nadic

// FUNCTION public
//	Predicate::IsNullImpl::Nadic::Nadic -- 
//
// NOTES
//
// ARGUMENTS
//	const VECTOR<int>& vecDataID_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

IsNullImpl::Nadic::
Nadic(const VECTOR<int>& vecDataID_)
	: Super(),
	  m_vecData()
{
	VECTOR<int>::CONSTITERATOR iterator = vecDataID_.begin();
	const VECTOR<int>::CONSTITERATOR last = vecDataID_.end();
	for (; iterator != last; ++iterator) {
		m_vecData.PUSHBACK(Action::DataHolder(*iterator));
	}
}

// FUNCTION public
//	Predicate::IsNullImpl::Nadic::explain -- 
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
IsNullImpl::Nadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushIndent();
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put("(");
		Opt::JoinRef(m_vecData,
					 boost::bind(&Action::DataHolderBase::explain,
								 _1,
								 boost::ref(cProgram_),
								 boost::ref(cExplain_)),
					 boost::bind(&Opt::Explain::putChar,
								 &cExplain_,
								 ','));
		cExplain_.put(")");
	}
	cExplain_.popIndent();
}

// FUNCTION public
//	Predicate::IsNullImpl::Nadic::initialize -- 
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
IsNullImpl::Nadic::
initialize(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_vecData.ISEMPTY() == false);
	if (m_vecData[0].isInitialized() == false) {
		initializeBase(cProgram_);
		Opt::ForEachRef(m_vecData,
						boost::bind(&Action::DataHolder::initialize,
									_1,
									boost::ref(cProgram_)));
	}
}

// FUNCTION public
//	Predicate::IsNullImpl::Nadic::terminate -- 
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
IsNullImpl::Nadic::
terminate(Interface::IProgram& cProgram_)
{
	; _SYDNEY_ASSERT(m_vecData.ISEMPTY() == false);
	if (m_vecData[0].isInitialized() == true) {
		Opt::ForEachRef(m_vecData,
						boost::bind(&Action::DataHolder::terminate,
									_1,
									boost::ref(cProgram_)));
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::IsNullImpl::Nadic::finish -- 
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
IsNullImpl::Nadic::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::IsNullImpl::Nadic::reset -- 
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
IsNullImpl::Nadic::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::IsNullImpl::Nadic::serialize -- for serialize
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
IsNullImpl::Nadic::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	Utility::SerializeObject(cArchive_,
							 m_vecData);
}

///////////////////////////////////////
// Predicate::IsNullImpl::All

// FUNCTION public
//	Predicate::IsNullImpl::All::getClassID -- 
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
IsNullImpl::All::
getClassID() const
{
	return Class::getClassID(Class::Category::IsNullAll);
}

// FUNCTION private
//	Predicate::IsNullImpl::All::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	IsNull::Boolean::Value
//
// EXCEPTIONS

//virtual
IsNull::Boolean::Value
IsNullImpl::All::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return Opt::IsAllRef(getData(),
						 boost::bind(&Common::Data::isNull,
									 boost::bind(&Action::DataHolder::getData,
												 _1)))
		? Boolean::True
		: Boolean::False;
}

///////////////////////////////////////
// Predicate::IsNullImpl::Any

// FUNCTION public
//	Predicate::IsNullImpl::Any::getClassID -- 
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
IsNullImpl::Any::
getClassID() const
{
	return Class::getClassID(Class::Category::IsNullAny);
}

// FUNCTION private
//	Predicate::IsNullImpl::Any::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	IsNull::Boolean::Value
//
// EXCEPTIONS

//virtual
IsNull::Boolean::Value
IsNullImpl::Any::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return Opt::IsAnyRef(getData(),
						 boost::bind(&Common::Data::isNull,
									 boost::bind(&Action::DataHolder::getData,
												 _1)))
		? Boolean::True
		: Boolean::False;
}

///////////////////////////////////////
// Predicate::IsNullImpl::Monadic

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::explain -- 
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
IsNullImpl::Monadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszExplainName);
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::initialize -- 
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
IsNullImpl::Monadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::terminate -- 
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
IsNullImpl::Monadic::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == true) {
		m_cData.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::finish -- 
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
IsNullImpl::Monadic::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::reset -- 
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
IsNullImpl::Monadic::
reset(Interface::IProgram& cProgram_)
{
}

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::getClassID -- 
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
IsNullImpl::Monadic::
getClassID() const
{
	return Class::getClassID(Class::Category::IsNull);
}

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::serialize -- for serialize
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
IsNullImpl::Monadic::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cData.serialize(cArchive_);
}

// FUNCTION public
//	Predicate::IsNullImpl::Monadic::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	IsNull::Boolean::Value
//
// EXCEPTIONS

//virtual
IsNull::Boolean::Value
IsNullImpl::Monadic::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return m_cData->isNull()
		? Boolean::True
		: Boolean::False;
}

//////////////////////////////
// Predicate::IsNull::All::

// FUNCTION public
//	Predicate::IsNull::All::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecDataID_
//	
// RETURN
//	IsNull*
//
// EXCEPTIONS

//static
IsNull*
IsNull::All::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecDataID_)
{
	if (vecDataID_.GETSIZE() == 1) {
		return IsNull::create(cProgram_, vecDataID_[0]);
	}

	AUTOPOINTER<This> pResult = new IsNullImpl::All(vecDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Predicate::IsNull::Any::

// FUNCTION public
//	Predicate::IsNull::Any::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecDataID_
//	
// RETURN
//	IsNull*
//
// EXCEPTIONS

//static
IsNull*
IsNull::Any::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecDataID_)
{
	if (vecDataID_.GETSIZE() == 1) {
		return IsNull::create(cProgram_, vecDataID_[0]);
	}

	AUTOPOINTER<This> pResult = new IsNullImpl::Any(vecDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Predicate::IsNull::

// FUNCTION public
//	Predicate::IsNull::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	IsNull*
//
// EXCEPTIONS

//static
IsNull*
IsNull::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new IsNullImpl::Monadic(iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::IsNull::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	IsNull*
//
// EXCEPTIONS

//static
IsNull*
IsNull::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::IsNullAll:
		{
			return new IsNullImpl::All;
		}
	case Class::Category::IsNullAny:
		{
			return new IsNullImpl::Any;
		}
	case Class::Category::IsNull:
		{
			return new IsNullImpl::Monadic;
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
