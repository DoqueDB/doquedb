// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/CollectionCheck.cpp --
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
const char moduleName[] = "Execution::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Predicate/CollectionCheck.h"
#include "Execution/Predicate/Class.h"

#include "Execution/Action/Collection.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszExplainName = "check by collection ";
}

namespace Impl
{
	// CLASS local
	//	Execution::Predicate::Impl::CollectionCheckImpl -- implementation class of CollectionCheck
	//
	// NOTES
	class CollectionCheckImpl
		: public Predicate::CollectionCheck
	{
	public:
		typedef CollectionCheckImpl This;
		typedef Predicate::CollectionCheck Super;

		// constructor
		CollectionCheckImpl()
			: Super(),
			  m_cCollection()
		{}
		CollectionCheckImpl(int iCollectionID_,
							int iDataID_)
			: Super(),
			  m_cCollection(iCollectionID_, iDataID_)
		{}

		// destructor
		virtual ~CollectionCheckImpl() {}

	///////////////////////////
	// Predicate::CollectionCheck::

	/////////////////////////////
	// Interface::IPredicate::
	//	virtual Boolean::Value check(Interface::IProgram& cProgram_);

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Interface::IProgram& cProgram_,
	//						Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
	/////////////////////////////
	// Base::
		virtual Boolean::Value evaluate(Interface::IProgram& cProgram_,
										Action::ActionList& cActionList_);

		// collection holder
		Action::Collection m_cCollection;
	};
} // namespace Impl

///////////////////////////////////////////////////
// Execution::Predicate::Impl::CollectionCheckImpl

// FUNCTION public
//	Predicate::Impl::CollectionCheckImpl::explain -- 
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
Impl::CollectionCheckImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszExplainName);
	m_cCollection.explain(pEnvironment_, cProgram_, cExplain_);
	m_cCollection.explainPutData(cProgram_, cExplain_);
}

// FUNCTION public
//	Predicate::Impl::CollectionCheckImpl::initialize -- 
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
Impl::CollectionCheckImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cCollection.isInitialized() == false) {
		m_cCollection.initialize(cProgram_);
		m_cCollection.preparePutInterface();
	}
}

// FUNCTION public
//	Predicate::Impl::CollectionCheckImpl::terminate -- 
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
Impl::CollectionCheckImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cCollection.terminate(cProgram_);
}

// FUNCTION public
//	Predicate::Impl::CollectionCheckImpl::finish -- 
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
Impl::CollectionCheckImpl::
finish(Interface::IProgram& cProgram_)
{
	m_cCollection.finish(cProgram_);
}

// FUNCTION public
//	Predicate::Impl::CollectionCheckImpl::reset -- 
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
Impl::CollectionCheckImpl::
reset(Interface::IProgram& cProgram_)
{
	// flush put interface
	m_cCollection.flush();
}

// FUNCTION public
//	Predicate::Impl::CollectionCheckImpl::getClassID -- 
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
Impl::CollectionCheckImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::CollectionCheck);
}

// FUNCTION public
//	Predicate::Impl::CollectionCheckImpl::serialize --
//
// NOTES
//
// ARGUMENTS
//	ModArchive& cArchive_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::CollectionCheckImpl::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cCollection.serialize(cArchive_);
}

// FUNCTION private
//	Predicate::Impl::CollectionCheckImpl::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	
// RETURN
//	Boolean::Value
//
// EXCEPTIONS

//virtual
CollectionCheck::Boolean::Value
Impl::CollectionCheckImpl::
evaluate(Interface::IProgram& cProgram_,
		 Action::ActionList& cActionList_)
{
	return m_cCollection.put(cProgram_) ? Boolean::True : Boolean::False;
}

/////////////////////////////////////
// Predicate::CollectionCheck

// FUNCTION public
//	Predicate::CollectionCheck::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iCollectionID_
//	int iDataID_
//	
// RETURN
//	CollectionCheck*
//
// EXCEPTIONS

//static
CollectionCheck*
CollectionCheck::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iCollectionID_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult =
		new Impl::CollectionCheckImpl(iCollectionID_,
									  iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::CollectionCheck::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	CollectionCheck*
//
// EXCEPTIONS

//static
CollectionCheck*
CollectionCheck::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::CollectionCheck);
	return new Impl::CollectionCheckImpl;
}

_SYDNEY_EXECUTION_PREDICATE_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
