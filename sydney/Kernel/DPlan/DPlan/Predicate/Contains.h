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

#ifndef __SYDNEY_DPLAN_PREDICATE_CONTAINS_H
#define __SYDNEY_DPLAN_PREDICATE_CONTAINS_H

#include "DPlan/Predicate/Module.h"

#include "Plan/Predicate/Contains.h"
#include "Execution/Interface/IIterator.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::Contains -- Predicate interface for contains predicate
//
//	NOTES
//		This class will not created directly

class Contains
	: public Plan::Predicate::Contains
{
public:
	typedef Plan::Predicate::Contains Super;
	typedef Contains This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cOperand_,
						const VECTOR<Plan::Interface::IScalar*>& vecOption_);


	virtual Schema::File* getFulltextIndex(Opt::Environment& cEnvironment_) = 0;

	virtual Plan::Interface::IScalar* getColumn() = 0;

	virtual Plan::Interface::IRelation* getExpand() = 0;

	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
												   Execution::Interface::IProgram& cProgram_,
												   Plan::Candidate::AdoptArgument& cArgument_) = 0;
	
	
	// destructor
	virtual ~Contains() {}

protected:
	// constructor
	Contains();

	private:
	// add to environment
	virtual void addToEnvironment(Opt::Environment& cEnvironment_) = 0;

};

_SYDNEY_DPLAN_PREDICATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_PREDICATE_CONTAINS_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
