// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/BitsetDisintegration.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Collection/BitsetDisintegration.h"
#include "Execution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

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
	//	Execution::Collection::Impl::BitsetDisintegrationImpl -- implementation class for collection::grouping
	//
	// NOTES

	class BitsetDisintegrationImpl
		: public Collection::BitsetDisintegration
	{
	public:
		typedef Collection::BitsetDisintegration Super;
		typedef BitsetDisintegrationImpl This;

		// constructor
		BitsetDisintegrationImpl()
			: Super(),
			  m_cPut(),
			  m_cGet()
		{}


		// destructor
		~BitsetDisintegrationImpl()
		{}

		// CLASS
		//	BitsetDisintegrationImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(BitsetDisintegrationImpl* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
			virtual void shift(Interface::IProgram& cProgram_);
		protected:
		private:
			BitsetDisintegrationImpl* m_pOuter;
		};

		// CLASS
		//	BitsetDisintegrationImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0) {}
			~GetImpl() {}

			void setOuter(BitsetDisintegrationImpl* pOuter_) {m_pOuter = pOuter_;}

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
			BitsetDisintegrationImpl* m_pOuter;
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
		virtual bool isEmptyGrouping();
		virtual bool isGetNextOperand();

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
		Common::DataArrayData* shiftData();
		PAIR<Common::Data::Pointer, Common::Data::Pointer> m_cData;

		PutImpl m_cPut;
		GetImpl m_cGet;
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::BitsetDisintegrationImpl::PutImpl

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::PutImpl::finish -- 
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
Impl::BitsetDisintegrationImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::PutImpl::terminate -- 
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
Impl::BitsetDisintegrationImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::PutImpl::putData -- 
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
Impl::BitsetDisintegrationImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	m_pOuter->pushData(pData_->copy());
	return true;
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::PutImpl::put -- 
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
Impl::BitsetDisintegrationImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::PutImpl::shift -- 
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
Impl::BitsetDisintegrationImpl::PutImpl::
shift(Interface::IProgram& cProgram_)
{
	m_pOuter->shiftData();
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::BitsetDisintegrationImpl::GetImpl

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::GetImpl::finish -- 
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
Impl::BitsetDisintegrationImpl::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::GetImpl::terminate -- 
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
Impl::BitsetDisintegrationImpl::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::GetImpl::getData -- 
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
Impl::BitsetDisintegrationImpl::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	Common::DataArrayData* pArrayData = m_pOuter->shiftData();
	if (pArrayData == 0) {
		// less than two data are stored
		return false;
	}

	if (pData_->getType() != Common::DataType::Array
		|| pData_->getElementType() != Common::DataType::Data) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	Utility::DataType::assignElements(_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_),
									  pArrayData);
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(grouping) "
			<< " getData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif
	return true;
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::GetImpl::get -- 
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
Impl::BitsetDisintegrationImpl::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::GetImpl::reset -- 
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
Impl::BitsetDisintegrationImpl::GetImpl::
reset()
{
	; // do nothing
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::BitsetDisintegrationImpl

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::explain -- 
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
Impl::BitsetDisintegrationImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("BitsetDisintegration");

}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::initialize -- 
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
Impl::BitsetDisintegrationImpl::
initialize(Interface::IProgram& cProgram_)
{
	m_cPut.setOuter(this);
	m_cGet.setOuter(this);
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::terminate -- 
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
Impl::BitsetDisintegrationImpl::
terminate(Interface::IProgram& cProgram_)
{
	clear();
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::clear -- 
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
Impl::BitsetDisintegrationImpl::
clear()
{
	m_cData.first = m_cData.second = Common::Data::Pointer();
	m_cArrayData.first = m_cArrayData.second = 0;
	m_cGet.reset();
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::isEmpty -- 
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
Impl::BitsetDisintegrationImpl::
isEmpty()
{
	return m_cData.first.get() == 0 && m_cData.second.get() == 0;
}


// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::getClassID -- 
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
Impl::BitsetDisintegrationImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::BitsetDisintegration);
}

// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::serialize -- 
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
Impl::BitsetDisintegrationImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
}

// FUNCTION private
//	Collection::Impl::BitsetDisintegrationImpl::pushData -- 
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
Impl::BitsetDisintegrationImpl::
pushData(const Common::Data::Pointer& pData_)
{
	if (pData_->getType() == Common::DataType::Array
		&& pData_->getElementType() == Common::DataType::Data) {
		Common::DataArrayData* pArrayData =
			_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());

		m_cData.second = pData_;
		m_cArrayData.second = pArrayData;

	} else {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(grouping) "
			<< " putData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif

}

// FUNCTION private
//	Collection::Impl::BitsetDisintegrationImpl::shiftData -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Common::DataArrayData*
//
// EXCEPTIONS

Common::DataArrayData*
Impl::BitsetDisintegrationImpl::
shiftData()
{
	// swap m_cData elements so that returned data is never freed
	SWAP(m_cData.first, m_cData.second);

	Common::DataArrayData* pResult = m_cArrayData.first;
	m_cArrayData.first = m_cArrayData.second;
	m_cArrayData.second = 0;

	return pResult;
}


///////////////////////////////////////////
// Execution::Collection::BitsetDisintegration

// FUNCTION public
//	Collection::BitsetDisintegration::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const VECTOR<int>& vecKeyPosition_
//	
// RETURN
//	BitsetDisintegration*
//
// EXCEPTIONS

//static
BitsetDisintegration*
BitsetDisintegration::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::BitsetDisintegrationImpl();
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::BitsetDisintegration::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	BitsetDisintegration*
//
// EXCEPTIONS

//static
BitsetDisintegration*
BitsetDisintegration::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::BitsetDisintegration);
	return new Impl::BitsetDisintegrationImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
