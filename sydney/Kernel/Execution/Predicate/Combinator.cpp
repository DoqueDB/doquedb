// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Combinator.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Predicate/Combinator.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/ActionHolder.h"
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
	struct _Type
	{
		enum Value {
			And = 0,
			Or,
			Not,
			ValueNum
		};
	};
	const char* const _pszExplainName[] = {
		"and ",
		"or ",
		"not ",
	};
}

namespace CombinatorImpl
{
	// CLASS local
	//	Execution::Predicate::CombinatorImpl::Nadic -- base class of implementation class of Combinator
	//
	// NOTES
	class Nadic
		: public Predicate::Combinator
	{
	public:
		typedef Nadic This;
		typedef Predicate::Combinator Super;

		// destructor
		virtual ~Nadic() {}

	///////////////////////////
	// Predicate::Combinator::

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

	protected:
		// constructor
		Nadic()
			: Super(),
			  m_vecAction()
		{}
		Nadic(const VECTOR<int>& vecID_)
			: Super(),
			  m_vecAction()
		{
			m_vecAction.reserve(vecID_.GETSIZE());
			VECTOR<int>::CONSTITERATOR iterator = vecID_.begin();
			const VECTOR<int>::CONSTITERATOR last = vecID_.end();
			for (; iterator != last; ++iterator) {
				m_vecAction.PUSHBACK(Action::ActionHolder(*iterator));
			}
		}

		// for serialize
		void serializeBase(ModArchive& cArchive_);

		// accessor
		VECTOR<Action::ActionHolder>& getAction() {return m_vecAction;}

	private:
		// explain operator
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

	///////////////////////////
	// Base::
	//	virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
	//									Action::ActionList& cActionList_) = 0;

		// operands
		VECTOR<Action::ActionHolder> m_vecAction;
	};

	// CLASS local
	//	Execution::Predicate::CombinatorImpl::Dyadic -- base class of implementation class of Combinator
	//
	// NOTES
	class Dyadic
		: public Predicate::Combinator
	{
	public:
		typedef Dyadic This;
		typedef Predicate::Combinator Super;

		// destructor
		virtual ~Dyadic() {}

	///////////////////////////
	// Predicate::Combinator::

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

	protected:
		// constructor
		Dyadic()
			: Super(),
			  m_cAction()
		{}
		Dyadic(const PAIR<int, int>& cID_)
			: Super(),
			  m_cAction()
		{
			m_cAction.first = Action::ActionHolder(cID_.first);
			m_cAction.second = Action::ActionHolder(cID_.second);
		}

		// for serialize
		void serializeBase(ModArchive& cArchive_);

		// accessor
		PAIR<Action::ActionHolder, Action::ActionHolder>& getAction() {return m_cAction;}

	private:
		// explain operator
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

	///////////////////////////
	// Base::
	//	virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
	//									Action::ActionList& cActionList_) = 0;

		// operands
		PAIR<Action::ActionHolder, Action::ActionHolder> m_cAction;
	};

	// CLASS local
	//	Execution::Predicate::CombinatorImpl::Monadic -- base class of implementation class of Combinator
	//
	// NOTES
	class Monadic
		: public Predicate::Combinator
	{
	public:
		typedef Monadic This;
		typedef Predicate::Combinator Super;

		// destructor
		virtual ~Monadic() {}

	///////////////////////////
	// Predicate::Combinator::

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

	protected:
		// constructor
		Monadic()
			: Super(),
			  m_cAction()
		{}
		Monadic(int iID_)
			: Super(),
			  m_cAction(iID_)
		{}

		// for serialize
		void serializeBase(ModArchive& cArchive_);

		// accessor
		Action::ActionHolder& getAction() {return m_cAction;}

	private:
		// explain operator
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) = 0;

	///////////////////////////
	// Base::
	//	virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
	//									Action::ActionList& cActionList_) = 0;

