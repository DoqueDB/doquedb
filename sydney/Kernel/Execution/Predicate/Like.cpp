// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Like.cpp --
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

#include "Execution/Predicate/Like.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"

#include "Exception/ArbitraryElementNotAllowed.h"
#include "Exception/InvalidEscape.h"
#include "Exception/InvalidLike.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Utility/CharTrait.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName[] = {
		"like",		// isnot == false
		"not like",	// isnot == true
	};
}
namespace LikeImpl
{
	// CLASS local
	//	Execution::Predicate::LikeImpl::Base -- base class of implementation classes of Like
	//
	// NOTES
	class Base
		: public Predicate::Like
	{
	public:
		typedef Base This;
		typedef Predicate::Like Super;

		// constructor
		Base()
			: Super(),
			  m_cPattern(),
			  m_cEscape(),
			  m_cEscapeChar(Common::UnicodeChar::usNull),
			  m_bIsNot(false),
			  m_bIsFirst(true),
			  m_bIsUnknown(false)
		{}
		Base(int iPatternID_,
			 int iEscapeID_,
			 bool bIsNot_)
			: Super(),
			  m_cPattern(iPatternID_),
			  m_cEscape(iEscapeID_),
			  m_cEscapeChar(Common::UnicodeChar::usNull),
			  m_bIsNot(bIsNot_),
			  m_bIsFirst(true),
			  m_bIsUnknown(false)
		{}

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Predicate::Like::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);

	///////////////////////////////
	// Common::Externalizable

	///////////////////////////////
	// ModSerializer

	protected:
		// initialize like argument at first check
		void initializeArgument();
		// check result is known as unknown
		bool isUnknown() {return m_bIsUnknown;}
		// check a value
		Boolean::Value check(const Common::StringData* pData_);

		void initializeBase(Interface::IProgram& cProgram_);
		void terminateBase(Interface::IProgram& cProgram_);
		void serializeBase(ModArchive& archiver_);

	private:
		// explain left operand
		virtual void explainData(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_) = 0;

		void initializeEscapeChar();
		
		const Common::StringData* getPattern() {return m_cPattern.getData();}
		const Common::StringData* getEscape() {return m_cEscape.getData();}

