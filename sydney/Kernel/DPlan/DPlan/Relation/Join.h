// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Join.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_RELATION_JOIN_H
#define __SYDNEY_DPLAN_RELATION_JOIN_H

#include "DPlan/Relation/Module.h"
#include "Plan/Relation/Join.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	DPlan::Relation::Join -- Interface implementation for joined table
//
//	NOTES
class Join
	: public Plan::Relation::Join
{
public:
	typedef Plan::Relation::Join Super;
	typedef Join This;

	// costructor
	static This* create(Opt::Environment& cEnvironment_,
						Plan::Tree::Node::Type eOperator_,
						Plan::Interface::IPredicate* pJoinPredicate_,
						const VECTOR<Plan::Interface::IRelation*>& vecOperand_);
	static This* create(Opt::Environment& cEnvironment_,
						Plan::Tree::Node::Type eOperator_,
						Plan::Interface::IPredicate* pJoinPredicate_,
						const PAIR<Plan::Interface::IRelation*, Plan::Interface::IRelation*>& cOperand_);
	// destructor
	virtual ~Join() {}

protected:
	// constructor
	Join(Plan::Tree::Node::Type eType_,
		 Plan::Interface::IPredicate* pJoinPredicate_)
		: Super(eType_, pJoinPredicate_)
	{}

private:

};

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_JOIN_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
