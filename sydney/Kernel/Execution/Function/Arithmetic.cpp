// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Arithmetic.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Execution/Function/Arithmetic.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/DataOperation.h"

#include "Exception/NotSupported.h"
#include "Exception/NumericValueOutOfRange.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST local
	//	_ArithmeticOperationTable -- mapping from treenodeinterface type to data operation
	//
	// NOTES
	const Common::DataOperation::Type _ArithmeticOperationTable[] =
	{
		Common::DataOperation::Unknown,			//ArithmeticStart
		Common::DataOperation::Addition,		//Add
		Common::DataOperation::Subtraction,		//Subtract
		Common::DataOperation::Multiplication,	//Multiply
		Common::DataOperation::Division,		//Divide
		Common::DataOperation::Modulus,			//Modulus
		Common::DataOperation::Negation,		//Negative
		Common::DataOperation::Unknown,			//StringConcatenate
		Common::DataOperation::AbsoluteValue,	//Absolute
		Common::DataOperation::Unknown,			//CharLength
		Common::DataOperation::Unknown,			//SubString
		Common::DataOperation::Unknown,			//Overlay
		Common::DataOperation::Unknown,			//Truncate
		Common::DataOperation::Unknown,			//Nop
		Common::DataOperation::Unknown,			//Case
		Common::DataOperation::Unknown,			//NullIf
		Common::DataOperation::Unknown,			//Coalesce
		Common::DataOperation::Unknown,			//Cast
		Common::DataOperation::Unknown,			//WordCount
		Common::DataOperation::Unknown,			//FullTextLength
		Common::DataOperation::Unknown,			//Normalize
		Common::DataOperation::Unknown,			//OctetLength
		Common::DataOperation::Unknown,			//CoalesceDefault
		Common::DataOperation::Unknown			//ArithmeticEnd
	};

	// FUNCTION local
	//	$$$::_getOperation -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	LogicalFile::TreeNodeInterface::Type eType_
	//	
	// RETURN
	//	Common::DataOperation::Type
	//
	// EXCEPTIONS
	Common::DataOperation::Type
	_getOperation(LogicalFile::TreeNodeInterface::Type eType_)
	{
		; _SYDNEY_ASSERT(eType_ > LogicalFile::TreeNodeInterface::ArithmeticStart
						 && eType_ < LogicalFile::TreeNodeInterface::ArithmeticEnd);
		return _ArithmeticOperationTable[eType_ - LogicalFile::TreeNodeInterface::ArithmeticStart];
	}

	// CONST
	//	_pszFunctionName -- function name for explain
	//
	// NOTES
	const char* const _pszMonadicFunctionName[] =
	{
		"++", // Common::DataOperation::Increment
		"--", // Common::DataOperation::Decrement
		"-", // Common::DataOperation::Negation
		"ABS", // Common::DataOperation::AbsoluteValue
		0
	};
	const char* const _pszDyadicFunctionName[] =
	{
		"+", // Common::DataOperation::Addition
		"-", // Common::DataOperation::Subtraction
		"*", // Common::DataOperation::Multiplication
		"/", // Common::DataOperation::Division
		"MOD", // Common::DataOperation::Modulus
		0
	};
}

namespace ArithmeticImpl
{
	// CLASS local
	//	Execution::Function::ArithmeticImpl::Monadic -- implementation class of Arithmetic
	//
	// NOTES
	class Monadic
		: public Function::Arithmetic
	{
	public:
		typedef Monadic This;
		typedef Function::Arithmetic Super;

		Monadic()
			: Super(),
			  m_eFunction(Common::DataOperation::Unknown),
			  m_cInData(),
			  m_cOutData()
		{}
		Monadic(Common::DataOperation::Type eType_,
				int iInDataID_,
				int iOutDataID_)
			: Super(),
			  m_eFunction(eType_),
			  m_cInData(iInDataID_),
			  m_cOutData(iOutDataID_)
		{}
		~Monadic()
		{}

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
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		const Common::Data* getInData() {return m_cInData.getData();}
		Common::Data* getOutData() {return m_cOutData.get();}

	private:
		void calculate();

		Common::DataOperation::Type m_eFunction;
		Action::DataHolder m_cInData;
		Action::DataHolder m_cOutData;
	};

	// CLASS local
	//	Execution::Function::ArithmeticImpl::Dyadic -- implementation class of Arithmetic
	//
	// NOTES
	class Dyadic
		: public Function::Arithmetic
	{
	public:
		typedef Dyadic This;
		typedef Function::Arithmetic Super;

		Dyadic()
			: Super(),
			  m_eFunction(Common::DataOperation::Unknown),
			  m_cInData0(),
			  m_cInData1(),
			  m_cOutData()
		{}
		Dyadic(Common::DataOperation::Type eType_,
				int iInDataID0_,
				int iInDataID1_,
				int iOutDataID_)
			: Super(),
			  m_eFunction(eType_),
			  m_cInData0(iInDataID0_),
			  m_cInData1(iInDataID1_),
			  m_cOutData(iOutDataID_)
		{}
		~Dyadic()
		{}

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
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		const Common::Data* getInData0() {return m_cInData0.getData();}
		const Common::Data* getInData1() {return m_cInData1.getData();}
		Common::Data* getOutData() {return m_cOutData.get();}

	private:
		void calculate();

		Common::DataOperation::Type m_eFunction;
		Action::DataHolder m_cInData0;
		Action::DataHolder m_cInData1;
		Action::DataHolder m_cOutData;
	};
}

