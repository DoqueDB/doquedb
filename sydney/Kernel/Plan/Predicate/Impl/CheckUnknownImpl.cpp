// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/CheckUnknownImpl.cpp --
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
const char moduleName[] = "Plan::Predicate::Impl";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Predicate/Impl/CheckUnknownImpl.h"
#include "Plan/Interface/IScalar.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Predicate/CheckUnknown.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_PREDICATE_USING

/////////////////////////////////////////////
//	Predicate::CheckUnknownImpl::Monadic::

// FUNCTION public
//	Predicate::CheckUnknownImpl::Monadic::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
CheckUnknownImpl::Monadic::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;	
	int iID = getOperand()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);

	Execution::Interface::IAction* pResult =
		Execution::Predicate::CheckUnknown::create(cProgram_,
												   pIterator_,
												   iID,
												   isArray());
	return pResult->getID();
}

/////////////////////////////////////////////
//	Predicate::CheckUnknownImpl::Nadic::

// FUNCTION public
//	Predicate::CheckUnknownImpl::Nadic::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
CheckUnknownImpl::Nadic::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	MapResult<int> vecID;
	mapOperand(vecID,
			   boost::bind(&Operand::generate,
						   _1,
						   boost::ref(cEnvironment_),
						   boost::ref(cProgram_),
						   pIterator_,
						   boost::ref(cArgument_)));

	Execution::Interface::IAction* pResult =
		Execution::Predicate::CheckUnknown::create(cProgram_,
												   pIterator_,
												   vecID,
												   isArray());
	return pResult->getID();
}

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
