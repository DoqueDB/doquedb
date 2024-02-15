// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Join.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
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

#include "Plan/Candidate/Join.h"
#include "Plan/Candidate/Impl/JoinImpl.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////
// Plan::Candidate::Join::NestedLoop

// FUNCTION public
//	Candidate::Join::NestedLoop::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Join* pJoin_
//	
// RETURN
//	Join*
//
// EXCEPTIONS

//static
Join*
Join::NestedLoop::
create(Opt::Environment& cEnvironment_,
	   Relation::Join* pJoin_)
{
	AUTOPOINTER<This> pResult = new JoinImpl::NestedLoop(pJoin_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//////////////////////////////////////////
// Plan::Candidate::Join::Merge

// FUNCTION public
//	Candidate::Join::Merge::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Join* pJoin_
//	
// RETURN
//	Join*
//
// EXCEPTIONS

//static
Join*
Join::Merge::
create(Opt::Environment& cEnvironment_,
	   Relation::Join* pJoin_)
{
	// not yet
	_SYDNEY_THROW0(Exception::NotSupported);
}

//////////////////////////////////////////
// Plan::Candidate::Join::Hash

// FUNCTION public
//	Candidate::Join::Hash::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Join* pJoin_
//	
// RETURN
//	Join*
//
// EXCEPTIONS

//static
Join*
Join::Hash::
create(Opt::Environment& cEnvironment_,
	   Relation::Join* pJoin_)
{
	// not yet
	_SYDNEY_THROW0(Exception::NotSupported);
}

//////////////////////////////////////////
// Plan::Candidate::Join::Exists

// FUNCTION public
//	Candidate::Join::Exists::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Join* pJoin_
//	
// RETURN
//	Join*
//
// EXCEPTIONS

//static
Join*
Join::Exists::
create(Opt::Environment& cEnvironment_,
	   Relation::Join* pJoin_)
{
	AUTOPOINTER<This> pResult = new JoinImpl::Exists(pJoin_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
