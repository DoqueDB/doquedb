// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Normalize.cpp --
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

#include "Execution/Function/Normalize.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataArrayData.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Utility/CharTrait.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	// CONST
	//	_pszOperator -- operator name for explain
	//
	// NOTES
	const char* const _pszOperator = "normalize";
	const char* const _pszDelimiter = " using ";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::NormalizeImpl -- implementation class of expand synonym
	//
	// NOTES
	class NormalizeImpl
		: public Function::Normalize
	{
	public:
		typedef NormalizeImpl This;
		typedef Function::Normalize Super;

		NormalizeImpl()
			: Super(),
			  m_cData(),
			  m_cOption(),
			  m_cOutData()
		{}
		NormalizeImpl(int iDataID_,
					  int iOptionID_,
					  int iOutDataID_)
			: Super(),
			  m_cData(iDataID_),
			  m_cOption(iOptionID_),
			  m_cOutData(iOutDataID_)
		{}
		~NormalizeImpl()
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
		const Common::StringData* getData() {return m_cData.getData();}
		const Common::StringData* getOption() {return m_cOption.getData();}
		Common::StringData* getOutData() {return m_cOutData.get();}

		// data type of operand and option are guaranteed by setExpectedType in analyzer
		Action::StringDataHolder m_cData;
		Action::StringDataHolder m_cOption;

		// data type of result is set at createDataType in plan
		Action::StringDataHolder m_cOutData;
	};
} // namespace Impl

/////////////////////////////////////////////////
// Execution::Function::Impl::NormalizeImpl

// FUNCTION public
//	Function::Impl::NormalizeImpl::explain -- 
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
Impl::NormalizeImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszOperator);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" ");
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.put(_pszDelimiter);
		m_cOption.explain(cProgram_, cExplain_);
		cExplain_.put(" to ");
		m_cOutData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::Impl::NormalizeImpl::initialize -- 
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
Impl::NormalizeImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		m_cData.initialize(cProgram_);
		m_cOption.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::NormalizeImpl::terminate -- 
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
Impl::NormalizeImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cData.terminate(cProgram_);
	m_cOption.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::Impl::NormalizeImpl::execute -- 
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
Impl::NormalizeImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		if (getData()->isNull()
			|| getOption()->isNull() ) {
			getOutData()->setNull();
		} else {
			ModUnicodeString cstrResult;
			Utility::CharTrait::normalize(getData()->getValue(),
										  getOption()->getValue(),
										  cstrResult);
			getOutData()->setValue(cstrResult);
		}
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::NormalizeImpl::finish -- 
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
Impl::NormalizeImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::NormalizeImpl::reset -- 
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
Impl::NormalizeImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::NormalizeImpl::getClassID -- 
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
Impl::NormalizeImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Normalize);
}

// FUNCTION public
//	Function::Impl::NormalizeImpl::serialize -- 
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
Impl::NormalizeImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cData.serialize(archiver_);
	m_cOption.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////
// Function::Normalize::

// FUNCTION public
//	Function::Normalize::create -- 
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
//	Normalize*
//
// EXCEPTIONS

//static
Normalize*
Normalize::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iIDataID_,
	   int iOption_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::NormalizeImpl(iIDataID_,
														iOption_,
														iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Normalize::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Normalize*
//
// EXCEPTIONS

//static
Normalize*
Normalize::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Normalize:
		{
			return new Impl::NormalizeImpl;
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
