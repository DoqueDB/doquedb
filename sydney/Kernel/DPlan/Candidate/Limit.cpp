// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Limit.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DPlan/Candidate/Limit.h"
#include "Plan/Candidate/Argument.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"

#include "Plan/Relation/Limit.h"


#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Operator/Limit.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	DPlan::Candidate::Limit

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
	   DPlan::Relation::Limit* pLimit_,
	   Plan::Interface::ICandidate* pOperand_)
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
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	// adopt operand -> used as result
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	Plan::AccessPlan::Limit cLimit(m_pLimit->getLimit());
	cMyArgument.m_cLimit = cLimit;
	Execution::Interface::IIterator* pResult =
		getOperand()->adopt(cEnvironment_, cProgram_, cMyArgument);
	
	PAIR<int, int> cLimitPair = cLimit.generate(cEnvironment_,
												cProgram_,
												pResult,
												cMyArgument);

	// operation limiting output number
	pResult->addCalculation(cProgram_,
							Execution::Operator::Limit::create(
								cProgram_,
								pResult,
								pResult->getID(),
								cLimitPair,
								false), // true in subprogram in distribution
							cMyArgument.m_eTarget);	

	return pResult;
}

// FUNCTION public
//	Candidate::Limit::Base::generateSQL
//
//
// NOTES
//	
//
// ARGUMENTS
//	Opt::Environment cEnvironment_
//
//	
// RETURN
//	Sql::Query*
//
// EXCEPTIONS

//virtual


Plan::Sql::Query*
Limit::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	pResult->setLimit(m_pLimit->getLimit().getDistributeSpec(cEnvironment_));
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
		   const Plan::AccessPlan::Source& cPlanSource_,
		   Plan::AccessPlan::Cost& cCost_)
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
