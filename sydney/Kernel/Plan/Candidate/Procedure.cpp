// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Procedure.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "SyDefault.h"

#include "Plan/Candidate/Procedure.h"
#include "Plan/Candidate/Impl/ProcedureImpl.h" 

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

//////////////////////////////////////
// Candidate::Procedure::

// FUNCTION public
//	Candidate::Procedure::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Procedure* pRelation_
//	
// RETURN
//	Procedure*
//
// EXCEPTIONS

//static
Procedure*
Procedure::
create(Opt::Environment& cEnvironment_,
	   Relation::Procedure* pRelation_)
{
	AUTOPOINTER<This> pResult = new Impl::ProcedureImpl(pRelation_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
