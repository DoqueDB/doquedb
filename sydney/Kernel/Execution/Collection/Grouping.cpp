// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Grouping.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Collection/Grouping.h"
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
	//	Execution::Collection::Impl::GroupingComparator --
	//
	// NOTES
	class GroupingComparator
	{
	public:
		GroupingComparator()
			: m_iKeyPosition(-1)
		{}
		GroupingComparator(int iKeyPosition_)
			: m_iKeyPosition(iKeyPosition_)
		{}
		GroupingComparator(const GroupingComparator& cOther_)
			: m_iKeyPosition(cOther_.m_iKeyPosition)
		{}

		int compare(Common::DataArrayData* p1_,
					Common::DataArrayData* p2_)
		{
			return p1_->getElement(m_iKeyPosition)->compareTo(
										 p2_->getElement(m_iKeyPosition).get());
		}
	private:
		int m_iKeyPosition;
	};

	// CLASS
	//	Execution::Collection::Impl::GroupingImpl -- implementation class for collection::grouping
	//
	// NOTES

	class GroupingImpl
		: public Collection::Grouping
	{
	public:
		typedef Collection::Grouping Super;
		typedef GroupingImpl This;

		// constructor
		GroupingImpl()
			: Super(),
			  m_vecKeyPosition(),
			  m_cData(),
			  m_cArrayData(0,0),
			  m_vecComparator(),
			  m_bDistribute(false)
		{}
		GroupingImpl(const VECTOR<int>& vecKeyPosition_, bool bDistribute_)
			: Super(),
			  m_vecKeyPosition(vecKeyPosition_),
			  m_cData(),
			  m_cArrayData(0,0),
			  m_vecComparator(),
			  m_bDistribute(bDistribute_)
		{}

		// destructor
		~GroupingImpl()
		{}

		// CLASS
		//	GroupingImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(GroupingImpl* pOuter_) {m_pOuter = pOuter_;}

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
			GroupingImpl* m_pOuter;
		};

		// CLASS
		//	GroupingImpl::GetImpl -- implementation of get interface
		//
		// NOTES
		class GetImpl
			: public Super::Get
		{
		public:
			GetImpl() : m_pOuter(0) {}
			~GetImpl() {}

			void setOuter(GroupingImpl* pOuter_) {m_pOuter = pOuter_;}

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
			GroupingImpl* m_pOuter;
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
		bool compare();

		VECTOR<int> m_vecKeyPosition;
		PAIR<Common::Data::Pointer, Common::Data::Pointer> m_cData;
		PAIR<Common::DataArrayData*, Common::DataArrayData*> m_cArrayData; // casted data pointer
		VECTOR<GroupingComparator> m_vecComparator;
		PutImpl m_cPut;
		GetImpl m_cGet;
		bool m_bDistribute;
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::GroupingImpl::PutImpl

// FUNCTION public
//	Collection::Impl::GroupingImpl::PutImpl::finish -- 
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
Impl::GroupingImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::PutImpl::terminate -- 
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
Impl::GroupingImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::PutImpl::putData -- 
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
Impl::GroupingImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	m_pOuter->pushData(pData_->copy());
	return m_pOuter->compare();
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::PutImpl::put -- 
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
Impl::GroupingImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::PutImpl::shift -- 
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
Impl::GroupingImpl::PutImpl::
shift(Interface::IProgram& cProgram_)
{
	m_pOuter->shiftData();
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::GroupingImpl::GetImpl

// FUNCTION public
//	Collection::Impl::GroupingImpl::GetImpl::finish -- 
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
Impl::GroupingImpl::GetImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::GetImpl::terminate -- 
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
Impl::GroupingImpl::GetImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::GetImpl::getData -- 
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
Impl::GroupingImpl::GetImpl::
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
//	Collection::Impl::GroupingImpl::GetImpl::get -- 
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
Impl::GroupingImpl::GetImpl::
get(Interface::IProgram& cProgram_,
	Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::GetImpl::reset -- 
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
Impl::GroupingImpl::GetImpl::
reset()
{
	; // do nothing
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::GroupingImpl

// FUNCTION public
//	Collection::Impl::GroupingImpl::explain -- 
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
Impl::GroupingImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("grouping(");
	int n = m_vecKeyPosition.GETSIZE();
	for (int i = 0; i < n; ++i) {
		if (i > 0) cExplain_.put(",");
		cExplain_.put(m_vecKeyPosition[i]);
	}
	cExplain_.put(")");
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::initialize -- 
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
Impl::GroupingImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_vecComparator.ISEMPTY()) {
		int n = m_vecKeyPosition.GETSIZE();
		for (int i = 0; i < n; ++i) {
			int iKeyPosition = m_vecKeyPosition[i];
			m_vecComparator.PUSHBACK(GroupingComparator(iKeyPosition));
		}

		m_cPut.setOuter(this);
		m_cGet.setOuter(this);
	}
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::terminate -- 
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
Impl::GroupingImpl::
terminate(Interface::IProgram& cProgram_)
{
	clear();
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::clear -- 
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
Impl::GroupingImpl::
clear()
{
	m_cData.first = m_cData.second = Common::Data::Pointer();
	m_cArrayData.first = m_cArrayData.second = 0;
	m_cGet.reset();
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::isEmpty -- 
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
Impl::GroupingImpl::
isEmpty()
{
	return m_cData.first.get() == 0 && m_cData.second.get() == 0;
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::isEmptyGrouping -- 
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
Impl::GroupingImpl::
isEmptyGrouping()
{
	return (m_vecKeyPosition.ISEMPTY() && !m_bDistribute);
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::getClassID -- 
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
Impl::GroupingImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Grouping);
}

// FUNCTION public
//	Collection::Impl::GroupingImpl::serialize -- 
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
Impl::GroupingImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_bDistribute);
	Utility::SerializeValue(archiver_, m_vecKeyPosition);
}

// FUNCTION private
//	Collection::Impl::GroupingImpl::pushData -- 
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
Impl::GroupingImpl::
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
//	Collection::Impl::GroupingImpl::shiftData -- 
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
Impl::GroupingImpl::
shiftData()
{
	// swap m_cData elements so that returned data is never freed
	SWAP(m_cData.first, m_cData.second);

	Common::DataArrayData* pResult = m_cArrayData.first;
	m_cArrayData.first = m_cArrayData.second;
	m_cArrayData.second = 0;

	return pResult;
}

// FUNCTION private
//	Collection::Impl::GroupingImpl::compare -- 
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

bool
Impl::GroupingImpl::
compare()
{
	if (m_cArrayData.first == 0
		|| m_cArrayData.second == 0) {
		// it means less than two data are stored
		// -> return false so that more data is put
		return false;
	}

	// compare two elements
	if (m_vecComparator.GETSIZE() == 1) {
		// if two data are equal, next data should be put
		return m_vecComparator[0].compare(m_cArrayData.first,
										  m_cArrayData.second) != 0;
	}
	VECTOR<GroupingComparator>::ITERATOR iterator = m_vecComparator.begin();
	const VECTOR<GroupingComparator>::ITERATOR last = m_vecComparator.end();
	for (; iterator != last; ++iterator) {
		switch ((*iterator).compare(m_cArrayData.first, m_cArrayData.second)) {
		case 0:
			{
				continue;
			}
		case -1:
		case 1:
			{
				// different group -> return true to denote group is changed
				return true;
			}
		default:
			{
				_SYDNEY_THROW0(Exception::Unexpected);
			}
		}
	}
	return false;
}

///////////////////////////////////////////
// Execution::Collection::Grouping

// FUNCTION public
//	Collection::Grouping::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	const VECTOR<int>& vecKeyPosition_
//	
// RETURN
//	Grouping*
//
// EXCEPTIONS

//static
Grouping*
Grouping::
create(Interface::IProgram& cProgram_,
	   const VECTOR<int>& vecKeyPosition_,
	   bool bDistribute_)
{
	AUTOPOINTER<This> pResult = new Impl::GroupingImpl(vecKeyPosition_, bDistribute_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Grouping::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Grouping*
//
// EXCEPTIONS

//static
Grouping*
Grouping::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Grouping);
	return new Impl::GroupingImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
