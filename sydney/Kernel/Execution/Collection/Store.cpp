// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Store.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Execution/Collection/Store.h"
#include "Execution/Collection/Allocator.h"
#include "Execution/Collection/Class.h"
#include "Execution/Collection/Data.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Externalizable.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace Impl
{
	// CLASS
	//	Execution::Collection::Impl::StoreImpl -- implementation class for collection::store
	//
	// NOTES

	class StoreImpl
		: public Collection::Store
	{
	public:
		typedef Collection::Store Super;
		typedef StoreImpl This;

		// constructor
		StoreImpl()
			: Super(),
			  m_vecStorage(),
			  m_cPut(),
			  m_cGet()
		{}

		// destructor
		~StoreImpl()
		{}

		// CLASS
		//	StoreImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(StoreImpl* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
			virtual	int getLastPosition();
		protected:
		private:
			StoreImpl* m_pOuter;
		};

		// CLASS
		//	StoreImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0), m_iCursor(-1) {}
			~GetImpl() {}

			void setOuter(StoreImpl* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Get
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool getData(Interface::IProgram& cProgram_,
								 Common::Data* pData_);
			virtual bool get(Interface::IProgram& cProgram_,
							 Common::Externalizable* pObject_);
			virtual bool getData(Interface::IProgram& cProgram_,
								 Common::Data* pData_,
								 int iPosition_);
			virtual void reset();
		protected:
		private:
			StoreImpl* m_pOuter;
			int m_iCursor;
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

		VECTOR<Data*> m_vecStorage; // serialized data
		Allocator m_cAllocator;
		PutImpl m_cPut;
		GetImpl m_cGet;
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::StoreImpl::PutImpl

// FUNCTION public
//	Collection::Impl::StoreImpl::PutImpl::finish -- 
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
Impl::StoreImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::StoreImpl::PutImpl::terminate -- 
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
Impl::StoreImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::StoreImpl::PutImpl::putData -- 
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
Impl::StoreImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	if (pData_->getType() == Common::DataType::Array &&
		pData_->getElementType() == Common::DataType::Data)
	{
		const Common::DataArrayData* pData
			= _SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData_);
		ModSize s = Data::getSize(*pData) * 4;	// ユニット長をバイトにする
		Data* p = syd_reinterpret_cast<Data*>(m_pOuter->m_cAllocator.get(s));
		p->dump(*pData);
		m_pOuter->m_vecStorage.PUSHBACK(p);
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File))
		{
			_OPT_EXECUTION_MESSAGE
				<< "Collection(store) "
				<< " putData = "
				<< Opt::Trace::toString(*pData)
				<< ModEndl;
		}
#endif
	}
	else
	{
		// single data -> create dataarray data
		AUTOPOINTER<Common::DataArrayData> pData = new Common::DataArrayData;
		pData->pushBack(pData_->copy());
		ModSize s = Data::getSize(*pData) * 4;	// ユニット長をバイトにする
		Data* p = syd_reinterpret_cast<Data*>(m_pOuter->m_cAllocator.get(s));
		p->dump(*pData);
		m_pOuter->m_vecStorage.PUSHBACK(p);
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File))
		{
			_OPT_EXECUTION_MESSAGE
				<< "Collection(store) "
				<< " putData = "
				<< Opt::Trace::toString(*pData)
				<< ModEndl;
		}
#endif
	}
	return false; // put all when used with filter
}

// FUNCTION public
//	Collection::Impl::StoreImpl::PutImpl::put -- 
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
Impl::StoreImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::StoreImpl::PutImpl::getLastPosition -- 
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

//virtual
int
Impl::StoreImpl::PutImpl::
getLastPosition()
{
	int p = static_cast<int>(m_pOuter->m_vecStorage.GETSIZE());
	return p - 1;
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::StoreImpl::GetImpl

// FUNCTION public
//	Collection::Impl::StoreImpl::GetImpl::finish -- 
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
Impl::StoreImpl::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::StoreImpl::GetImpl::terminate -- 
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
Impl::StoreImpl::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::StoreImpl::GetImpl::getData -- 
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
Impl::StoreImpl::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	if (static_cast<unsigned int>(++m_iCursor)
		< m_pOuter->m_vecStorage.GETSIZE())	{
		if (pData_->getType() != Common::DataType::Array
			|| pData_->getElementType() != Common::DataType::Data) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		Common::DataArrayData* pData
			= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_);
		m_pOuter->m_vecStorage[m_iCursor]->getData(*pData);

#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Collection(store) "
				<< " getData = "
				<< Opt::Trace::toString(*pData_)
				<< ModEndl;
		}
#endif
		return true;
	}
	return false;
}

// FUNCTION public
//	Collection::Impl::StoreImpl::GetImpl::get -- 
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
Impl::StoreImpl::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::StoreImpl::GetImpl::getData -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const Common::Data* pData_
//	int iPotision_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::StoreImpl::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_,
		int iPosition_)
{
	if (iPosition_ >= 0 &&
		iPosition_ < static_cast<int>(m_pOuter->m_vecStorage.GETSIZE()))
	{
		if (pData_->getType() != Common::DataType::Array
			|| pData_->getElementType() != Common::DataType::Data) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		Common::DataArrayData* pData
			= _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_);
		m_pOuter->m_vecStorage[iPosition_]->getData(*pData);

#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Collection(store) "
				<< " getData = "
				<< Opt::Trace::toString(*pData_)
				<< ModEndl;
		}
#endif
		return true;
	}
	return false;
}

// FUNCTION public
//	Collection::Impl::StoreImpl::GetImpl::reset -- 
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
Impl::StoreImpl::GetImpl::
reset()
{
	m_iCursor = -1;
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::StoreImpl

// FUNCTION public
//	Collection::Impl::StoreImpl::explain -- 
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
Impl::StoreImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("store");
}

// FUNCTION public
//	Collection::Impl::StoreImpl::initialize -- 
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
Impl::StoreImpl::
initialize(Interface::IProgram& cProgram_)
{
	m_cPut.setOuter(this);
	m_cGet.setOuter(this);
}

// FUNCTION public
//	Collection::Impl::StoreImpl::terminate -- 
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
Impl::StoreImpl::
terminate(Interface::IProgram& cProgram_)
{
	clear();
}

// FUNCTION public
//	Collection::Impl::StoreImpl::clear -- 
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
Impl::StoreImpl::
clear()
{
	m_vecStorage.clear();
	m_cAllocator.clear();

	m_cGet.reset();
}

// FUNCTION public
//	Collection::Impl::StoreImpl::isEmpty -- 
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
Impl::StoreImpl::
isEmpty()
{
	return m_vecStorage.ISEMPTY();
}

// FUNCTION public
//	Collection::Impl::StoreImpl::getClassID -- 
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
Impl::StoreImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Store);
}

// FUNCTION public
//	Collection::Impl::StoreImpl::serialize -- 
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
Impl::StoreImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
}

///////////////////////////////////////////
// Execution::Collection::Store

// FUNCTION public
//	Collection::Store::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Store*
//
// EXCEPTIONS

//static
Store*
Store::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::StoreImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Store::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Store*
//
// EXCEPTIONS

//static
Store*
Store::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Store);
	return new Impl::StoreImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
