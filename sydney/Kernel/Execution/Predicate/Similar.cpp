// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Similar.cpp --
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

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/Similar.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/UnicodeString.h"

#include "Exception/ArbitraryElementNotAllowed.h"
#include "Exception/InvalidRegularExpression.h"
#include "Exception/InvalidSimilar.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Explain.h"

#include "Utility/CharTrait.h"

#include "ModUnicodeRegularExpression.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName[] = {
		"similar",		// isnot == false
		"not similar",	// isnot == true
	};
}
namespace SimilarImpl
{
	// CLASS local
	//	Execution::Predicate::SimilarImpl::Base -- base class of implementation classes of Similar
	//
	// NOTES
	class Base
		: public Predicate::Similar
	{
	public:
		typedef Base This;
		typedef Predicate::Similar Super;

		// constructor
		Base()
			: Super(),
			  m_cPattern(),
			  m_pCompiledPattern(0),
			  m_bIsNot(false),
			  m_bIsUnknown(false)
		{}
		Base(int iPatternID_,
			 bool bIsNot_)
			: Super(),
			  m_cPattern(iPatternID_),
			  m_pCompiledPattern(0),
			  m_bIsNot(bIsNot_),
			  m_bIsUnknown(false)
		{}

		// destructor
		virtual ~Base() {}

	///////////////////////////
	// Predicate::Similar::

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

		// compile pattern
		void compilePattern();
		// delete compiled pattern
		void erasePattern();

		// accessor
		const Common::StringData* getPattern() {return m_cPattern.getData();}

		// data type of pattern is guaranteed by setExpectedType in analyzer
		Action::StringDataHolder m_cPattern;
		ModUnicodeRegularExpression* m_pCompiledPattern;
		bool m_bIsNot;
		bool m_bIsUnknown;
	};

	// CLASS local
	//	Execution::Predicate::SimilarImpl::Normal -- implementation class of Similar
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
			   bool bIsNot_)
			: Super(iDataID1_, bIsNot_),
			  m_cData(iDataID0_)
		{}

		// destructor
		virtual ~Normal() {}

	///////////////////////////
	// Predicate::Similar::

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
	// SimilarImpl::Base::
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
	//	Execution::Predicate::SimilarImpl::AnyElement -- implementation class of Similar
	//
	// NOTES
	class AnyElement
		: public Base
	{
	public:
		typedef AnyElement This;
		typedef Base Super;

		// constructor
		AnyElement()
			: Super()
		{}
		AnyElement(int iDataID0_,
				   int iDataID1_,
				   bool bIsNot_)
			: Super(iDataID1_, bIsNot_),
			  m_cArray(iDataID0_)
		{}

		// destructor
		~AnyElement() {}

	///////////////////////////
	// Predicate::Similar::

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
	private:
	///////////////////////////////
	// SimilarImpl::Base::
		virtual void explainData(Opt::Environment* pEnvironment_,
								 Interface::IProgram& cProgram_,
								 Opt::Explain& cExplain_);
	///////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// accessor
		const Common::DataArrayData* getArray();

		Action::DataHolder m_cArray;
	};
} // namespace SimilarImpl

///////////////////////////////////////
// Predicate::SimilarImpl::Base

// FUNCTION public
//	Predicate::SimilarImpl::Base::explain -- 
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
SimilarImpl::Base::
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
//	Predicate::SimilarImpl::Base::check -- check a value
//
// NOTES
//
// ARGUMENTS
//	const Common::StringData* pData_
//	
// RETURN
//	Similar::Boolean::Value
//
// EXCEPTIONS

Similar::Boolean::Value
SimilarImpl::Base::
check(const Common::StringData* pData_)
{
	if (pData_->isNull()) {
		return Boolean::Unknown;
	}

	return pData_->similar_NotNull(m_pCompiledPattern) == m_bIsNot
		? Boolean::False : Boolean::True;
}

// FUNCTION protected
//	Predicate::SimilarImpl::Base::initializeBase -- 
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
SimilarImpl::Base::
initializeBase(Interface::IProgram& cProgram_)
{
	if (m_cPattern.isInitialized() == false) {
		m_cPattern.initialize(cProgram_);
		Super::initializeBase(cProgram_);
		m_bIsUnknown = getPattern()->isNull();
		if (m_bIsUnknown == false) {
			compilePattern();
		}
	}
}

// FUNCTION protected
//	Predicate::SimilarImpl::Base::terminateBase -- 
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
SimilarImpl::Base::
terminateBase(Interface::IProgram& cProgram_)
{
	if (m_cPattern.isInitialized() == true) {
		erasePattern();
		Super::terminateBase(cProgram_);
		m_cPattern.terminate(cProgram_);
		m_bIsUnknown = false;
	}
}

// FUNCTION protected
//	Predicate::SimilarImpl::Base::serializeBase -- 
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
SimilarImpl::Base::
serializeBase(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cPattern.serialize(archiver_);
	archiver_(m_bIsNot);
}

// FUNCTION private
//	Predicate::SimilarImpl::Base::compilePattern -- compile pattern
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
SimilarImpl::Base::
compilePattern()
{
	try {
		m_pCompiledPattern = new ModUnicodeRegularExpression;
		m_pCompiledPattern->compile(getPattern()->getValue());
	} catch (ModException& e) {
		if (e.getErrorNumber() == ModCommonErrorBadArgument) {
			// invalid similar pattern
			_SYDNEY_THROW0(Exception::InvalidRegularExpression);
		}
		_SYDNEY_RETHROW;
	}
}

