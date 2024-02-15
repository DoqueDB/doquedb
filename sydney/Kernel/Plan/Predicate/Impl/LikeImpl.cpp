// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/LikeImpl.cpp --
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

#include "Plan/Predicate/Impl/LikeImpl.h"
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
#include "Execution/Predicate/Like.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = " like ";
}

/////////////////////////////////////////////
//	Plan::Predicate::Impl::LikeImpl

// FUNCTION public
//	Predicate::Impl::LikeImpl::rewrite -- 
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
Impl::LikeImpl::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	Interface::IPredicate* pPredicate = this;

	if (m_bIsNot && !isArbitraryElement()) {
		// x not like p -> not (x like p)
		pPredicate = convertNot(cEnvironment_);
		; _SYDNEY_ASSERT(pPredicate);

		pPredicate = Predicate::Combinator::create(cEnvironment_,
												   Tree::Node::Not,
												   pPredicate);
	}
	return Interface::IPredicate::RewriteResult(pRelation_, pPredicate);
}

// FUNCTION public
//	Predicate::Impl::LikeImpl::convertNot -- 
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
Impl::LikeImpl::
convertNot(Opt::Environment& cEnvironment_)
{
	// like operator should not convert into not-like
	// so that index can be used
	if (m_bIsNot == false) return 0;

	// if left operand is arbitrary element specification,
	// it can not convert into not expression.
	if (isArbitraryElement()) return 0;

	return Like::create(cEnvironment_,
						getArgument(),
						getOptionArgument(),
						!m_bIsNot);
}

// FUNCTION public
//	Predicate::Impl::LikeImpl::check -- 
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
Impl::LikeImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	// if right operand or options are using non-constant, not supported
	if (!getOperand1()->isRefering(0)
		|| !isAllOption(boost::bind(&Interface::IScalar::isRefering,
									_1,
									static_cast<Interface::IRelation*>(0)))) {
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

	// get searchable files for operand0
	Utility::FileSet cFile;
	if (!Scalar::Field::getSearchFile(cEnvironment_,
									  Scalar::GetFileArgument(
												  getOperand0(),
												  this,
												  cFile))) {
		return CheckedInterface::create(cEnvironment_,
										this);
	}
	// get table candidate
	Candidate::Table* pCandidate =
		Scalar::Field::getCandidate(cEnvironment_,
									getOperand0(),
									cArgument_.m_pCandidate);

	if (pCandidate == 0) {
		// no index can be used
		return CheckedInterface::create(cEnvironment_,
										this);
	}

	return CheckedInterface::create(cEnvironment_,
									this,
									pCandidate,
									cFile);
}

// FUNCTION public
//	Predicate::Impl::LikeImpl::estimateCost -- 
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
Impl::LikeImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cResult_)
{
	// estimate selection rate by operator
	// it can be assumed there are no index for the column

	double dblValueCount;		// value variation number

	switch (getType()) {
	case Tree::Node::Like:
		{
			dblValueCount = 10;
			break;
		}
	case Tree::Node::NotLike:
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
//	Predicate::Impl::LikeImpl::hasParameter -- 
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
Impl::LikeImpl::
hasParameter()
{
	return getOperand1()->hasParameter()
		|| isAnyOption(boost::bind(&Interface::IScalar::hasParameter,
								   _1));
}

// FUNCTION public
//	Predicate::Impl::LikeImpl::isArbitraryElement -- 
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
Impl::LikeImpl::
isArbitraryElement()
{
	// for like predicate, if operand is array, it means arbitrary element 
	return getOperand0()->isArbitraryElement()
		|| getOperand0()->getDataType().isArrayType();
}

// FUNCTION public
//	Predicate::Impl::LikeImpl::explain -- 
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
Impl::LikeImpl::
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
//	Predicate::Impl::LikeImpl::createSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Encoder& cEnvironment_
//	const Plan::Sql::QueryArgument& cArgument_
//
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
STRING
Impl::LikeImpl::
toSQLStatement(Opt::Environment& cEnvironment_, const Plan::Sql::QueryArgument& cArgument_) const
{
	OSTRSTREAM cStream;
	cStream << getOperand0()->toSQLStatement(cEnvironment_, cArgument_);
	if (m_bIsNot) cStream << " not ";
	cStream << _pszOperatorName;
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
//	
//	
// RETURN
//	void
//
// EXCEPTIONS

//virtual
void
Impl::LikeImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	getOperand0()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
	if (m_bIsNot) cExec.append(" not ");
	cExec.append(_pszOperatorName);
	getOperand1()->setParameter(cEnvironment_, cProgram_, pIterator_, cExec, cArgument_);
}



// FUNCTION public
//	Predicate::Impl::LikeImpl::generate -- 
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
Impl::LikeImpl::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_bGenerateForPredicate = true;
	
	int iData0 = getOperand0()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	int iData1 = getOperand1()->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);

	// escape option is preceds to language
	// so, checking first option is enough
	int iEscape = -1;
	if (getOptionSize() > 0
		&& getOptioni(0)->getType() != Tree::Node::Language) {
		iEscape = getOptioni(0)->generate(cEnvironment_, cProgram_, pIterator_, cMyArgument);
	}

	Execution::Predicate::Like* pResult =
		Execution::Predicate::Like::create(cProgram_,
										   pIterator_,
										   iData0,
										   iData1,
										   iEscape,
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
