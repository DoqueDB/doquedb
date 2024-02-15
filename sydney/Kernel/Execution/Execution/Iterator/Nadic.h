// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Execution/Iterator/Nadic.h --
// 
// Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_EXECUTION_ITERATOR_NADIC_H
#define __SYDNEY_EXECUTION_ITERATOR_NADIC_H

#include "boost/bind.hpp"

#include "Execution/Iterator/Module.h"
#include "Execution/Declaration.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Base.h"
#include "Execution/Iterator/OperandElement.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Object.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_ITERATOR_BEGIN

////////////////////////////////////////////////////////////////////////////////
//	TEMPLATE CLASS
//	Execution::Iterator::Nadic -- iterator class with nadic operands
//
//	TEMPLATE ARGUMENT
//	class Base_
//
//	NOTES
//		This class is not constructed directly
template <class Base_>
class Nadic
	: public Base_
{
public:
	typedef Base_ Super;
	typedef Nadic<Base_> This;

	// constructor
	Nadic()
		: Super(),
		  m_vecOperand(),
		  m_cOutData()
	{}

	// destructor
	virtual ~Nadic() {}

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
	virtual int getOutData(Interface::IProgram& cProgram_);

///////////////////////////////
// ModSerializer
	void serialize(ModArchive& archiver_);

protected:
	// set result data of i-th operand
	void setResult(int iPos_);
	// explain operand
	virtual void explainOperand(Opt::Environment* pEnvironment_,
								Interface::IProgram& cProgram_,
								Opt::Explain& cExplain_);

///////////////////////////
// Iterator::Base::
	virtual void addOutData(Interface::IProgram& cProgram_,
							const Action::Argument& cAction_);
	virtual void addInput(Interface::IProgram& cProgram_,
						  const Action::Argument& cAction_);

	void initializeOperand(Interface::IProgram& cProgram_);
	void terminateOperand(Interface::IProgram& cProgram_);
	Action::Status::Value startUpOperand(Interface::IProgram& cProgram_);
	void finishOperand(Interface::IProgram& cProgram_);
	void resetOperand(Interface::IProgram& cProgram_);
	void serializeOperand(ModArchive& archiver_);

	VECTOR<OperandElement>& getOperand() {return m_vecOperand;}
	int getOperandSize() {return m_vecOperand.GETSIZE();}
	OperandElement& getElement(int iPosition_) {return m_vecOperand[iPosition_];}
	Common::DataArrayData* getOutData() {return m_cOutData.get();}

private:
	VECTOR<OperandElement> m_vecOperand;
	Action::ArrayDataHolder m_cOutData;
};

/////////////////////////////////////////////
// Execution::Iterator::Nadic

