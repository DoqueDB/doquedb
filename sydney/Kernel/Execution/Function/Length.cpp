// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Length.cpp --
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Execution/Function/Length.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	struct _Type
	{
		enum Value {
			CharLength,
			OctetLength,
			ValueNum
		};
	};

	// CONST
	//	_pszOperator -- operator name for explain
	//
	// NOTES
	const char* const _pszOperator[] = {
		"char_length",
		"octet_length"
	};
}

namespace LengthImpl
{
	// CLASS local
	//	Execution::Function::LengthImpl::Base -- base class of implementation class
	//
	// NOTES
	class Base
		: public Function::Length
	{
	public:
		typedef Base This;
		typedef Function::Length Super;

		Base()
			: Super(),
			  m_cInData(),
			  m_cOutData()
		{}
		Base(int iInDataID_,
			 int iOutDataID_)
			: Super(),
			  m_cInData(iInDataID_),
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
		const Common::Data* getInData() {return m_cInData.getData();}
		Common::UnsignedIntegerData* getOutData() {return m_cOutData.get();}

	private:
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_) = 0;
		virtual unsigned int calculate(const Common::Data& cData_) = 0;

		Action::DataHolder m_cInData;

		// out data type is set at createDataType in plan
		Action::UnsignedIntegerDataHolder m_cOutData;
	};

	// CLASS local
	//	Execution::Function::LengthImpl::Char -- implementation class of Length
	//
	// NOTES
	class Char
		: public Base
	{
	public:
		typedef Char This;
		typedef Base Super;

		Char()
			: Super()
		{}
		Char(int iInDataID_,
			 int iOutDataID_)
			: Super(iInDataID_, iOutDataID_)
		{}
		~Char()
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
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual unsigned int calculate(const Common::Data& cData_);
	};

	// CLASS local
	//	Execution::Function::LengthImpl::Octet -- implementation class of Length
	//
	// NOTES
	class Octet
		: public Base
	{
	public:
		typedef Octet This;
		typedef Base Super;

		Octet()
			: Super()
		{}
		Octet(int iInDataID_,
			  int iOutDataID_)
			: Super(iInDataID_, iOutDataID_)
		{}
		~Octet()
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
		virtual void explainThis(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
		virtual unsigned int calculate(const Common::Data& cData_);
	};
}

/////////////////////////////////////////////////
// Execution::Function::LengthImpl::Base

// FUNCTION public
//	Function::LengthImpl::Base::explain -- 
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
void
LengthImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	explainThis(pEnvironment_, cProgram_, cExplain_);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put("(");
		m_cInData.explain(cProgram_, cExplain_);
		cExplain_.put(") to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::LengthImpl::Base::initialize -- 
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
LengthImpl::Base::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData.isInitialized() == false) {
		m_cInData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::LengthImpl::Base::terminate -- 
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
LengthImpl::Base::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::LengthImpl::Base::execute -- 
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
LengthImpl::Base::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getInData()->isNull()) {
			getOutData()->setNull();
		} else {
			unsigned int iLength = 0;
			switch (getInData()->getType()) {
			case Common::DataType::Array:
				{
					; _SYDNEY_ASSERT(getInData()->getElementType() == Common::DataType::Data);
					const Common::DataArrayData* pArray =
						_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, getInData());
					for (int i = 0; i < pArray->getCount(); ++i) {
						iLength += calculate(*pArray->getElement(i));
					}
					break;
				}
			default:
				{
					iLength = calculate(*getInData());
					break;
				}
			}
			getOutData()->setValue(iLength);
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::LengthImpl::Base::finish -- 
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
LengthImpl::Base::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::LengthImpl::Base::reset -- 
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
LengthImpl::Base::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::LengthImpl::Base::serialize -- 
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
LengthImpl::Base::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cInData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////////////////
// Execution::Function::LengthImpl::Char

// FUNCTION public
//	Function::LengthImpl::Char::getClassID -- 
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
LengthImpl::Char::
getClassID() const
{
	return Class::getClassID(Class::Category::CharLength);
}

// FUNCTION private
//	Function::LengthImpl::Char::explainThis -- 
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
void
LengthImpl::Char::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperator[_Type::CharLength]);
}

// FUNCTION private
//	Function::LengthImpl::Char::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	unsigned int
//
// EXCEPTIONS

//virtual
unsigned int
LengthImpl::Char::
calculate(const Common::Data& cData_)
{
	switch (cData_.getType()) {
	case Common::DataType::String:
		{
			return _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_).getLength();
		}
	case Common::DataType::Binary:
		{
			return _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, cData_).getSize();
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

/////////////////////////////////////////////////
// Execution::Function::LengthImpl::Octet

// FUNCTION public
//	Function::LengthImpl::Octet::getClassID -- 
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
LengthImpl::Octet::
getClassID() const
{
	return Class::getClassID(Class::Category::OctetLength);
}

// FUNCTION private
//	Function::LengthImpl::Octet::explainThis -- 
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
void
LengthImpl::Octet::
explainThis(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperator[_Type::OctetLength]);
}

// FUNCTION private
//	Function::LengthImpl::Octet::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data& cData_
//	
// RETURN
//	unsigned int
//
// EXCEPTIONS

//virtual
unsigned int
LengthImpl::Octet::
calculate(const Common::Data& cData_)
{
	switch (cData_.getType()) {
	case Common::DataType::String:
		{
			return _SYDNEY_DYNAMIC_CAST(const Common::StringData&, cData_).getLength() * 2;
		}
	case Common::DataType::Binary:
		{
			return _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&, cData_).getSize();
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

/////////////////////////////////////
// Function::Length::Char::

// FUNCTION public
//	Function::Length::Char::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID_
//	int iOutDataID_
//	
// RETURN
//	Length*
//
// EXCEPTIONS

//static
Length*
Length::Char::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new LengthImpl::Char(iInDataID_,
													 iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::Length::Octet::

// FUNCTION public
//	Function::Length::Octet::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID_
//	int iOutDataID_
//	
// RETURN
//	Length*
//
// EXCEPTIONS

//static
Length*
Length::Octet::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new LengthImpl::Octet(iInDataID_,
													  iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Function::Length::

// FUNCTION public
//	Function::Length::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Length*
//
// EXCEPTIONS

//static
Length*
Length::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::CharLength:
		{
			return new LengthImpl::Char;
		}
	case Class::Category::OctetLength:
		{
			return new LengthImpl::Octet;
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
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
