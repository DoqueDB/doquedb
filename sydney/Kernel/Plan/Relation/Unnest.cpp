// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Unnest.cpp --
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

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Relation/Unnest.h"
#include "Plan/Relation/Impl/UnnestImpl.h"
#include "Plan/Interface/IScalar.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

////////////////////////////////////
// Relation::Unnest::

// FUNCTION public
//	Relation::Unnest::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pValue_
//	
// RETURN
//	Unnest*
//
// EXCEPTIONS

//static
Unnest*
Unnest::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pValue_)
{
	pValue_->retrieve(cEnvironment_);
	AUTOPOINTER<This> pResult = new Impl::UnnestImpl(pValue_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Unnest::Unnest -- constructor
//
// NOTES
//
// ARGUMENTS
//	Execution::Unnest* pUnnest_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Unnest::
Unnest()
	: Super(Node::Unnest)
{}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
