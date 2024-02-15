// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Sort.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_SORT_H
#define __SYDNEY_PLAN_CANDIDATE_SORT_H

#include "Plan/Candidate/Base.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::Sort -- implementation class of Interface::ICandidate for sort
//
// NOTES
class Sort
	: public Base
{
public:
	typedef Sort This;
	typedef Base Super;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Order::Specification* pSpecification_,
						Interface::ICandidate* pOperand_);
	struct Partial
	{
		static This* create(Opt::Environment& cEnvironment_,
							Order::Specification* pSpecification_,
							const AccessPlan::Limit& cLimit_,
							Interface::ICandidate* pOperand_);
	};

	// destructor
	virtual ~Sort() {}

/////////////////////////////
// Interface::ICandidate::
	virtual Order::Specification* getOrder() {return m_pSpecification;}

protected:
	// constructor
	Sort(Order::Specification* pSpecification_)
		: Super(),
		  m_pSpecification(pSpecification_)
	{}

	//accessor
	void setOrder(Order::Specification* pOrder_) {m_pSpecification = pOrder_;}

private:
	Order::Specification* m_pSpecification;
};

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_SORT_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