		// operands
		Action::ActionHolder m_cAction;
	};

	// CLASS local
	//	Execution::Predicate::CombinatorImpl::And -- implementation class of Combinator (and)
	//
	// NOTES
	class And
		: public Nadic
	{
	public:
		typedef And This;
		typedef Nadic Super;

		And() : Super() {}
		And(const VECTOR<int>& vecID_)
			: Super(vecID_)
		{}
		~And() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::Combinator::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_) {serializeBase(archiver_);}

	protected:
	private:
	/////////////////////////////
	// Nadic::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) ;

	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);
	};

	// CLASS local
	//	Execution::Predicate::CombinatorImpl::AndDyadic -- implementation class of Combinator (and)
	//
	// NOTES
	class AndDyadic
		: public Dyadic
	{
	public:
		typedef AndDyadic This;
		typedef Dyadic Super;

		AndDyadic() : Super() {}
		AndDyadic(const PAIR<int, int>& cID_)
			: Super(cID_)
		{}
		~AndDyadic() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::Combinator::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_) {serializeBase(archiver_);}

	protected:
	private:
	/////////////////////////////
	// Dyadic::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) ;

	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);
	};

	// CLASS local
	//	Execution::Predicate::CombinatorImpl::Or -- implementation class of Combinator (or)
	//
	// NOTES
	class Or
		: public Nadic
	{
	public:
		typedef Or This;
		typedef Nadic Super;

		Or() : Super() {}
		Or(const VECTOR<int>& vecID_)
			: Super(vecID_)
		{}
		~Or() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::Combinator::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_) {serializeBase(archiver_);}

	protected:
	private:
	/////////////////////////////
	// Nadic::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) ;

	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);
	};

	// CLASS local
	//	Execution::Predicate::CombinatorImpl::OrDyadic -- implementation class of Combinator (or)
	//
	// NOTES
	class OrDyadic
		: public Dyadic
	{
	public:
		typedef OrDyadic This;
		typedef Dyadic Super;

		OrDyadic() : Super() {}
		OrDyadic(const PAIR<int, int>& cID_)
			: Super(cID_)
		{}
		~OrDyadic() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::Combinator::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_) {serializeBase(archiver_);}

	protected:
	private:
	/////////////////////////////
	// Dyadic::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) ;

	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);
	};

	// CLASS local
	//	Execution::Predicate::CombinatorImpl::Not -- implementation class of Combinator (not)
	//
	// NOTES
	class Not
		: public Monadic
	{
	public:
		typedef Not This;
		typedef Monadic Super;

		Not() : Super() {}
		Not(int iID_)
			: Super(iID_)
		{}
		~Not() {}

	/////////////////////////////
	// Base::

	///////////////////////////
	// Predicate::Combinator::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_) {serializeBase(archiver_);}

	protected:
	private:
	/////////////////////////////
	// Nadic::
		virtual void explainOperator(Interface::IProgram& cProgram_,
									 Opt::Explain& cExplain_) ;

	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);
	};
}

///////////////////////////////////////
// Predicate::CombinatorImpl::Nadic

// FUNCTION public
//	Predicate::CombinatorImpl::Nadic::explain -- 
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
CombinatorImpl::Nadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	if (!m_vecAction.ISEMPTY()) {
		if (m_vecAction.GETSIZE() > 1) {
			cExplain_.put("(");
		}
		cExplain_.pushIndent();
		Opt::Join(m_vecAction,
				  boost::bind(&Action::ActionHolder::explain,
							  _1,
							  pEnvironment_,
							  boost::ref(cProgram_),
							  boost::ref(cExplain_)),
				  boost::bind(&This::explainOperator,
							  this,
							  boost::ref(cProgram_),
							  boost::ref(cExplain_)));
		if (m_vecAction.GETSIZE() > 1) {
			cExplain_.put(")");
		}
		cExplain_.popIndent(true /* force new line */);
	}
}

