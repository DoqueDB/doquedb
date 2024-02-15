// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Contains.h --
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

#ifndef __SYDNEY_PLAN_PREDICATE_CONTAINS_H
#define __SYDNEY_PLAN_PREDICATE_CONTAINS_H

#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::Contains -- Predicate interface for contains predicate
//
//	NOTES
//		This class will not created directly

class Contains
	: public Interface::IPredicate
{
public:
	typedef Interface::IPredicate Super;
	typedef Contains This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						const VECTOR<Interface::IScalar*>& vecOption_);


	static This* create(Opt::Environment& cEnvironment_,
						 const PAIR<VECTOR<Interface::IScalar*>, Interface::IScalar*>& cOperand_,
						 const VECTOR<Interface::IScalar*>& vecOption_);

	
	// destructor
	virtual ~Contains() {}

	// set options
	virtual void setExpand(Interface::IRelation* pQuery_) = 0;
	virtual void setRankFrom(Interface::IRelation* pQuery_) = 0;

	// kwic option
	virtual void createKwicOption(Opt::Environment& cEnvironment_,
								  Interface::IScalar* pKwicSize_) = 0;
	virtual File::KwicOption* getKwicOption() = 0;

//////////////////////////////
// Interface::IPredicate::
	virtual bool isNeedIndex();

protected:
	// constructor
	Contains();


	// register to environment
	void registerToEnvironment(Opt::Environment& cEnvironment_);

private:
	// add to environment
	virtual void addToEnvironment(Opt::Environment& cEnvironment_) = 0;
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_CONTAINS_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
