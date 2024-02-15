// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Concatenate.cpp --
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Execution/Function/Concatenate.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotCompatible.h"
#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszOperator -- operator name for explain
	//
	// NOTES
	const char* const _pszOperator = "||";

	// FUNCTION local
	//	$$$::_connectString -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::StringData& cData0_
	//	const Common::StringData& cData1_
	//	Common::Data& cOutData_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_connectString(const Common::StringData& cData0_,
				   const Common::StringData& cData1_,
				   Common::Data& cOutData_)
	{
		Common::StringData cResult(cData0_);
		cResult.connect(&cData1_);
		cOutData_.assign(&cResult);
	}

	// FUNCTION local
	//	$$$::_connectBinary -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::BinaryData& cData0_
	//	const Common::BinaryData& cData1_
	//	Common::Data& cOutData_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_connectBinary(const Common::BinaryData& cData0_,
				   const Common::BinaryData& cData1_,
				   Common::Data& cOutData_)
	{
		Common::BinaryData cResult(cData0_);
		cResult.connect(&cData1_);
		cOutData_.assign(&cResult);
	}

	// FUNCTION local
	//	$$$::_connectArray -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::DataArrayData& cData0_
	//	const Common::DataArrayData& cData1_
	//	Common::Data& cOutData_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_connectArray(const Common::DataArrayData& cData0_,
				   const Common::DataArrayData& cData1_,
				   Common::Data& cOutData_)
	{
		Common::DataArrayData cResult(cData0_);
		cResult.connect(&cData1_);
		cOutData_.assign(&cResult);
	}
}

namespace ConcatenateImpl
{
	// TEMPLATE CLASS local
	//	Execution::Function::ConcatenateImpl::Base -- base class of implementation class
	//
	// TEMPLATE ARGUMENTS
	//	class Holder_
	//
	// NOTES
	template <class Holder_>
	class Base
		: public Function::Concatenate
	{
	public:
		typedef Base<Holder_> This;
		typedef Function::Concatenate Super;

		Base()
			: Super(),
			  m_cInData0(),
			  m_cInData1(),
			  m_cOutData()
		{}
		Base(int iInDataID0_,
			 int iInDataID1_,
			 int iOutDataID_)
			: Super(),
			  m_cInData0(iInDataID0_),
			  m_cInData1(iInDataID1_),
			  m_cOutData(iOutDataID_)
		{}
		virtual ~Base()
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
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		const Holder_& getInData0() {return m_cInData0;}
		const Holder_& getInData1() {return m_cInData1;}
		Action::DataHolder& getOutData() {return m_cOutData;}

	private:
		virtual void calculate() = 0;

		Holder_ m_cInData0;
		Holder_ m_cInData1;
		Action::DataHolder m_cOutData;
	};

	// CLASS local
	//	Execution::Function::ConcatenateImpl::String -- implementation class of Concatenate
	//
	// NOTES
	class String
		: public Base<Action::StringDataHolder>
	{
	public:
		typedef String This;
		typedef Base<Action::StringDataHolder> Super;

		String()
			: Super()
		{}
		String(int iInDataID0_,
			   int iInDataID1_,
			   int iOutDataID_)
			: Super(iInDataID0_, iInDataID1_, iOutDataID_)
		{}
		~String()
		{}

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer

	protected:
	private:
	///////////////////////////////
	// Super::
		virtual void calculate();
	};

	// CLASS local
	//	Execution::Function::ConcatenateImpl::Binary -- implementation class of Concatenate
	//
	// NOTES
	class Binary
		: public Base<Action::BinaryDataHolder>
	{
	public:
		typedef Binary This;
		typedef Base<Action::BinaryDataHolder> Super;

		Binary()
			: Super()
		{}
		Binary(int iInDataID0_,
			   int iInDataID1_,
			   int iOutDataID_)
			: Super(iInDataID0_, iInDataID1_, iOutDataID_)
		{}
		~Binary()
		{}

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer

	protected:
	private:
	///////////////////////////////
	// Super::
		virtual void calculate();
	};

	// CLASS local
	//	Execution::Function::ConcatenateImpl::Array -- implementation class of Concatenate
	//
	// NOTES
	class Array
		: public Base<Action::ArrayDataHolder>
	{
	public:
		typedef Array This;
		typedef Base<Action::ArrayDataHolder> Super;

		Array()
			: Super()
		{}
		Array(int iInDataID0_,
			   int iInDataID1_,
			   int iOutDataID_)
			: Super(iInDataID0_, iInDataID1_, iOutDataID_)
		{}
		~Array()
		{}

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer

	protected:
	private:
	///////////////////////////////
	// Super::
		virtual void calculate();
	};

