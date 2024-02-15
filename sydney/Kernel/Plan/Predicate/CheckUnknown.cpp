// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/CheckUnknown.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Predicate/CheckUnknown.h"
#include "Plan/Predicate/Impl/CheckUnknownImpl.h"
#include "Plan/Interface/IScalar.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_PREDICATE_USING

////////////////////////////////////
//	Plan::Predicate::CheckUnknown

// FUNCTION public
//	Predicate::CheckUnknown::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecCheckKey_
//	bool bArray_
//	
// RETURN
//	CheckUnknown*
//
// EXCEPTIONS

//static
CheckUnknown*
CheckUnknown::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecCheckKey_,
	   bool bArray_)
{
	AUTOPOINTER<This> pResult;
	switch (vecCheckKey_.GETSIZE()) {
	case 0:
		{
			return 0;
		}
	case 1:
		{
			pResult = new CheckUnknownImpl::Monadic(vecCheckKey_[0], bArray_);
			break;
		}
	default:
		{
			pResult = new CheckUnknownImpl::Nadic(vecCheckKey_, bArray_);
			break;
		}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
