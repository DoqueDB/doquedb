// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Bulk.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_BULK_H
#define __SYDNEY_PLAN_RELATION_BULK_H

#include "Plan/Interface/IRelation.h"
#include "Plan/Utility/Algorithm.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Bulk -- Interface implementation for bulk input/output
//
//	NOTES
class Bulk
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Bulk This;
	typedef Utility::Storage3<Interface::IScalar*,
							  Interface::IScalar*,
							  Interface::IScalar*> Argument;

	// costructor
	struct Input
	{
		static Interface::IRelation*
						create(Opt::Environment& cEnvironment_,
							   const Argument& cArgument_);
	};
	struct Output
	{
		static Interface::IRelation*
						create(Opt::Environment& cEnvironment_,
							   const Argument& cArgument_,
							   Interface::IRelation* pInput_);
	};
	// destructor
	virtual ~Bulk() {}

	// get bulk options
	virtual const Argument& getBulkOption() = 0;

protected:
	// constructor
	Bulk()
		: Super(Tree::Node::Undefined)
	{}

private:
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_BULK_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
