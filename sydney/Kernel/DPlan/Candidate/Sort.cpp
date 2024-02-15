// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Sort.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"

#include "DPlan/Candidate/Sort.h"
#include "DPlan/Candidate/Impl/SortImpl.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Sort

// FUNCTION public
//	Candidate::Sort::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Order::Specification* pSpecification_
//	Plan::Interface::ICandidate* pOperand_
//	
// RETURN
//	Sort*
//
// EXCEPTIONS

//static
Sort*
Sort::
create(Opt::Environment& cEnvironment_,
	   Plan::Order::Specification* pSpecification_,
	   Plan::Interface::ICandidate* pOperand_)
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
//	Plan::Order::Specification* pSpecification_
//	const Plan::AccessPlan::Limit& cLimit_
//	Plan::Interface::ICandidate* pOperand_
//	
// RETURN
//	Sort*
//
// EXCEPTIONS

//static
Sort*
Sort::Partial::
create(Opt::Environment& cEnvironment_,
	   Plan::Order::Specification* pSpecification_,
	   const Plan::AccessPlan::Limit& cLimit_,
	   Plan::Interface::ICandidate* pOperand_)
{
   	_SYDNEY_THROW0(Exception::NotSupported);
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
