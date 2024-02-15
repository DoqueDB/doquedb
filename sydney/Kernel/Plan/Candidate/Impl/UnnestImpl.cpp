// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/UnnestImpl.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Impl/UnnestImpl.h"
#include "Plan/Candidate/Unnest.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/Unnest.h"

#include "Common/Assert.h"

#include "Exception/Unexpected.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Array.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

//////////////////////////////////////////////
// Candidate::Impl::UnnestImpl

// FUNCTION public
//	Candidate::Impl::UnnestImpl::isReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::UnnestImpl::
isReferingRelation(Interface::IRelation* pRelation_)
{
	return pRelation_ == m_pRelation;
}

// FUNCTION public
//	Candidate::Impl::UnnestImpl::createReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cRelationSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
createReferingRelation(Utility::RelationSet& cRelationSet_)
{
	cRelationSet_.add(m_pRelation);
}

// FUNCTION public
//	Candidate::Impl::UnnestImpl::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
Impl::UnnestImpl::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	// create tuple iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Array::create(cProgram_);

	Interface::IScalar* pValue = m_pRelation->getValue();
	if (pValue == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	int iDataID = pValue->generate(cEnvironment_,
								   cProgram_,
								   pResult,
								   cArgument_);
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(InData,
										 iDataID));

	// set result tuple
	; _SYDNEY_ASSERT(m_pRelation->getDegree(cEnvironment_) == 1);
	Interface::IScalar* pVariable = m_pRelation->getScalar(cEnvironment_,
														   0);
	int iResultID = pVariable->generate(cEnvironment_,
										cProgram_,
										pResult,
										cArgument_);
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iResultID));

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);

		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	cArgument_.setCandidate(this);
	return pResult;
}

// FUNCTION private
//	Candidate::Unnest::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::UnnestImpl::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	cCost_.setOverhead(0);
	cCost_.setTupleCount(100); // dummy data
	AccessPlan::Cost::Value cSizeCost(8);
	cSizeCost /= static_cast<double>(LogicalFile::Estimate::getTransferSpeed(
													 LogicalFile::Estimate::Memory));
	cCost_.setTotalCost(cCost_.getTupleCount() * cSizeCost);
}

// FUNCTION private
//	Candidate::Impl::UnnestImpl::createRow -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
Impl::UnnestImpl::
createRow(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pResult = Candidate::Row::create(cEnvironment_);
	int n = m_pRelation->getDegree(cEnvironment_);
	for (int i = 0; i < n; ++i) {
		pResult->addScalar(m_pRelation->getScalar(cEnvironment_,
												  i));
	}
	return pResult;
}

// FUNCTION private
//	Candidate::Impl::UnnestImpl::createKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
Impl::UnnestImpl::
createKey(Opt::Environment& cEnvironment_)
{
	return Candidate::Row::create(cEnvironment_);
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
