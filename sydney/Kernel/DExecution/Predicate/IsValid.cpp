// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/IsValid.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DExecution::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DExecution/Predicate/IsValid.h"
#include "DExecution/Predicate/Class.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Algorithm.h"
#include "Opt/Explain.h"

#include "Schema/Cascade.h"

_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszExplainName = "is valid";
}
namespace Impl
{
	// CLASS local
	//	DExecution::Predicate::Impl::IsValidImpl -- implementation class of IsValid
	//
	// NOTES
	class IsValidImpl
		: public IsValid
	{
	public:
		typedef IsValidImpl This;
		typedef IsValid Super;

		// constructor
		IsValidImpl()
			: Super(),
			  m_cstrCascadeName(),
			  m_cData()
		{}
		IsValidImpl(Schema::Cascade* pCascade_,
					int iDataID_)
			: Super(),
			  m_cstrCascadeName(pCascade_->getName()),
			  m_cData(iDataID_)
		{}

		// destructor
		~IsValidImpl() {}

	///////////////////////////
	// Predicate::IsValid::

	/////////////////////////////
	// Interface::IPredicate::

	/////////////////////////////
	// Interface::IAction::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Opt::Explain& cExplain_);
		virtual void initialize(Execution::Interface::IProgram& cProgram_);
		virtual void terminate(Execution::Interface::IProgram& cProgram_);

	//	virtual Action::Status::Value
	//				execute(Execution::Interface::IProgram& cProgram_,
	//						Execution::Action::ActionList& cActionList_);
		virtual void finish(Execution::Interface::IProgram& cProgram_);
		virtual void reset(Execution::Interface::IProgram& cProgram_);

	///////////////////////////////
	// Common::Externalizable
		int getClassID() const;

	///////////////////////////////
	// ModSerializer
		virtual void serialize(ModArchive& archiver_);

	protected:
	private:
	///////////////////////////////
	// Predicate::Base::
		virtual Boolean::Value evaluate(Execution::Interface::IProgram& cProgram_,
										Execution::Action::ActionList& cActionList_);

		STRING m_cstrCascadeName;
		Execution::Action::DataHolder m_cData;
	};
} // namespace IsValidImpl

///////////////////////////////////////
// Predicate::Impl::IsValidImpl

// FUNCTION public
//	Predicate::Impl::IsValidImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::IsValidImpl::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cData.explain(cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		cExplain_.put(" with ").put(m_cstrCascadeName);
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::Impl::IsValidImpl::initialize -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::IsValidImpl::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cData.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::Impl::IsValidImpl::terminate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::IsValidImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	if (m_cData.isInitialized() == true) {
		m_cData.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::Impl::IsValidImpl::finish -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::IsValidImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::IsValidImpl::reset -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::IsValidImpl::
reset(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::IsValidImpl::getClassID -- 
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
Impl::IsValidImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::IsValid);
}

// FUNCTION public
//	Predicate::Impl::IsValidImpl::serialize -- for serialize
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

//virtual
void
Impl::IsValidImpl::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cData.serialize(cArchive_);
	cArchive_(m_cstrCascadeName);
}

// FUNCTION public
//	Predicate::Impl::IsValidImpl::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::ActionList& cActionList_
//	
// RETURN
//	IsValid::Boolean::Value
//
// EXCEPTIONS

//virtual
IsValid::Boolean::Value
Impl::IsValidImpl::
evaluate(Execution::Interface::IProgram& cProgram_,
		 Execution::Action::ActionList& cActionList_)
{
	Common::StringData cData;
	cData.assign(m_cData.getData());
	return cData.getValue().compare(m_cstrCascadeName, ModFalse) == 0
		? Boolean::True
		: Boolean::False;
}

///////////////////////////////////
// Predicate::IsValid::

// FUNCTION public
//	Predicate::IsValid::create -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	IsValid*
//
// EXCEPTIONS

//static
IsValid*
IsValid::
create(Execution::Interface::IProgram& cProgram_,
	   Execution::Interface::IIterator* pIterator_,
	   Schema::Cascade* pCascade_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::IsValidImpl(pCascade_, iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::IsValid::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	IsValid*
//
// EXCEPTIONS

//static
IsValid*
IsValid::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::IsValid:
		{
			return new Impl::IsValidImpl;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}
}

_SYDNEY_DEXECUTION_PREDICATE_END
_SYDNEY_DEXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
