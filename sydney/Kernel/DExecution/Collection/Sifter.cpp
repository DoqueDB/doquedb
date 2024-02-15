// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Sifter.cpp --
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
#include "DExecution/Collection/Sifter.h"
#include "DExecution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataType.h"
#include "Common/DataArrayData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/StringData.h"
#include "Common/WordData.h"

#include "Exception/Unexpected.h"
#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_COLLECTION_BEGIN


namespace Impl
{

	// CLASS
	//	Execution::Collection::Impl::SifterImpl -- implementation class for collection::word extraction
	//
	// NOTES

	
	class SifterImpl
		: public Collection::Sifter
	{
	public:
		typedef Collection::Sifter Super;
		typedef SifterImpl This;

		// constructor
		SifterImpl(int iTermID_)
			: Super(),
			  m_cGet(),
			  m_cPut(),
			  m_cFulltext(iTermID_),
			  m_bFirst(true)
		{}

		// constructor
		SifterImpl()
			: Super(),
			  m_cGet(),
			  m_cPut(),
			  m_cFulltext(),
			  m_bFirst(true)
		{}


		// destructor
		~SifterImpl()
		{}

		// CLASS
		//	SifterImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(SifterImpl* pOuter_) {m_pOuter = pOuter_;}

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
			SifterImpl* m_pOuter;
		};

		// CLASS
		//	SifterImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0) {}
			~GetImpl() {}

			void setOuter(SifterImpl* pOuter_) {m_pOuter = pOuter_;}

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
			SifterImpl* m_pOuter;
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
		bool m_bFirst;
	};
}





///////////////////////////////////////////////////////////
// Execution::Collection::Impl::SifterImpl::PutImpl

// FUNCTION public
//	Collection::Impl::SifterImpl::PutImpl::finish -- 
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
Impl::SifterImpl::PutImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Collection::Impl::SifterImpl::PutImpl::terminate -- 
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
Impl::SifterImpl::PutImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::SifterImpl::PutImpl::putData -- 
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
Impl::SifterImpl::PutImpl::
putData(Execution::Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	m_pOuter->pushData(pData_);
	return false;
}

// FUNCTION public
//	Collection::Impl::SifterImpl::PutImpl::put -- 
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
Impl::SifterImpl::PutImpl::
put(Execution::Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::SifterImpl::PutImpl::shift -- 
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
Impl::SifterImpl::PutImpl::
shift(Execution::Interface::IProgram& cProgram_)
{

}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::SifterImpl::GetImpl

// FUNCTION public
//	Collection::Impl::SifterImpl::GetImpl::finish -- 
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
Impl::SifterImpl::GetImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::SifterImpl::GetImpl::terminate -- 
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
Impl::SifterImpl::GetImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::SifterImpl::GetImpl::getData -- 
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
Impl::SifterImpl::GetImpl::
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
			<< "Collection(Sifter) "
			<< " getData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif
	return true;
}

// FUNCTION public
//	Collection::Impl::SifterImpl::GetImpl::get -- 
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
Impl::SifterImpl::GetImpl::
get(Execution::Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::SifterImpl::GetImpl::reset -- 
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
Impl::SifterImpl::GetImpl::
reset()
{
	; // do nothing
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::SifterImpl

// FUNCTION public
//	Collection::Impl::SifterImpl::explain -- 
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
Impl::SifterImpl::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("sifter ");
	m_cFulltext.explain(pEnvironment_, cProgram_, cExplain_);
	cExplain_.put(" ");
}

// FUNCTION public
//	Collection::Impl::SifterImpl::initialize -- 
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
Impl::SifterImpl::
initialize(Execution::Interface::IProgram& cProgram_)
{
	m_cPut.setOuter(this);
	m_cGet.setOuter(this);
	m_cFulltext.initialize(cProgram_);
}

// FUNCTION public
//	Collection::Impl::SifterImpl::terminate -- 
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
Impl::SifterImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	clear();
	m_cFulltext->clear();
}

// FUNCTION public
//	Collection::Impl::SifterImpl::clear -- 
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
Impl::SifterImpl::
clear()
{
	m_cGet.reset();
}

// FUNCTION public
//	Collection::Impl::SifterImpl::isEmpty -- 
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
Impl::SifterImpl::
isEmpty()
{
	return m_cFulltext->isEmpty();
}



// FUNCTION public
//	Collection::Impl::SifterImpl::getClassID -- 
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
Impl::SifterImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Sifter);
}


// FUNCTION public
//	Collection::Impl::SifterImpl::serialize -- 
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
Impl::SifterImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cFulltext.serialize(archiver_);
}

// FUNCTION private
//	Collection::Impl::SifterImpl::pushData -- 
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
Impl::SifterImpl::
pushData(const Common::Data::Pointer& pData_)
{
	if (pData_->getType() != Common::DataType::Array
		|| pData_->getElementType() != Common::DataType::Data) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Common::DataArrayData* pArrayData =
		_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());

	;_SYDNEY_ASSERT(pArrayData->getCount() == 1);
	
	if (pArrayData->getElement(0)->getType() == Common::DataType::UnsignedInteger) {
		const Common::UnsignedIntegerData* pCountData =
			_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*, pArrayData->getElement(0).get());
		m_cFulltext->setDocumentFrequency(pCountData->getValue());
	} else if (pArrayData->getElement(0)->getType() == Common::DataType::Word) {
		const Common::WordData* pWordData =
			_SYDNEY_DYNAMIC_CAST(const Common::WordData*, pArrayData->getElement(0).get());
		m_cFulltext->setDocumentFrequency(pWordData);
	} else {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
		
}


// FUNCTION private
//	Collection::Impl::SifterImpl::getData -- 
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
Impl::SifterImpl::
getData(Common::DataArrayData* pData_)
{

	
	if (pData_->getCount() != 1) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	if (pData_->getElement(0)->getType() == Common::DataType::Word) {
		if (!m_cFulltext->nextSelection()) return false;
		Action::Fulltext::ConditionParticle& cResult = m_cFulltext->getSelection();
		; _SYDNEY_ASSERT(cResult.getWordData() != 0);
		pData_->getElement(0)->assign(cResult.getWordData());
	} else if (pData_->getElement(0)->getType() == Common::DataType::String) {
		if (!m_bFirst) return false;
		Common::StringData* pString =
			_SYDNEY_DYNAMIC_CAST(Common::StringData*, pData_->getElement(0).get());
		pString->setValue(m_cFulltext->toStatement());
		m_bFirst = false;
	} else {
		_SYDNEY_THROW0(Exception::NotSupported);		
	}
	return true;
}



///////////////////////////////////////////
// Execution::Collection::Sifter

// FUNCTION public
//	Collection::Sifter::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	const VECTOR<int>& vecKeyPosition_
//	
// RETURN
//	Sifter*
//
// EXCEPTIONS

//static
Sifter*
Sifter::
create(Execution::Interface::IProgram& cProgram_, int iFulltextID_)
{
	AUTOPOINTER<This> pResult = new Impl::SifterImpl(iFulltextID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Sifter::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Sifter*
//
// EXCEPTIONS

//static
Sifter*
Sifter::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Sifter);
	return new Impl::SifterImpl;
}

_SYDNEY_DEXECUTION_COLLECTION_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
