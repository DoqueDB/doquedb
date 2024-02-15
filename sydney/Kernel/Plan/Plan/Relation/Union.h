// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Union.h --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_UNION_H
#define __SYDNEY_PLAN_RELATION_UNION_H

#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Union -- Interface implementation for unioned table
//
//	NOTES
class Union
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Union This;

	// costructor
	struct DistinctByKey
	{
		static Interface::IRelation*
						create(Opt::Environment& cEnvironment_,
							   const VECTOR<Interface::IRelation*>& vecOperand_);
	};
	struct Distinct
	{
		static Interface::IRelation*
						create(Opt::Environment& cEnvironment_,
							   const VECTOR<Interface::IRelation*>& vecOperand_);
	};
	struct All
	{
		static Interface::IRelation*
						create(Opt::Environment& cEnvironment_,
							   const VECTOR<Interface::IRelation*>& vecOperand_);
	};
	// destructor
	virtual ~Union() {}

	// get union type
	virtual bool isAll() = 0;
	virtual bool isDistinct() = 0;
	virtual bool isDistinctByKey() = 0;

protected:
	// constructor
	Union()
		: Super(Tree::Node::Union)
	{}

private:
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_UNION_H

//
//	Copyright (c) 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
