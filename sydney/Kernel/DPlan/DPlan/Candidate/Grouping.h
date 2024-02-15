// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Grouping.h --
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

#ifndef __SYDNEY_DPLAN_CANDIDATE_GROUPING_H
#define __SYDNEY_DPLAN_CANDIDATE_GROUPING_H

#include "DPlan/Module.h"
#include "DPlan/Declaration.h"

#include "Plan/Candidate/Base.h"



_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::Grouping -- implementation class of Interface::ICandidate for grouping
//
// NOTES
class Grouping
	: public Plan::Candidate::Base
{
public:
	typedef Grouping This;
	typedef Plan::Candidate::Base Super;

	struct Normal
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Plan::Order::Specification* pSpecification_,
							const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
							const Plan::Utility::ScalarSet& cAggregationOperand_,
							Plan::Interface::ICandidate* pOperand_);
	};
	
	struct Simple
	{
		static This* create(Opt::Environment& cEnvironment_,
							const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
							const Plan::Utility::ScalarSet& cAggregationOperand_,
							Plan::Interface::ICandidate* pOperand_);
	};

	struct Replicate
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Plan::Order::Specification* pSpecification_,
							const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
							const Plan::Utility::ScalarSet& cAggregationOperand_,
							Plan::Interface::ICandidate* pOperand_);
	};

	// destructor
	virtual ~Grouping() {}


protected:
	Grouping()
		:Super()
	{}
	
private:
};

_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_GROUPING_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