		// data type of patten and escape are guaranteed by setExpectedType in analyzer
		Action::StringDataHolder m_cPattern;
		Action::StringDataHolder m_cEscape;
		ModUnicodeChar m_cEscapeChar;
		bool m_bIsNot;
		bool m_bIsFirst;
		bool m_bIsUnknown;
	};

	// CLASS local
	//	Execution::Predicate::LikeImpl::Normal -- implementation class of Like
	//
	// NOTES
	class Normal
		: public Base
	{
	public:
		typedef Normal This;
		typedef Base Super;

		// constructor
		Normal()
			: Super(),
			  m_cData()
		{}
		Normal(int iDataID0_,
			   int iDataID1_,
			   int iEscapeID_,
			   bool bIsNot_)
			: Super(iDataID1_, iEscapeID_, bIsNot_),
			  m_cData(iDataID0_)
		{}

		// destructor
		virtual ~Normal() {}

	///////////////////////////
	// Predicate::Like::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::
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
		const Common::StringData* getData();

	private:
	///////////////////////////////
	// LikeImpl::Base::
		virtual void explainData(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// operands
		Action::DataHolder m_cData;
	};

	// CLASS local
	//	Execution::Predicate::LikeImpl::AnyElement -- implementation class of Like
	//
	// NOTES
	class AnyElement
		: public Normal
	{
	public:
		typedef AnyElement This;
		typedef Normal Super;

		// constructor
		AnyElement()
			: Super()
		{}
		AnyElement(int iDataID0_,
				   int iDataID1_,
				   int iEscapeID_,
				   bool bIsNot_)
			: Super(-1, iDataID1_, iEscapeID_, bIsNot_),
			  m_cArray(iDataID0_)
		{}

		// destructor
		~AnyElement() {}

	///////////////////////////
	// Predicate::Like::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
		// accessor
		const Common::DataArrayData* getArray();

	private:
	///////////////////////////////
	// LikeImpl::Base::
		virtual void explainData(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		Action::DataHolder m_cArray;
	};
} // namespace LikeImpl

///////////////////////////////////////
// Predicate::LikeImpl::Base

// FUNCTION public
//	Predicate::LikeImpl::Base::explain -- 
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
LikeImpl::Base::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		explainData(pEnvironment_, cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszOperatorName[m_bIsNot?1:0]);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cPattern.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION protected
//	Predicate::LikeImpl::Base::initializeArgument -- initialize like argument at first check
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
LikeImpl::Base::
initializeArgument()
{
	if (m_bIsFirst) {
		if (m_cEscape.isValid()) {
			initializeEscapeChar();
			m_bIsUnknown = ((getPattern() == 0 || getPattern()->isNull())
							|| (getEscape() == 0 || getEscape()->isNull()));
		} else {
			m_bIsUnknown = getPattern() == 0 || getPattern()->isNull();
		}
		m_bIsFirst = false;
	}
}

// FUNCTION protected
//	Predicate::LikeImpl::Base::check -- check a value
//
// NOTES
//
// ARGUMENTS
//	const Common::StringData* pData_
//	
// RETURN
//	Like::Boolean::Value
//
// EXCEPTIONS

Like::Boolean::Value
LikeImpl::Base::
check(const Common::StringData* pData_)
{
	if (pData_->isNull()) {
		return Boolean::Unknown;
	}

	return _TRMEISTER::Utility::CharTrait::like_NotNull(
						pData_,
						getPattern(),
						static_cast<_TRMEISTER::Utility::CharTrait::NormalizingMethod::Value>(
							  Opt::Configuration::getLikeNormalizedString().get()),
						m_cEscapeChar) == m_bIsNot
		? Boolean::False : Boolean::True;
}

// FUNCTION protected
//	Predicate::LikeImpl::Base::initializeBase -- 
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

void
LikeImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	if (m_cPattern.isInitialized() == false) {
		Super::initializeBase(cProgram_);
		m_cPattern.initialize(cProgram_);

		if (m_cEscape.isValid()) {
			m_cEscape.initialize(cProgram_);
		}
	}
}

// FUNCTION protected
//	Predicate::LikeImpl::Base::terminateBase -- 
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

void
LikeImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	if (m_cPattern.isInitialized() == true) {
		Super::terminateBase(cProgram_);
		m_cPattern.terminate(cProgram_);
		m_cEscape.terminate(cProgram_);
		m_bIsFirst = false;
		m_bIsUnknown = false;
	}
}

// FUNCTION protected
//	Predicate::LikeImpl::Base::serializeBase -- 
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
LikeImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cPattern.serialize(archiver_);
	m_cEscape.serialize(archiver_);
	archiver_(m_bIsNot);
}

// FUNCTION private
//	Predicate::LikeImpl::Base::initializeEscapeChar -- 
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
LikeImpl::Base::
initializeEscapeChar()
{
	if (m_cEscape.isValid()
		&& getEscape()
		&& getEscape()->isNull() == false) {
		if (getEscape()->getLength() != 1) {
			// escape char must be 1-length string
			_SYDNEY_THROW0(Exception::InvalidEscape);
		}
		m_cEscapeChar = *static_cast<const ModUnicodeChar*>(getEscape()->getValue());
	}
}

///////////////////////////////////////
// Predicate::LikeImpl::Normal

// FUNCTION public
//	Predicate::LikeImpl::Normal::initialize -- 
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
LikeImpl::Normal::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::LikeImpl::Normal::terminate -- 
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
LikeImpl::Normal::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized()) {
		m_cData.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::LikeImpl::Normal::finish -- 
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
LikeImpl::Normal::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::LikeImpl::Normal::reset -- 
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
LikeImpl::Normal::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::LikeImpl::Normal::getClassID -- 
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
LikeImpl::Normal::
getClassID() const
{
	return Class::getClassID(Class::Category::Like);
}

// FUNCTION public
//	Predicate::LikeImpl::Normal::serialize -- for serialize
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
LikeImpl::Normal::
serialize(ModArchive& cArchive_)
{
	serializeBase(cArchive_);
	m_cData.serialize(cArchive_);
}

// FUNCTION protected
//	Predicate::LikeImpl::Normal::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::StringData*
//
// EXCEPTIONS

const Common::StringData*
LikeImpl::Normal::
getData()
{
	if (m_cData->getType() != Common::DataType::String) {
		_SYDNEY_THROW0(Exception::InvalidLike);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::StringData*, m_cData.getData());
}

// FUNCTION private
//	Predicate::LikeImpl::Normal::explainData -- 
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
LikeImpl::Normal::
explainData(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	m_cData.explain(cProgram_, cExplain_);
}

// FUNCTION private
//	Predicate::LikeImpl::Normal::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Like::Boolean::Value
//
// EXCEPTIONS

//virtual
Like::Boolean::Value
LikeImpl::Normal::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	initializeArgument();
	if (isUnknown()) {
		return Boolean::Unknown;
	}
	return check(getData());
}

