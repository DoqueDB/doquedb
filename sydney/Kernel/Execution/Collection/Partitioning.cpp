// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Partitioning.cpp --
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

#include "Execution/Collection/Partitioning.h"
#include "Execution/Collection/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/Limit.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/HashKey.h"
#include "Execution/Externalizable.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/Hasher.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"

#include "Os/Limits.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace Impl
{
	// CLASS
	//	Execution::Collection::Impl::PartitioningImpl -- implementation class for collection::partitioning
	//
	// NOTES

	class PartitioningImpl
		: public Collection::Partitioning
	{
	public:
		typedef Collection::Partitioning Super;
		typedef PartitioningImpl This;

		// constructor
		PartitioningImpl()
			: Super(),
			  m_cKey(),
			  m_cLimit(),
			  m_iLimit(-1),
			  m_iOffset(-1),
			  m_iPartition(-1),
			  m_iCurrent(-1),
			  m_vecStorage(),
			  m_vecvecData(),
			  m_mapData()
		{}
		PartitioningImpl(int iKeyID_,
						 const PAIR<int, int>& cLimitPair_)
			: Super(),
			  m_cKey(iKeyID_),
			  m_cLimit(cLimitPair_),
			  m_iLimit(-1),
			  m_iOffset(-1),
			  m_iPartition(-1),
			  m_iCurrent(-1),
			  m_vecStorage(),
			  m_vecvecData(),
			  m_mapData()
		{}

		// destructor
		~PartitioningImpl()
		{}

		// CLASS
		//	PartitioningImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(PartitioningImpl* pOuter_) {m_pOuter = pOuter_;}

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
			PartitioningImpl* m_pOuter;
		};

		// CLASS
		//	PartitioningImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0) {}
			~GetImpl() {}

			void setOuter(PartitioningImpl* pOuter_) {m_pOuter = pOuter_;}

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
			PartitioningImpl* m_pOuter;
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
		typedef HASHMAP<Utility::HashKey, int, Common::Hasher> Map;
		typedef VECTOR< VECTOR<Common::DataArrayData*> > Vector;

		void addData(Interface::IProgram& cProgram_,
					 const Common::Data* pData_);
		Common::DataArrayData* getData();

		Action::DataHolder m_cKey;
		Action::Limit m_cLimit;

		VECTOR<Common::Data::Pointer> m_vecStorage; // holding pointer itself (not partitioned)
		Vector m_vecvecData;// casted data pointer (partitioned)
		Map m_mapData;

		int m_iLimit;
		int m_iOffset;
		int m_iPartition;
		int m_iCurrent;

		PutImpl m_cPut;
		GetImpl m_cGet;
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::PartitioningImpl::PutImpl

// FUNCTION public
//	Collection::Impl::PartitioningImpl::PutImpl::finish -- 
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
Impl::PartitioningImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::PutImpl::terminate -- 
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
Impl::PartitioningImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::PutImpl::putData -- 
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
Impl::PartitioningImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	m_pOuter->addData(cProgram_, pData_);
	return false;
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::PutImpl::put -- 
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
Impl::PartitioningImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::PartitioningImpl::GetImpl

// FUNCTION public
//	Collection::Impl::PartitioningImpl::GetImpl::finish -- 
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
Impl::PartitioningImpl::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::GetImpl::terminate -- 
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
Impl::PartitioningImpl::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::GetImpl::getData -- 
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
Impl::PartitioningImpl::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	Common::DataArrayData* pArrayData = m_pOuter->getData();
	if (pArrayData == 0) {
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
			<< "Collection(partitioning) "
			<< " getData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif
	return true;
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::GetImpl::get -- 
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
Impl::PartitioningImpl::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::GetImpl::reset -- 
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
Impl::PartitioningImpl::GetImpl::
reset()
{
	; // do nothing
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::PartitioningImpl

// FUNCTION public
//	Collection::Impl::PartitioningImpl::explain -- 
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
Impl::PartitioningImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("partitioning");
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" by ");
		m_cKey.explain(cProgram_,
					   cExplain_);
	}
	if (m_cLimit.isValid()) {
		cExplain_.put(" limit ");
		m_cLimit.explain(pEnvironment_,
						 cProgram_,
						 cExplain_);
	}
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::initialize -- 
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
Impl::PartitioningImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cKey.isInitialized() == false) {
		m_cKey.initialize(cProgram_);
		m_cLimit.initialize(cProgram_);

		m_cPut.setOuter(this);
		m_cGet.setOuter(this);
	}
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::terminate -- 
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
Impl::PartitioningImpl::
terminate(Interface::IProgram& cProgram_)
{
	clear();
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::clear -- 
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
Impl::PartitioningImpl::
clear()
{
	m_vecStorage.clear();
	m_vecvecData.clear();
	m_mapData.clear();
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::isEmpty -- 
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
Impl::PartitioningImpl::
isEmpty()
{
	return m_vecStorage.ISEMPTY();
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::getClassID -- 
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
Impl::PartitioningImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Partitioning);
}

// FUNCTION public
//	Collection::Impl::PartitioningImpl::serialize -- 
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
Impl::PartitioningImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cKey.serialize(archiver_);
	m_cLimit.serialize(archiver_);
}

