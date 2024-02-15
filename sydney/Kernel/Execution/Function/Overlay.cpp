// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Overlay.cpp --
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

#include "Execution/Function/Overlay.h"
#include "Execution/Function/Class.h"
#include "Execution/Function/SubString.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotCompatible.h"
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
	const char* const _pszOperator = "overlay";

	// FUNCTION local
	//	$$$::_stringOverlay -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::StringData& cData0_
	//	const Common::StringData& cData1_
	//	int iStart_
	//	int iLength_
	//	Common::Data& cResult_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_stringOverlay(const Common::StringData& cData0_,
				   const Common::StringData& cData1_,
				   int iStart_,
				   int iLength_,
				   Common::Data& cResult_)
	{
		const ModUnicodeString& cstrValue0 = cData0_.getValue();
		const ModUnicodeString& cstrValue1 = cData1_.getValue();
		ModUnicodeString cstrResult;
		ModSize iLength0 = cstrValue0.getLength();

		// first part [0, start]
		int iFirstPartStart = 0;
		int iFirstPartLength = iStart_;
		SubString::checkArgument(&iFirstPartStart, &iFirstPartLength, iLength0);
		if (iFirstPartLength > 0) {
			cstrValue0.copy(cstrResult, iFirstPartStart, iFirstPartLength);
		}
		// second part (data1)
		cstrResult.append(cstrValue1);
		// last part [start+length, <end>]
		int iLastPartStart = iStart_ + iLength_;
		int iLastPartLength = Os::Limits<int>::getMax();
		SubString::checkArgument(&iLastPartStart, &iLastPartLength, iLength0);
		if (iLastPartLength > 0) {
			cstrResult.append(static_cast<const ModUnicodeChar*>(cstrValue0) + iLastPartStart,
							  iLastPartLength);
		}
		Common::StringData cResult(cstrResult);
		cResult_.assign(&cResult);
	}

	// FUNCTION local
	//	$$$::_binaryOverlay -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::BinaryData& cData0_
	//	const Common::BinaryData& cData1_
	//	int iStart_
	//	int iLength_
	//	Common::Data& cResult_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_binaryOverlay(const Common::BinaryData& cData0_,
				   const Common::BinaryData& cData1_,
				   int iStart_,
				   int iLength_,
				   Common::Data& cResult_)
	{
		const void* pValue0 = cData0_.getValue();
		const void* pValue1 = cData1_.getValue();
		unsigned int iSize0 = cData0_.getSize();
		unsigned int iSize1 = cData1_.getSize();

		Common::BinaryData cResult;

		// first part [0, start]
		int iFirstPartStart = 0;
		int iFirstPartLength = iStart_;
		SubString::checkArgument(&iFirstPartStart, &iFirstPartLength, iSize0);
		if (iFirstPartLength > 0) {
			cResult.connect(pValue0, iFirstPartLength);
		}
		// second part (data1)
		if (iSize1 > 0) {
			cResult.connect(pValue1, iSize1);
		}
		// last part [start+length, <end>]
		int iLastPartStart = iStart_ + iLength_;
		int iLastPartLength = Os::Limits<int>::getMax();
		SubString::checkArgument(&iLastPartStart, &iLastPartLength, iSize0);
		if (iLastPartLength > 0) {
			cResult.connect(reinterpret_cast<const char*>(pValue0) + iLastPartStart,
							iLastPartLength);
		}
		cResult_.assign(&cResult);
	}

	// FUNCTION local
	//	$$$::_arrayOverlay -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	const Common::DataArrayData& cData0_
	//	const Common::DataArrayData& cData1_
	//	int iStart_
	//	int iLength_
	//	Common::Data& cResult_
	//	
	// RETURN
	//	Nothing
	//
	// EXCEPTIONS

	void
	_arrayOverlay(const Common::DataArrayData& cData0_,
				  const Common::DataArrayData& cData1_,
				  int iStart_,
				  int iLength_,
				  Common::Data& cResult_)
	{
		const ModVector<Common::Data::Pointer>& vecValue0 = cData0_.getValue();
		const ModVector<Common::Data::Pointer>& vecValue1 = cData1_.getValue();
		ModSize iSize0 = vecValue0.getSize();
		ModSize iSize1 = vecValue1.getSize();

		ModVector<Common::Data::Pointer> vecResult;

		// first part [0, start]
		int iFirstPartStart = 0;
		int iFirstPartLength = iStart_;
		SubString::checkArgument(&iFirstPartStart, &iFirstPartLength, iSize0);
		if (iFirstPartLength > 0) {
			vecResult.insert(vecResult.end(),
							 vecValue0.begin(),
							 vecValue0.begin() + iFirstPartLength);
		}
		// second part (data1)
		if (iSize1 > 0) {
			vecResult.insert(vecResult.end(),
							 vecValue1.begin(), vecValue1.end());
		}
		// last part [start+length, <end>]
		int iLastPartStart = iStart_ + iLength_;
		int iLastPartLength = Os::Limits<int>::getMax();
		SubString::checkArgument(&iLastPartStart, &iLastPartLength, iSize0);
		if (iLastPartLength > 0) {
			vecResult.insert(vecResult.end(),
							 vecValue0.begin() + iLastPartStart,
							 vecValue0.end());
		}
		Common::DataArrayData cResult(vecResult);
		cResult_.assign(&cResult);
	}
}

