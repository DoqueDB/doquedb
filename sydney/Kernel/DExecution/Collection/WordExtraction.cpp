// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/WordExtraction.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution::Collection";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "DExecution/Action/Fulltext.h"
#include "DExecution/Collection/WordExtraction.h"
#include "DExecution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataType.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"
#include "Common/LanguageData.h"

#include "Exception/Unexpected.h"
#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"


_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_COLLECTION_BEGIN

namespace
{
	const ModLanguageSet cLangJa = ModLanguageSet();
}
namespace Impl
{

	// CLASS
	//	Execution::Collection::Impl::WordExtractionImpl -- implementation class for collection::word extraction
	//
	// NOTES

	class WordExtractionImpl
		: public Collection::WordExtraction
	{
	public:
		typedef Collection::WordExtraction Super;
		typedef WordExtractionImpl This;

		// constructor
		WordExtractionImpl(int iFulltextID_)
			: Super(),
			  m_cGet(),
			  m_cPut(),
			  m_cFulltext(iFulltextID_)
		{}

		// constructor
		WordExtractionImpl()
			: Super(),
			  m_cGet(),
			  m_cPut(),
			  m_cFulltext()
		{}


		// destructor
		~WordExtractionImpl()
		{}

		// CLASS
		//	WordExtractionImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(WordExtractionImpl* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Execution::Interface::IProgram& cProgram_);
			virtual void terminate(Execution::Interface::IProgram& cProgram_);
			virtual bool putData(Execution::Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Execution::Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
			virtual void shift(Execution::Interface::IProgram& cProgram_);
		protected:
		private:
			WordExtractionImpl* m_pOuter;
		};

		// CLASS
		//	WordExtractionImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0) {}
			~GetImpl() {}

			void setOuter(WordExtractionImpl* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Get
			virtual void finish(Execution::Interface::IProgram& cProgram_);
			virtual void terminate(Execution::Interface::IProgram& cProgram_);
			virtual bool getData(Execution::Interface::IProgram& cProgram_,
								 Common::Data* pData_);
			virtual bool get(Execution::Interface::IProgram& cProgram_,
							 Common::Externalizable* pObject_);
			virtual void reset();
		protected:
		private:
			WordExtractionImpl* m_pOuter;
		};

	////////////////////////
	// ICollection::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);

		virtual void clear();
		virtual bool isEmpty();

		virtual Put* getPutInterface() {return &m_cPut;}
		virtual Get* getGetInterface() {return &m_cGet;}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		friend class PutImpl;
		friend class GetImpl;
		
		void pushData(const Common::Data::Pointer& pData_);
		bool getData(Common::DataArrayData* pData_);


		Action::FulltextHolder m_cFulltext;
		PutImpl m_cPut;
		GetImpl m_cGet;
	};
}