// FUNCTION private
//	Collection::Impl::PartitioningImpl::addData -- 
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

void
Impl::PartitioningImpl::
addData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	if (m_iLimit < 0) {
		if (m_cLimit.isValid()) {
			m_cLimit.setValues(cProgram_);
			m_iLimit = m_cLimit.getLimit();
			m_iOffset = m_cLimit.getOffset();
		} else {
			m_iLimit = Os::Limits<int>::getMax();
		}
	}

	const Common::Data* pKey = m_cKey.getData();

	Utility::HashKey cKey(pKey);
	int iPosition = -1;
	Map::Iterator found = m_mapData.find(cKey);
	if (found == m_mapData.end()) {
		// new partition
		if (m_vecvecData.GETSIZE() < m_iLimit) {
			iPosition = m_vecvecData.GETSIZE();
			m_vecvecData.PUSHBACK(VECTOR<Common::DataArrayData*>());
			m_mapData.insert(cKey.copy(), iPosition, ModTrue /* no check */);
		}
	} else {
		iPosition = (*found).second;
		; _SYDNEY_ASSERT(m_vecvecData.GETSIZE() > iPosition);
	}

	if (iPosition >= 0) {
		Common::Data::Pointer pCopy = pData_->copy();
		Common::DataArrayData* pData = _SYDNEY_DYNAMIC_CAST(Common::DataArrayData*, pCopy.get());
		; _SYDNEY_ASSERT(pData);
		m_vecStorage.PUSHBACK(pCopy);
		if (iPosition >= m_iOffset - 1) {
			m_vecvecData[iPosition].PUSHBACK(pData);
#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
				_OPT_EXECUTION_MESSAGE
					<< "Collection(partitioning) "
					<< " putData = "
					<< Opt::Trace::toString(*m_vecvecData[iPosition].GETBACK())
					<< ModEndl;
			}
#endif
		}
	}
}

// FUNCTION private
//	Collection::Impl::PartitioningImpl::getData -- 
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
Impl::PartitioningImpl::
getData()
{
	if (m_iPartition >= 0
		&& m_iCurrent >= m_vecvecData[m_iPartition].GETSIZE() - 1) {
		m_iCurrent = -1;
	}
	if (++m_iCurrent == 0) {
		// go to next partition
		while (++m_iPartition < m_iOffset - 1) {}
		if (m_iPartition >= m_iLimit
			|| m_iPartition >= m_vecvecData.GETSIZE()) {
			return 0;
		}
	}
	return m_vecvecData[m_iPartition][m_iCurrent];
}

///////////////////////////////////////////
// Execution::Collection::Partitioning

// FUNCTION public
//	Collection::Partitioning::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iKeyID_
//	const PAIR<int, int>& cLimitPair_
//	
// RETURN
//	Partitioning*
//
// EXCEPTIONS

//static
Partitioning*
Partitioning::
create(Interface::IProgram& cProgram_,
	   int iKeyID_,
	   const PAIR<int, int>& cLimitPair_)
{
	AUTOPOINTER<This> pResult = new Impl::PartitioningImpl(iKeyID_, cLimitPair_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Partitioning::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Partitioning*
//
// EXCEPTIONS

//static
Partitioning*
Partitioning::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Partitioning);
	return new Impl::PartitioningImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
