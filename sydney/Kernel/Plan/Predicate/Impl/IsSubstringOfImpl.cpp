// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/IsSubstringOfImpl.cpp --
// 
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
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

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Impl/IsSubstringOfImpl.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/NotSupported.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Predicate/IsSubstringOf.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = " is substring of ";
}

/////////////////////////////////////////////
//	Plan::Predicate::Impl::IsSubstringOfImpl

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::check -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::IsSubstringOfImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// check operand's status
	Interface::IScalar::Check::Value iStatus0 =
		getOperand0()->check(cEnvironment_,
							 cArgument_);
	Interface::IScalar::Check::Value iStatus1 =
		getOperand1()->check(cEnvironment_,
							 cArgument_);

	if (Interface::IScalar::Check::isOn(iStatus0, Interface::IScalar::Check::NotYet)
		|| Interface::IScalar::Check::isOn(iStatus1, Interface::IScalar::Check::NotYet)) {
		// operand can not be evaluated for now
		return this;
	}
	// no index can be used
	return CheckedInterface::create(cEnvironment_,
									this);
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::IsSubstringOfImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	// estimate selection rate by operator
	// it can be assumed there are no index for the column

	double dblValueCount = 10;

	double dblSize = getOperand0()->getDataType().getDataSize()
		+ getOperand1()->getDataType().getDataSize();
	double dblRetrieveCost = dblSize / LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::Memory);

	cResult_.setOverhead(0);
	cResult_.setTotalCost(0);
	cResult_.setRetrieveCost(dblRetrieveCost);
	cResult_.setRate(1.0 / dblValueCount);
	if (cResult_.isSetRate()) {
		cResult_.setRate(1.0 / dblValueCount);
	}
	if (cResult_.isSetCount()) {
		AccessPlan::Cost::Value cCount = cResult_.getTableCount();
		if (cCount.isInfinity()) {
			// use default
			cCount = 100000;
		}
		cResult_.setTupleCount(cCount / dblValueCount);
	}
	return true;
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::hasParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::IsSubstringOfImpl::
hasParameter()
{
	return getOperand0()->hasParameter() || getOperand1()->hasParameter();
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::IsSubstringOfImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.pushNoNewLine();
	getOperand0()->explain(pEnvironment_, cExplain_);
	cExplain_.put(_pszOperatorName);
	cExplain_.popNoNewLine();
	getOperand1()->explain(pEnvironment_, cExplain_);
}


// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::explain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Explain& cExplain_
//	const Plan::Sql::QueryArgument& cArgument_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
STRING
Impl::IsSubstringOfImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperand0()->toSQLStatement(cEnvironment_, cArgument_);
	cStream << _pszOperatorName;
	cStream << getOperand1()->toSQLStatement(cEnvironment_, cArgument_);
	return cStream.getString();
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::setParameter -- 
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
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
Impl::IsSubstringOfImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand0()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
	cExec.append(_pszOperatorName);
	getOperand1()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
}

// FUNCTION public
//	Predicate::Impl::IsSubstringOfImpl::generate -- 
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
Impl::IsSubstringOfImpl::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;
	
	int iData0 = getOperand0()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	int iData1 = getOperand1()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);

	Execution::Predicate::IsSubstringOf* pResult =
		Execution::Predicate::IsSubstringOf::create(cProgram_,
													pIterator_,
													iData0,
													iData1);
	return pResult->getID();
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
