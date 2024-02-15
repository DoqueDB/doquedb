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
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataType.h"
#include "Common/DataArrayData.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/BitSet.h"



#include "Exception/Unexpected.h"
#include "Exception/NotSupported.h"

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
			  m_cGet(),
			  m_pBitSet(0)
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
		bool getData(Common::DataArrayData* pData_);

		Common::BitSet* m_pBitSet;
		Common::Data::Pointer m_pData;
		Common::BitSet::ConstIterator m_ite;
		
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
			<< "Collection(BitsetDisintegration) "
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
	m_pData = Common::Data::Pointer();
	m_pBitSet = 0;
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
	return m_pBitSet == 0 && m_pData.get() == 0;
}


// FUNCTION public
//	Collection::Impl::BitsetDisintegrationImpl::isGetNextOperand ----
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
isGetNextOperand()
{
	if(m_pBitSet  == 0) return true;
	return (m_ite == m_pBitSet->end());
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
	m_pData = pData_;
	if (pData_->getType() != Common::DataType::Array
		|| pData_->getElementType() != Common::DataType::Data)
		_SYDNEY_THROW0(Exception::Unexpected);
	
	Common::DataArrayData* pArrayData =
		_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pData_.get());

	Common::Data::Pointer pData = pArrayData->getElement(pArrayData->getCount() - 1);

	// 配列の最後の要素がBitSet形式のRowID
	if (pData->getType() != Common::DataType::BitSet)
		_SYDNEY_THROW0(Exception::Unexpected);
	m_pBitSet = _SYDNEY_DYNAMIC_CAST(Common::BitSet*, pData.get());
	m_ite = m_pBitSet->begin();

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(BitsetDisintegration) "
			<< " putData = "
			<< Opt::Trace::toString(*pData)
			<< ModEndl;
	}
#endif

}


// FUNCTION private
//	Collection::Impl::BitsetDisintegrationImpl::getData -- 
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
Impl::BitsetDisintegrationImpl::
getData(Common::DataArrayData* pData_)
{

	if(m_ite == m_pBitSet->end()) return false;

	Common::DataArrayData* pSourceData =
		_SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, m_pData.get());
	if (pData_->getCount() != pSourceData->getCount())
		_SYDNEY_THROW0(Exception::Unexpected);
	
	int i = 0;
	for (; i < pData_->getCount() -1; i++ ){
		pData_->getElement(i)->assign(pSourceData->getElement(i).get());
	}

	if (pData_->getElement(i)->getType() != Common::DataType::UnsignedInteger)
		_SYDNEY_THROW0(Exception::Unexpected);

	Common::UnsignedIntegerData* pIntData =
		_SYDNEY_DYNAMIC_CAST(Common::UnsignedIntegerData*, pData_->getElement(i).get());
	pIntData->setValue(*m_ite);
	m_ite++;
	return true;
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
