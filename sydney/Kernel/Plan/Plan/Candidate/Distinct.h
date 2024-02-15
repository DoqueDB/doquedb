// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Distinct.h --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_DISTINCT_H
#define __SYDNEY_PLAN_CANDIDATE_DISTINCT_H

#include "Plan/Candidate/Base.h"
#include "Plan/Candidate/Monadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::Distinct -- implementation class of Interface::ICandidate for distinct
//
// NOTES
class Distinct
	: public Monadic<Base>
{
public:
	typedef Distinct This;
	typedef Monadic<Base> Super;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Candidate::Row* pKey_,
						Interface::ICandidate* pOperand_);

	// destructor
	~Distinct() {}

/////////////////////////////
// Interface::ICandidate::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_);
//	virtual const AccessPlan::Cost& getCost();

	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
												   Execution::Interface::IProgram& cProgram_,
												   Candidate::AdoptArgument& cArgument_);

	virtual Sql::Query* generateSQL(Opt::Environment& cEnvironment_)
	{
		Sql::Query* pResult = Super::generateSQL(cEnvironment_);
		if (pResult != 0) {
			pResult->setDistinct();
		}
		
		return pResult;
	}
protected:
private:
	// constructor
	Distinct(Candidate::Row* pKey_,
			 Interface::ICandidate* pOperand_)
		: Super(pOperand_),
		  m_pKey(pKey_)
	{}

//////////////////////////
// Candidate::Base::
	virtual void createCost(Opt::Environment& cEnvironment_,
							const AccessPlan::Source& cPlanSource_,
							AccessPlan::Cost& cCost_);

	bool isReplicate(Opt::Environment& cEnvironment_);

	Candidate::Row* m_pKey;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_DISTINCT_H

//
//	Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
