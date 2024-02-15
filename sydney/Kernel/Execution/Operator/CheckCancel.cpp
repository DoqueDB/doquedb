// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/CheckCancel.cpp --
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
const char moduleName[] = "Execution::Operator";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Execution/Operator/CheckCancel.h"
#include "Execution/Operator/Class.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Utility/FakeError.h"

#include "Exception/Cancel.h"

#include "Common/Assert.h"

#include "Opt/Explain.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "check cancel";
}

namespace Impl
{

	// CLASS local
	//	Execution::Operator::Impl::CheckCancelImpl -- implementation class of CheckCancel
	//
	// NOTES
	class CheckCancelImpl
		: public Operator::CheckCancel
	{
	public:
		typedef CheckCancelImpl This;
		typedef Operator::CheckCancel Super;

		CheckCancelImpl()
			: Super(),
			  m_pTransaction(0)
		{}
		~CheckCancelImpl()
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
	private:
		Trans::Transaction* m_pTransaction;
	};
}

/////////////////////////////////////////////
// Execution::Operator::Impl::CheckCancelImpl

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::explain -- 
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
Impl::CheckCancelImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
}

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::initialize -- 
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
Impl::CheckCancelImpl::
initialize(Interface::IProgram& cProgram_)
{
	m_pTransaction = cProgram_.getTransaction();
}

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::terminate -- 
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
Impl::CheckCancelImpl::
terminate(Interface::IProgram& cProgram_)
{
	m_pTransaction = 0;
}

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::execute -- 
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
Impl::CheckCancelImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	_EXECUTION_FAKE_ERROR(Executor::execute_IsInterrupted, Cancel);

	if (m_pTransaction->isCanceledStatement()) {
		_SYDNEY_THROW0(Exception::Cancel);
	}
	return Action::Status::Success;
}

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::finish -- 
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
Impl::CheckCancelImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::reset -- 
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
Impl::CheckCancelImpl::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::getClassID -- 
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
Impl::CheckCancelImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::CheckCancel);
}

// FUNCTION public
//	Operator::Impl::CheckCancelImpl::serialize -- 
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
Impl::CheckCancelImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
}

//////////////////////////////
// Operator::CheckCancel::

// FUNCTION public
//	Operator::CheckCancel::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	
// RETURN
//	CheckCancel*
//
// EXCEPTIONS

//static
CheckCancel*
CheckCancel::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_)
{
	AUTOPOINTER<This> pResult = new Impl::CheckCancelImpl;
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::CheckCancel::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	CheckCancel*
//
// EXCEPTIONS

//static
CheckCancel*
CheckCancel::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::CheckCancel);
	return new Impl::CheckCancelImpl;
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
