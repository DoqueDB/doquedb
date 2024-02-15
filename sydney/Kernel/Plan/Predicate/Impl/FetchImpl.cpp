// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/FetchImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Predicate::Impl";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Plan/Predicate/Impl/FetchImpl.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IProgram.h"

#include "Schema/Field.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_PREDICATE_USING

namespace
{
	// compare
	class _Compare
	{
	public:
		bool operator()(Interface::IScalar* pScalar0_,
						Interface::IScalar* pScalar1_) const
		{
			Scalar::Field* pField0 = pScalar0_->getField();
			Scalar::Field* pField1 = pScalar1_->getField();
			; _SYDNEY_ASSERT(pField0);
			; _SYDNEY_ASSERT(pField1);
			; _SYDNEY_ASSERT(pField0->getSchemaField());
			; _SYDNEY_ASSERT(pField1->getSchemaField());
			return pField0->getSchemaField()->getPosition()
				< pField1->getSchemaField()->getPosition();
		}
	};
}

////////////////////////////////////
//	Predicate::Impl::SingleFetch

// FUNCTION public
//	Predicate::Impl::SingleFetch::getKeyAndValue -- 
//
// NOTES
//
// ARGUMENTS
//	VECTOR<Interface::IScalar*>& vecKey_
//	VECTOR<Interface::IScalar*>& vecValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::SingleFetch::
getKeyAndValue(VECTOR<Interface::IScalar*>& vecKey_,
			   VECTOR<Interface::IScalar*>& vecValue_)
{
	if (getOperand1()->isArbitraryElement()) {
		// can't merge fetch
		return false;
	}
	VECTOR<Interface::IScalar*>::ITERATOR iterator =
		LOWERBOUND(vecKey_.begin(), vecKey_.end(), getOperand0(), _Compare());
	SIZE pos = iterator - vecKey_.begin();

	vecKey_.insert(iterator, getOperand0());
	vecValue_.insert(vecValue_.begin() + pos, getOperand1());

	return true;
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::delay -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::SingleFetch::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	// fetch condition can't be delayed
	require(cEnvironment_, pCandidate_);
	return false;
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::SingleFetch::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::getFetchKey -- get fetch key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::SingleFetch::
getFetchKey(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	if (cFetchKey_.isContaining(getOperand0())) {
		// fetchkey is already used
		return false;
	}
	cFetchKey_.add(getOperand0());
	return true;
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::generateKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::SingleFetch::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecID;
	vecID.pushBack(getOperand1()->generate(cEnvironment_, cProgram_, pIterator_, cArgument_));
	return cProgram_.addVariable(vecID);
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::SingleFetch::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	return m_pOriginal->estimateCost(cEnvironment_,
									 cResult_);
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SingleFetch::
getUsedTable(Utility::RelationSet& cResult_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->getUsedTable(cResult_);
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SingleFetch::
getUsedField(Utility::FieldSet& cResult_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->getUsedField(cResult_);
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SingleFetch::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->getUnknownKey(cEnvironment_,
							   cResult_);
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::SingleFetch::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->explain(pEnvironment_, cExplain_);
}


// FUNCTION public
//	Predicate::Impl::SingleFetch::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Encoder& cEnconder_
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
STRING
Impl::SingleFetch::
toSQLStatement(Opt::Environment& cEnvironment_, const Plan::Sql::QueryArgument& cArgument_) const
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return m_pOriginal->toSQLStatement(cEnvironment_, cArgument_);
}


// FUNCTION public
//	Predicate::Impl::SigleFietch::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	const Plan::Sql::QueryArgument& cArgument_
//
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
Impl::SingleFetch::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return m_pOriginal->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);
}

// FUNCTION public
//	Predicate::Impl::SingleFetch::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::SingleFetch::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// generate using original
	return m_pOriginal->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}

////////////////////////////////////
//	Predicate::Impl::MultipleFetch

