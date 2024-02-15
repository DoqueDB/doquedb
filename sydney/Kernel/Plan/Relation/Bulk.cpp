// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Bulk.cpp --
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
const char moduleName[] = "Plan::Relation";
}

#include "SyDefault.h"

#include "Plan/Relation/Bulk.h"
#include "Plan/Relation/Impl/BulkImpl.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

////////////////////////////////////
// Relation::Bulk::Input::

// FUNCTION public
//	Relation::Bulk::Input::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IRelation*>& vecOperand_
//	
// RETURN
//	Interface::IRelation*
//
// EXCEPTIONS

//static
Interface::IRelation*
Bulk::Input::
create(Opt::Environment& cEnvironment_,
	   const Argument& cArgument_)
{
	AUTOPOINTER<This> pResult =
		new BulkImpl::Input(cArgument_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

////////////////////////////////////
// Relation::Bulk::Output::

// FUNCTION public
//	Relation::Bulk::Output::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Argument& cArgument_
//	Interface::IRelation* pInput_
//	
// RETURN
//	Interface::IRelation*
//
// EXCEPTIONS

//static
Interface::IRelation*
Bulk::Output::
create(Opt::Environment& cEnvironment_,
	   const Argument& cArgument_,
	   Interface::IRelation* pInput_)
{
	AUTOPOINTER<This> pResult =
		new BulkImpl::Output(cArgument_, pInput_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
