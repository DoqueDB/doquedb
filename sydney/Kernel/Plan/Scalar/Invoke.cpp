// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Invoke.cpp --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Invoke.h"
#include "Plan/Scalar/Impl/InvokeImpl.h"

#include "Exception/NotSupported.h"
#include "Exception/StoredFunctionNotFound.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_SCALAR_USING

//////////////////////////////
//	Plan::Scalar::Invoke

// FUNCTION public
//	Scalar::Invoke::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrFunctionName_
//	const VECTOR<Interface::IScalar*>& vecOperand_
//	const STRING& cstrName_
//	
// RETURN
//	Interface::IScalar*
//
// EXCEPTIONS

//static
Interface::IScalar*
Invoke::
create(Opt::Environment& cEnvironment_,
	   const STRING& cstrFunctionName_,
	   const VECTOR<Interface::IScalar*>& vecOperand_,
	   const STRING& cstrName_)
{
	Schema::Function* pSchemaFunction =
		cEnvironment_.getDatabase()->getFunction(cstrFunctionName_,
												 cEnvironment_.getTransaction());
	if (pSchemaFunction == 0) {
		_SYDNEY_THROW2(Exception::StoredFunctionNotFound,
					   cstrFunctionName_,
					   cEnvironment_.getDatabase()->getName());
	}

	AUTOPOINTER<This> pResult = new Impl::InvokeImpl(pSchemaFunction,
													 vecOperand_,
													 cstrName_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
