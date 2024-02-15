// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Cardinality.cpp --
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

#include "Execution/Function/Cardinality.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/IntegerData.h"

#include "Exception/InvalidCardinality.h"
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
	const char* const _pszOperator = "cardinality";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::CardinalityImpl -- implementation class of Cardinality
	//
	// NOTES
	class CardinalityImpl
		: public Function::Cardinality
	{
	public:
		typedef CardinalityImpl This;
		typedef Function::Cardinality Super;

		CardinalityImpl()
			: Super(),
			  m_cInData(),
			  m_cOutData()
		{}
		CardinalityImpl(int iInDataID_,
						int iOutDataID_)
			: Super(),
			  m_cInData(iInDataID_),
			  m_cOutData(iOutDataID_)
		{}
		~CardinalityImpl()
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
		Common::IntegerData* getOutData() {return m_cOutData.get();}

	private:
		Action::DataHolder m_cInData;
		Action::IntegerDataHolder m_cOutData;
	};
}

/////////////////////////////////////////////////
// Execution::Function::Impl::CardinalityImpl

// FUNCTION public
//	Function::Impl::CardinalityImpl::explain -- 
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
Impl::CardinalityImpl::
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
//	Function::Impl::CardinalityImpl::initialize -- 
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
Impl::CardinalityImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData.isInitialized() == false) {
		m_cInData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::CardinalityImpl::terminate -- 
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
Impl::CardinalityImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::Impl::CardinalityImpl::execute -- 
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
Impl::CardinalityImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		const Common::Data* pData = getInData();
		if (pData->isNull()) {
			getOutData()->setNull();

		} else {
			if (pData->getType() != Common::DataType::Array
				|| pData->getElementType() != Common::DataType::Data) {
				_SYDNEY_THROW0(Exception::InvalidCardinality);
			} else {
				const Common::DataArrayData* pArrayData =
					_SYDNEY_DYNAMIC_CAST(const Common::DataArrayData*, pData);
				; _SYDNEY_ASSERT(pArrayData);
				getOutData()->setValue(pArrayData->getCount());
			}
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::CardinalityImpl::finish -- 
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
Impl::CardinalityImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::CardinalityImpl::reset -- 
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
Impl::CardinalityImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::CardinalityImpl::getClassID -- 
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
Impl::CardinalityImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Cardinality);
}

// FUNCTION public
//	Function::Impl::CardinalityImpl::serialize -- 
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
Impl::CardinalityImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cInData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////
// Function::Cardinality::

// FUNCTION public
//	Function::Cardinality::create -- 
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
//	Cardinality*
//
// EXCEPTIONS

//static
Cardinality*
Cardinality::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::CardinalityImpl(iInDataID_,
														  iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Cardinality::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Cardinality*
//
// EXCEPTIONS

//static
Cardinality*
Cardinality::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Cardinality:
		{
			return new Impl::CardinalityImpl;
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
