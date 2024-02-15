// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/SubString.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyReinterpretCast.h"

#include "Execution/Function/SubString.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/SubStringError.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Os/Limits.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszOperator -- operator name for explain
	//
	// NOTES
	const char* const _pszOperator = "substring";

	// FUNCTION local
	//	$$$::_stringSubString -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::StringData& cData_
	//	int iStart_
	//	int iLength_
	//	Common::Data& cResult_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_stringSubString(const Common::StringData& cData_,
					 int iStart_,
					 int iLength_,
					 Common::Data& cResult_)
	{
		const ModUnicodeString& cstrValue = cData_.getValue();
		SubString::checkArgument(&iStart_, &iLength_, cstrValue.getLength());

		ModUnicodeString cstrResult;
		if (iLength_ > 0) {
			cstrValue.copy(cstrResult, iStart_, iLength_);
		}
		Common::StringData cResult(cstrResult);
		cResult_.assign(&cResult);
	}

	// FUNCTION local
	//	$$$::_binarySubString -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::BinaryData& cData_
	//	int iStart_
	//	int iLength_
	//	Common::Data& cResult_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_binarySubString(const Common::BinaryData& cData_,
					 int iStart_,
					 int iLength_,
					 Common::Data& cResult_)
	{
		SubString::checkArgument(&iStart_, &iLength_, cData_.getSize());
		Common::BinaryData cResult;

		if (iLength_ > 0) {
			cResult.setValue(syd_reinterpret_cast<const char*>(cData_.getValue()) + iStart_,
							 iLength_);
		} else {
			cResult.setValue(0, 0);
		}
		cResult_.assign(&cResult);
	}

	// FUNCTION local
	//	$$$::_arraySubString -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::DataArrayData& cData_
	//	int iStart_
	//	int iLength_
	//	Common::Data& cResult_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_arraySubString(const Common::DataArrayData& cData_,
					int iStart_,
					int iLength_,
					Common::Data& cResult_)
	{
		const ModVector<Common::Data::Pointer>& vecValue = cData_.getValue();
		SubString::checkArgument(&iStart_, &iLength_, vecValue.getSize());

		ModVector<Common::Data::Pointer> vecResult;
		if (iLength_ > 0) {
			vecResult.assign(vecValue.begin() + iStart_,
							 vecValue.begin() + iStart_ + iLength_);
		}
		Common::DataArrayData cResult(vecResult);
		cResult_.assign(&cResult);
	}
}

namespace SubStringImpl
{
	// TEMPLATE CLASS local
	//	Execution::Function::SubStringImpl::Base -- base class of implementation class
	//
	// TEMPLATE ARGUMENTS
	//	class Holder_
	//
	// NOTES
	template <class Holder_>
	class Base
		: public Function::SubString
	{
	public:
		typedef Base<Holder_> This;
		typedef Function::SubString Super;

		Base()
			: Super(),
			  m_cData(),
			  m_cStart(),
			  m_cLength(),
			  m_cOutData()
		{}
		Base(int iDataID_,
			 int iOptionID0_,
			 int iOptionID1_,
			 int iOutDataID_)
			: Super(),
			  m_cData(iDataID_),
			  m_cStart(iOptionID0_),
			  m_cLength(iOptionID1_),
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
		const Holder_& getData() {return m_cData;}
		const Action::IntegerDataHolder& getStart() {return m_cStart;}
		const Action::IntegerDataHolder& getLength() {return m_cLength;}
		Action::DataHolder& getOutData() {return m_cOutData;}

	private:
		virtual void calculate(int iStart_, int iLength_) = 0;

		Holder_ m_cData;
		Action::IntegerDataHolder m_cStart;
		Action::IntegerDataHolder m_cLength;
		Action::DataHolder m_cOutData;
	};

