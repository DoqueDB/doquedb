// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Like.cpp --
// 
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
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

#include "Plan/Predicate/Like.h"
#include "Plan/Predicate/Impl/LikeImpl.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::Like

// FUNCTION public
//	Predicate::Like::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	const VECTOR<Interface::IScalar*>& vecOption_
//	bool bIsNot_
//	
// RETURN
//	Like*
//
// EXCEPTIONS

//static
Like*
Like::
create(Opt::Environment& cEnvironment_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   const VECTOR<Interface::IScalar*>& vecOption_,
	   bool bIsNot_)
{
	AUTOPOINTER<This> pResult =
		new Impl::LikeImpl(cOperand_, vecOption_, bIsNot_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Predicate::Like::Like -- constructor
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

Like::
Like()
	: Super(Tree::Node::Like)
{}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
