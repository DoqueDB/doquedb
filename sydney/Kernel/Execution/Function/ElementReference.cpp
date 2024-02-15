// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/ElementReference.cpp --
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

#include "Execution/Function/ElementReference.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"
#include "Common/StringData.h"

#include "Exception/BadArrayElement.h"
#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Utility/CharTrait.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::ElementReferenceImpl -- implementation class of element reference
	//
	// NOTES
	class ElementReferenceImpl
		: public Function::ElementReference
	{
	public:
		typedef ElementReferenceImpl This;
		typedef Function::ElementReference Super;

		ElementReferenceImpl()
			: Super(),
			  m_cData(),
			  m_cOption(),
			  m_cOutData()
		{}
		ElementReferenceImpl(int iDataID_,
							 int iOptionID_,
							 int iOutDataID_)
			: Super(),
			  m_cData(iDataID_),
			  m_cOption(iOptionID_),
			  m_cOutData(iOutDataID_)
		{}
		~ElementReferenceImpl()
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

	///////////////////////////////
	// Common::Externalizable
		virtual int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		const Common::DataArrayData* getData() {return m_cData.getData();}
		const Common::IntegerData* getOption() {return m_cOption.getData();}
		Common::Data* getOutData() {return m_cOutData.get();}

		// datatype of operand and option are guaranteed by analyzer
		Action::ArrayDataHolder m_cData;
		Action::IntegerDataHolder m_cOption;
		Action::DataHolder m_cOutData;
	};
} // namespace Impl

/////////////////////////////////////////////////
// Execution::Function::Impl::ElementReferenceImpl

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::explain -- 
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
Impl::ElementReferenceImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	m_cData.explain(cProgram_, cExplain_);
	cExplain_.put("[");
	m_cOption.explain(cProgram_, cExplain_);
	cExplain_.put("]");
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::initialize -- 
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
Impl::ElementReferenceImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		m_cData.initialize(cProgram_);
		m_cOption.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::terminate -- 
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
Impl::ElementReferenceImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cData.terminate(cProgram_);
	m_cOption.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::execute -- 
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
Impl::ElementReferenceImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getData()->isNull()
			|| getOption()->isNull() ) {
			getOutData()->setNull();
		} else {
			int n = getData()->getCount();
			int e = getOption()->getValue() - 1; // 1-base -> 0-base

			if (e < 0 || e >= n) {
				// data exception -- array element error
				_SYDNEY_THROW0(Exception::BadArrayElement);
			}

			getOutData()->assign(getData()->getElement(e).get());
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::finish -- 
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
Impl::ElementReferenceImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::reset -- 
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
Impl::ElementReferenceImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::getClassID -- 
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

//virtual
int
Impl::ElementReferenceImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::ElementReference);
}

// FUNCTION public
//	Function::Impl::ElementReferenceImpl::serialize -- 
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
Impl::ElementReferenceImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cData.serialize(archiver_);
	m_cOption.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////
// Function::ElementReference::

// FUNCTION public
//	Function::ElementReference::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iIDataID_
//	int iOption_
//	int iOutDataID_
//	
// RETURN
//	ElementReference*
//
// EXCEPTIONS

//static
ElementReference*
ElementReference::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIDataID_,
	   int iOption_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::ElementReferenceImpl(iIDataID_,
															   iOption_,
															   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::ElementReference::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	ElementReference*
//
// EXCEPTIONS

//static
ElementReference*
ElementReference::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::ElementReference:
		{
			return new Impl::ElementReferenceImpl;
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