	// CLASS local
	//	Execution::Function::ConcatenateImpl::AnyType -- implementation class of Concatenate
	//
	// NOTES
	class AnyType
		: public Base<Action::DataHolder>
	{
	public:
		typedef AnyType This;
		typedef Base<Action::DataHolder> Super;

		AnyType()
			: Super()
		{}
		AnyType(int iInDataID0_,
				int iInDataID1_,
				int iOutDataID_)
			: Super(iInDataID0_, iInDataID1_, iOutDataID_)
		{}
		~AnyType()
		{}

	/////////////////////////////
	// Interface::IAction::

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer

	///////////////////////////////
	// ModSerializer

	protected:
	private:
	///////////////////////////////
	// Super::
		virtual void calculate();
	};
}

/////////////////////////////////////////////////
// Execution::Function::ConcatenateImpl::Base

// TEMPLATE FUNCTION public
//	Function::ConcatenateImpl::Base<Holder_>::explain -- 
//
// TEMPLATE ARGUMENTS
//	class Holder_
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
template <class Holder_>
void
ConcatenateImpl::Base<Holder_>::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cInData0.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszOperator);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cInData1.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// TEMPLATE FUNCTION public
//	Function::ConcatenateImpl::Base<Holder_>::initialize -- 
//
// TEMPLATE ARGUMENTS
//	class Holder_
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
template <class Holder_>
void
ConcatenateImpl::Base<Holder_>::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData0.isInitialized() == false) {
		m_cInData0.initialize(cProgram_);
		m_cInData1.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// TEMPLATE FUNCTION public
//	Function::ConcatenateImpl::Base<Holder_>::terminate -- 
//
// TEMPLATE ARGUMENTS
//	class Holder_
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
template <class Holder_>
void
ConcatenateImpl::Base<Holder_>::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData0.terminate(cProgram_);
	m_cInData1.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// TEMPLATE FUNCTION public
//	Function::ConcatenateImpl::Base<Holder_>::execute -- 
//
// TEMPLATE ARGUMENTS
//	class Holder_
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
template <class Holder_>
Action::Status::Value
ConcatenateImpl::Base<Holder_>::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getInData0()->isNull()
			|| getInData1()->isNull() ) {
			getOutData()->setNull();
		} else if (getInData0()->getType() != getInData1()->getType()
				   || getInData0()->getElementType() != getInData1()->getElementType()) {
			_SYDNEY_THROW0(Exception::NotCompatible);
		} else {
			calculate();
		}
		done();
	}
	return Action::Status::Success;
}

// TEMPLATE FUNCTION public
//	Function::ConcatenateImpl::Base<Holder_>::finish -- 
//
// TEMPLATE ARGUMENTS
//	class Holder_
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
template <class Holder_>
void
ConcatenateImpl::Base<Holder_>::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// TEMPLATE FUNCTION public
//	Function::ConcatenateImpl::Base<Holder_>::reset -- 
//
// TEMPLATE ARGUMENTS
//	class Holder_
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
template <class Holder_>
void
ConcatenateImpl::Base<Holder_>::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// TEMPLATE FUNCTION public
//	Function::ConcatenateImpl::Base<Holder_>::serialize -- 
//
// TEMPLATE ARGUMENTS
//	class Holder_
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

template <class Holder_>
void
ConcatenateImpl::Base<Holder_>::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cInData0.serialize(archiver_);
	m_cInData1.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////////////////
// Execution::Function::ConcatenateImpl::String

// FUNCTION public
//	Function::ConcatenateImpl::String::getClassID -- 
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
ConcatenateImpl::String::
getClassID() const
{
	return Class::getClassID(Class::Category::Concatenate);
}

