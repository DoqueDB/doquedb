// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Between.h --
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

#ifndef __SYDNEY_PLAN_PREDICATE_BETWEEN_H
#define __SYDNEY_PLAN_PREDICATE_BETWEEN_H

#include "Plan/Interface/IPredicate.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::Between -- Predicate interface for between predicate
//
//	NOTES
//		This class will not created directly

class Between
	: public Interface::IPredicate
{
public:
	typedef Interface::IPredicate Super;
	typedef Between This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						const VECTOR<Interface::IScalar*>& vecScalar_,
						bool bIsNot_);
	// destructor
	virtual ~Between() {}

protected:
	// constructor
	Between();

private:
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_BETWEEN_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