// FUNCTION public
//	Predicate::CombinatorImpl::Nadic::initialize -- 
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
CombinatorImpl::Nadic::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	Opt::ForEach(m_vecAction,
				 boost::bind(&Action::ActionHolder::initialize,
							 _1,
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Predicate::CombinatorImpl::Nadic::terminate -- 
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
CombinatorImpl::Nadic::
terminate(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecAction,
				 boost::bind(&Action::ActionHolder::terminate,
							 _1,
							 boost::ref(cProgram_)));
	terminateBase(cProgram_);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Nadic::finish -- 
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
CombinatorImpl::Nadic::
finish(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecAction,
				 boost::bind(&Interface::IAction::finish,
							 boost::bind(&Action::ActionHolder::getAction,
										 _1),
							 boost::ref(cProgram_)));
}

// FUNCTION public
//	Predicate::CombinatorImpl::Nadic::reset -- 
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
CombinatorImpl::Nadic::
reset(Interface::IProgram& cProgram_)
{
	Opt::ForEach(m_vecAction,
				 boost::bind(&Interface::IAction::reset,
							 boost::bind(&Action::ActionHolder::getAction,
										 _1),
							 boost::ref(cProgram_)));
}

// FUNCTION protected
//	Predicate::CombinatorImpl::Nadic::serializeBase -- for serialize
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

void
CombinatorImpl::Nadic::
serializeBase(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	Utility::SerializeObject(cArchive_, m_vecAction);
}

///////////////////////////////////////
// Predicate::CombinatorImpl::Dyadic

// FUNCTION public
//	Predicate::CombinatorImpl::Dyadic::explain -- 
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
CombinatorImpl::Dyadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("(");
	cExplain_.pushIndent();
	m_cAction.first.explain(pEnvironment_,
							cProgram_,
							cExplain_);
	explainOperator(cProgram_,
					cExplain_);
	m_cAction.second.explain(pEnvironment_,
							 cProgram_,
							 cExplain_);
	cExplain_.put(")");
	cExplain_.popIndent(true /* force new line */);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Dyadic::initialize -- 
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
CombinatorImpl::Dyadic::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cAction.first.initialize(cProgram_);
	m_cAction.second.initialize(cProgram_);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Dyadic::terminate -- 
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
CombinatorImpl::Dyadic::
terminate(Interface::IProgram& cProgram_)
{
	m_cAction.first.terminate(cProgram_);
	m_cAction.second.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Dyadic::finish -- 
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
CombinatorImpl::Dyadic::
finish(Interface::IProgram& cProgram_)
{
	m_cAction.first.getAction()->finish(cProgram_);
	m_cAction.second.getAction()->finish(cProgram_);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Dyadic::reset -- 
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
CombinatorImpl::Dyadic::
reset(Interface::IProgram& cProgram_)
{
	m_cAction.first.getAction()->reset(cProgram_);
	m_cAction.second.getAction()->reset(cProgram_);
}

// FUNCTION protected
//	Predicate::CombinatorImpl::Dyadic::serializeBase -- for serialize
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

void
CombinatorImpl::Dyadic::
serializeBase(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cAction.first.serialize(cArchive_);
	m_cAction.second.serialize(cArchive_);
}

///////////////////////////////////////
// Predicate::CombinatorImpl::Monadic

// FUNCTION public
//	Predicate::CombinatorImpl::Monadic::explain -- 
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
CombinatorImpl::Monadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushIndent();
	explainOperator(cProgram_, cExplain_);
	m_cAction.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent();
}

// FUNCTION public
//	Predicate::CombinatorImpl::Monadic::initialize -- 
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
CombinatorImpl::Monadic::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	m_cAction.initialize(cProgram_);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Monadic::terminate -- 
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
CombinatorImpl::Monadic::
terminate(Interface::IProgram& cProgram_)
{
	m_cAction.terminate(cProgram_);
	terminateBase(cProgram_);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Monadic::finish -- 
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
CombinatorImpl::Monadic::
finish(Interface::IProgram& cProgram_)
{
	m_cAction->finish(cProgram_);
}

// FUNCTION public
//	Predicate::CombinatorImpl::Monadic::reset -- 
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
CombinatorImpl::Monadic::
reset(Interface::IProgram& cProgram_)
{
	m_cAction->reset(cProgram_);
}

// FUNCTION protected
//	Predicate::CombinatorImpl::Monadic::serializeBase -- for serialize
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

void
CombinatorImpl::Monadic::
serializeBase(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cAction.serialize(cArchive_);
}

///////////////////////////////////////
// Predicate::CombinatorImpl::And

// FUNCTION public
//	Predicate::CombinatorImpl::And::getClassID -- 
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
CombinatorImpl::And::
getClassID() const
{
	return Class::getClassID(Class::Category::CombinatorAnd);
}

// FUNCTION private
//	Predicate::CombinatorImpl::And::explainOperator -- explain operator
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
CombinatorImpl::And::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.newLine(true /* force */);
	cExplain_.put(_pszExplainName[_Type::And]);
}

// FUNCTION private
//	Predicate::CombinatorImpl::And::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Interface::IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
Interface::IPredicate::Boolean::Value
CombinatorImpl::And::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	Boolean::Value eValue = Boolean::True;
	VECTOR<Action::ActionHolder>::ITERATOR iterator = getAction().begin();
	const VECTOR<Action::ActionHolder>::ITERATOR last = getAction().end();
	for (; iterator != last; ++iterator) {
		// If result is true, no need to combine
		if ((*iterator)->execute(cProgram_, cActionList_) == Action::Status::False) {
			Interface::IPredicate* pPredicate =
				_SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, (*iterator).getAction());
			eValue = Boolean::boolAnd(eValue, pPredicate->check(cProgram_));
			if (eValue == Boolean::False) break;
		}
	}
	return eValue;
}

