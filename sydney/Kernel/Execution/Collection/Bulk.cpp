// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Bulk.cpp --
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
const char moduleName[] = "Execution::Collection";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Collection/Bulk.h"
#include "Execution/Collection/Class.h"
#include "Execution/Action/BulkParameter.h"
#include "Execution/Action/BulkExecutor.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"
#include "Execution/Externalizable.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace Impl
{
	// CLASS
	//	Execution::Collection::Impl::BulkImpl -- implementation class for collection::bulk
	//
	// NOTES

	class BulkImpl
		: public Collection::Bulk
	{
	public:
		typedef Collection::Bulk Super;
		typedef BulkImpl This;

		// constructor
		BulkImpl()
			: Super(),
			  m_cPath(),
			  m_cWith(),
			  m_cHint(),
			  m_bInput(false),
			  m_vecDataType(),
			  m_pParameter(0),
			  m_pExecutor(0),
			  m_cPut(),
			  m_cGet()
		{}
		BulkImpl(int iPathID_,
				 int iWithID_,
				 int iHintID_,
				 bool bInput_,
				 const VECTOR<Common::SQLData>& vecDataType_)
			: Super(),
			  m_cPath(iPathID_),
			  m_cWith(iWithID_),
			  m_cHint(iHintID_),
			  m_bInput(bInput_),
			  m_vecDataType(vecDataType_),
			  m_pParameter(0),
			  m_pExecutor(0),
			  m_cPut(),
			  m_cGet()
		{}

		// destructor
		~BulkImpl()
		{try{destruct();}catch(...){}}

		// CLASS
		//	BulkImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(BulkImpl* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
		protected:
		private:
			BulkImpl* m_pOuter;
		};

		// CLASS
		//	BulkImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0) {}
			~GetImpl() {}

			void setOuter(BulkImpl* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Get
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool getData(Interface::IProgram& cProgram_,
								 Common::Data* pData_);
			virtual bool get(Interface::IProgram& cProgram_,
							 Common::Externalizable* pObject_);
			virtual void reset();
		protected:
		private:
			BulkImpl* m_pOuter;
		};

	////////////////////////
	// ICollection::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual void clear();
		virtual bool isEmpty();
	//	virtual bool isEmptyGrouping();

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

		bool addData(const Common::Data* pData_);
		bool getData(Common::Data* pData_);

		void createBulkExecutor(Interface::IProgram& cProgram_);
		void destruct();

		// path, with, hint is known as string by setExpectedType at analyzer
		Action::StringDataHolder m_cPath;
		Action::StringDataHolder m_cWith;
		Action::StringDataHolder m_cHint;

		bool m_bInput;
		VECTOR<Common::SQLData> m_vecDataType;

		Action::BulkParameter* m_pParameter;
		Action::BulkExecutor* m_pExecutor;

		PutImpl m_cPut;
		GetImpl m_cGet;
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::BulkImpl::PutImpl

// FUNCTION public
//	Collection::Impl::BulkImpl::PutImpl::finish -- 
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
Impl::BulkImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BulkImpl::PutImpl::terminate -- 
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
Impl::BulkImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BulkImpl::PutImpl::putData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::BulkImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	return m_pOuter->addData(pData_);
}

// FUNCTION public
//	Collection::Impl::BulkImpl::PutImpl::put -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Externalizable* pObject_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::BulkImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::BulkImpl::GetImpl

// FUNCTION public
//	Collection::Impl::BulkImpl::GetImpl::finish -- 
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
Impl::BulkImpl::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BulkImpl::GetImpl::terminate -- 
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
Impl::BulkImpl::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BulkImpl::GetImpl::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
bool
Impl::BulkImpl::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	return m_pOuter->getData(pData_);
}

// FUNCTION public
//	Collection::Impl::BulkImpl::GetImpl::get -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Externalizable* pObject_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
bool
Impl::BulkImpl::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::BulkImpl::GetImpl::reset -- 
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
Impl::BulkImpl::GetImpl::
reset()
{
	m_pOuter->m_pExecutor->reset();
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::BulkImpl

// FUNCTION public
//	Collection::Impl::BulkImpl::explain -- 
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
Impl::BulkImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("bulk");
}

