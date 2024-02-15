// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Distinct.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Execution/Collection/Distinct.h"
#include "Execution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"
#include "Execution/Utility/HashKey.h"
#include "Execution/Externalizable.h"

#include "Common/Assert.h"
#include "Common/BitSet.h"
#include "Common/Data.h"
#include "Common/Hasher.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace
{
	struct _Type
	{
		enum Value
		{
			Normal,
			ByRowID,
			ValueNum
		};
	};
	const char* _pszExplainName[] =
		{
			"distinct",
			"distinct by rowid"
		};

} // namespace

namespace Impl
{
	// CLASS
	//	Execution::Collection::Impl::DistinctImpl -- implementation class for collection::distinct
	//
	// NOTES

	class DistinctImpl
		: public Collection::Distinct
	{
	public:
		typedef Collection::Distinct Super;
		typedef DistinctImpl This;

		// constructor
		DistinctImpl()
			: Super(),
			  m_mapData(),
			  m_cPut()
		{}

		// destructor
		~DistinctImpl()
		{}

		// CLASS
		//	DistinctImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(This* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
			virtual void flush();
		protected:
		private:
			DistinctImpl* m_pOuter;
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
		virtual Get* getGetInterface() {return 0;}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		friend class PutImpl;

		bool addData(const Common::Data::Pointer& pData_);
		void clearMap();

		typedef HASHMAP< Utility::HashKey, bool, Common::Hasher > Map;
		Map m_mapData;

		PutImpl m_cPut;
	};

	// CLASS
	//	Execution::Collection::Impl::DistinctByRowIDImpl -- implementation class for collection::distinct
	//
	// NOTES

