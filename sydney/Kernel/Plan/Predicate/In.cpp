// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/In.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Predicate";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/In.h"
#include "Plan/Predicate/Impl/InImpl.h"
#include "Plan/Predicate/Combinator.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::In::SubQuery

// FUNCTION public
//	Predicate::In::SubQuery::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecScalar_
//	Interface::IRelation* pOperand_
//	const Utility::RelationSet& cOuterRelation_
//	bool bIsNot_
//	
// RETURN
//	In*
//
// EXCEPTIONS

//static
In*
In::SubQuery::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecScalar_,
	   Interface::IRelation* pOperand_,
	   const Utility::RelationSet& cOuterRelation_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	switch (vecScalar_.GETSIZE()) {
	case 0:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	case 1:
		{
			pResult = new InImpl::SubQuery::SingleKey(vecScalar_[0],
													  pOperand_,
													  cOuterRelation_,
													  bIsNot_);
			break;
		}
	default:
		{
			pResult = new InImpl::SubQuery::MultiKey(vecScalar_,
													 pOperand_,
													 cOuterRelation_,
													 bIsNot_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////
//	Plan::Predicate::In::ValueList

// FUNCTION public
//	Predicate::In::ValueList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecScalar_
//	Interface::IRelation* pOperand_
//	const Utility::RelationSet& cOuterRelation_
//	bool bIsNot_
//	
// RETURN
//	In*
//
// EXCEPTIONS

//static
In*
In::ValueList::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecScalar_,
	   Interface::IRelation* pOperand_,
	   const Utility::RelationSet& cOuterRelation_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	switch (vecScalar_.GETSIZE()) {
	case 0:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	case 1:
		{
			pResult = new InImpl::ValueList::SingleKey(vecScalar_[0],
													   pOperand_,
													   cOuterRelation_,
													   bIsNot_);
			break;
		}
	default:
		{
			pResult = new InImpl::ValueList::MultiKey(vecScalar_,
													  pOperand_,
													  cOuterRelation_,
													  bIsNot_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////
//	Plan::Predicate::In::Variable

// FUNCTION public
//	Predicate::In::Variable::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecScalar_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	bool bIsNot_
//	
// RETURN
//	In*
//
// EXCEPTIONS

//static
Interface::IPredicate*
In::Variable::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecScalar_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult;
	switch (vecScalar_.GETSIZE()) {
	case 0:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	case 1:
		{
			pResult = new InImpl::Variable::SingleKey(vecScalar_[0],
													  vecOperand_[0]);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	if (bIsNot_) {
		return Combinator::create(cEnvironment_,
								  Tree::Node::Not,
								  pResult.release());
	} else {
		return pResult.release();
	}
}

////////////////////////////////////
//	Plan::Predicate::In

// FUNCTION protected
//	Predicate::In::In -- constructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

In::
In()
	: Super(Tree::Node::In)
{}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
