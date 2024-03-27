// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/Monadic.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_MONADIC_H
#define __SYDNEY_EXECUTION_ITERATOR_MONADIC_H

#include "Execution/Iterator/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Base.h"
#include "Execution/Iterator/OperandElement.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Object.h"

#include "Exception/NotSupported.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	TEMPLATE CLASS
//	Execution::Iterator::Monadic -- iterator class with monadic operands
//
//	TEMPLATE ARGUMENT
//	class Base_
//
//	NOTES
//		This class is not constructed directly
template <class Base_>
class Monadic
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Monadic This;

	// constructor
	Monadic()
		: Super(),
		  m_cOperand()
	{}

	template <class A1_, class A2_, class A3_>
	Monadic(A1_ a1, A2_ a2, A3_ a3)
		: Super(a1, a2, a3),
		  m_cOperand()
	{}

	// destructor
	virtual ~Monadic() {}

///////////////////////////
//Interface::IIterator::
	virtual void explain(Opt::Environment* pEnvironment_,
						 Interface::IProgram& cProgram_,
						 Opt::Explain& cExplain_);
	virtual void initialize(Interface::IProgram& cProgram_);
	virtual void terminate(Interface::IProgram& cProgram_);
	virtual Action::Status::Value startUp(Interface::IProgram& cProgram_);
	virtual void finish(Interface::IProgram& cProgram_);
//	virtual bool next(Interface::IProgram& cProgram_);
	virtual void reset(Interface::IProgram& cProgram_);
//	virtual void setWasLast(Interface::IProgram& cProgram_);
//	virtual void addStartUp(Interface::IProgram& cProgram_,
//							const Action::Argument& cAction_);
//	virtual void addAction(Interface::IProgram& cProgram_,
//						   const Action::Argument& cAction_);
//	virtual Action::Status::Value doAction(Interface::IProgram& cProgram_);
//	virtual bool isEndOfData();

///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_);

	using Base_::explainBase;
	using Base_::explainThis;
	using Base_::explainAction;
	using Base_::initializeBase;
	using Base_::terminateBase;
	using Base_::startUpBase;
	using Base_::finishBase;
	using Base_::serializeBase;

protected:
///////////////////////////
// Iterator::Base::
	virtual void addInput(Interface::IProgram& cProgram_,
						  const Action::Argument& cAction_);

	void initializeOperand(Interface::IProgram& cProgram_);
	void terminateOperand(Interface::IProgram& cProgram_);
	Action::Status::Value startUpOperand(Interface::IProgram& cProgram_);
	void finishOperand(Interface::IProgram& cProgram_);
	void resetOperand(Interface::IProgram& cProgram_);
	void serializeOperand(ModArchive& archiver_);

	OperandElement& getOperand() {return m_cOperand;}

private:
	OperandElement m_cOperand;
};

/////////////////////////////////////////////
// Execution::Iterator::Monadic

// TEMPLATE FUNCTION public
//	Iterator::Monadic::explain -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
//	
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
template <class Base_>
void
Monadic<Base_>::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	bool bPush = explainBase(cExplain_);
	explainThis(pEnvironment_, cProgram_, cExplain_);

	cExplain_.pushIndent();
	m_cOperand.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent();

	explainAction(pEnvironment_, cProgram_, cExplain_);

	if (bPush) {
		cExplain_.popIndent();
	}
}

// TEMPLATE FUNCTION public
//	Iterator::Monadic::initialize -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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
template <class Base_>
void
Monadic<Base_>::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	initializeOperand(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Monadic::terminate -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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
template <class Base_>
void
Monadic<Base_>::
terminate(Interface::IProgram& cProgram_)
{
	terminateOperand(cProgram_);
	terminateBase(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Monadic::startUp -- do once before iteration
//
// TEMPLATE ARGUMENTS
//	class Base_
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
template <class Base_>
Action::Status::Value
Monadic<Base_>::
startUp(Interface::IProgram& cProgram_)
{
	Action::Status::Value eResult = Action::Status::Success;
	if ((eResult = startUpBase(cProgram_)) != Action::Status::Break) {
		eResult = startUpOperand(cProgram_);
	}
	return eResult;
}

// TEMPLATE FUNCTION public
//	Iterator::Monadic::finish -- do once after iteration
//
// TEMPLATE ARGUMENTS
//	class Base_
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
template <class Base_>
void
Monadic<Base_>::
finish(Interface::IProgram& cProgram_)
{
	finishOperand(cProgram_);
	finishBase(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Monadic<Base_>::reset -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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
template <class Base_>
void
Monadic<Base_>::
reset(Interface::IProgram& cProgram_)
{
	resetOperand(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Monadic::serialize -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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

template <class Base_>
void
Monadic<Base_>::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	serializeOperand(archiver_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Monadic<Base_>::addInput -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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
template <class Base_>
void
Monadic<Base_>::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	// instanceID is iterator's ID, argumentID is data ID
	m_cOperand.setIteratorID(cAction_.getInstanceID());
	m_cOperand.setDataID(cAction_.getArgumentID());
}

// TEMPLATE FUNCTION protected
//	Iterator::Monadic<Base_>::initializeOperand -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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

template <class Base_>
void
Monadic<Base_>::
initializeOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.initialize(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Monadic<Base_>::terminateOperand -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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

template <class Base_>
void
Monadic<Base_>::
terminateOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.terminate(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Monadic<Base_>::startUpOperand -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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

template <class Base_>
Action::Status::Value
Monadic<Base_>::
startUpOperand(Interface::IProgram& cProgram_)
{
	return m_cOperand.startUp(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Monadic<Base_>::finishOperand -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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

template <class Base_>
void
Monadic<Base_>::
finishOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.finish(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Monadic<Base_>::resetOperand -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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

template <class Base_>
void
Monadic<Base_>::
resetOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.reset(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Monadic<Base_>::serializeOperand -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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

template <class Base_>
void
Monadic<Base_>::
serializeOperand(ModArchive& archiver_)
{
	m_cOperand.serialize(archiver_);
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_MONADIC_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