	// CLASS local
	//	Execution::Function::SubStringImpl::String -- implementation class of SubString
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
		String(int iDataID_,
			   int iOptionID0_,
			   int iOptionID1_,
			   int iOutDataID_)
			: Super(iDataID_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_, int iLength_);
	};

	// CLASS local
	//	Execution::Function::SubStringImpl::Binary -- implementation class of SubString
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
		Binary(int iDataID_,
			   int iOptionID0_,
			   int iOptionID1_,
			   int iOutDataID_)
			: Super(iDataID_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_, int iLength_);
	};

	// CLASS local
	//	Execution::Function::SubStringImpl::Array -- implementation class of SubString
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
		Array(int iDataID_,
			  int iOptionID0_,
			  int iOptionID1_,
			  int iOutDataID_)
			: Super(iDataID_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_, int iLength_);
	};

	// CLASS local
	//	Execution::Function::SubStringImpl::AnyType -- implementation class of SubString
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
		AnyType(int iDataID_,
				int iOptionID0_,
				int iOptionID1_,
				int iOutDataID_)
			: Super(iDataID_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_, int iLength_);
	};
}

/////////////////////////////////////////////////
// Execution::Function::SubStringImpl::Base

// TEMPLATE FUNCTION public
//	Function::SubStringImpl::Base<Holder_>::explain -- 
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
SubStringImpl::Base<Holder_>::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperator);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.put(" from ");
		m_cStart.explain(cProgram_, cExplain_);
		if (m_cLength.isValid()) {
			cExplain_.put(" for ");
			m_cLength.explain(cProgram_, cExplain_);
		}
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// TEMPLATE FUNCTION public
//	Function::SubStringImpl::Base<Holder_>::initialize -- 
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
SubStringImpl::Base<Holder_>::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		m_cData.initialize(cProgram_);
		m_cStart.initialize(cProgram_);
		m_cLength.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// TEMPLATE FUNCTION public
//	Function::SubStringImpl::Base<Holder_>::terminate -- 
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
SubStringImpl::Base<Holder_>::
terminate(Interface::IProgram& cProgram_)
{
	m_cData.terminate(cProgram_);
	m_cStart.terminate(cProgram_);
	m_cLength.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// TEMPLATE FUNCTION public
//	Function::SubStringImpl::Base<Holder_>::execute -- 
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
SubStringImpl::Base<Holder_>::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getData()->isNull()
			|| getStart()->isNull()
			|| (getLength().isValid() && getLength()->isNull())) {
			getOutData()->setNull();
		} else {
			int iStart = getStart()->getValue() - 1; // 1-base to 0-base
			int iLength = getLength().isValid()
				? getLength()->getValue()
				: Os::Limits<int>::getMax();
			calculate(iStart, iLength);
		}
		done();
	}
	return Action::Status::Success;
}

// TEMPLATE FUNCTION public
//	Function::SubStringImpl::Base<Holder_>::finish -- 
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
SubStringImpl::Base<Holder_>::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// TEMPLATE FUNCTION public
//	Function::SubStringImpl::Base<Holder_>::reset -- 
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
SubStringImpl::Base<Holder_>::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// TEMPLATE FUNCTION public
//	Function::SubStringImpl::Base<Holder_>::serialize -- 
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
SubStringImpl::Base<Holder_>::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cData.serialize(archiver_);
	m_cStart.serialize(archiver_);
	m_cLength.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////////////////
// Execution::Function::SubStringImpl::String

// FUNCTION public
//	Function::SubStringImpl::String::getClassID -- 
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
SubStringImpl::String::
getClassID() const
{
	return Class::getClassID(Class::Category::SubString);
}

