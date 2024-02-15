// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Grouping.h --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_GROUPING_H
#define __SYDNEY_PLAN_CANDIDATE_GROUPING_H

#include "Plan/Candidate/Base.h"
#include "Plan/Candidate/Monadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::Grouping -- implementation class of Interface::ICandidate for grouping
//
// NOTES
class Grouping
	: public Monadic<Base>
{
public:
	typedef Grouping This;
	typedef Monadic<Base> Super;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Order::Specification* pSpecification_,
						const VECTOR<Interface::IScalar*>& vecAggregation_,
						const Utility::ScalarSet& cAggregationOperand_,
						Interface::ICandidate* pOperand_);

	// destructor
	~Grouping() {}

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
	Grouping(Order::Specification* pSpecification_,
			 const VECTOR<Interface::IScalar*>& vecAggregation_,
			 const Utility::ScalarSet& cAggregationOperand_,
			 Interface::ICandidate* pOperand_)
		: Super(pOperand_),
		  m_pSpecification(pSpecification_),
		  m_vecAggregation(vecAggregation_),
		  m_cAggregationOperand(cAggregationOperand_)
	{}

/////////////////////////////
// Candidate::Base::
	virtual void createCost(Opt::Environment& cEnvironment_,
							const AccessPlan::Source& cPlanSource_,
							AccessPlan::Cost& cCost_);
	virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
	virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);

	Order::Specification* m_pSpecification;
	VECTOR<Interface::IScalar*> m_vecAggregation;
	Utility::ScalarSet m_cAggregationOperand;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_GROUPING_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
