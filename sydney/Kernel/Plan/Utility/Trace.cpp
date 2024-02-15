// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/Trace.cpp --
// 
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Utility";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Utility/Trace.h"
#include "Plan/Interface/IPredicate.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_UTILITY_BEGIN

// FUNCTION public
//	Utility::Trace::toString -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IPredicate* pPredicate_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

STRING
Trace::
toString(Opt::Environment& cEnvironment_,
		 Interface::IPredicate* pPredicate_)
{
	Opt::Explain cExplain(Opt::Explain::Option::File, 0);
	cExplain.initialize();
	if (pPredicate_) {
		pPredicate_->explain(&cEnvironment_, cExplain);
	}
	return cExplain.getString();
}

_SYDNEY_PLAN_UTILITY_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
