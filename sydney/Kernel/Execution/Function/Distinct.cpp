// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Distinct.cpp --
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
const char moduleName[] = "Execution::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Execution/Function/Distinct.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Action/Collection.h"
#include "Execution/Collection/Distinct.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszOperator -- operator name for explain
	//
	// NOTES
	const char* const _pszOperator = "distinct";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::DistinctImpl -- implementation class of Distinct
	//
	// NOTES
	class DistinctImpl
		: public Function::Distinct
	{
	public:
		typedef DistinctImpl This;
		typedef Function::Distinct Super;

		DistinctImpl()
			: Super(),
			  m_cInData(),
			  m_cOutData(),
			  m_cCollection()
		{}
		DistinctImpl(int iInDataID_,
					 int iOutDataID_,
					 int iCollectionID_)
			: Super(),
			  m_cInData(iInDataID_),
			  m_cOutData(iOutDataID_),
			  m_cCollection(iCollectionID_,
							iInDataID_)
		{}
		~DistinctImpl()
		{}

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Interface::IProgram& cProgram_);
		virtual void terminate(Interface::IProgram& cProgram_);

		virtual Action::Status::Value
					execute(Interface::IProgram& cProgram_,
							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
	//	virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
		// accessor
		const Common::Data* getInData() {return m_cInData.getData();}
		Common::Data* getOutData() {return m_cOutData.get();}

	private:
		Action::DataHolder m_cInData;
		Action::DataHolder m_cOutData;
		Action::Collection m_cCollection;
	};
}

/////////////////////////////////////////////////
// Execution::Function::Impl::DistinctImpl

// FUNCTION public
//	Function::Impl::DistinctImpl::explain -- 
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment* pEnvironment_
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
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperator);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cInData.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::Impl::DistinctImpl::initialize -- 
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
	if (m_cInData.isInitialized() == false) {
		m_cInData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
		m_cCollection.initialize(cProgram_);
		m_cCollection.preparePutInterface();
	}
}

// FUNCTION public
//	Function::Impl::DistinctImpl::terminate -- 
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
	m_cInData.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
	m_cCollection.terminate(cProgram_);
}

// FUNCTION public
//	Function::Impl::DistinctImpl::execute -- 
//	
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Action::ActionList& cActionList_
//	
// RETURN
//	Action::Status::Value
//
// EXCEPTIONS

//virtual
Action::Status::Value
Impl::DistinctImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (m_cCollection.put(cProgram_)) {
			getOutData()->assign(getInData());
		} else {
			getOutData()->setNull();
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::DistinctImpl::finish -- 
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
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::DistinctImpl::reset -- 
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
reset(Interface::IProgram& cProgram_)
{
	// flush put interface
	m_cCollection.flush();
}

// FUNCTION public
//	Function::Impl::DistinctImpl::getClassID -- 
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
//	Function::Impl::DistinctImpl::serialize -- 
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
	m_cInData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
	m_cCollection.serialize(archiver_);
}

/////////////////////////////////////
// Function::Distinct::

// FUNCTION public
//	Function::Distinct::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iInDataID_
//	int iOutDataID_
//	
// RETURN
//	Distinct*
//
// EXCEPTIONS

//static
Distinct*
Distinct::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID_,
	   int iOutDataID_)
{
	Interface::ICollection* pDistinct = Collection::Distinct::create(cProgram_);

	AUTOPOINTER<This> pResult = new Impl::DistinctImpl(iInDataID_,
													   iOutDataID_,
													   pDistinct->getID());
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Distinct::getInstance -- for serialize
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
	switch(iCategory_) {
	case Class::Category::Distinct:
		{
			return new Impl::DistinctImpl;
		}
	default:
		{
			; _SYDNEY_ASSERT(false);
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
