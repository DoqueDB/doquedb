// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/PatternImpl.cpp --
// 
// Copyright (c) 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Impl/PatternImpl.h"
#include "Plan/Interface/IRelation.h"

#include "Common/Assert.h"
#include "Common/Data.h"
#include "Common/DataOperation.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::PatternImpl::Pattern
// FUNCTION public
//	Scalar::PatternImpl::Pattern::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
PatternImpl::Monadic::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand()->setParameter(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cExec,
							   cArgument_);
}

////////////////////////////////////////
//	Scalar::PatternImpl::Pattern
// FUNCTION public
//	Scalar::PatternImpl::Pattern::setParameter
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Execution::Interface::IProgram& cProgram_,
//	Execution::Interface::IIterator* pIterator_,
//	DExecution::Action::StatementConstruction& cExec
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
PatternImpl::MonadicWithMonadicOption::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand()->setParameter(cEnvironment_,
							   cProgram_,
							   pIterator_,
							   cExec,
							   cArgument_);
	
	getOption()->setParameter(cEnvironment_,
							  cProgram_,
							  pIterator_,
							  cExec,
							  cArgument_);
}



_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
