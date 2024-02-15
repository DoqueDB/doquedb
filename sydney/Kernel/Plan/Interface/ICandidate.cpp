// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Interface/ICandidate.cpp --
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
const char moduleName[] = "Plan::Interface";
}

#include "SyDefault.h"

#include "Plan/Interface/ICandidate.h"

#include "Plan/Candidate/Row.h"
#include "Plan/Interface/IFile.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Order/Specification.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_INTERFACE_USING

// FUNCTION public
//	Interface::ICandidate::erase -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	This* pThis_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
ICandidate::
erase(Opt::Environment& cEnvironment_,
	  This* pThis_)
{
	if (pThis_) {
		pThis_->eraseFromEnvironment(cEnvironment_);
		pThis_->destruct(cEnvironment_);
		delete pThis_;
	}
}

// FUNCTION protected
//	Interface::ICandidate::registerToEnvironment -- register to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ICandidate::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	m_iID = cEnvironment_.addObject(this);
}

// FUNCTION protected
//	Interface::ICandidate::eraseFromEnvironment -- erase from environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
ICandidate::
eraseFromEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.eraseObject(m_iID);
}

//
// Copyright (c) 2008, 2009, 2010, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