// FUNCTION private
//	Predicate::SimilarImpl::Base::erasePattern -- delete compiled pattern
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
SimilarImpl::Base::
erasePattern()
{
	if (m_pCompiledPattern) {
		delete m_pCompiledPattern, m_pCompiledPattern = 0;
	}
}

///////////////////////////////////////
// Predicate::SimilarImpl::Normal

// FUNCTION public
//	Predicate::SimilarImpl::Normal::initialize -- 
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
SimilarImpl::Normal::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		m_cData.initialize(cProgram_);
		initializeBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::SimilarImpl::Normal::terminate -- 
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
SimilarImpl::Normal::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized()) {
		terminateBase(cProgram_);
		m_cData.terminate(cProgram_);
	}
}

// FUNCTION public
//	Predicate::SimilarImpl::Normal::finish -- 
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
SimilarImpl::Normal::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::SimilarImpl::Normal::reset -- 
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
SimilarImpl::Normal::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::SimilarImpl::Normal::getClassID -- 
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
SimilarImpl::Normal::
getClassID() const
{
	return Class::getClassID(Class::Category::Similar);
}

// FUNCTION public
//	Predicate::SimilarImpl::Normal::serialize -- for serialize
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
SimilarImpl::Normal::
serialize(ModArchive& cArchive_)
{
	serializeBase(cArchive_);
	m_cData.serialize(cArchive_);
}

// FUNCTION protected
//	Predicate::SimilarImpl::Normal::getData -- 
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
SimilarImpl::Normal::
getData()
{
	if (m_cData->getType() != Common::DataType::String) {
		_SYDNEY_THROW0(Exception::InvalidSimilar);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::StringData*, m_cData.getData());
}

// FUNCTION private
//	Predicate::SimilarImpl::Normal::explainData -- 
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
SimilarImpl::Normal::
explainData(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	m_cData.explain(cProgram_, cExplain_);
}

// FUNCTION private
//	Predicate::SimilarImpl::Normal::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Similar::Boolean::Value
//
// EXCEPTIONS

//virtual
Similar::Boolean::Value
SimilarImpl::Normal::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	if (isUnknown()) {
		return Boolean::Unknown;
	}
	return check(getData());
}

/////////////////////////////////////////////////
// Predicate::SimilarImpl::AnyElement

// FUNCTION public
//	Predicate::SimilarImpl::AnyElement::initialize -- 
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
SimilarImpl::AnyElement::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cArray.isInitialized() == false) {
		m_cArray.initialize(cProgram_);
		initializeBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::SimilarImpl::AnyElement::terminate -- 
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
SimilarImpl::AnyElement::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cArray.isInitialized() == true) {
		terminateBase(cProgram_);
		m_cArray.terminate(cProgram_);
	}
}

// FUNCTION public
//	Predicate::SimilarImpl::AnyElement::finish -- 
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
SimilarImpl::AnyElement::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::SimilarImpl::AnyElement::reset -- 
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
SimilarImpl::AnyElement::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::SimilarImpl::AnyElement::getClassID -- 
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
SimilarImpl::AnyElement::
getClassID() const
{
	return Class::getClassID(Class::Category::SimilarAnyElement);
}

// FUNCTION public
//	Predicate::SimilarImpl::AnyElement::serialize -- 
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
SimilarImpl::AnyElement::
serialize(ModArchive& archiver_)
{
	serializeBase(archiver_);
	m_cArray.serialize(archiver_);
}

// FUNCTION private
//	Predicate::SimilarImpl::AnyElement::explainData -- 
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
SimilarImpl::AnyElement::
explainData(Opt::Environment* pEnvironment_,
			Interface::IProgram& cProgram_,
			Opt::Explain& cExplain_)
{
	m_cArray.explain(cProgram_, cExplain_);
}

// FUNCTION private
//	Predicate::SimilarImpl::AnyElement::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Similar::Boolean::Value
//
// EXCEPTIONS

//virtual
Similar::Boolean::Value
SimilarImpl::AnyElement::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	if (isUnknown() || getArray()->isNull()) {
		return Boolean::Unknown;
	}
	Boolean::Value eResult = Boolean::False;
	int n = getArray()->getCount();
	for (int i = 0; i < n; ++i) {
		Common::Data::Pointer pElement = getArray()->getElement(i);
		if (pElement->getType() != Common::DataType::String) {
			_SYDNEY_THROW0(Exception::InvalidSimilar);
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

// FUNCTION protected
//	Predicate::SimilarImpl::AnyElement::getArray -- 
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
SimilarImpl::AnyElement::
getArray()
{
	if (m_cArray->getType() != Common::DataType::Array
		|| m_cArray->getElementType() != Common::DataType::Data) {
		_SYDNEY_THROW0(Exception::ArbitraryElementNotAllowed);
	}
	return _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, m_cArray.getData());
}

//////////////////////////////
// Predicate::Similar::

// FUNCTION public
//	Predicate::Similar::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID0_
//	int iDataID1_
//	bool bArbitraryElement_
//	bool bIsNot_
//	
// RETURN
//	Similar*
//
// EXCEPTIONS

//static
Similar*
Similar::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID0_,
	   int iDataID1_,
	   bool bArbitraryElement_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	if (bArbitraryElement_) {
		pResult = new SimilarImpl::AnyElement(iDataID0_, iDataID1_, bIsNot_);
	} else {
		pResult = new SimilarImpl::Normal(iDataID0_, iDataID1_, bIsNot_);
	}
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Similar::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Similar*
//
// EXCEPTIONS

//static
Similar*
Similar::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Similar:
		{
			return new SimilarImpl::Normal;
		}
	case Class::Category::SimilarAnyElement:
		{
			return new SimilarImpl::AnyElement;
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