	class DistinctByRowIDImpl
		: public Collection::Distinct
	{
	public:
		typedef Collection::Distinct Super;
		typedef DistinctByRowIDImpl This;

		// constructor
		DistinctByRowIDImpl()
			: Super(),
			  m_cBitSet(),
			  m_cPut()
		{}

		// destructor
		~DistinctByRowIDImpl()
		{}

		// CLASS
		//	DistinctByRowIDImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(This* pOuter_) {m_pOuter = pOuter_;}

		/////////////////////////////
		// Super::Put
			virtual void finish(Interface::IProgram& cProgram_);
			virtual void terminate(Interface::IProgram& cProgram_);
			virtual bool putData(Interface::IProgram& cProgram_,
								 const Common::Data* pData_);
			virtual bool put(Interface::IProgram& cProgram_,
							 const Common::Externalizable* pObject_);
			virtual void flush();
		protected:
		private:
			DistinctByRowIDImpl* m_pOuter;
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
		virtual Get* getGetInterface() {return 0;}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		friend class PutImpl;

		bool addData(const Common::Data::Pointer& pData_);
		void clearMap();

		Common::BitSet m_cBitSet;

		PutImpl m_cPut;
	};
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::DistinctImpl::PutImpl

// FUNCTION public
//	Collection::Impl::DistinctImpl::PutImpl::finish -- 
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
Impl::DistinctImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::PutImpl::terminate -- 
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
Impl::DistinctImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::PutImpl::putData -- 
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
Impl::DistinctImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	return m_pOuter->addData(Common::Data::Pointer(pData_));
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::PutImpl::put -- 
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
Impl::DistinctImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::PutImpl::flush -- 
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
Impl::DistinctImpl::PutImpl::
flush()
{
	m_pOuter->clearMap();
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::DistinctImpl

// FUNCTION public
//	Collection::Impl::DistinctImpl::explain -- 
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
Impl::DistinctImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::Normal]);
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::initialize -- 
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
Impl::DistinctImpl::
initialize(Interface::IProgram& cProgram_)
{
	m_cPut.setOuter(this);
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::terminate -- 
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
Impl::DistinctImpl::
terminate(Interface::IProgram& cProgram_)
{
	clearMap();
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::clear -- 
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
Impl::DistinctImpl::
clear()
{
	clearMap();
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::isEmpty -- 
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
Impl::DistinctImpl::
isEmpty()
{
	return m_mapData.ISEMPTY();
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::getClassID -- 
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
Impl::DistinctImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Distinct);
}

// FUNCTION public
//	Collection::Impl::DistinctImpl::serialize -- 
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
Impl::DistinctImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
}

// FUNCTION private
//	Collection::Impl::DistinctImpl::addData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data::Pointer& pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::DistinctImpl::
addData(const Common::Data::Pointer& pData_)
{
	Utility::HashKey cKey(pData_);
	Map::Iterator found = m_mapData.find(cKey);
	if (found == m_mapData.end()) {
		m_mapData.insert(cKey.copy(), true, ModTrue /* no check */);
		return true;
	}
	return false;
}

// FUNCTION private
//	Collection::Impl::DistinctImpl::clearMap -- 
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
Impl::DistinctImpl::
clearMap()
{
	m_mapData.clear();
}

///////////////////////////////////////////////////////////
// Execution::Collection::Impl::DistinctByRowIDImpl::PutImpl

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::PutImpl::finish -- 
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
Impl::DistinctByRowIDImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::PutImpl::terminate -- 
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
Impl::DistinctByRowIDImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::PutImpl::putData -- 
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
Impl::DistinctByRowIDImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	return m_pOuter->addData(Common::Data::Pointer(pData_));
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::PutImpl::put -- 
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
Impl::DistinctByRowIDImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::PutImpl::flush -- 
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
Impl::DistinctByRowIDImpl::PutImpl::
flush()
{
	m_pOuter->clearMap();
}

/////////////////////////////////////////////////
// Execution::Collection::Impl::DistinctByRowIDImpl

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::explain -- 
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
Impl::DistinctByRowIDImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName[_Type::ByRowID]);
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::initialize -- 
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
Impl::DistinctByRowIDImpl::
initialize(Interface::IProgram& cProgram_)
{
	m_cPut.setOuter(this);
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::terminate -- 
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
Impl::DistinctByRowIDImpl::
terminate(Interface::IProgram& cProgram_)
{
	clearMap();
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::clear -- 
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
Impl::DistinctByRowIDImpl::
clear()
{
	clearMap();
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::isEmpty -- 
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
Impl::DistinctByRowIDImpl::
isEmpty()
{
	return m_cBitSet.none();
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::getClassID -- 
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
Impl::DistinctByRowIDImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::DistinctByRowID);
}

// FUNCTION public
//	Collection::Impl::DistinctByRowIDImpl::serialize -- 
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
Impl::DistinctByRowIDImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
}

// FUNCTION private
//	Collection::Impl::DistinctByRowIDImpl::addData -- 
//
// NOTES
//
// ARGUMENTS
//	const Common::Data::Pointer& pData_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Impl::DistinctByRowIDImpl::
addData(const Common::Data::Pointer& pData_)
{
	unsigned int iValue = pData_->getUnsignedInt();
	if (m_cBitSet.test(iValue)) {
		return false;
	}
	m_cBitSet.set(iValue);
	return true;
}

// FUNCTION private
//	Collection::Impl::DistinctByRowIDImpl::clearMap -- 
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
Impl::DistinctByRowIDImpl::
clearMap()
{
	m_cBitSet.clear();
}

///////////////////////////////////////////
// Execution::Collection::Distinct

// FUNCTION public
//	Collection::Distinct::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Distinct*
//
// EXCEPTIONS

//static
Distinct*
Distinct::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::DistinctImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Distinct::ByRowID::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Distinct*
//
// EXCEPTIONS

//static
Distinct*
Distinct::ByRowID::
create(Interface::IProgram& cProgram_)
{
	AUTOPOINTER<This> pResult = new Impl::DistinctByRowIDImpl;
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::Distinct::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Distinct*
//
// EXCEPTIONS

//static
Distinct*
Distinct::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::Distinct:
		{
			return new Impl::DistinctImpl;
		}
	case Class::Category::DistinctByRowID:
		{
			return new Impl::DistinctByRowIDImpl;
		}
	default:
		{
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
