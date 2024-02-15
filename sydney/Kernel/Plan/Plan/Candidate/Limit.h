// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Limit.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_LIMIT_H
#define __SYDNEY_PLAN_CANDIDATE_LIMIT_H

#include "Plan/Candidate/Base.h"
#include "Plan/Candidate/Monadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::Limit -- implementation class of Interface::ICandidate for limit
//
// NOTES
class Limit
	: public Monadic<Base>
{
public:
	typedef Limit This;
	typedef Monadic<Base> Super;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Relation::Limit* pLimit_,
						Interface::ICandidate* pOperand_);

	// destructor
	~Limit() {}

/////////////////////////////
// Interface::ICandidate::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_);
//	virtual const AccessPlan::Cost& getCost();

	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
												   Execution::Interface::IProgram& cProgram_,
												   Candidate::AdoptArgument& cArgument_);
protected:
private:
	// constructor
	Limit(Relation::Limit* pLimit_,
		  Interface::ICandidate* pOperand_)
		: Super(pOperand_),
		  m_pLimit(pLimit_)
	{}

///////////////////////////////
// Candidate::Base::
	virtual void createCost(Opt::Environment& cEnvironment_,
							const AccessPlan::Source& cPlanSource_,
							AccessPlan::Cost& cCost_);

	Relation::Limit* m_pLimit;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_LIMIT_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
