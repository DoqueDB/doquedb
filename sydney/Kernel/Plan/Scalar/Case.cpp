// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Case.cpp --
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
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Case.h"
#include "Plan/Scalar/Impl/CaseImpl.h"
#include "Plan/Interface/IPredicate.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

//////////////////////////////
//	Plan::Scalar::Case

// FUNCTION public
//	Scalar::Case::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecResultList_
//	const VECTOR<Interface::IPredicate*>& vecCaseList_
//	const STRING& cstrName_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//static
Interface::IScalar*
Case::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IPredicate*>& vecCaseList_,
	   const VECTOR<Interface::IScalar*>& vecResultList_,
	   const STRING& cstrName_)
{
	if (vecResultList_.GETSIZE() == 0
		|| (vecResultList_.GETSIZE() != vecCaseList_.GETSIZE()
			&& vecResultList_.GETSIZE() != vecCaseList_.GETSIZE() + 1)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	AUTOPOINTER<This> pResult = new Impl::CaseImpl(vecCaseList_,
												   vecResultList_,
												   cstrName_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
