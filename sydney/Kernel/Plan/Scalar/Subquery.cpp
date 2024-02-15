// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Subquery.cpp --
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
const char moduleName[] = "Plan::Scalar";
}

#include "SyDefault.h"

#include "Plan/Scalar/Subquery.h"
#include "Plan/Scalar/Impl/SubqueryImpl.h"
#include "Plan/Interface/IRelation.h"

#include "Common/DefaultData.h"
#include "Common/NullData.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

//////////////////////////////
//	Plan::Scalar::Subquery

// FUNCTION public
//	Scalar::Subquery::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pSubRelation_
//	const Utility::RelationSet& cOuterRelation_
//	int iPosition_
//	
// RETURN
//	Subquery*
//
// EXCEPTIONS

//static
Subquery*
Subquery::
create(Opt::Environment& cEnvironment_,
	   Interface::IRelation* pSubRelation_,
	   const Utility::RelationSet& cOuterRelation_,
	   int iPosition_)
{
	const STRING& cstrName = pSubRelation_->getScalarName(cEnvironment_, iPosition_);
	AUTOPOINTER<This> pResult = new Impl::SubqueryImpl(pSubRelation_,
													   cOuterRelation_,
													   iPosition_,
													   cstrName);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
