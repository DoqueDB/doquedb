// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/DistinctImpl.cpp --
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
const char moduleName[] = "Plan::Relation";
}

#include "SyDefault.h"

#include "Plan/Relation/Impl/DistinctImpl.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Distinct.h"
#include "Plan/Candidate/Row.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Tree/Monadic.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

/////////////////////////////////////
// Relation::Impl::DistinctImpl

// FUNCTION public
//	Relation::Impl::DistinctImpl::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
Impl::DistinctImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	// get rowinfo to distinct
	Utility::RowElementSet cDistinctKey;
	if (getRowInfo(cEnvironment_)->getDistinctKey(cEnvironment_, cDistinctKey) == false) {
		// if resultrow is already distinct, use operand's plan
		return getOperand()->createAccessPlan(cEnvironment_, cPlanSource_);
	}

	AccessPlan::Source cSource(cPlanSource_);
	cSource.eraseLimit();

	// create operand's access plan without limit specification
	Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cSource);

	// create key
	VECTOR<Interface::IScalar*> vecKey;
	Opt::MapContainer(cDistinctKey, vecKey,
					  boost::bind(&Relation::RowElement::getScalar,
								  _1,
								  boost::ref(cEnvironment_)));
	Opt::ForEach(vecKey,
				 boost::bind(&Interface::IScalar::require,
							 _1,
							 boost::ref(cEnvironment_),
							 pOperandCandidate));

	// create result
	Interface::ICandidate* pResult =
		Candidate::Distinct::create(cEnvironment_,
									Candidate::Row::create(cEnvironment_,
														   vecKey),
									pOperandCandidate);

	return pResult;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