// FUNCTION private
//	Function::ConcatenateImpl::String::calculate -- 
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
ConcatenateImpl::String::
calculate()
{
	_connectString(*getInData0(),
				   *getInData1(),
				   *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::ConcatenateImpl::Binary

// FUNCTION public
//	Function::ConcatenateImpl::Binary::getClassID -- 
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
ConcatenateImpl::Binary::
getClassID() const
{
	return Class::getClassID(Class::Category::ConcatenateBinary);
}

// FUNCTION private
//	Function::ConcatenateImpl::Binary::calculate -- 
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
ConcatenateImpl::Binary::
calculate()
{
	_connectBinary(*getInData0(),
				   *getInData1(),
				   *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::ConcatenateImpl::Array

// FUNCTION public
//	Function::ConcatenateImpl::Array::getClassID -- 
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
ConcatenateImpl::Array::
getClassID() const
{
	return Class::getClassID(Class::Category::ConcatenateArray);
}

// FUNCTION private
//	Function::ConcatenateImpl::Array::calculate -- 
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
ConcatenateImpl::Array::
calculate()
{
	_connectArray(*getInData0(),
				  *getInData1(),
				  *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::ConcatenateImpl::AnyType

// FUNCTION public
//	Function::ConcatenateImpl::AnyType::getClassID -- 
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
ConcatenateImpl::AnyType::
getClassID() const
{
	return Class::getClassID(Class::Category::ConcatenateAnyType);
}

// FUNCTION private
//	Function::ConcatenateImpl::AnyType::calculate -- 
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
ConcatenateImpl::AnyType::
calculate()
{
	switch (getInData0()->getType()) {
	case Common::DataType::String:
		{
			_connectString(_SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												*getInData0()),
						   _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												*getInData1()),
						   *getOutData());
			break;
		}
	case Common::DataType::Binary:
		{
			_connectBinary(_SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												*getInData0()),
						   _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												*getInData1()),
						   *getOutData());
			break;
		}
	case Common::DataType::Array:
		{
			_connectArray(_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
											   *getInData0()),
						  _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
											   *getInData1()),
						  *getOutData());
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

/////////////////////////////////////
// Function::Concatenate::String::

// FUNCTION public
//	Function::Concatenate::String::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID0_
//	int iInDataID1_
//	int iOutDataID_
//	
// RETURN
//	Concatenate*
//
// EXCEPTIONS

//static
Concatenate*
Concatenate::String::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID0_,
	   int iInDataID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new ConcatenateImpl::String(iInDataID0_,
															iInDataID1_,
															iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::Concatenate::Binary::

// FUNCTION public
//	Function::Concatenate::Binary::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID0_
//	int iInDataID1_
//	int iOutDataID_
//	
// RETURN
//	Concatenate*
//
// EXCEPTIONS

//static
Concatenate*
Concatenate::Binary::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID0_,
	   int iInDataID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new ConcatenateImpl::Binary(iInDataID0_,
															iInDataID1_,
															iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::Concatenate::Array::

// FUNCTION public
//	Function::Concatenate::Array::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID0_
//	int iInDataID1_
//	int iOutDataID_
//	
// RETURN
//	Concatenate*
//
// EXCEPTIONS

//static
Concatenate*
Concatenate::Array::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID0_,
	   int iInDataID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new ConcatenateImpl::Array(iInDataID0_,
														   iInDataID1_,
														   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::Concatenate::

// FUNCTION public
//	Function::Concatenate::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID0_
//	int iInDataID1_
//	int iOutDataID_
//	
// RETURN
//	Concatenate*
//
// EXCEPTIONS

//static
Concatenate*
Concatenate::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID0_,
	   int iInDataID1_,
	   int iOutDataID_)
{
	// check operand's data type
	switch (cProgram_.getVariable(iInDataID0_)->getType()) {
	case Common::DataType::String:
		{
			return String::create(cProgram_,
								  pIterator_,
								  iInDataID0_,
								  iInDataID1_,
								  iOutDataID_);
		}
	case Common::DataType::Binary:
		{
			return Binary::create(cProgram_,
								  pIterator_,
								  iInDataID0_,
								  iInDataID1_,
								  iOutDataID_);
		}
	case Common::DataType::Array:
		{
			return Array::create(cProgram_,
								 pIterator_,
								 iInDataID0_,
								 iInDataID1_,
								 iOutDataID_);
		}
	default:
		{
			break;
		}
	}

	AUTOPOINTER<This> pResult = new ConcatenateImpl::AnyType(iInDataID0_,
															 iInDataID1_,
															 iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Function::Concatenate::

// FUNCTION public
//	Function::Concatenate::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Concatenate*
//
// EXCEPTIONS

//static
Concatenate*
Concatenate::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Concatenate:
		{
			return new ConcatenateImpl::String;
		}
	case Class::Category::ConcatenateBinary:
		{
			return new ConcatenateImpl::Binary;
		}
	case Class::Category::ConcatenateArray:
		{
			return new ConcatenateImpl::Array;
		}
	case Class::Category::ConcatenateAnyType:
		{
			return new ConcatenateImpl::AnyType;
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
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
