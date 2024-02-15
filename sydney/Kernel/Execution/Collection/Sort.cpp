// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Sort.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Collection/Sort.h"
#include "Execution/Collection/Allocator.h"
#include "Execution/Collection/Data.h"
#include "Execution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/WordData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"
#include "Opt/Sort.h"
#include "Opt/Trace.h"

#include "LogicalFile/TreeNodeInterface.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace Impl
{
	// CLASS
	//	Execution::Collection::Impl::SortComparator --
	//
	// NOTES
	class SortComparator
	{
	public:
		SortComparator()
			: m_iKeyPosition(-1),
			  m_iDirection(0),
			  m_iWordPosition(0)
		{}
		SortComparator(int iKeyPosition_,
					   int iDirection_,
					   int iWordPosition_)
			: m_iKeyPosition(iKeyPosition_),
			  m_iDirection(iDirection_ == 0 ? 1 : -1),
			  m_iWordPosition(iWordPosition_)
		{}
		SortComparator(const SortComparator& cOther_)
			: m_iKeyPosition(cOther_.m_iKeyPosition),
			  m_iDirection(cOther_.m_iDirection),
			  m_iWordPosition(cOther_.m_iWordPosition)
		{}

		int compare(const Data* p1_,
					const Data* p2_)
		{
			if (m_iWordPosition == 0)
			{
				return m_iDirection * p1_->compare(p2_, m_iKeyPosition);
			}
			else
			{
				if (m_iWordPosition
					== LogicalFile::TreeNodeInterface::WordDf)
				{
					return m_iDirection * p1_->compare(p2_, m_iKeyPosition,
													   Data::Type::Word_Df);
				}
				else if (m_iWordPosition
						 == LogicalFile::TreeNodeInterface::WordScale)
				{
					return m_iDirection * p1_->compare(p2_, m_iKeyPosition,
													   Data::Type::Word_Scale);
				}
				else
				{
					_SYDNEY_THROW0(Exception::Unexpected);					
				}
			}
		}
		
	private:
		int m_iKeyPosition;
		int m_iDirection;
		int m_iWordPosition;
	};

	// CLASS
	//	Execution::Collection::Impl::SortImpl -- implementation class for collection::sort
	//
	// NOTES

	class SortImpl
		: public Collection::Sort
	{
	public:
		typedef Collection::Sort Super;
		typedef SortImpl This;

		// constructor
		SortImpl()
			: Super(),
			  m_vecKeyPosition(),
			  m_vecDirection(),
			  m_vecStorage(),
			  m_vecComparator(),
			  m_bSorted(false)
		{}
		SortImpl(const VECTOR<int>& vecKeyPosition_,
				 const VECTOR<int>& vecDirection_,
				 const VECTOR<int>& vecWordPosition_)
			: Super(),
			  m_vecKeyPosition(vecKeyPosition_),
			  m_vecDirection(vecDirection_),
			  m_vecWordPosition(vecWordPosition_),
			  m_vecStorage(),
			  m_vecComparator(),
			  m_bSorted(false)
		{}

		// destructor
		~SortImpl()
		{}

		// CLASS
		//	SortImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(SortImpl* pOuter_) {m_pOuter = pOuter_;}

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
			SortImpl* m_pOuter;
		};

		// CLASS
		//	SortImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0), m_iCursor(-1) {}
			~GetImpl() {}

			void setOuter(SortImpl* pOuter_) {m_pOuter = pOuter_;}

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
			SortImpl* m_pOuter;
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

		// sort data
		void sort();
		// comparator
		bool compare(const Data* p1_,
					 const Data* p2_);

		VECTOR<int> m_vecKeyPosition;
		VECTOR<int> m_vecDirection;
		
		// ソートキーがWordDataの場合、単純比較できないため、
		// 別の配列で管理する.
		VECTOR<int> m_vecWordPosition;
		VECTOR<Data*> m_vecStorage;
		Allocator m_cAllocator;
		VECTOR<SortComparator> m_vecComparator;
		bool m_bSorted;
		PutImpl m_cPut;
		GetImpl m_cGet;
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::SortImpl::PutImpl

// FUNCTION public
//	Collection::Impl::SortImpl::PutImpl::finish -- 
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
Impl::SortImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	m_pOuter->sort();
}

// FUNCTION public
//	Collection::Impl::SortImpl::PutImpl::terminate -- 
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
Impl::SortImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::SortImpl::PutImpl::putData -- 
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
Impl::SortImpl::PutImpl::
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
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Collection(sort) "
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
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
			_OPT_EXECUTION_MESSAGE
				<< "Collection(sort) "
				<< " putData = "
				<< Opt::Trace::toString(*pData)
				<< ModEndl;
		}
#endif
	}

	return false; // put all when used with filter
}

// FUNCTION public
//	Collection::Impl::SortImpl::PutImpl::put -- 
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
Impl::SortImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::SortImpl::GetImpl