// FUNCTION public
//	Collection::Impl::BulkImpl::initialize -- 
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
Impl::BulkImpl::
initialize(Interface::IProgram& cProgram_)
{
	createBulkExecutor(cProgram_);
	m_cPut.setOuter(this);
	m_cGet.setOuter(this);
}

// FUNCTION public
//	Collection::Impl::BulkImpl::terminate -- 
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
Impl::BulkImpl::
terminate(Interface::IProgram& cProgram_)
{
	destruct();
}

// FUNCTION public
//	Collection::Impl::BulkImpl::clear -- 
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
Impl::BulkImpl::
clear()
{
	;	// do nothing
}

// FUNCTION public
//	Collection::Impl::BulkImpl::isEmpty -- 
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
Impl::BulkImpl::
isEmpty()
{
	// regard as always not empty
	return false;
}

// FUNCTION public
//	Collection::Impl::BulkImpl::getClassID -- 
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
Impl::BulkImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Bulk);
}

// FUNCTION public
//	Collection::Impl::BulkImpl::serialize -- 
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
Impl::BulkImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cPath.serialize(archiver_);
	m_cWith.serialize(archiver_);
	m_cHint.serialize(archiver_);
	archiver_(m_bInput);
	Utility::SerializeSQLDataType(archiver_, m_vecDataType);
}

// FUNCTION private
//	Collection::Impl::BulkImpl::addData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::BulkImpl::
addData(const Common::Data* pData_)
{
	if (pData_->getType() == Common::DataType::Array
		&& pData_->getElementType() == Common::DataType::Data) {
		m_pExecutor->put(*_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_));

	} else {
		// single data -> create dataarray data
		AUTOPOINTER<Common::DataArrayData> pData = new Common::DataArrayData;
		pData->pushBack(pData_);
		m_pExecutor->put(*pData);
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(bulk) "
			<< " putData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif
	return true;
}

// FUNCTION private
//	Collection::Impl::BulkImpl::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Common::Data* pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::BulkImpl::
getData(Common::Data* pData_)
{
	if (m_pExecutor->next()
		&& m_pExecutor->get(*_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_))) {
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Collection(bulk) "
				<< " getData = "
				<< Opt::Trace::toString(*pData_)
				<< ModEndl;
		}
#endif
		return true;
	}
	return false;
}

// FUNCTION private
//	Collection::Impl::BulkImpl::createBulkExecutor -- 
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
Impl::BulkImpl::
createBulkExecutor(Interface::IProgram& cProgram_)
{
	m_cPath.initialize(cProgram_);
	m_cWith.initialize(cProgram_);
	m_cHint.initialize(cProgram_);

	if (m_cPath.isInitialized() == false) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	STRING cstrWith;
	STRING cstrHint;
	if (m_cWith.isInitialized()) {
		cstrWith = m_cWith->getValue();
	}
	if (m_cHint.isInitialized()) {
		cstrHint = m_cHint->getValue();
	}
	m_pParameter = new Action::BulkParameter;
	m_pParameter->setValues(m_cPath->getValue(),
							cstrWith,
							cstrHint,
							m_bInput);
	m_pExecutor = new Action::BulkExecutor(*m_pParameter, m_vecDataType);
	m_pExecutor->initialize();
}

// FUNCTION private
//	Collection::Impl::BulkImpl::destruct -- 
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
Impl::BulkImpl::
destruct()
{
	delete m_pParameter, m_pParameter = 0;
	delete m_pExecutor, m_pExecutor = 0;
}

///////////////////////////////////////////
// Execution::Collection::Bulk

// FUNCTION public
//	Collection::Bulk::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iPathID_
//	int iWithID_
//	int iHintID_
//	bool bInput_
//	const VECTOR<Common::SQLData>& vecDataType_
//	
// RETURN
//	Bulk*
//
// EXCEPTIONS

//static
Bulk*
Bulk::
create(Interface::IProgram& cProgram_,
	   int iPathID_,
	   int iWithID_,
	   int iHintID_,
	   bool bInput_,
	   const VECTOR<Common::SQLData>& vecDataType_)
{
	AUTOPOINTER<This> pResult = new Impl::BulkImpl(iPathID_,
												   iWithID_,
												   iHintID_,
												   bInput_,
												   vecDataType_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Bulk::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Bulk*
//
// EXCEPTIONS

//static
Bulk*
Bulk::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Bulk);
	return new Impl::BulkImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
