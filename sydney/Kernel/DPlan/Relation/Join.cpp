// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Join.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2012, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "DPlan::Relation";
}

#include "SyDefault.h"

#include "DPlan/Relation/Join.h"
#include "DPlan/Relation/Impl/JoinImpl.h"

#include "Exception/NotSupported.h"



_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_RELATION_USING

/////////////////////////
// Relation::Join

// FUNCTION public
//	Relation::Join::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IPredicate* pJoinPredicate_
//	const VECTOR<Interface::IRelation*>& vecOperand_
//	
// RETURN
//	Join*
//
// EXCEPTIONS

//static
Join*
Join::
create(Opt::Environment& cEnvironment_,
	   Plan::Tree::Node::Type eOperator_,
	   Plan::Interface::IPredicate* pJoinPredicate_,
	   const VECTOR<Plan::Interface::IRelation*>& vecOperand_)
{
	switch (vecOperand_.GETSIZE()) {
	case 0:
	case 1:
		{
			; _SYDNEY_ASSERT(false);
			return 0;
		}
	case 2:
		{
			return create(cEnvironment_,
						  eOperator_,
						  pJoinPredicate_,
						  MAKEPAIR(vecOperand_[0], vecOperand_[1]));
		}
	default:
		{
			break;
		}
	}
	AUTOPOINTER<This> pResult =
		new JoinImpl::Nadic(eOperator_, pJoinPredicate_, vecOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Relation::Join::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IPredicate* pJoinPredicate_
//	const PAIR<Interface::IRelation*, Interface::IRelation*>& cOperand_
//	
// RETURN
//	Join*
//
// EXCEPTIONS

//static
Join*
Join::
create(Opt::Environment& cEnvironment_,
	   Plan::Tree::Node::Type eOperator_,
	   Plan::Interface::IPredicate* pJoinPredicate_,
	   const PAIR<Plan::Interface::IRelation*, Plan::Interface::IRelation*>& cOperand_)
{
	AUTOPOINTER<This> pResult;
	switch (eOperator_) {
	case Plan::Tree::Node::Exists:
	case Plan::Tree::Node::NotExists:
		_SYDNEY_THROW0(Exception::NotSupported);

	case Plan::Tree::Node::SimpleJoin:
	case Plan::Tree::Node::LeftOuterJoin:
	case Plan::Tree::Node::RightOuterJoin:
	case Plan::Tree::Node::FullOuterJoin:
		pResult = new JoinImpl::Dyadic(eOperator_, pJoinPredicate_, cOperand_);
		break;
	default:
		_SYDNEY_THROW0(Exception::NotSupported);

	}
	
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2008, 2009, 2010, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
