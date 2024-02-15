// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Grouping.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "DPlan/Candidate/Grouping.h"
#include "DPlan/Candidate/Impl/GroupingImpl.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"
#include "Plan/Candidate/Table.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Relation/Grouping.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Collection/Grouping.h"
#include "Execution/Iterator/Filter.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Grouping

// FUNCTION public
//	Candidate::Grouping::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	const VECTOR<Interface::IScalar*>& vecAggregation_
//	const Utility::ScalarSet& cAggregationOperand_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Grouping*
//
// EXCEPTIONS

//static
Grouping*
Grouping::Normal::
create(Opt::Environment& cEnvironment_,
	   Plan::Order::Specification* pSpecification_,
	   const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
	   const Plan::Utility::ScalarSet& cAggregationOperand_,
	   Plan::Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new GroupingImpl::Normal(pSpecification_,
														 vecAggregation_,
														 cAggregationOperand_,
														 pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Grouping::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	const VECTOR<Interface::IScalar*>& vecAggregation_
//	const Utility::ScalarSet& cAggregationOperand_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Grouping*
//
// EXCEPTIONS

//static
Grouping*
Grouping::Simple::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
	   const Plan::Utility::ScalarSet& cAggregationOperand_,
	   Plan::Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new GroupingImpl::Simple(vecAggregation_,
														 cAggregationOperand_,
														 pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Candidate::Grouping::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	const VECTOR<Interface::IScalar*>& vecAggregation_
//	const Utility::ScalarSet& cAggregationOperand_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Grouping*
//
// EXCEPTIONS

//static
Grouping*
Grouping::Replicate::
create(Opt::Environment& cEnvironment_,
	   Plan::Order::Specification* pSpecification_,
	   const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
	   const Plan::Utility::ScalarSet& cAggregationOperand_,
	   Plan::Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new GroupingImpl::Replicate(pSpecification_,
															vecAggregation_,
															cAggregationOperand_,
															pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