namespace OverlayImpl
{
	// TEMPLATE CLASS local
	//	Execution::Function::OverlayImpl::Base -- base class of implementation class
	//
	// TEMPLATE ARGUMENTS
	//	class Holder_
	//
	// NOTES
	template <class Holder_>
	class Base
		: public Function::Overlay
	{
	public:
		typedef Base<Holder_> This;
		typedef Function::Overlay Super;

		Base()
			: Super(),
			  m_cData0(),
			  m_cData1(),
			  m_cStart(),
			  m_cLength(),
			  m_cOutData()
		{}
		Base(int iDataID0_,
			 int iDataID1_,
			 int iOptionID0_,
			 int iOptionID1_,
			 int iOutDataID_)
			: Super(),
			  m_cData0(iDataID0_),
			  m_cData1(iDataID1_),
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
		const Holder_& getData0() {return m_cData0;}
		const Holder_& getData1() {return m_cData1;}
		const Action::IntegerDataHolder& getStart() {return m_cStart;}
		const Action::IntegerDataHolder& getLength() {return m_cLength;}
		Action::DataHolder& getOutData() {return m_cOutData;}

	private:
		virtual void calculate(int iStart_) = 0;
		virtual void calculate(int iStart_, int iLength_) = 0;

		Holder_ m_cData0;
		Holder_ m_cData1;

		// data type of start and length are guaranteed by setExpectedType in analyzer
		Action::IntegerDataHolder m_cStart;
		Action::IntegerDataHolder m_cLength;
		Action::DataHolder m_cOutData;
	};

	// CLASS local
	//	Execution::Function::OverlayImpl::String -- implementation class of Overlay
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
		String(int iDataID0_,
			   int iDataID1_,
			   int iOptionID0_,
			   int iOptionID1_,
			   int iOutDataID_)
			: Super(iDataID0_, iDataID1_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_);
		virtual void calculate(int iStart_, int iLength_);
	};

	// CLASS local
	//	Execution::Function::OverlayImpl::Binary -- implementation class of Overlay
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
		Binary(int iDataID0_,
			   int iDataID1_,
			   int iOptionID0_,
			   int iOptionID1_,
			   int iOutDataID_)
			: Super(iDataID0_, iDataID1_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_);
		virtual void calculate(int iStart_, int iLength_);
	};

	// CLASS local
	//	Execution::Function::OverlayImpl::Array -- implementation class of Overlay
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
		Array(int iDataID0_,
			  int iDataID1_,
			  int iOptionID0_,
			  int iOptionID1_,
			  int iOutDataID_)
			: Super(iDataID0_, iDataID1_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_);
		virtual void calculate(int iStart_, int iLength_);
	};

	// CLASS local
	//	Execution::Function::OverlayImpl::AnyType -- implementation class of Overlay
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
		AnyType(int iDataID0_,
				int iDataID1_,
				int iOptionID0_,
				int iOptionID1_,
				int iOutDataID_)
			: Super(iDataID0_, iDataID1_, iOptionID0_, iOptionID1_, iOutDataID_)
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
		virtual void calculate(int iStart_);
		virtual void calculate(int iStart_, int iLength_);
	};
}

/////////////////////////////////////////////////
// Execution::Function::OverlayImpl::Base

