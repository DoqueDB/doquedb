// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Expand.h --
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

#ifndef __SYDNEY_PLAN_RELATION_EXPAND_H
#define __SYDNEY_PLAN_RELATION_EXPAND_H

#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Expand -- Interface implementation for expand
//
//	NOTES
class Expand
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Expand This;

	// costructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IRelation* pOperand_,
						Order::Specification* pSortSpecification_,
						Interface::IScalar* pLimit_);
	// destructor
	virtual ~Expand() {}

protected:
	// costructor
	Expand();

private:
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_EXPAND_H

//
//	Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
