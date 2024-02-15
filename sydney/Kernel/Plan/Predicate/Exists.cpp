// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Exists.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Plan/Predicate/Exists.h"
#include "Plan/Predicate/Impl/ExistsImpl.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::Exists

// FUNCTION public
//	Predicate::Exists::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pOperand_
//	const Utility::RelationSet& cOuterRelation_
//	bool bUnderExists_
//	bool bIsNot_ = false
//	
// RETURN
//	Exists*
//
// EXCEPTIONS

//static
Exists*
Exists::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pOperand_,
	   const Utility::RelationSet& cOuterRelation_,
	   bool bUnderExists_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult =
		new Impl::ExistsImpl(pOperand_, cOuterRelation_, bUnderExists_, bIsNot_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Predicate::Exists::Exists -- constructor
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

Exists::
Exists()
	: Super(Tree::Node::Exists)
{}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
