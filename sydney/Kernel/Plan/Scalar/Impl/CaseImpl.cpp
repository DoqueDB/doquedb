// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/CaseImpl.cpp --
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
const char moduleName[] = "Plan::Scalar::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Scalar/Impl/CaseImpl.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Scalar/Value.h"

#include "Common/Assert.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IIterator.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::Impl::CaseImpl

// FUNCTION protected
//	Scalar::Impl::CaseImpl::generateThis -- generate
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
Impl::CaseImpl::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	int nResult = getSize();
	int nCase = getOptionSize();
	; _SYDNEY_ASSERT(nCase <= nResult);

	for (int i = 0; i < nCase; ++i) {
		cArgument_.pushScope();
		Interface::IPredicate* pPredicate = getOptioni(i);
		; _SYDNEY_ASSERT(pPredicate);

		int iCaseID = pPredicate->generate(cEnvironment_,
										   cProgram_,
										   pIterator_,
										   cArgument_);
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT1_T(If,
												  iCaseID,
												  cArgument_.m_eTarget));
		Interface::IScalar* pResult = getOperandi(i);
		; _SYDNEY_ASSERT(pResult);

		int iResultID = pResult->generate(cEnvironment_,
										  cProgram_,
										  pIterator_,
										  cArgument_);
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT_T(Assign,
												 iResultID,
												 iDataID_,
												 cArgument_.m_eTarget));
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(Else,
												  cArgument_.m_eTarget));
		cArgument_.popScope();
	}
	// add last choice
	int iElseID = -1;
	if (nCase < nResult) {
		cArgument_.pushScope();
		iElseID = getOperandi(nCase)->generate(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument_);
		cArgument_.popScope();
	} else {
		iElseID = Scalar::Value::Null::create(cEnvironment_)->generate(cEnvironment_,
																	   cProgram_,
																	   pIterator_,
																	   cArgument_);
	}
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT_T(Assign,
											 iElseID,
											 iDataID_,
											 cArgument_.m_eTarget));
	// put endif for all ifs
	for (int i = 0; i < nCase; ++i) {
		pIterator_->addAction(cProgram_,
							  _ACTION_ARGUMENT0_T(EndIf,
												  cArgument_.m_eTarget));
	}
	return iDataID_;
}

_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
