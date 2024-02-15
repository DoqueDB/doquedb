// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Cast.cpp --
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

#include "Plan/Scalar/Cast.h"
#include "Plan/Scalar/Impl/CastImpl.h"

#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

//////////////////////////////
//	Plan::Scalar::Cast

// FUNCTION public
//	Scalar::Cast::create -- construct
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IScalar* pOperand_
//	const DataType& cToType_
//	bool bForComparison_
//	bool bNoThrow_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//static
Interface::IScalar*
Cast::
create(Opt::Environment& cEnvironment_,
	   Interface::IScalar* pOperand_,
	   const DataType& cToType_,
	   bool bForComparison_,
	   bool bNoThrow_)
{
	if (pOperand_->getDataType().isNoType()
		|| DataType::isAssignable(pOperand_->getDataType(),
								  cToType_) == false) {
		AUTOPOINTER<This> pResult =
			new CastImpl::Monadic(cToType_,
								  pOperand_,
								  bForComparison_,
								  bNoThrow_);
		pResult->registerToEnvironment(cEnvironment_);
		return pResult.release();

	} else {
		return pOperand_;
	}
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