// TEMPLATE FUNCTION public
//	Iterator::Nadic::explain -- 
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
Nadic<Base_>::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	bool bPush = explainBase(cExplain_);
	if (getOutData()) {
		cExplain_.put("out to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	explainThis(pEnvironment_, cProgram_, cExplain_);
	explainOperand(pEnvironment_, cProgram_, cExplain_);
	explainAction(pEnvironment_, cProgram_, cExplain_);

	if (bPush) {
		cExplain_.popIndent();
	}
}

// TEMPLATE FUNCTION public
//	Iterator::Nadic::initialize -- 
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
Nadic<Base_>::
initialize(Interface::IProgram& cProgram_)
{
	initializeBase(cProgram_);
	initializeOperand(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Nadic::terminate -- 
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
Nadic<Base_>::
terminate(Interface::IProgram& cProgram_)
{
	terminateOperand(cProgram_);
	terminateBase(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Nadic::startUp -- do once before iteration
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
Nadic<Base_>::
startUp(Interface::IProgram& cProgram_)
{
	Action::Status::Value eResult = Action::Status::Success;
	if ((eResult = startUpBase(cProgram_)) != Action::Status::Break) {
		eResult = startUpOperand(cProgram_);
	}
	return eResult;
}

// TEMPLATE FUNCTION public
//	Iterator::Nadic::finish -- do once after iteration
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
Nadic<Base_>::
finish(Interface::IProgram& cProgram_)
{
	finishOperand(cProgram_);
	finishBase(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Nadic<Base_>::reset -- 
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
Nadic<Base_>::
reset(Interface::IProgram& cProgram_)
{
	resetOperand(cProgram_);
}

// TEMPLATE FUNCTION public
//	Iterator::Nadic<Base_>::getOutData -- 
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
//	int
//
// EXCEPTIONS

//virtual
template <class Base_>
int
Nadic<Base_>::
getOutData(Interface::IProgram& cProgram_)
{
	return m_cOutData.getDataID();
}

// TEMPLATE FUNCTION public
//	Iterator::Nadic::serialize -- 
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
Nadic<Base_>::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	serializeOperand(archiver_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::setResult -- set result data of i-th operand
//
// TEMPLATE ARGUMENTS
//	class Base_
//	
// NOTES
//
// ARGUMENTS
//	int iPos_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

template <class Base_>
void
Nadic<Base_>::
setResult(int iPos_)
{
	Utility::DataType::assignElements(getOutData(), getOperand()[iPos_].getData());
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::explainOperand -- 
//
// TEMPLATE ARGUMENTS
//	class Base_
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
template <class Base_>
void
Nadic<Base_>::
explainOperand(Opt::Environment* pEnvironment_,
			   Interface::IProgram& cProgram_,
			   Opt::Explain& cExplain_)
{
	cExplain_.pushIndent();
	FOREACH(m_vecOperand.begin(),
			m_vecOperand.end(),
			boost::bind(&OperandElement::explain,
						_1,
						pEnvironment_,
						boost::ref(cProgram_),
						boost::ref(cExplain_)));
	cExplain_.popIndent();
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::addOutData -- 
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
Nadic<Base_>::
addOutData(Interface::IProgram& cProgram_,
		   const Action::Argument& cAction_)
{
	if (m_cOutData.isValid() == true) {
		const char srcFile[] = __FILE__;
		const char moduleName[] = "Execution::Iterator";
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// variable for result data is set through addAction(Output)
	m_cOutData.setDataID(cAction_.getInstanceID());
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::addInput -- 
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
Nadic<Base_>::
addInput(Interface::IProgram& cProgram_,
		 const Action::Argument& cAction_)
{
	// instanceID is iterator's ID, argumentID is data ID
	m_vecOperand.PUSHBACK(OperandElement(cAction_.getInstanceID(),
										 cAction_.getArgumentID()));
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::initializeOperand -- 
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
Nadic<Base_>::
initializeOperand(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecOperand.begin(),
			m_vecOperand.end(),
			boost::bind(&OperandElement::initialize,
						_1,
						boost::ref(cProgram_)));

	if (m_cOutData.isValid()) {
		m_cOutData.initialize(cProgram_);
		if (m_cOutData.isInitialized() == false) {
			const char srcFile[] = __FILE__;
			const char moduleName[] = "Execution::Iterator";
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	setHasData(true);
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::terminateOperand -- 
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
Nadic<Base_>::
terminateOperand(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecOperand.begin(),
			m_vecOperand.end(),
			boost::bind(&OperandElement::terminate,
						_1,
						boost::ref(cProgram_)));
	m_cOutData.terminate(cProgram_);
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::startUpOperand -- 
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
Nadic<Base_>::
startUpOperand(Interface::IProgram& cProgram_)
{
	Action::Status::Value eResult = Action::Status::Success;
	VECTOR<OperandElement>::ITERATOR iterator = m_vecOperand.begin();
	const VECTOR<OperandElement>::ITERATOR last = m_vecOperand.end();
	for (; iterator != last; ++iterator) {
		if ((eResult = (*iterator).startUp(cProgram_)) == Action::Status::Break)
			break;
	}
	return eResult;
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::finishOperand -- 
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
Nadic<Base_>::
finishOperand(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecOperand.begin(),
			m_vecOperand.end(),
			boost::bind(&OperandElement::finish,
						_1,
						boost::ref(cProgram_)));
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::resetOperand -- 
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
Nadic<Base_>::
resetOperand(Interface::IProgram& cProgram_)
{
	FOREACH(m_vecOperand.begin(),
			m_vecOperand.end(),
			boost::bind(&OperandElement::reset,
						_1,
						boost::ref(cProgram_)));
}

// TEMPLATE FUNCTION protected
//	Iterator::Nadic<Base_>::serializeOperand -- 
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
Nadic<Base_>::
serializeOperand(ModArchive& archiver_)
{
	Utility::SerializeObject(archiver_, m_vecOperand);
	m_cOutData.serialize(archiver_);
}

_SYDNEY_EXECUTION_ITERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

#endif // __SYDNEY_EXECUTION_ITERATOR_NADIC_H

//
//	Copyright (c) 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