/////////////////////////////////////////////////
// Execution::Function::ArithmeticImpl::Monadic

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::explain -- 
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
void
ArithmeticImpl::Monadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszMonadicFunctionName[m_eFunction - 100]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cInData.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::initialize -- 
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
ArithmeticImpl::Monadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData.isInitialized() == false) {
		m_cInData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::terminate -- 
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
ArithmeticImpl::Monadic::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::execute -- 
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
ArithmeticImpl::Monadic::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		calculate();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::finish -- 
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
ArithmeticImpl::Monadic::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::reset -- 
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
ArithmeticImpl::Monadic::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::getClassID -- 
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
ArithmeticImpl::Monadic::
getClassID() const
{
	return Class::getClassID(Class::Category::ArithmeticMonadic);
}

// FUNCTION public
//	Function::ArithmeticImpl::Monadic::serialize -- 
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
ArithmeticImpl::Monadic::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	if (archiver_.isLoad()) {
		int iFunction;
		archiver_ >> iFunction;
		m_eFunction = static_cast<Common::DataOperation::Type>(iFunction);
	} else {
		int iFunction = static_cast<int>(m_eFunction);
		archiver_ << iFunction;
	}
	m_cInData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Function::ArithmeticImpl::Monadic::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ArithmeticImpl::Monadic::
calculate()
{
	if (getInData()->isNull()) {
		getOutData()->setNull();
	} else {
		Common::Data::Pointer pResult;
		if (getInData()->operateWith(m_eFunction, pResult)) {
			// calculated
			getOutData()->assign(pResult.get());
		} else {
			if (Opt::Configuration::getOverflowNull().get() == true) {
				// set null instead of throw
				getOutData()->setNull();
			} else {
				_SYDNEY_THROW0(Exception::NumericValueOutOfRange);
			}
		}
	}
}

/////////////////////////////////////////////////
// Execution::Function::ArithmeticImpl::Dyadic

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::explain -- 
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
void
ArithmeticImpl::Dyadic::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cInData0.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszDyadicFunctionName[m_eFunction]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cInData1.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::initialize -- 
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
ArithmeticImpl::Dyadic::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData0.isInitialized() == false) {
		m_cInData0.initialize(cProgram_);
		m_cInData1.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::terminate -- 
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
ArithmeticImpl::Dyadic::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData0.terminate(cProgram_);
	m_cInData1.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::execute -- 
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
ArithmeticImpl::Dyadic::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		calculate();
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::finish -- 
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
ArithmeticImpl::Dyadic::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::reset -- 
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
ArithmeticImpl::Dyadic::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::getClassID -- 
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
ArithmeticImpl::Dyadic::
getClassID() const
{
	return Class::getClassID(Class::Category::ArithmeticDyadic);
}

// FUNCTION public
//	Function::ArithmeticImpl::Dyadic::serialize -- 
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
ArithmeticImpl::Dyadic::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	if (archiver_.isLoad()) {
		int iFunction;
		archiver_ >> iFunction;
		m_eFunction = static_cast<Common::DataOperation::Type>(iFunction);
	} else {
		int iFunction = static_cast<int>(m_eFunction);
		archiver_ << iFunction;
	}
	m_cInData0.serialize(archiver_);
	m_cInData1.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

// FUNCTION private
//	Function::ArithmeticImpl::Dyadic::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ArithmeticImpl::Dyadic::
calculate()
{
	if (getInData0()->isNull()
		||getInData1()->isNull() ) {
		getOutData()->setNull();
	} else {
		Common::Data::Pointer pResult;
		if (getInData0()->operateWith(m_eFunction, getInData1(), pResult)) {
			// calculated
			getOutData()->assign(pResult.get());
		} else {
			if (Opt::Configuration::getOverflowNull().get() == true) {
				// set null instead of throw
				getOutData()->setNull();
			} else {
				_SYDNEY_THROW0(Exception::NumericValueOutOfRange);
			}
		}
	}
}

//////////////////////////////
// Function::Arithmetic::

// FUNCTION public
//	Function::Arithmetic::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	int iInDataID_
//	int iOutDataID_
//	
// RETURN
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   int iInDataID_,
	   int iOutDataID_)
{
	Common::DataOperation::Type eOperationType = _getOperation(eType_);
	; _SYDNEY_ASSERT(eOperationType != Common::DataOperation::Unknown);
	AUTOPOINTER<This> pResult = new ArithmeticImpl::Monadic(eOperationType,
															iInDataID_,
															iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Arithmetic::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	LogicalFile::TreeNodeInterface::Type eType_
//	int iInDataID0_
//	int iInDataID1_
//	int iOutDataID_
//	
// RETURN
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   LogicalFile::TreeNodeInterface::Type eType_,
	   int iInDataID0_,
	   int iInDataID1_,
	   int iOutDataID_)
{
	Common::DataOperation::Type eOperationType = _getOperation(eType_);
	; _SYDNEY_ASSERT(eOperationType != Common::DataOperation::Unknown);
	AUTOPOINTER<This> pResult = new ArithmeticImpl::Dyadic(eOperationType,
														   iInDataID0_,
														   iInDataID1_,
														   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Arithmetic::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Arithmetic*
//
// EXCEPTIONS

//static
Arithmetic*
Arithmetic::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::ArithmeticMonadic:
		{
			return new ArithmeticImpl::Monadic;
		}
	case Class::Category::ArithmeticDyadic:
		{
			return new ArithmeticImpl::Dyadic;
		}
	default:
		{
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