// TEMPLATE FUNCTION public
//	Function::OverlayImpl::Base<Holder_>::explain -- 
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
OverlayImpl::Base<Holder_>::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperator);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cData0.explain(cProgram_, cExplain_);
		cExplain_.put(" placing ");
		m_cData1.explain(cProgram_, cExplain_);
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
//	Function::OverlayImpl::Base<Holder_>::initialize -- 
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
OverlayImpl::Base<Holder_>::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData0.isInitialized() == false) {
		m_cData0.initialize(cProgram_);
		m_cData1.initialize(cProgram_);
		m_cStart.initialize(cProgram_);
		m_cLength.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// TEMPLATE FUNCTION public
//	Function::OverlayImpl::Base<Holder_>::terminate -- 
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
OverlayImpl::Base<Holder_>::
terminate(Interface::IProgram& cProgram_)
{
	m_cData0.terminate(cProgram_);
	m_cData1.terminate(cProgram_);
	m_cStart.terminate(cProgram_);
	m_cLength.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// TEMPLATE FUNCTION public
//	Function::OverlayImpl::Base<Holder_>::execute -- 
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
OverlayImpl::Base<Holder_>::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getData0()->isNull()
			|| getData1()->isNull()
			|| getStart()->isNull()
			|| (getLength().isValid() && getLength()->isNull())) {
			getOutData()->setNull();
		} else if (getData0()->getType() != getData1()->getType()
				   || getData0()->getElementType() != getData1()->getElementType()) {
			_SYDNEY_THROW0(Exception::NotCompatible);
		} else {
			int iStart = getStart()->getValue() - 1; // 1-base to 0-base
			if (getLength().isValid()) {
				int iLength = getLength()->getValue();
				calculate(iStart, iLength);
			} else {
				calculate(iStart);
			}
		}
		done();
	}
	return Action::Status::Success;
}

// TEMPLATE FUNCTION public
//	Function::OverlayImpl::Base<Holder_>::finish -- 
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
OverlayImpl::Base<Holder_>::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// TEMPLATE FUNCTION public
//	Function::OverlayImpl::Base<Holder_>::reset -- 
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
OverlayImpl::Base<Holder_>::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// TEMPLATE FUNCTION public
//	Function::OverlayImpl::Base<Holder_>::serialize -- 
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
OverlayImpl::Base<Holder_>::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cData0.serialize(archiver_);
	m_cData1.serialize(archiver_);
	m_cStart.serialize(archiver_);
	m_cLength.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////////////////
// Execution::Function::OverlayImpl::String

// FUNCTION public
//	Function::OverlayImpl::String::getClassID -- 
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
OverlayImpl::String::
getClassID() const
{
	return Class::getClassID(Class::Category::Overlay);
}

// FUNCTION private
//	Function::OverlayImpl::String::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
OverlayImpl::String::
calculate(int iStart_)
{
	// use data1's length as the 2nd parameter
	calculate(iStart_, getData1()->getLength());
}

// FUNCTION private
//	Function::OverlayImpl::String::calculate -- 
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
OverlayImpl::String::
calculate(int iStart_,
		  int iLength_)
{
	_stringOverlay(*getData0(),
				   *getData1(),
				   iStart_,
				   iLength_,
				   *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::OverlayImpl::Binary

// FUNCTION public
//	Function::OverlayImpl::Binary::getClassID -- 
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
OverlayImpl::Binary::
getClassID() const
{
	return Class::getClassID(Class::Category::OverlayBinary);
}

// FUNCTION private
//	Function::OverlayImpl::Binary::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
OverlayImpl::Binary::
calculate(int iStart_)
{
	// use data1's length as the 2nd parameter
	calculate(iStart_, getData1()->getSize());
}

// FUNCTION private
//	Function::OverlayImpl::Binary::calculate -- 
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
OverlayImpl::Binary::
calculate(int iStart_,
		  int iLength_)
{
	_binaryOverlay(*getData0(),
				   *getData1(),
				   iStart_,
				   iLength_,
				   *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::OverlayImpl::Array

// FUNCTION public
//	Function::OverlayImpl::Array::getClassID -- 
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
OverlayImpl::Array::
getClassID() const
{
	return Class::getClassID(Class::Category::OverlayArray);
}

// FUNCTION private
//	Function::OverlayImpl::Array::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
OverlayImpl::Array::
calculate(int iStart_)
{
	// use data1's length as the 2nd parameter
	calculate(iStart_, getData1()->getCount());
}

// FUNCTION private
//	Function::OverlayImpl::Array::calculate -- 
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
OverlayImpl::Array::
calculate(int iStart_,
		  int iLength_)
{
	_arrayOverlay(*getData0(),
				  *getData1(),
				  iStart_,
				  iLength_,
				  *getOutData());
}

/////////////////////////////////////////////////
// Execution::Function::OverlayImpl::AnyType

// FUNCTION public
//	Function::OverlayImpl::AnyType::getClassID -- 
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
OverlayImpl::AnyType::
getClassID() const
{
	return Class::getClassID(Class::Category::OverlayAnyType);
}

// FUNCTION private
//	Function::OverlayImpl::AnyType::calculate -- 
//
// NOTES
//
// ARGUMENTS
//	int iStart_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
OverlayImpl::AnyType::
calculate(int iStart_)
{
	switch (getData0()->getType()) {
	case Common::DataType::String:
		{
			_stringOverlay(_SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												*getData0()),
						   _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												*getData1()),
						   iStart_,
						   _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												*getData1()).getLength(),
						   *getOutData());
			break;
		}
	case Common::DataType::Binary:
		{
			_binaryOverlay(_SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												*getData0()),
						   _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												*getData1()),
						   iStart_,
						   _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												*getData1()).getSize(),
						   *getOutData());
			break;
		}
	case Common::DataType::Array:
		{
			_arrayOverlay(_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
											   *getData0()),
						  _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
											   *getData1()),
						  iStart_,
						  _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
											   *getData1()).getCount(),
						  *getOutData());
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