///////////////////////////////////////
// Predicate::CombinatorImpl::AndDyadic

// FUNCTION public
//	Predicate::CombinatorImpl::AndDyadic::getClassID -- 
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
CombinatorImpl::AndDyadic::
getClassID() const
{
	return Class::getClassID(Class::Category::CombinatorAndDyadic);
}

// FUNCTION private
//	Predicate::CombinatorImpl::AndDyadic::explainOperator -- explain operator
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
CombinatorImpl::AndDyadic::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.newLine(true /* force */);
	cExplain_.put(_pszExplainName[_Type::And]);
}

// FUNCTION private
//	Predicate::CombinatorImpl::AndDyadic::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Interface::IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
Interface::IPredicate::Boolean::Value
CombinatorImpl::AndDyadic::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	Boolean::Value eValue = Boolean::True;
	if (getAction().first->execute(cProgram_, cActionList_) == Action::Status::False) {
		Interface::IPredicate* pPredicate0 =
			_SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, getAction().first.getAction());
		eValue = pPredicate0->check(cProgram_);
	}
	if (eValue != Boolean::False
		&& getAction().second->execute(cProgram_, cActionList_) == Action::Status::False) {
		Interface::IPredicate* pPredicate1 =
			_SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, getAction().second.getAction());
		eValue = Boolean::boolAnd(eValue, pPredicate1->check(cProgram_));
	}
	return eValue;
}

///////////////////////////////////////
// Predicate::CombinatorImpl::Or

// FUNCTION public
//	Predicate::CombinatorImpl::Or::getClassID -- 
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
CombinatorImpl::Or::
getClassID() const
{
	return Class::getClassID(Class::Category::CombinatorOr);
}

// FUNCTION private
//	Predicate::CombinatorImpl::Or::explainOperator -- explain operator
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
CombinatorImpl::Or::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.newLine(true /* force */);
	cExplain_.put(_pszExplainName[_Type::Or]);
}

// FUNCTION private
//	Predicate::CombinatorImpl::Or::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Interface::IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
Interface::IPredicate::Boolean::Value
CombinatorImpl::Or::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	Boolean::Value eValue = Boolean::False;
	VECTOR<Action::ActionHolder>::ITERATOR iterator = getAction().begin();
	const VECTOR<Action::ActionHolder>::ITERATOR last = getAction().end();
	for (; iterator != last; ++iterator) {
		if ((*iterator)->execute(cProgram_, cActionList_) == Action::Status::False) {
			Interface::IPredicate* pPredicate =
				_SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, (*iterator).getAction());
			eValue = Boolean::boolOr(eValue, pPredicate->check(cProgram_));
		} else {
			eValue = Boolean::True;
			break;
		}
	}
	return eValue;
}

///////////////////////////////////////
// Predicate::CombinatorImpl::OrDyadic

// FUNCTION public
//	Predicate::CombinatorImpl::OrDyadic::getClassID -- 
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
CombinatorImpl::OrDyadic::
getClassID() const
{
	return Class::getClassID(Class::Category::CombinatorOrDyadic);
}

