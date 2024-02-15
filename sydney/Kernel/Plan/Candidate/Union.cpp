// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Union.cpp --
// 
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
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
#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Union.h"
#include "Plan/Candidate/Impl/UnionImpl.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////
// Plan::Candidate::Union::

// FUNCTION public
//	Candidate::Union::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Union* pUnion_
//	const VECTOR<Interface::ICandidate*>& vecOperand_
//	
// RETURN
//	Union*
//
// EXCEPTIONS

//static
Union*
Union::
create(Opt::Environment& cEnvironment_,
	   Relation::Union* pUnion_,
	   const VECTOR<Interface::ICandidate*>& vecOperand_,
	   AccessPlan::Source& cPlanSource_)
{
	AUTOPOINTER<This> pResult;
	if (cPlanSource_.getOrder()) {
		pResult = new UnionImpl::Sort(pUnion_, vecOperand_, cPlanSource_.getOrder());
	} else {
		pResult = new UnionImpl::Cascade(pUnion_, vecOperand_);
	}
													   
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
