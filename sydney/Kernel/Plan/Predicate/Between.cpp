// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Between.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Predicate";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Between.h"
#include "Plan/Predicate/Comparison.h"
#include "Plan/Predicate/Impl/BetweenImpl.h"

#include "Exception/Unexpected.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::Between

// FUNCTION public
//	Predicate::Between::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecScalar_
//	bool bIsNot_
//	
// RETURN
//	Between*
//
// EXCEPTIONS

//static
Between*
Between::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecScalar_,
	   bool bIsNot_)
{
	if (vecScalar_.GETSIZE() != 3) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	Interface::IScalar* pOperand0 = vecScalar_[0];
	Interface::IScalar* pOperand1 = vecScalar_[1];
	Interface::IScalar* pOperand2 = vecScalar_[2];
	Comparison::checkComparability(cEnvironment_,
								   Tree::Node::Between,
								   &pOperand0,
								   &pOperand1);
	Comparison::checkComparability(cEnvironment_,
								   Tree::Node::Between,
								   &pOperand0,
								   &pOperand2);

	VECTOR<Interface::IScalar*> vecOperand;
	vecOperand.PUSHBACK(pOperand0);
	vecOperand.PUSHBACK(pOperand1);
	vecOperand.PUSHBACK(pOperand2);

	AUTOPOINTER<This> pResult = new Impl::BetweenImpl(vecOperand, bIsNot_);
	if (bIsNot_) pResult->setNodeType(Tree::Node::NotBetween);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////
//	Plan::Predicate::Between

// FUNCTION protected
//	Predicate::Between::Between -- constructor
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

Between::
Between()
	: Super(Tree::Node::Between)
{}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