///////////////////////////////////////////////////////////
// Execution::Collection::Impl::WordExtractionImpl::PutImpl

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::PutImpl::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::PutImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::PutImpl::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::PutImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::PutImpl::putData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::WordExtractionImpl::PutImpl::
putData(Execution::Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	m_pOuter->pushData(pData_);
	return false;
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::PutImpl::put -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const Common::Externalizable* pObject_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::WordExtractionImpl::PutImpl::
put(Execution::Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::PutImpl::shift -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::PutImpl::
shift(Execution::Interface::IProgram& cProgram_)
{

}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::WordExtractionImpl::GetImpl

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::GetImpl::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::GetImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::GetImpl::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::GetImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::GetImpl::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::WordExtractionImpl::GetImpl::
getData(Execution::Interface::IProgram& cProgram_,
		Common::Data* pData_)
{

	if (pData_->getType() != Common::DataType::Array
		|| pData_->getElementType() != Common::DataType::Data) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	if(!m_pOuter->getData(_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_))) {
		return false;
	}

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(WordExtraction) "
			<< " getData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif
	return true;
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::GetImpl::get -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const Common::Externalizable* pObject_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
bool
Impl::WordExtractionImpl::GetImpl::
get(Execution::Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::GetImpl::reset -- 
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

//virtual
void
Impl::WordExtractionImpl::GetImpl::
reset()
{
	; // do nothing
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::WordExtractionImpl

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(" word extraction ");
	m_cFulltext.explain(pEnvironment_, cProgram_, cExplain_);
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::
initialize(Execution::Interface::IProgram& cProgram_)
{
	m_cPut.setOuter(this);
	m_cGet.setOuter(this);
	m_cFulltext.initialize(cProgram_);
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::WordExtractionImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	clear();
	m_cFulltext->clear();
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::clear -- 
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

//virtual
void
Impl::WordExtractionImpl::
clear()
{
	m_cGet.reset();
}

// FUNCTION public
//	Collection::Impl::WordExtractionImpl::isEmpty -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::WordExtractionImpl::
isEmpty()
{
	return m_cFulltext->isEmpty();
}



// FUNCTION public
//	Collection::Impl::WordExtractionImpl::getClassID -- 
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
Impl::WordExtractionImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::WordExtraction);
}


// FUNCTION public
//	Collection::Impl::WordExtractionImpl::serialize -- 
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
Impl::WordExtractionImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cFulltext.serialize(archiver_);
}

// FUNCTION private
//	Collection::Impl::WordExtractionImpl::pushData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data::Pointer& pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::WordExtractionImpl::
pushData(const Common::Data::Pointer& pData_)
{
	
	if (pData_.get() == 0
		|| pData_->getType() != Common::DataType::Array
		|| pData_->getElementType() != Common::DataType::Data) {
		return;
	}

	Common::DataArrayData* pArrayData =
		_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());
	
	if (pArrayData->getCount() != 1 &&
		pArrayData->getCount() != 2) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	
	if (pArrayData->getElement(0)->getType() == Common::DataType::String) {
		Common::StringData* pTextData =
			_SYDNEY_DYNAMIC_CAST(Common::StringData*, pArrayData->getElement(0).get());
		if (pArrayData->getCount() == 2) {
			;_SYDNEY_ASSERT(pArrayData->getElement(1)->getType() == Common::DataType::Language);
						
			if (pArrayData->getElement(1)->getType() == Common::DataType::Language) {
				Common::LanguageData* pLangData =
					_SYDNEY_DYNAMIC_CAST(Common::LanguageData*, pArrayData->getElement(1).get());
				m_cFulltext->expandPool(pTextData->getValue(), pLangData->getValue());
			} else if (pArrayData->getElement(1)->getType() == Common::DataType::String) {
				m_cFulltext->expandPool(pTextData->getValue(),
										pArrayData->getElement(1)->getString());
			} else {
				;_SYDNEY_ASSERT(false);
			}
		} else {
			m_cFulltext->expandPool(pTextData->getValue(), cLangJa);
		}
	} else if (pArrayData->getElement(0)->getType() == Common::DataType::Array) {
		Common::DataArrayData* pArrayString =
			_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pArrayData->getElement(0).get());
		Common::DataArrayData* pArrayLang = 0;
		if (pArrayData->getCount() == 2)
			pArrayLang =
				_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pArrayData->getElement(1).get());
		
		for (int i = 0; i < pArrayString->getCount(); ++i ) {
			;_SYDNEY_ASSERT(pArrayString->getElement(i)->getType() == Common::DataType::String);
			Common::StringData* pTextData =
				_SYDNEY_DYNAMIC_CAST(Common::StringData*, pArrayString->getElement(i).get());
			if (pArrayLang != 0) {
				if (pArrayLang->getElement(i)->getType() == Common::DataType::Language) {
					Common::LanguageData* pLangData =
						_SYDNEY_DYNAMIC_CAST(Common::LanguageData*, pArrayLang->getElement(i).get());
					m_cFulltext->expandPool(pTextData->getValue(), pLangData->getValue());
				} else if (pArrayLang->getElement(i)->getType() == Common::DataType::String) {
					m_cFulltext->expandPool(pTextData->getValue(),
											pArrayLang->getElement(i)->getString());
				} else {
					;_SYDNEY_ASSERT(false);
				}
			} else {
				m_cFulltext->expandPool(pTextData->getValue(), cLangJa);
			}
		}
	} else {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	
	
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(WordExtraction) "
			<< " putData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif

}


// FUNCTION private
//	Collection::Impl::WordExtractionImpl::getData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::UnsignedIntegerData* pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

bool
Impl::WordExtractionImpl::
getData(Common::DataArrayData* pData_)
{
	
	if (pData_->getCount() != 1) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}


	if (!m_cFulltext->nextCandidate()) return false;
	const STRING& pResult = m_cFulltext->getCandidateTerm();

	Common::DataType::Type eType = pData_->getElement(0)->getType();
	if (eType == Common::DataType::String) {
		Common::StringData* pTermData =		
			_SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_->getElement(0).get());
		pTermData->setValue(pResult);
	} else {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	
	return true;
}



///////////////////////////////////////////
// Execution::Collection::WordExtraction

// FUNCTION public
//	Collection::WordExtraction::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const VECTOR<int>& vecKeyPosition_
//	
// RETURN
//	WordExtraction*
//
// EXCEPTIONS

//static
WordExtraction*
WordExtraction::
create(Execution::Interface::IProgram& cProgram_,
	   int iFulltextID_)
{
	
	AUTOPOINTER<This> pResult = new Impl::WordExtractionImpl(iFulltextID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}



// FUNCTION public
//	Collection::WordExtraction::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	WordExtraction*
//
// EXCEPTIONS

//static
WordExtraction*
WordExtraction::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::WordExtraction);
	return new Impl::WordExtractionImpl;
}

_SYDNEY_DEXECUTION_COLLECTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
