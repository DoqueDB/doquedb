// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/HasNextCandidate.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "DExecution/Predicate/HasNextCandidate.h"
#include "DExecution/Predicate/Class.h"
#include "DExecution/Action/Fulltext.h"

#include "Execution/Action/DataHolder.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Explain.h"


_SYDNEY_BEGIN
_SYDNEY_DEXECUTION_BEGIN
_SYDNEY_DEXECUTION_PREDICATE_BEGIN

namespace
{
	const char* const _pszExplainName = "has next candidate";
}
namespace Impl
{
	// CLASS local
	//	DExecution::Predicate::Impl::HasNextCandidateImpl -- implementation class of HasNextCandidate
	//
	// NOTES
	class HasNextCandidateImpl
		: public HasNextCandidate
	{
	public:
		typedef HasNextCandidateImpl This;
		typedef HasNextCandidate Super;

		// constructor
		HasNextCandidateImpl()
			: Super(),
			  m_cFulltext()
		{}
		HasNextCandidateImpl(int iDataID_)
							 
			: Super(),
			  m_cFulltext(iDataID_)
		{}

		// destructor
		~HasNextCandidateImpl() {}

	///////////////////////////
	// Predicate::HasNextCandidate::

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

		DExecution::Action::FulltextHolder m_cFulltext;
	};
} // namespace HasNextCandidateImpl

///////////////////////////////////////
// Predicate::Impl::HasNextCandidateImpl

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::explain -- 
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
Impl::HasNextCandidateImpl::
explain(Opt::Environment* pEnvironment_,
		Execution::Interface::IProgram& cProgram_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
		m_cFulltext.explain(pEnvironment_, cProgram_, cExplain_);
		cExplain_.put(" ");
	}
	cExplain_.put(_pszExplainName);
	if (cExplain_.isOn(Opt::Explain::Option::Data)) {
	}
	cExplain_.popNoNewLine();
}

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::initialize -- 
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
Impl::HasNextCandidateImpl::
initialize(Execution::Interface::IProgram& cProgram_)
{
	if (m_cFulltext.isInitialized() == false) {
		initializeBase(cProgram_);
		m_cFulltext.initialize(cProgram_);
	}
}

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::terminate -- 
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
Impl::HasNextCandidateImpl::
terminate(Execution::Interface::IProgram& cProgram_)
{
	if (m_cFulltext.isInitialized() == true) {
		m_cFulltext.terminate(cProgram_);
		terminateBase(cProgram_);
	}
}

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::finish -- 
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
Impl::HasNextCandidateImpl::
finish(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::reset -- 
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
Impl::HasNextCandidateImpl::
reset(Execution::Interface::IProgram& cProgram_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::getClassID -- 
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
Impl::HasNextCandidateImpl::
getClassID() const
{
	return Class::getClassID(Class::Category::HasNextCandidate);
}

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::serialize -- for serialize
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
Impl::HasNextCandidateImpl::
serialize(ModArchive& cArchive_)
{
	serializeID(cArchive_);
	m_cFulltext.serialize(cArchive_);
}

// FUNCTION public
//	Predicate::Impl::HasNextCandidateImpl::evaluate -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Action::ActionList& cActionList_
//	
// RETURN
//	HasNextCandidate::Boolean::Value
//
// EXCEPTIONS

//virtual
HasNextCandidate::Boolean::Value
Impl::HasNextCandidateImpl::
evaluate(Execution::Interface::IProgram& cProgram_,
		 Execution::Action::ActionList& cActionList_)
{
	return m_cFulltext->hasNextCandidate() ? Boolean::True
		: Boolean::False;
}

///////////////////////////////////
// Predicate::HasNextCandidate::

// FUNCTION public
//	Predicate::HasNextCandidate::create -- 
//
// NOTES
//
// ARGUMENTS
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	int iDataID_
//	
// RETURN
//	HasNextCandidate*
//
// EXCEPTIONS

//static
HasNextCandidate*
HasNextCandidate::
create(Execution::Interface::IProgram& cProgram_,
	   Execution::Interface::IIterator* pIterator_,
	   int iDataID_)
{
	AUTOPOINTER<This> pResult = new Impl::HasNextCandidateImpl(iDataID_);
	pResult->registerToProgram(cProgram_, pIterator_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::HasNextCandidate::getInstance -- for serialize
//
// NOTES
//
// ARGUMENTS
//	int iCategory_
//	
// RETURN
//	HasNextCandidate*
//
// EXCEPTIONS

//static
HasNextCandidate*
HasNextCandidate::
getInstance(int iCategory_)
{
	switch (iCategory_) {
	case Class::Category::HasNextCandidate:
		{
			return new Impl::HasNextCandidateImpl;
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
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
