// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Collection/Variable.cpp --
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

#include "Execution/Collection/Variable.h"
#include "Execution/Collection/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/Serialize.h"

#include "Common/Assert.h"
#include "Common/ClassID.h"
#include "Common/BitSet.h"
#include "Common/UnsignedIntegerData.h"
#include "Common/Externalizable.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Declaration.h"
#include "Opt/Explain.h"
#include "Opt/Trace.h"



_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_COLLECTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Execution::Collection::Impl::BitSetVariableImpl -- implementation class for collection::variable
	//
	// NOTES

	class BitSetVariableImpl
		: public Collection::Variable
	{
	public:
		typedef Collection::Variable Super;
		typedef BitSetVariableImpl This;


		// constructor
		BitSetVariableImpl()
			: Super(),
			  m_iSessionID(),
			  m_cstrValName(),
			  m_iTableID(),
			  m_cBitSet(),
			  m_cPut()
		{}
		
		// constructor
		BitSetVariableImpl(Server::SessionID iSessionID_,
						   const STRING& cstrValName_,
						   Schema::Object::ID::Value iTableID_)
			: Super(),
			  m_iSessionID(iSessionID_),
			  m_cstrValName(cstrValName_),
			  m_iTableID(iTableID_),
			  m_cBitSet(),
			  m_cPut()
		{}

		// destructor
		~BitSetVariableImpl()
		{}

		// CLASS
		//	BitSetVariableImpl::PutImpl -- implementation of put interface
		//
		// NOTES
		class PutImpl
			: public Super::Put
		{
		public:
			PutImpl() : m_pOuter(0) {}
			~PutImpl() {}

			void setOuter(BitSetVariableImpl* pOuter_)
			{
				m_pOuter = pOuter_;
			}

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
			BitSetVariableImpl* m_pOuter;
		};

		bool pushData(const Common::Data* pData_);
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
		virtual Get* getGetInterface() {_SYDNEY_THROW0(Exception::NotSupported);}

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		friend class PutImpl;

		void open();
		void close();

		Common::BitSet m_cBitSet;
		PutImpl m_cPut;
		
		Schema::Object::ID::Value m_iTableID;
		STRING m_cstrValName;
		Server::SessionID m_iSessionID;
	};
}


// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::PutImpl::finish ------
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
Impl::BitSetVariableImpl::PutImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}


// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::PutImpl::terminate -- 
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
Impl::BitSetVariableImpl::PutImpl::
terminate(Interface::IProgram& cProgram_)
{
	
	; // do nothing
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::PutImpl::putData -- 
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
Impl::BitSetVariableImpl::PutImpl::
putData(Interface::IProgram& cProgram_,
		const Common::Data* pData_)
{
	bool bResult = m_pOuter->pushData(pData_);
#ifndef NO_TRACE
	if (pData_ && _OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::File)) {
		_OPT_EXECUTION_MESSAGE
			<< "Collection(connection)"
			<< " putData = "
			<< Opt::Trace::toString(*pData_)
			<< ModEndl;
	}
#endif
	return bResult;
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::PutImpl::put -- 
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
Impl::BitSetVariableImpl::PutImpl::
put(Interface::IProgram& cProgram_,
	const Common::Externalizable* pObject_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}





/////////////////////////////////////////////////
// Execution::Collection::Impl::BitSetVariableImpl


// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::putData -- 
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
Impl::BitSetVariableImpl::
pushData(const Common::Data* pData_)
{
	if (pData_->getType() == Common::DataType::UnsignedInteger) {
		const Common::UnsignedIntegerData* pUIntData =
			_SYDNEY_DYNAMIC_CAST(const Common::UnsignedIntegerData*, pData_);
		m_cBitSet[pUIntData->getValue()] = 1;
	}
		
	return true;
}


// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::BitSetVariableImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put("<BietSetVariable").put(">");
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::initialize -- 
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
Impl::BitSetVariableImpl::
initialize(Interface::IProgram& cProgram_)
{
	m_cBitSet.reset();
	m_cPut.setOuter(this);
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::terminate -- 
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
Impl::BitSetVariableImpl::
terminate(Interface::IProgram& cProgram_)
{
	Common::Data* pData = cProgram_.getOutputVariable(m_iSessionID,
													  m_cstrValName,
													  m_iTableID);
	if (pData->getType() != Common::DataType::BitSet) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	Common::BitSet* pBitSet = _SYDNEY_DYNAMIC_CAST(Common::BitSet*, pData);
	pBitSet->reset();
	pBitSet->assign(&m_cBitSet);
	
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::clear -- 
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
Impl::BitSetVariableImpl::
clear()
{
	m_cBitSet.reset();
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::isEmpty -- 
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
Impl::BitSetVariableImpl::
isEmpty()
{
	// regard as always not empty
	return false;
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::getClassID -- 
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
Impl::BitSetVariableImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::BitSetVariable);
}

// FUNCTION public
//	Collection::Impl::BitSetVariableImpl::serialize -- 
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
Impl::BitSetVariableImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	archiver_(m_iSessionID);
	archiver_(m_cstrValName);
	archiver_(m_iTableID);
}



// FUNCTION private
//	Collection::Impl::BitSetVariableImpl::close -- 
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
Impl::BitSetVariableImpl::
close()
{

}

///////////////////////////////////////////
// Execution::Collection::BitSetVariable

// FUNCTION public
//	Collection::BitSetVariable::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	int iBitSetVariableID_
//	
// RETURN
//	BitSetVariable*
//
// EXCEPTIONS

//static
Variable*
Variable::
create(Interface::IProgram& cProgram_,
	   Server::SessionID iSessionID_,
	   const STRING& cstrValName_,
	   Schema::Object::ID::Value iTableID_)	   
{
	
	AUTOPOINTER<This> pResult;
	pResult = new Impl::BitSetVariableImpl(iSessionID_,
										   cstrValName_,
										   iTableID_);
	pResult->registerToProgram(cProgram_);
	return pResult.release();
}

// FUNCTION public
//	Collection::BitSetVariable::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	BitSetVariable*
//
// EXCEPTIONS

//static
Variable*
Variable::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::BitSetVariable);
	return new Impl::BitSetVariableImpl;
}

_SYDNEY_EXECUTION_COLLECTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
