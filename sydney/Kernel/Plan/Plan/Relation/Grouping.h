// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Grouping.h --
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

#ifndef __SYDNEY_PLAN_RELATION_GROUPING_H
#define __SYDNEY_PLAN_RELATION_GROUPING_H

#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Grouping -- Interface implementation for grouping
//
//	NOTES
class Grouping
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Grouping This;

	// costructor
	struct Simple
	{
		// simple case
		static This* create(Opt::Environment& cEnvironment_,
							Order::Specification* pSortSpecification_,
							Interface::IRelation* pOperand_);
	};
	// normal case
	static This* create(Opt::Environment& cEnvironment_,
						Order::Specification* pSortSpecification_,
						Interface::IRelation* pOperand_);
	// destructor
	virtual ~Grouping() {}

protected:
	// costructor
	Grouping();

private:
	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_GROUPING_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
