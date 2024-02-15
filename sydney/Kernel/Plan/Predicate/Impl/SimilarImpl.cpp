// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/SimilarImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
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

#include "Plan/Predicate/Impl/SimilarImpl.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/Combinator.h"

#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Exception/NotSupported.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Predicate/Similar.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = " similar ";
}

/////////////////////////////////////////////
//	Plan::Predicate::Impl::SimilarImpl

// FUNCTION public
//	Predicate::Impl::SimilarImpl::rewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
Interface::IPredicate::RewriteResult
Impl::SimilarImpl::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	Interface::IPredicate* pPredicate = this;

	if (m_bIsNot && !isArbitraryElement()) {
		// x not similar p -> not (x similar p)
		pPredicate = convertNot(cEnvironment_);
		; _SYDNEY_ASSERT(pPredicate);

		pPredicate = Predicate::Combinator::create(cEnvironment_,
												   Tree::Node::Not,
												   pPredicate);
	}
	return Interface::IPredicate::RewriteResult(pRelation_, pPredicate);
}

// FUNCTION public
//	Predicate::Impl::SimilarImpl::convertNot -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Interface::IPredicate*
Impl::SimilarImpl::
convertNot(Opt::Environment& cEnvironment_)
{
	// similar operator should not convert into not-similar
	// so that index can be used
	// [NOTE] for now, no index can process similar
	if (m_bIsNot == false) return 0;

	// if left operand is arbitrary element specification,
	// it can not convert into not expression.
	if (isArbitraryElement()) return 0;

	return Similar::create(cEnvironment_,
						   getArgument(),
						   !m_bIsNot);
}

// FUNCTION public
//	Predicate::Impl::SimilarImpl::check -- 
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
Impl::SimilarImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// if right operand or options are using non-constant, not supported
	if (!getOperand1()->isRefering(0)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	// check 1st operand's status
	Interface::IScalar::Check::Value iStatus =
		getOperand0()->check(cEnvironment_,
							 cArgument_);

	if (Interface::IScalar::Check::isOn(iStatus, Interface::IScalar::Check::NotYet)) {
		// operand can not be evaluated for now
		return this;
	}
	// no index can be used
	return CheckedInterface::create(cEnvironment_,
									this);
}

// FUNCTION public
//	Predicate::Impl::SimilarImpl::estimateCost -- 
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
Impl::SimilarImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	// estimate selection rate by operator
	// it can be assumed there are no index for the column

	double dblValueCount;		// value variation number

	switch (getType()) {
	case Tree::Node::Similar:
		{
			dblValueCount = 10;
			break;
		}
	case Tree::Node::NotSimilar:
		{
			dblValueCount = 3;
			break;
		}
	default:
		{
			dblValueCount = 2;
			break;
		}
	}
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
//	Predicate::Impl::SimilarImpl::hasParameter -- 
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
Impl::SimilarImpl::
hasParameter()
{
	return getOperand1()->hasParameter();
}

// FUNCTION public
//	Predicate::Impl::SimilarImpl::isArbitraryElement -- 
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
Impl::SimilarImpl::
isArbitraryElement()
{
	// for similar predicate, if operand is array, it means arbitrary element 
	return getOperand0()->isArbitraryElement();
}

// FUNCTION public
//	Predicate::Impl::SimilarImpl::explain -- 
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
Impl::SimilarImpl::
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
//	Predicate::Impl::SimilarImpl::explain -- 
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
Impl::SimilarImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperand0()->toSQLStatement(cEnvironment_, cArgument_);
	if (m_bIsNot) cStream << " not ";	
	cStream << _pszOperatorName << "to ";
	cStream << getOperand1()->toSQLStatement(cEnvironment_, cArgument_);
	return cStream.getString();
	

}


// FUNCTION public
//	Predicate::Impl::LikeImpl::setParameter -- 
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
Impl::SimilarImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand0()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
	if (m_bIsNot) cExec.append(" not ");
	cExec.append(_pszOperatorName).append("to ");
	getOperand1()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
}



// FUNCTION public
//	Predicate::Impl::SimilarImpl::generate -- 
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
Impl::SimilarImpl::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;
	
	int iData0 = getOperand0()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	int iData1 = getOperand1()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);

	Execution::Predicate::Similar* pResult =
		Execution::Predicate::Similar::create(cProgram_,
											  pIterator_,
											  iData0,
											  iData1,
											  isArbitraryElement(),
											  m_bIsNot);
	return pResult->getID();
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
