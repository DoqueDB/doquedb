// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Projection.h --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_CANDIDATE_PROJECTION_H
#define __SYDNEY_DPLAN_CANDIDATE_PROJECTION_H

#include "DPlan/Candidate/Module.h"
#include "Plan/Candidate/Base.h"
#include "Plan/Candidate/Monadic.h"

#include "Plan/Sql/Query.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::Projection -- implementation class of Interface::ICandidate for projection
//
// NOTES
class Projection
	: public Plan::Candidate::Monadic<Plan::Candidate::Base>
{
public:
	typedef Projection This;
	typedef Plan::Candidate::Monadic<Plan::Candidate::Base> Super;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Plan::Relation::Projection* pProjection_,
						Plan::Interface::ICandidate* pOperand_);

	// destructor
	~Projection() {}

/////////////////////////////
// Interface::ICandidate::
//	virtual void createCost(Opt::Environment& cEnvironment_,
//							const AccessPlan::Source& cPlanSource_);
//	virtual const AccessPlan::Cost& getCost();

	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
												   Execution::Interface::IProgram& cProgram_,
												   Plan::Candidate::AdoptArgument& cArgument_);

	virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

protected:
private:
	// constructor
	Projection(Plan::Relation::Projection* pProjection_,
			   Plan::Interface::ICandidate* pOperand_)
		: Super(pOperand_),
		  m_pProjection(pProjection_)
	{}

///////////////////////////////
// Candidate::Base::
	virtual Plan::Candidate::Row* createRow(Opt::Environment& cEnvironment_);
//	virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);

	Plan::Relation::Projection* m_pProjection;
};

_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_PROJECTION_H

//
//	Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
