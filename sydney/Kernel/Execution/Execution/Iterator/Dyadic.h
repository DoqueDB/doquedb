// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/Dyadic.h --
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_DYADIC_H
#define __SYDNEY_EXECUTION_ITERATOR_DYADIC_H

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
//	Execution::Iterator::Dyadic -- iterator class with dyadic operands
//
//	TEMPLATE ARGUMENT
//	class Base_
//
//	NOTES
//		This class is not constructed directly
template <class Base_>
class Dyadic
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Dyadic This;

	// constructor
	Dyadic()
		: Super(),
		  m_cOperand()
	{}

	// destructor
	virtual ~Dyadic() {}

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

	OperandElement& getOperand0() {return m_cOperand.first;}
	OperandElement& getOperand1() {return m_cOperand.second;}

private:
	PAIR<OperandElement, OperandElement> m_cOperand;
};

/////////////////////////////////////////////
// Execution::Iterator::Dyadic

// TEMPLATE FUNCTION public
//	Iterator::Dyadic::explain -- 
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
Dyadic<Base_>::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	bool bPush = explainBase(cExplain_);
	explainThis(pEnvironment_, cProgram_, cExplain_);

	cExplain_.pushIndent();
	m_cOperand.first.explain(pEnvironment_, cProgram_, cExplain_);
	m_cOperand.second.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.popIndent();

	explainAction(pEnvironment_, cProgram_, cExplain_);

	if (bPush) {
		cExplain_.popIndent();
	}
}

// TEMPLATE FUNCTION public
//	Iterator::Dyadic::initialize -- 
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
Dyadic<Base_>::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	initializeOperand(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Dyadic::terminate -- 
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
Dyadic<Base_>::
terminate(Interface::IProgram& cProgram_)
{
	terminateOperand(cProgram_);
	terminateBase(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Dyadic::startUp -- do once before iteration
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
Dyadic<Base_>::
startUp(Interface::IProgram& cProgram_)
{
	Action::Status::Value eResult = Action::Status::Success;
	if ((eResult = startUpBase(cProgram_)) != Action::Status::Break) {
		eResult = startUpOperand(cProgram_);
	}
	return eResult;
}

// TEMPLATE FUNCTION public
//	Iterator::Dyadic::finish -- do once after iteration
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
Dyadic<Base_>::
finish(Interface::IProgram& cProgram_)
{
	finishOperand(cProgram_);
	finishBase(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Dyadic<Base_>::reset -- 
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
Dyadic<Base_>::
reset(Interface::IProgram& cProgram_)
{
	resetOperand(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Dyadic::serialize -- 
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
Dyadic<Base_>::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	serializeOperand(archiver_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Dyadic<Base_>::addInput -- 
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
Dyadic<Base_>::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	OperandElement* pOperand = (m_cOperand.first.isValid())
		? &m_cOperand.second : &m_cOperand.first;

	// instanceID is iterator's ID, argumentID is data ID
	pOperand->setIteratorID(cAction_.getInstanceID());
	pOperand->setDataID(cAction_.getArgumentID());
}

// TEMPLATE FUNCTION protected
//	Iterator::Dyadic<Base_>::initializeOperand -- 
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
Dyadic<Base_>::
initializeOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.first.initialize(cProgram_);
	m_cOperand.second.initialize(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Dyadic<Base_>::terminateOperand -- 
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
Dyadic<Base_>::
terminateOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.first.terminate(cProgram_);
	m_cOperand.second.terminate(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Dyadic<Base_>::startUpOperand -- 
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
Dyadic<Base_>::
startUpOperand(Interface::IProgram& cProgram_)
{
	Action::Status::Value eResult = Action::Status::Success;
	if ((eResult = m_cOperand.first.startUp(cProgram_)) != Action::Status::Break) {
		eResult = m_cOperand.second.startUp(cProgram_);
	}
	return eResult;
}

// TEMPLATE FUNCTION protected
//	Iterator::Dyadic<Base_>::finishOperand -- 
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
Dyadic<Base_>::
finishOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.first.finish(cProgram_);
	m_cOperand.second.finish(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Dyadic<Base_>::resetOperand -- 
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
Dyadic<Base_>::
resetOperand(Interface::IProgram& cProgram_)
{
	m_cOperand.first.reset(cProgram_);
	m_cOperand.second.reset(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Dyadic<Base_>::serializeOperand -- 
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
Dyadic<Base_>::
serializeOperand(ModArchive& archiver_)
{
	m_cOperand.first.serialize(archiver_);
	m_cOperand.second.serialize(archiver_);
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_DYADIC_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