/////////////////////////////////////////////////
// Predicate::LikeImpl::AnyElement

// FUNCTION public
//	Predicate::LikeImpl::AnyElement::initialize -- 
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
LikeImpl::AnyElement::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cArray.isInitialized() == false) {
		Super::initialize(cProgram_);
		m_cArray.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::LikeImpl::AnyElement::terminate -- 
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
LikeImpl::AnyElement::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cArray.isInitialized() == true) {
		Super::terminate(cProgram_);
		m_cArray.terminate(cProgram_);
	}
}

// FUNCTION public
//	Predicate::LikeImpl::AnyElement::getClassID -- 
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
LikeImpl::AnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::LikeAnyElement);
}

// FUNCTION public
//	Predicate::LikeImpl::AnyElement::serialize -- 
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

//virtual
void
LikeImpl::AnyElement::
serialize(ModArchive& archiver_)
{
	Super::serialize(archiver_);
	m_cArray.serialize(archiver_);
}

// FUNCTION protected
//	Predicate::LikeImpl::AnyElement::getArray -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	const Common::DataArrayData*
//
// EXCEPTIONS

const Common::DataArrayData*
LikeImpl::AnyElement::
getArray()
{
	if (m_cArray->getType() != Common::DataType::Array
		|| m_cArray->getElementType() != Common::DataType::Data) {
		_SYDNEY_THROW0(Exception::ArbitraryElementNotAllowed);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, m_cArray.getData());
}

// FUNCTION private
//	Predicate::LikeImpl::AnyElement::explainData -- 
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
LikeImpl::AnyElement::
explainData(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	m_cArray.explain(cProgram_, cExplain_);
}

// FUNCTION private
//	Predicate::LikeImpl::AnyElement::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Like::Boolean::Value
//
// EXCEPTIONS

//virtual
Like::Boolean::Value
LikeImpl::AnyElement::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	initializeArgument();
	if (isUnknown() || getArray()->isNull()) {
		return Boolean::Unknown;
	}
	Boolean::Value eResult = Boolean::False;
	int n = getArray()->getCount();
	for (int i = 0; i < n; ++i) {
		Common::Data::Pointer pElement = getArray()->getElement(i);
		if (pElement->getType() != Common::DataType::String) {
			_SYDNEY_THROW0(Exception::InvalidLike);
		}
		if (pElement->isNull() == false) {
			eResult = Boolean::boolOr(eResult,
									  check(_SYDNEY_DYNAMIC_CAST(Common::StringData*,
																 pElement.get())));
			if (eResult == Boolean::True) break;
		}
	}
	return eResult;
}

//////////////////////////////
// Predicate::Like::

// FUNCTION public
//	Predicate::Like::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	int iEscape_
//	bool bArbitraryElement_
//	bool bIsNot_
//	
// RETURN
//	Like*
//
// EXCEPTIONS

//static
Like*
Like::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   int iEscape_,
	   bool bArbitraryElement_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	if (bArbitraryElement_) {
		pResult = new LikeImpl::AnyElement(iDataID0_, iDataID1_, iEscape_, bIsNot_);
	} else {
		pResult = new LikeImpl::Normal(iDataID0_, iDataID1_, iEscape_, bIsNot_);
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Like::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Like*
//
// EXCEPTIONS

//static
Like*
Like::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Like:
		{
			return new LikeImpl::Normal;
		}
	case Class::Category::LikeAnyElement:
		{
			return new LikeImpl::AnyElement;
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
