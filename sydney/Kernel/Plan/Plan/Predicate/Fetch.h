// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Fetch.h --
// 
// Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_FETCH_H
#define __SYDNEY_PLAN_PREDICATE_FETCH_H

#include "Plan/Interface/IPredicate.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::Fetch -- Predicate interface for fetch predicate
//
//	NOTES
//		This class will not created directly

class Fetch
	: public Interface::IPredicate
{
public:
	typedef Interface::IPredicate Super;
	typedef Fetch This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pOriginal_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_);
	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Interface::IPredicate* pOriginal_,
						const VECTOR<Interface::IScalar*>& vecKey_,
						const VECTOR<Interface::IScalar*>& vecValue_);
	// destructor
	virtual ~Fetch() {}

	// merge multiple fetch
	static Interface::IPredicate* merge(Opt::Environment& cEnvironment_,
										Interface::IPredicate* pOriginal_,
										const VECTOR<Interface::IPredicate*>& vecChecked_);

	// take keys and values
	virtual bool getKeyAndValue(VECTOR<Interface::IScalar*>& vecKey_,
								VECTOR<Interface::IScalar*>& vecValue_) = 0;

////////////////////////////
// Interface::IPredicate::
	virtual bool isFetch() {return true;}

protected:
	// constructor
	Fetch() : Super(Super::Fetch) {}

private:
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_FETCH_H

//
//	Copyright (c) 2008, 2010, 2011, 2014, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
