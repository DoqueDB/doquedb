// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/CastImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "Plan/Scalar/Impl/CastImpl.h"

#include "Common/Assert.h"
#include "Common/Data.h"

#include "Execution/Function/Cast.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SCALAR_BEGIN

////////////////////////////////////////
//	Scalar::CastImpl::Monadic


// FUNCTION protected
//	Scalar::CastImpl::Monadic::toSQLStatement -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	STRING
//
// EXCEPTIONS

//virtual
STRING
CastImpl::Monadic::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	return getOperand()->toSQLStatement(cEnvironment_, cArgument_);
}


// FUNCTION public
//	Plan::Scalar::CastImpl::Monadic::setParameter
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
CastImpl::Monadic::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
}



			 
// FUNCTION protected
//	Scalar::CastImpl::Monadic::generateThis -- generate
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
CastImpl::Monadic::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	int iOperandID = getOperand()->generate(cEnvironment_,
											cProgram_,
											pIterator_,
											cArgument_);
	Execution::Interface::IIterator* pTargetIterator =
		pIterator_->getGenerateIterator(iOperandID);
	pTargetIterator->addCalculation(
							cProgram_,
							Execution::Function::Cast::create(
										 cProgram_,
										 pTargetIterator,
										 getOperand()->getDataType(),
										 getDataType(),
										 iOperandID,
										 iDataID_,
										 m_bForComparison,
										 m_bNoThrow),
							cArgument_.m_eTarget);
	return iDataID_;
}

// FUNCTION public
//	Plan::Scalar::CastImpl::Monadic::retrieveFromCascade
//
// NOTES
//	castした場合はOperandをretrieveする.
//	
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_,
//	Plan::Sql::Query* pQuery_
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
CastImpl::Monadic::
retrieveFromCascade(Opt::Environment& cEnvironment_,
					Plan::Sql::Query* pQuery_)
{
	getOperand()->retrieveFromCascade(cEnvironment_, pQuery_);
}


_SYDNEY_PLAN_SCALAR_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