// FUNCTION private
//	Function::SubStringImpl::String::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	int iLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
SubStringImpl::String::
calculate(int iStart_, int iLength_)
{
	_stringSubString(*getData(),
					 iStart_,
					 iLength_,
					 *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::SubStringImpl::Binary

// FUNCTION public
//	Function::SubStringImpl::Binary::getClassID -- 
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
SubStringImpl::Binary::
getClassID() const
{
	return Class::getClassID(Class::Category::SubStringBinary);
}

// FUNCTION private
//	Function::SubStringImpl::Binary::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	int iLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
SubStringImpl::Binary::
calculate(int iStart_, int iLength_)
{
	_binarySubString(*getData(),
					 iStart_,
					 iLength_,
					 *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::SubStringImpl::Array

// FUNCTION public
//	Function::SubStringImpl::Array::getClassID -- 
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
SubStringImpl::Array::
getClassID() const
{
	return Class::getClassID(Class::Category::SubStringArray);
}

// FUNCTION private
//	Function::SubStringImpl::Array::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	int iLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
SubStringImpl::Array::
calculate(int iStart_,
		  int iLength_)
{
	_arraySubString(*getData(),
					iStart_,
					iLength_,
					*getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::SubStringImpl::AnyType

// FUNCTION public
//	Function::SubStringImpl::AnyType::getClassID -- 
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
SubStringImpl::AnyType::
getClassID() const
{
	return Class::getClassID(Class::Category::SubStringAnyType);
}

// FUNCTION private
//	Function::SubStringImpl::AnyType::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	int iLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
SubStringImpl::AnyType::
calculate(int iStart_,
		  int iLength_)
{
	switch (getData()->getType()) {
	case Common::DataType::String:
		{
			_stringSubString(_SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												  *getData()),
							 iStart_,
							 iLength_,
							 *getOutData());
			break;
		}
	case Common::DataType::Binary:
		{
			_binarySubString(_SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												  *getData()),
							 iStart_,
							 iLength_,
							 *getOutData());
			break;
		}
	case Common::DataType::Array:
		{
			_arraySubString(_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
												 *getData()),
							iStart_,
							iLength_,
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
// Function::SubString::String::

// FUNCTION public
//	Function::SubString::String::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	SubString*
//
// EXCEPTIONS

//static
SubString*
SubString::String::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new SubStringImpl::String(iDataID_,
														  iOptionID0_,
														  iOptionID1_,
														  iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::SubString::Binary::

// FUNCTION public
//	Function::SubString::Binary::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	SubString*
//
// EXCEPTIONS

//static
SubString*
SubString::Binary::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new SubStringImpl::Binary(iDataID_,
														  iOptionID0_,
														  iOptionID1_,
														  iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::SubString::Array::

// FUNCTION public
//	Function::SubString::Array::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	SubString*
//
// EXCEPTIONS

//static
SubString*
SubString::Array::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new SubStringImpl::Array(iDataID_,
														 iOptionID0_,
														 iOptionID1_,
														 iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::SubString::

// FUNCTION public
//	Function::SubString::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	SubString*
//
// EXCEPTIONS

//static
SubString*
SubString::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	// check operand's data type
	switch (cProgram_.getVariable(iDataID_)->getType()) {
	case Common::DataType::String:
		{
			return String::create(cProgram_,
								  pIterator_,
								  iDataID_,
								  iOptionID0_,
								  iOptionID1_,
								  iOutDataID_);
		}
	case Common::DataType::Binary:
		{
			return Binary::create(cProgram_,
								  pIterator_,
								  iDataID_,
								  iOptionID0_,
								  iOptionID1_,
								  iOutDataID_);
		}
	case Common::DataType::Array:
		{
			return Array::create(cProgram_,
								 pIterator_,
								 iDataID_,
								 iOptionID0_,
								 iOptionID1_,
								 iOutDataID_);
		}
	default:
		{
			break;
		}
	}

	AUTOPOINTER<This> pResult = new SubStringImpl::AnyType(iDataID_,
														   iOptionID0_,
														   iOptionID1_,
														   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Function::SubString::

// FUNCTION public
//	Function::SubString::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	SubString*
//
// EXCEPTIONS

//static
SubString*
SubString::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::SubString:
		{
			return new SubStringImpl::String;
		}
	case Class::Category::SubStringBinary:
		{
			return new SubStringImpl::Binary;
		}
	case Class::Category::SubStringArray:
		{
			return new SubStringImpl::Array;
		}
	case Class::Category::SubStringAnyType:
		{
			return new SubStringImpl::AnyType;
		}
	default:
		{
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

// FUNCTION public
//	Function::checkArgument -- check substring argument (commonly used with overlay)
//
// NOTES
//
// ARGUMENTS
//	int* piStart_
//	int* piLength_
//	int iMaxLength_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
SubString::
checkArgument(int* piStart_,
			  int* piLength_,
			  int iMaxLength_)
{
	if (*piLength_ < 0) {
		// illegal argument for substring
		_SYDNEY_THROW0(Exception::SubStringError);
	}

	if (*piStart_ >= iMaxLength_) {
		*piStart_ = *piLength_ = 0;

	} else {
		int iEnd = *piStart_ + MIN(iMaxLength_ - *piStart_, *piLength_);
		int iStart1 = MAX(*piStart_, 0);
		int iEnd1 = MIN(iEnd, iMaxLength_);
		*piLength_ = MAX(iEnd1 - iStart1, 0);
		*piStart_ = iStart1;
	}
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
