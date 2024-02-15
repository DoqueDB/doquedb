// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Copy.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Execution/Function/Copy.h"
#include "Execution/Function/Class.h"

#include "Execution/Action/DataHolder.h"
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
	const char* const _pszOperator = "copy";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::CopyImpl -- implementation class of Copy
	//
	// NOTES
	class CopyImpl
		: public Function::Copy
	{
	public:
		typedef CopyImpl This;
		typedef Function::Copy Super;

		CopyImpl()
			: Super(),
			  m_cInData(),
			  m_cOutData()
		{}
		CopyImpl(int iInDataID_,
				 int iOutDataID_)
			: Super(),
			  m_cInData(iInDataID_),
			  m_cOutData(iOutDataID_)
		{}
		~CopyImpl()
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
	};
}

/////////////////////////////////////////////////
// Execution::Function::Impl::CopyImpl

// FUNCTION public
//	Function::Impl::CopyImpl::explain -- 
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
Impl::CopyImpl::
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
//	Function::Impl::CopyImpl::initialize -- 
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
Impl::CopyImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cInData.isInitialized() == false) {
		m_cInData.initialize(cProgram_);
		m_cOutData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::CopyImpl::terminate -- 
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
Impl::CopyImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_cInData.terminate(cProgram_);
	m_cOutData.terminate(cProgram_);
}

// FUNCTION public
//	Function::Impl::CopyImpl::execute -- 
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
Impl::CopyImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		getOutData()->assign(getInData());
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::CopyImpl::finish -- 
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
Impl::CopyImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::CopyImpl::reset -- 
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
Impl::CopyImpl::
reset(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Function::Impl::CopyImpl::getClassID -- 
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
Impl::CopyImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Copy);
}

// FUNCTION public
//	Function::Impl::CopyImpl::serialize -- 
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
Impl::CopyImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cInData.serialize(archiver_);
	m_cOutData.serialize(archiver_);
}

/////////////////////////////////////
// Function::Copy::

// FUNCTION public
//	Function::Copy::create -- 
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
//	Copy*
//
// EXCEPTIONS

//static
Copy*
Copy::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iInDataID_,
	   int iOutDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::CopyImpl(iInDataID_,
												   iOutDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::Copy::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Copy*
//
// EXCEPTIONS

//static
Copy*
Copy::
getInstance(int iCategory_)
{
	switch(iCategory_) {
	case Class::Category::Copy:
		{
			return new Impl::CopyImpl;
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
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
