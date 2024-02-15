// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Sort.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"

#include "Plan/Candidate/Sort.h"
#include "Plan/Candidate/Impl/SortImpl.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Sort

// FUNCTION public
//	Candidate::Sort::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Sort*
//
// EXCEPTIONS

//static
Sort*
Sort::
create(Opt::Environment& cEnvironment_,
	   Order::Specification* pSpecification_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new SortImpl::Normal(pSpecification_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////////////
//	Plan::Candidate::Sort::Partial::

// FUNCTION public
//	Candidate::Sort::Partial::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	const AccessPlan::Limit& cLimit_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Sort*
//
// EXCEPTIONS

//static
Sort*
Sort::Partial::
create(Opt::Environment& cEnvironment_,
	   Order::Specification* pSpecification_,
	   const AccessPlan::Limit& cLimit_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new SortImpl::Partial(pSpecification_, cLimit_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
