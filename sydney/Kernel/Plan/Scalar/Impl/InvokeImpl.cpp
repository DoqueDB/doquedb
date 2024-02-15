// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/InvokeImpl.cpp --
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
const char moduleName[] = "Plan::Scalar::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/InvokeImpl.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Function/Invoke.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::Impl::InvokeImpl

// FUNCTION protected
//	Scalar::Impl::InvokeImpl::generateThis -- generate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::InvokeImpl::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	MapIntResult vecOperandData;
	mapOperand(vecOperandData,
			   boost::bind(&Operand::generate,
						   _1,
						   boost::ref(cEnvironment_),
						   boost::ref(cProgram_),
						   pIterator_,
						   boost::ref(cArgument_)));
	pIterator_->addCalculation(cProgram_,
							   Execution::Function::Invoke::create(
										 cProgram_,
										 pIterator_,
										 m_pSchemaFunction,
										 cProgram_.addVariable(vecOperandData),
										 iDataID_),
							   cArgument_.m_eTarget);
	return iDataID_;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