// FUNCTION private
//	Function::OverlayImpl::AnyType::calculate -- 
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
OverlayImpl::AnyType::
calculate(int iStart_,
		  int iLength_)
{
	switch (getData0()->getType()) {
	case Common::DataType::String:
		{
			_stringOverlay(_SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												*getData0()),
						   _SYDNEY_DYNAMIC_CAST(const Common::StringData&,
												*getData1()),
						   iStart_,
						   iLength_,
						   *getOutData());
			break;
		}
	case Common::DataType::Binary:
		{
			_binaryOverlay(_SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												*getData0()),
						   _SYDNEY_DYNAMIC_CAST(const Common::BinaryData&,
												*getData1()),
						   iStart_,
						   iLength_,
						   *getOutData());
			break;
		}
	case Common::DataType::Array:
		{
			_arrayOverlay(_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
											   *getData0()),
						  _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData&,
											   *getData1()),
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
// Function::Overlay::String::

// FUNCTION public
//	Function::Overlay::String::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	Overlay*
//
// EXCEPTIONS

//static
Overlay*
Overlay::String::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new OverlayImpl::String(iDataID0_,
														iDataID1_,
														iOptionID0_,
														iOptionID1_,
														iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::Overlay::Binary::

// FUNCTION public
//	Function::Overlay::Binary::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	Overlay*
//
// EXCEPTIONS

//static
Overlay*
Overlay::Binary::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new OverlayImpl::Binary(iDataID0_,
														iDataID1_,
														iOptionID0_,
														iOptionID1_,
														iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::Overlay::Array::

// FUNCTION public
//	Function::Overlay::Array::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	Overlay*
//
// EXCEPTIONS

//static
Overlay*
Overlay::Array::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new OverlayImpl::Array(iDataID0_,
													   iDataID1_,
													   iOptionID0_,
													   iOptionID1_,
													   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

/////////////////////////////////////
// Function::Overlay::

// FUNCTION public
//	Function::Overlay::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	int iOptionID0_
//	int iOptionID1_
//	int iOutDataID_
//	
// RETURN
//	Overlay*
//
// EXCEPTIONS

//static
Overlay*
Overlay::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   int iOptionID0_,
	   int iOptionID1_,
	   int iOutDataID_)
{
	// check operand's data type
	switch (cProgram_.getVariable(iDataID0_)->getType()) {
	case Common::DataType::String:
		{
			return String::create(cProgram_,
								  pIterator_,
								  iDataID0_,
								  iDataID1_,
								  iOptionID0_,
								  iOptionID1_,
								  iOutDataID_);
		}
	case Common::DataType::Binary:
		{
			return Binary::create(cProgram_,
								  pIterator_,
								  iDataID0_,
								  iDataID1_,
								  iOptionID0_,
								  iOptionID1_,
								  iOutDataID_);
		}
	case Common::DataType::Array:
		{
			return Array::create(cProgram_,
								 pIterator_,
								 iDataID0_,
								 iDataID1_,
								 iOptionID0_,
								 iOptionID1_,
								 iOutDataID_);
		}
	default:
		{
			break;
		}
	}

	AUTOPOINTER<This> pResult = new OverlayImpl::AnyType(iDataID0_,
														 iDataID1_,
														 iOptionID0_,
														 iOptionID1_,
														 iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

//////////////////////////////
// Function::Overlay::

// FUNCTION public
//	Function::Overlay::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Overlay*
//
// EXCEPTIONS

//static
Overlay*
Overlay::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Overlay:
		{
			return new OverlayImpl::String;
		}
	case Class::Category::OverlayBinary:
		{
			return new OverlayImpl::Binary;
		}
	case Class::Category::OverlayArray:
		{
			return new OverlayImpl::Array;
		}
	case Class::Category::OverlayAnyType:
		{
			return new OverlayImpl::AnyType;
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
