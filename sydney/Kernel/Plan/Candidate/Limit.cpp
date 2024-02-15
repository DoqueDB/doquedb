// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Limit.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Limit.h"
#include "Plan/Candidate/Argument.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Relation/Limit.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Operator/Limit.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Limit

// FUNCTION public
//	Candidate::Limit::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Limit* pLimit_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Limit*
//
// EXCEPTIONS

//static
Limit*
Limit::
create(Opt::Environment& cEnvironment_,
	   Relation::Limit* pLimit_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new Limit(pLimit_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Limit::adopt -- 
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
Limit::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	if (getOperand()->isLimitAvailable(cEnvironment_)) {
		// limit can be used as hint
		cArgument_.m_cLimit = m_pLimit->getLimit();
	}

	// adopt operand -> used as result
	Execution::Interface::IIterator* pResult =
		getOperand()->adopt(cEnvironment_, cProgram_, cArgument_);

	// clear limit hint
	cArgument_.m_cLimit = AccessPlan::Limit();

	if (m_pLimit->getLimit().isSpecified()) {
		AccessPlan::Limit cLimit(m_pLimit->getLimit());
		PAIR<int, int> cLimitPair = cLimit.generate(cEnvironment_,
													cProgram_,
													pResult,
													cArgument_);

		// operation limiting output number
		pResult->addCalculation(cProgram_,
								Execution::Operator::Limit::create(
										   cProgram_,
										   pResult,
										   pResult->getID(),
										   cLimitPair,
										   cLimit.isIntermediate()), // true in subprogram in distribution
								cArgument_.m_eTarget);
	}

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);

		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	return pResult;
}

// FUNCTION private
//	Candidate::Limit::createCost -- 
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
Limit::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		AccessPlan::Source cSource(cPlanSource_);
		AccessPlan::Cost::Value cEstimateLimit;
		if (m_pLimit->getLimit().estimateCount(cEnvironment_,
											   cEstimateLimit)) {
			cSource.setEstimateLimit(cEstimateLimit);
		}
		getOperand()->createCost(cEnvironment_, cSource);
		cCost_ = getOperand()->getCost();
	}
}

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
