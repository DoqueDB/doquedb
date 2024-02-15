// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operator/Throw.cpp --
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

#include "Execution/Operator/Throw.h"
#include "Execution/Operator/Class.h"
#include "Execution/Interface/IProgram.h"

#include "Exception/Object.h"

#include "Common/Assert.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_OPERATOR_BEGIN

namespace
{
	// CONST
	//	_pszOperatorName -- operator name for explain
	//
	// NOTES
	const char* const _pszOperatorName = "throw exception";
}

namespace Impl
{

	// CLASS local
	//	Execution::Operator::Impl::ThrowImpl -- implementation class of Throw
	//
	// NOTES
	class ThrowImpl
		: public Operator::Throw
	{
	public:
		typedef ThrowImpl This;
		typedef Operator::Throw Super;

		ThrowImpl()
			: Super(),
			  m_cException()
		{}
		ThrowImpl(const Exception::Object& cException_)
			: Super(),
			  m_cException(cException_)
		{}
		~ThrowImpl()
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
		Exception::Object m_cException;
	};
}

/////////////////////////////////////////////
// Execution::Operator::Impl::ThrowImpl

// FUNCTION public
//	Operator::Impl::ThrowImpl::explain -- 
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
Impl::ThrowImpl::
explain(Opt::Environment* pEnvironment_,
		Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
}

// FUNCTION public
//	Operator::Impl::ThrowImpl::initialize -- 
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
Impl::ThrowImpl::
initialize(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::ThrowImpl::terminate -- 
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
Impl::ThrowImpl::
terminate(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::ThrowImpl::execute -- 
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
Impl::ThrowImpl::
execute(Interface::IProgram& cProgram_,
		Action::ActionList& cActionList_)
{
	Exception::throwClassInstance(m_cException);
	return Action::Status::Success; // never reached
}

// FUNCTION public
//	Operator::Impl::ThrowImpl::finish -- 
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
Impl::ThrowImpl::
finish(Interface::IProgram& cProgram_)
{
	; // do nothing
}

// FUNCTION public
//	Operator::Impl::ThrowImpl::reset -- 
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
Impl::ThrowImpl::
reset(Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Operator::Impl::ThrowImpl::getClassID -- 
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
Impl::ThrowImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::Throw);
}

// FUNCTION public
//	Operator::Impl::ThrowImpl::serialize -- 
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
Impl::ThrowImpl::
serialize(ModArchive& archiver_)
{
	serializeID(archiver_);
	m_cException.serialize(archiver_);
}

//////////////////////////////
// Operator::Throw::

// FUNCTION public
//	Operator::Throw::create -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IProgram& cProgram_
//	Interface::IIterator* pIterator_
//	const Exception::Object& cException_
//	
// RETURN
//	Throw*
//
// EXCEPTIONS

//static
Throw*
Throw::
create(Interface::IProgram& cProgram_,
	   Interface::IIterator* pIterator_,
	   const Exception::Object& cException_)
{
	AUTOPOINTER<This> pResult = new Impl::ThrowImpl(cException_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Operator::Throw::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	Throw*
//
// EXCEPTIONS

//static
Throw*
Throw::
getInstance(int iCategory_)
{
	; _SYDNEY_ASSERT(iCategory_ == Class::Category::Throw);
	return new Impl::ThrowImpl;
}

_SYDNEY_EXECUTION_OPERATOR_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