// FUNCTION public
//	Collection::Impl::SortImpl::GetImpl::finish -- 
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
Impl::SortImpl::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::SortImpl::GetImpl::terminate -- 
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
Impl::SortImpl::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::SortImpl::GetImpl::getData -- 
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
Impl::SortImpl::GetImpl::
getData(Interface::IProgram& cProgram_,
		Common::Data* pData_)
{
	// sort at first getting
	if (++m_iCursor == 0) m_pOuter->sort();

	if (static_cast<unsigned int>(m_iCursor)
		< m_pOuter->m_vecStorage.GETSIZE())
	{
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
				<< "Collection(sort) "
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
//	Collection::Impl::SortImpl::GetImpl::get -- 
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
Impl::SortImpl::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::SortImpl::GetImpl::reset -- 
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
Impl::SortImpl::GetImpl::
reset()
{
	m_iCursor = -1;
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::SortImpl

// FUNCTION public
//	Collection::Impl::SortImpl::explain -- 
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
Impl::SortImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("sort(");
	int n = m_vecKeyPosition.GETSIZE();
	for (int i = 0; i < n; ++i) {
		if (i > 0) cExplain_.put(",");
		cExplain_.put(m_vecKeyPosition[i]);
		if (static_cast<int>(m_vecDirection.GETSIZE()) > i &&
			m_vecDirection[i] != 0) {
			cExplain_.put(" desc");
		}
	}
	cExplain_.put(")");
}

// FUNCTION public
//	Collection::Impl::SortImpl::initialize -- 
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
Impl::SortImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_vecComparator.ISEMPTY()) {
		int n = m_vecKeyPosition.GETSIZE();
		for (int i = 0; i < n; ++i) {
			int iKeyPosition = m_vecKeyPosition[i];
			int iDirection = (static_cast<int>(m_vecDirection.GETSIZE()) > i) ?
				m_vecDirection[i] : 0;
			int iWordPosition
				= (static_cast<int>(m_vecWordPosition.GETSIZE()) > i) ?
				m_vecWordPosition[i] : 0;
			m_vecComparator.PUSHBACK(SortComparator(iKeyPosition, iDirection, iWordPosition));
		}

		m_cPut.setOuter(this);
		m_cGet.setOuter(this);
	}
}

// FUNCTION public
//	Collection::Impl::SortImpl::terminate -- 
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
Impl::SortImpl::
terminate(Interface::IProgram& cProgram_)
{
	clear();
}

// FUNCTION public
//	Collection::Impl::SortImpl::clear -- 
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
Impl::SortImpl::
clear()
{
	m_vecStorage.clear();
	m_cAllocator.clear();
	m_bSorted = false;
	
	m_cGet.reset();
}

// FUNCTION public
//	Collection::Impl::SortImpl::isEmpty -- 
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
Impl::SortImpl::
isEmpty()
{
	return m_vecStorage.ISEMPTY();
}

// FUNCTION public
//	Collection::Impl::SortImpl::getClassID -- 
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
Impl::SortImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Sort);
}

// FUNCTION public
//	Collection::Impl::SortImpl::serialize -- 
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
Impl::SortImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	Utility::SerializeValue(archiver_, m_vecKeyPosition);
	Utility::SerializeValue(archiver_, m_vecDirection);
	Utility::SerializeValue(archiver_, m_vecWordPosition);
}

// FUNCTION private
//	Collection::Impl::SortImpl::sort -- 
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
Impl::SortImpl::
sort()
{
	if (m_bSorted == false)
	{
		SORT(m_vecStorage.begin(),
			 m_vecStorage.end(),
			 boost::bind(&This::compare,
						 this,
						 _1,
						 _2));
		m_bSorted = true;
	}
}

//
//	FUNCTION private
//	Execution::Collection::Impl::SortImpl::compare -- 比較関数
//
//	NOTES
//
//	ARGUMENTS
//	const Execution::Collection::Data* p1_
//	const Execution::Collection::Data* p2_
//
//	RETURN
//	bool
//
//	EXCEPTIONS
//
bool
Impl::SortImpl::
compare(const Data* p1_, const Data* p2_)
{
	VECTOR<SortComparator>::ITERATOR iterator = m_vecComparator.begin();
	const VECTOR<SortComparator>::ITERATOR last = m_vecComparator.end();
	for (; iterator != last; ++iterator)
	{
		int c = (*iterator).compare(p1_, p2_);
		if (c == 0)
			continue;

		return (c < 0) ? true : false;
	}
	return false;
}

///////////////////////////////////////////
// Execution::Collection::Sort

// FUNCTION public
//	Collection::Sort::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const VECTOR<int>& vecKeyPosition_
//	const VECTOR<int>& vecDirection_
//	
// RETURN
//	Sort*
//
// EXCEPTIONS

//static
Sort*
Sort::
create(Interface::IProgram& cProgram_,
	   const VECTOR<int>& vecKeyPosition_,
	   const VECTOR<int>& vecDirection_,
	   const VECTOR<int>& vecWordPosition_)
{
	AUTOPOINTER<This> pResult = new Impl::SortImpl(vecKeyPosition_, vecDirection_, vecWordPosition_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}


///////////////////////////////////////////
// Execution::Collection::Sort

// FUNCTION public
//	Collection::Sort::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const VECTOR<int>& vecKeyPosition_
//	const VECTOR<int>& vecDirection_
//	
// RETURN
//	Sort*
//
// EXCEPTIONS

//static
Sort*
Sort::
create(Interface::IProgram& cProgram_,
	   const VECTOR<int>& vecKeyPosition_,
	   const VECTOR<int>& vecDirection_)
{
	VECTOR<int> vecWordPosition;
	AUTOPOINTER<This> pResult = new Impl::SortImpl(vecKeyPosition_, vecDirection_, vecWordPosition);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}


// FUNCTION public
//	Collection::Sort::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Sort*
//
// EXCEPTIONS

//static
Sort*
Sort::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Sort);
	return new Impl::SortImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