// FUNCTION private
//	Predicate::CombinatorImpl::OrDyadic::explainOperator -- explain operator
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
CombinatorImpl::OrDyadic::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.newLine(true /* force */);
	cExplain_.put(_pszExplainName[_Type::Or]);
}

// FUNCTION private
//	Predicate::CombinatorImpl::OrDyadic::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Interface::IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
Interface::IPredicate::Boolean::Value
CombinatorImpl::OrDyadic::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	Boolean::Value eValue = Boolean::False;
	if (getAction().first->execute(cProgram_, cActionList_) == Action::Status::True) {
		eValue = Boolean::True;
	} else {
		Interface::IPredicate* pPredicate0 =
			_SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, getAction().first.getAction());
		eValue = pPredicate0->check(cProgram_);
	}

	if (eValue != Boolean::True) {
		if (getAction().second->execute(cProgram_, cActionList_) == Action::Status::False) {
			Interface::IPredicate* pPredicate1 =
				_SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, getAction().second.getAction());
			eValue = Boolean::boolOr(eValue, pPredicate1->check(cProgram_));
		} else {
			eValue = Boolean::True;
		}
	}
	return eValue;
}

///////////////////////////////////////
// Predicate::CombinatorImpl::Not

// FUNCTION public
//	Predicate::CombinatorImpl::Not::getClassID -- 
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
CombinatorImpl::Not::
getClassID() const
{
	return Class::getClassID(Class::Category::CombinatorNot);
}

// FUNCTION private
//	Predicate::CombinatorImpl::Not::explainOperator -- explain operator
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
CombinatorImpl::Not::
explainOperator(Interface::IProgram& cProgram_,
				Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Not]);
}

// FUNCTION private
//	Predicate::CombinatorImpl::Not::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Interface::IPredicate::Boolean::Value
//
// EXCEPTIONS

//virtual
Interface::IPredicate::Boolean::Value
CombinatorImpl::Not::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	if (getAction()->execute(cProgram_, cActionList_) == Action::Status::False) {
		Interface::IPredicate* pPredicate =
			_SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, getAction().getAction());
		return Boolean::boolNot(pPredicate->check(cProgram_));
	}
	return Boolean::False;
}

//////////////////////////////
// Predicate::Combinator::

// FUNCTION public
//	Predicate::Combinator::And::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const PAIR<int, int>& cID_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::And::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const PAIR<int, int>& cID_)
{
	AUTOPOINTER<This> pResult = new CombinatorImpl::AndDyadic(cID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Combinator::And::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecID_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::And::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecID_)
{
	if (vecID_.GETSIZE() == 1) {
		return _SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, cProgram_.getAction(vecID_[0]));
	}

	AUTOPOINTER<This> pResult = new CombinatorImpl::And(vecID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Combinator::Or::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const PAIR<int, int>& cID_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::Or::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const PAIR<int, int>& cID_)
{
	AUTOPOINTER<This> pResult = new CombinatorImpl::OrDyadic(cID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Combinator::Or::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const VECTOR<int>& vecID_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::Or::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const VECTOR<int>& vecID_)
{
	if (vecID_.GETSIZE() == 1) {
		return _SYDNEY_DYNAMIC_CAST(Interface::IPredicate*, cProgram_.getAction(vecID_[0]));
	}

	AUTOPOINTER<This> pResult = new CombinatorImpl::Or(vecID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Combinator::Not::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iID_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::Not::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iID_)
{
	AUTOPOINTER<This> pResult = new CombinatorImpl::Not(iID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Combinator::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Combinator*
//
// EXCEPTIONS

//static
Combinator*
Combinator::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::CombinatorAnd:
		{
			return new CombinatorImpl::And;
		}
	case Class::Category::CombinatorAndDyadic:
		{
			return new CombinatorImpl::AndDyadic;
		}
	case Class::Category::CombinatorOr:
		{
			return new CombinatorImpl::Or;
		}
	case Class::Category::CombinatorOrDyadic:
		{
			return new CombinatorImpl::OrDyadic;
		}
	case Class::Category::CombinatorNot:
		{
			return new CombinatorImpl::Not;
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
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
