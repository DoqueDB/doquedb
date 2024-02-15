// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Unnest.h --
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

#ifndef __SYDNEY_PLAN_RELATION_UNNEST_H
#define __SYDNEY_PLAN_RELATION_UNNEST_H

#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Unnest -- Interface implementation for unnest such as table value constructor
//
//	NOTES
class Unnest
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Unnest This;

	// costructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IScalar* pValue_);

	// destructor
	virtual ~Unnest() {}

	// accessor
	virtual Interface::IScalar* getValue() = 0;

protected:
	// costructor
	Unnest();

private:
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_UNNEST_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
