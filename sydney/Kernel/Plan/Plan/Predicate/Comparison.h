// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Comparison.h --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_COMPARISON_H
#define __SYDNEY_PLAN_PREDICATE_COMPARISON_H

#include "Plan/Interface/IPredicate.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Predicate::Comparison -- Predicate interface for comparison predicate
//
//	NOTES
//		This class will not created directly

class Comparison
	: public Interface::IPredicate
{
public:
	typedef Interface::IPredicate Super;
	typedef Comparison This;

	// constructor
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
						bool bCheckComparability_ = true);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IScalar* pOperand_);
	static Super* create(Opt::Environment& cEnvironment_,
						 Tree::Node::Type eOperator_,
						 const PAIR< VECTOR<Interface::IScalar*>, VECTOR<Interface::IScalar*> >& cOperand_,
						 bool bCheckComparability_ = true);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						const PAIR<Interface::IRow*, Interface::IRow*>& cOperand_);
	// destructor
	virtual ~Comparison() {}

	// check availability
	static bool isDyadic(Tree::Node::Type eOperator_);
	static bool isMonadic(Tree::Node::Type eOperator_);

	// check comparability
	static bool checkComparability(Opt::Environment& cEnvironment_,
								   Tree::Node::Type eOperator_,
								   Interface::IScalar** ppOperand0_,
								   Interface::IScalar** ppOperand1_);

protected:
	// constructor
	Comparison(Tree::Node::Type eOperator_)
		: Super(eOperator_)
	{}
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
	Comparison()
		: Super(Tree::Node::Undefined)
	{}
	void setArgument(Tree::Node::Type eOperator_)
	{
		setType(eOperator_);
	}
#endif

private:
};

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_COMPARISON_H

//
//	Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
