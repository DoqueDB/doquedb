// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Join.h --
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

#ifndef __SYDNEY_PLAN_RELATION_JOIN_H
#define __SYDNEY_PLAN_RELATION_JOIN_H

#include "Plan/Interface/IRelation.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	Plan::Relation::Join -- Interface implementation for joined table
//
//	NOTES
class Join
	: public Interface::IRelation
{
public:
	typedef Interface::IRelation Super;
	typedef Join This;

	// costructor
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IPredicate* pJoinPredicate_,
						const VECTOR<Interface::IRelation*>& vecOperand_);
	static This* create(Opt::Environment& cEnvironment_,
						Tree::Node::Type eOperator_,
						Interface::IPredicate* pJoinPredicate_,
						const PAIR<Interface::IRelation*, Interface::IRelation*>& cOperand_);
	// destructor
	virtual ~Join() {}

	// accessor
	Interface::IPredicate* getJoinPredicate() {return m_pJoinPredicate;}
	void setJoinPredicate(Interface::IPredicate* pPredicate_) {m_pJoinPredicate = pPredicate_;}
	virtual bool isOuter() = 0;
	virtual bool isExists() = 0;

protected:
	// constructor
	Join(Tree::Node::Type eType_,
		 Interface::IPredicate* pJoinPredicate_)
		: Super(eType_),
		  m_pJoinPredicate(pJoinPredicate_)
	{}
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
	Join()
		: Super(Tree::Node::Undefined),
		  m_pJoinPredicate(0)
	{}
	void setArgument(Tree::Node::Type eType_,
					 Interface::IPredicate* pJoinPredicate_)
	{
		setType(eType_);
		m_pJoinPredicate = pJoinPredicate_;
	}
#endif

private:
	Interface::IPredicate* m_pJoinPredicate;
};

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_JOIN_H

//
//	Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