// FUNCTION public
//	Predicate::Impl::MultipleFetch::getKeyAndValue -- 
//
// NOTES
//
// ARGUMENTS
//	VECTOR<Interface::IScalar*>& vecKey_
//	VECTOR<Interface::IScalar*>& vecValue_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::MultipleFetch::
getKeyAndValue(VECTOR<Interface::IScalar*>& vecKey_,
			   VECTOR<Interface::IScalar*>& vecValue_)
{
	VECTOR<Interface::IScalar*>::ITERATOR keyIterator = m_cKey.begin();
	const VECTOR<Interface::IScalar*>::ITERATOR keyLast = m_cKey.end();
	VECTOR<Interface::IScalar*>::ITERATOR valueIterator = m_vecValue.begin();
	const VECTOR<Interface::IScalar*>::ITERATOR valueLast = m_vecValue.end();
	; _SYDNEY_ASSERT(m_cKey.GETSIZE() == m_vecValue.GETSIZE());

	for (; keyIterator != keyLast; ++keyIterator, ++valueIterator) {
		VECTOR<Interface::IScalar*>::ITERATOR iterator =
			LOWERBOUND(vecKey_.begin(), vecKey_.end(), *keyIterator, _Compare());
		SIZE pos = iterator - vecKey_.begin();

		vecKey_.insert(iterator, *keyIterator);
		vecValue_.insert(vecValue_.begin() + pos, *valueIterator);
	}
	return true;
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::require -- extract scalar operands refering a relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->require(cEnvironment_,
						 pCandidate_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::retrieve -- extract scalar operands refering a relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
retrieve(Opt::Environment& cEnvironment_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->retrieve(cEnvironment_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::retrieve -- extract scalar operands refering a relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->retrieve(cEnvironment_,
						  pCandidate_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::use -- extract scalar operands refering a relation
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->use(cEnvironment_,
					 pCandidate_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::delay -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::MultipleFetch::
delay(Opt::Environment& cEnvironment_,
	  Interface::ICandidate* pCandidate_,
	  Scalar::DelayArgument& cArgument_)
{
	// fetch condition can't be delayed
	require(cEnvironment_, pCandidate_);
	return false;
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::MultipleFetch::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// never called
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::getFetchKey -- get fetch key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::ScalarSet& cFetchKey_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::MultipleFetch::
getFetchKey(Opt::Environment& cEnvironment_,
			Utility::ScalarSet& cFetchKey_)
{
	if (Opt::IsAny(m_cKey.begin(), m_cKey.end(),
				   boost::bind(&Utility::ScalarSet::isContainingObject,
							   boost::cref(cFetchKey_),
							   _1))) {
		return false;
	}
	cFetchKey_.add(m_cKey.begin(), m_cKey.end());
	return true;
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::generateKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::MultipleFetch::
generateKey(Opt::Environment& cEnvironment_,
			Execution::Interface::IProgram& cProgram_,
			Execution::Interface::IIterator* pIterator_,
			Candidate::AdoptArgument& cArgument_)
{
	VECTOR<int> vecID;
	Opt::MapContainer(m_vecValue, vecID,
					  boost::bind(&Interface::IScalar::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pIterator_,
								  boost::ref(cArgument_)));
	return cProgram_.addVariable(vecID);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::MultipleFetch::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	return m_pOriginal->estimateCost(cEnvironment_,
									 cResult_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
getUsedTable(Utility::RelationSet& cResult_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->getUsedTable(cResult_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
getUsedField(Utility::FieldSet& cResult_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->getUsedField(cResult_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->getUnknownKey(cEnvironment_,
							   cResult_);
}

// FUNCTION public
//	Predicate::Impl::MultipleFetch::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	m_pOriginal->explain(pEnvironment_, cExplain_);
}


// FUNCTION public
//	Predicate::Impl::MultipleFetch::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Encoder& cEnconder_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
STRING
Impl::MultipleFetch::
toSQLStatement(Opt::Environment& cEnvironment_, const Plan::Sql::QueryArgument& cArgument_) const

{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	return m_pOriginal->toSQLStatement(cEnvironment_, cArgument_);
}



// FUNCTION public
//	Predicate::Impl::NotImpl::setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
Impl::MultipleFetch::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	return m_pOriginal->setParameter(cEnvironment_, cProgram_, pIterator_, cExec_, cArgument_);
}


// FUNCTION public
//	Predicate::Impl::MultipleFetch::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::MultipleFetch::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	if (m_pOriginal == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// generate using original
	return m_pOriginal->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
