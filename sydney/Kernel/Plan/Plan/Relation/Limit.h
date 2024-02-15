// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Limit.h --
// 
// Copyright (c) 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_LIMIT_H
#define __SYDNEY_PLAN_RELATION_LIMIT_H

#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Limit -- Interface implementation for limit
//
//	NOTES
class Limit
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Limit This;

	// costructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IScalar* pLimit_,
						Interface::IRelation* pOperand_);
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IScalar* pLimit_,
						Interface::IScalar* pOffset_,
						Interface::IRelation* pOperand_);
	// destructor
	virtual ~Limit() {}

	// get limit specification
	virtual const AccessPlan::Limit& getLimit() = 0;

protected:
	// costructor
	Limit();

private:
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_LIMIT_H

//
//	Copyright (c) 2008, 2009, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
