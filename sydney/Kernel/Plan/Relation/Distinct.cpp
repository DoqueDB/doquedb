// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Distinct.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Relation";
}

#include "SyDefault.h"

#include "Plan/Relation/Distinct.h"
#include "Plan/Relation/Impl/DistinctImpl.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

/////////////////////////////////////
// Relation::Distinct

// FUNCTION public
//	Relation::Distinct::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Distinct*
//
// EXCEPTIONS

//static
Distinct*
Distinct::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pOperand_)
{
	AUTOPOINTER<This> pResult = new Impl::DistinctImpl(pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Distinct::Distinct -- constructor
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

Distinct::
Distinct()
	: Super(Tree::Node::Distinct)
{}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
