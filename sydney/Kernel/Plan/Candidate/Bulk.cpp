// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Bulk.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Candidate/Bulk.h"
#include "Plan/Candidate/Impl/BulkImpl.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

//////////////////////////////////////
// Candidate::Bulk::Input::

// FUNCTION public
//	Candidate::Bulk::Input::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Bulk* pRelation_
//	
// RETURN
//	Bulk*
//
// EXCEPTIONS

//static
Bulk*
Bulk::Input::
create(Opt::Environment& cEnvironment_,
	   Relation::Bulk* pRelation_)
{
	AUTOPOINTER<This> pResult = new BulkImpl::Input(pRelation_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////
// Candidate::Bulk::Output::

// FUNCTION public
//	Candidate::Bulk::Output::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Bulk* pRelation_
//	Relation::RowInfo* pRowInfo_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Bulk*
//
// EXCEPTIONS

//static
Bulk*
Bulk::Output::
create(Opt::Environment& cEnvironment_,
	   Relation::Bulk* pRelation_,
	   Relation::RowInfo* pRowInfo_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new BulkImpl::Output(pRelation_, pRowInfo_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
