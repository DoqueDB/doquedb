// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// AccessPlan/Source.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::AccessPlan";
}

#include "SyDefault.h"

#include "Plan/AccessPlan/Source.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Specification.h"
#include "Plan/Predicate/Combinator.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_ACCESSPLAN_USING

////////////////////////////////////
//	Plan::AccessPlan::Source

// FUNCTION public
//	AccessPlan::Source::isCheckPartial -- 
//
// NOTES
//
// ARGUMENTS
//	Order::Specification* pOrder_
//	const Limit& cLimit_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Source::
isCheckPartial(Order::Specification* pOrder_,
			   const Limit& cLimit_) const
{
	return pOrder_ && pOrder_->getKeySize() > 1
		&& cLimit_.isSpecified();
}

// FUNCTION public
//	AccessPlan::Source::checkJoinMaxCandidates -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//		true means the number of candidates reaches the limit
//
// EXCEPTIONS

bool
Source::
checkJoinMaxCandidates()
{
	if (m_iJoinMaxCandidates < 0) {
		// set initial value
		m_iJoinMaxCandidates = Opt::Configuration::getJoinMaxCandidates().get();
	} else if (m_iJoinMaxCandidates > 0) {
		--m_iJoinMaxCandidates;
	}
	return m_iJoinMaxCandidates == 0;
}

// FUNCTION public
//	AccessPlan::Source::addPrecedingCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Source::
addPrecedingCandidate(Interface::ICandidate* pCandidate_)
{
	m_vecPrecedingCandidate.PUSHFRONT(pCandidate_);
}

// FUNCTION public
//	AccessPlan::Source::addPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Source::
addPredicate(Opt::Environment& cEnvironment_,
			 Interface::IPredicate* pPredicate_)
{
	if (pPredicate_) {
		if (m_pPredicate) {
			m_pPredicate = Predicate::Combinator::create(cEnvironment_,
														 Tree::Node::And,
														 MAKEPAIR(m_pPredicate,
																  pPredicate_));
		} else {
			m_pPredicate = pPredicate_;
		}
	}
}

// FUNCTION public
//	AccessPlan::Source::estimateLimit -- 
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

void
Source::
estimateLimit(Opt::Environment& cEnvironment_)
{
	if (m_cLimit.isSpecified()) {
		(void) m_cLimit.estimateCount(cEnvironment_, m_cEstimateLimit);
	}
}

//
// Copyright (c) 2008, 2009, 2010, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
