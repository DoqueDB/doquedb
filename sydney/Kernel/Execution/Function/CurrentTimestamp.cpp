// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/CurrentTimestamp.cpp --
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
#include "SyReinterpretCast.h"

#include "Execution/Function/CurrentTimestamp.h"
#include "Execution/Function/Class.h"
#include "Execution/Action/DataHolder.h"
#include "Execution/Action/Timestamp.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/DataType.h"

#include "Common/Assert.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_FUNCTION_BEGIN

namespace
{
	const char* const _pszExplainName = "current_timestamp";
}

namespace Impl
{
	// CLASS local
	//	Execution::Function::Impl::CurrentTimestampImpl -- implementation classes of CurrentTimestamp
	//
	// NOTES
	class CurrentTimestampImpl
		: public Function::CurrentTimestamp
	{
	public:
		typedef CurrentTimestampImpl This;
		typedef Function::CurrentTimestamp Super;

		// constructor
		CurrentTimestampImpl()
			: Super(),
			  m_pTimestamp(0),
			  m_cData()
		{}
		CurrentTimestampImpl(int iDataID_)
			: Super(),
			  m_pTimestamp(0),
			  m_cData(iDataID_)
		{}

		// destructor
		~CurrentTimestampImpl() {}

	///////////////////////////////////
	// Function::CurrentTimestamp::

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
	//	virtual void accumulate(Interface::IProgram& cProgram_,
	//							Action::ActionList& cActionList_);
		virtual void finish(Interface::IProgram& cProgram_);
		virtual void reset(Interface::IProgram& cProgram_);
		virtual void undone(Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		void serialize(ModArchive& archiver_);

	protected:
	private:
		Action::Timestamp* m_pTimestamp;
		Action::DataHolder m_cData;
	};
} // namespace Impl

////////////////////////////////////////////////////////////
// Execution::Function::Impl::CurrentTimestampImpl

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::explain -- 
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
Impl::CurrentTimestampImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" to ");
		m_cData.explain(cProgram_, cExplain_);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::initialize -- 
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
Impl::CurrentTimestampImpl::
initialize(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		m_cData.initialize(cProgram_);
		m_pTimestamp = Action::Timestamp::create(cProgram_);
	}
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::terminate -- 
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
Impl::CurrentTimestampImpl::
terminate(Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == true) {
		m_cData.terminate(cProgram_);
		m_pTimestamp = 0;
	}
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::finish -- 
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
Impl::CurrentTimestampImpl::
finish(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::serialize -- 
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
Impl::CurrentTimestampImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cData.serialize(archiver_);
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::execute -- 
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
Impl::CurrentTimestampImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	if (isDone() == false) {
		m_pTimestamp->assign(m_cData.get());
		done();
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::reset -- 
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
Impl::CurrentTimestampImpl::
reset(Interface::IProgram& cProgram_)
{
	m_pTimestamp->reset();
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::undone -- 
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
Impl::CurrentTimestampImpl::
undone(Interface::IProgram& cProgram_)
{
	Super::undone(cProgram_);
	m_pTimestamp->reset();
}

// FUNCTION public
//	Function::Impl::CurrentTimestampImpl::getClassID -- 
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
Impl::CurrentTimestampImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::CurrentTimestamp);
}

/////////////////////////////////
// Function::CurrentTimestamp

// FUNCTION public
//	Function::CurrentTimestamp::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	CurrentTimestamp*
//
// EXCEPTIONS

//static
CurrentTimestamp*
CurrentTimestamp::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::CurrentTimestampImpl(iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Function::CurrentTimestamp::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	CurrentTimestamp*
//
// EXCEPTIONS

//static
CurrentTimestamp*
CurrentTimestamp::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::CurrentTimestamp);
	return new Impl::CurrentTimestampImpl;
}

_SYDNEY_EXECUTION_FUNCTION_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
