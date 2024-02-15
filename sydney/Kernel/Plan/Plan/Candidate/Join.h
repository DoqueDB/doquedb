// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Join.h --
// 
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_CANDIDATE_JOIN_H
#define __SYDNEY_PLAN_CANDIDATE_JOIN_H

#include "Plan/Candidate/Base.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

//////////////////////////////////
// CLASS
//	Plan::Candidate::Join
//
// NOTES
class Join
	: public Base
{
public:
	typedef Join This;
	typedef Base Super;

	struct NestedLoop			// nested loop join
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Join* pJoin_);
	};
	struct Merge				// sort merge join (not used for now)
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Join* pJoin_);
	};
	struct Hash					// hash join (not used for now)
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Join* pJoin_);
	};
	struct Exists				// exists join
	{
		// constructor
		static This* create(Opt::Environment& cEnvironment_,
							Relation::Join* pJoin_);
	};

	// destructor
	virtual ~Join() {};

	////////////////////
	// creating plan

	// set first operand's plan
	virtual void setFirstPlan(Interface::ICandidate* pPlan_) = 0;
	// create rest operand's plan
	virtual void createPlan(Opt::Environment& cEnvironment_,
							AccessPlan::Source& cPlanSource_,
							Interface::IRelation* pRelation_) = 0;

	// get results
	virtual Interface::ICandidate* getFirstPlan() = 0;
	virtual Interface::ICandidate* getSecondPlan() = 0;

protected:
	// constructor
	Join(Relation::Join* pJoin_)
		: Super(),
		  m_pJoin(pJoin_)
	{}

	// accessor
	Relation::Join* getJoin() {return m_pJoin;}

private:
	Relation::Join* m_pJoin;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_JOIN_H

//
//	Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
