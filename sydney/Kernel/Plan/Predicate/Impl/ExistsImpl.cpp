// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ExistsImpl.cpp --
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

#include "Plan/Predicate/Impl/ExistsImpl.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Relation/Join.h"
#include "Plan/Relation/Selection.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "DExecution/Action/StatementConstruction.h"

#include "Execution/Action/Argument.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Predicate/Combinator.h"
#include "Execution/Predicate/IsEmpty.h"


#include "LogicalFile/Estimate.h"

#include "Opt/Configuration.h"
#include "Opt/Explain.h"

#include "Os/Math.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace
{
	const char* const _pszOperatorName = "exists (...)";
}

/////////////////////////////////////////////
//	Plan::Predicate::Impl::ExistsImpl

// FUNCTION public
//	Predicate::Impl::ExistsImpl::require -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExistsImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pOperand->require(cEnvironment_,
						pCandidate_);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::retrieve -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExistsImpl::
retrieve(Opt::Environment& cEnvironment_)
{
	// require outer reference in operand
	m_pOperand->require(cEnvironment_);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::retrieve -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExistsImpl::
retrieve(Opt::Environment& cEnvironment_,
		 Interface::ICandidate* pCandidate_)
{
	// require outer reference in operand
	m_pOperand->require(cEnvironment_,
						pCandidate_);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::use -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExistsImpl::
use(Opt::Environment& cEnvironment_,
	Interface::ICandidate* pCandidate_)
{
	// require outer reference in operand
	m_pOperand->require(cEnvironment_,
						pCandidate_);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::convertNot -- 
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
Impl::ExistsImpl::
convertNot(Opt::Environment& cEnvironment_)
{
	return create(cEnvironment_,
				  m_pOperand,
				  m_cOuterRelation,
				  m_bUnderExists,
				  getType() == Tree::Node::Exists);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::estimateRewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
Impl::ExistsImpl::
estimateRewrite(Opt::Environment& cEnvironment_)
{
	return m_pOperand->estimateUnion(cEnvironment_);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::rewrite -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	IRelation* pRelation_
//	Predicate::RewriteArgument& cArgument_
//	
// RETURN
//	Interface::IPredicate::RewriteResult
//
// EXCEPTIONS

//virtual
Interface::IPredicate::RewriteResult
Impl::ExistsImpl::
rewrite(Opt::Environment& cEnvironment_,
		Interface::IRelation* pRelation_,
		Predicate::RewriteArgument& cArgument_)
{
	if (cArgument_.m_bNoRelationChange || cEnvironment_.hasCascade()) {
		return Interface::IPredicate::RewriteResult(pRelation_, this);
	}

	// unwind subquery
	PAIR<Interface::IRelation*, Interface::IPredicate*> cUnwind =
		m_pOperand->unwind(cEnvironment_);

	// convert into exists join
	Interface::IRelation* pResult =
		Relation::Join::create(cEnvironment_,
							   getType() == Tree::Node::Exists && m_bUnderExists
							   ? Tree::Node::SimpleJoin
							   : getType(),
							   cUnwind.second /* join predicate */,
							   MAKEPAIR(pRelation_, cUnwind.first));

	return Interface::IPredicate::RewriteResult(pResult, 0);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::check -- 
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
Impl::ExistsImpl::
check(Opt::Environment& cEnvironment_,
	  const CheckArgument& cArgument_)
{
	Check::Value cResult = Check::Constant;
	m_cOuterRelation.foreachElement(boost::bind(&This::checkOuterRelation,
												this,
												_1,
												boost::ref(cEnvironment_),
												boost::ref(cArgument_),
												&cResult));
	if (Check::isOn(cResult, Check::NotYet)) {
		// unwind subquery
		require(cEnvironment_,
				cArgument_.m_pCandidate);
		return this;
	}
	// no index files
	return CheckedInterface::create(cEnvironment_,
									this);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::hasSubquery -- is using subquery?
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
Impl::ExistsImpl::
hasSubquery()
{
	return true;
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::estimateCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Impl::ExistsImpl::
estimateCost(Opt::Environment& cEnvironment_,
			 AccessPlan::Cost& cCost_)
{
	// estimate tuple size assuming average column size=8
	double dblTupleSize = MAX(4, m_pOperand->getDegree(cEnvironment_)) * 8;
	double dblProcessCost = dblTupleSize / LogicalFile::Estimate::getTransferSpeed(LogicalFile::Estimate::File);

	cCost_.setOverhead(dblProcessCost);
	cCost_.setTotalCost(dblProcessCost);

	// hit rate is 1/100^(the number of outer relations+1)
	double dblCount = 1.0;
	double dblOuterRelations = MIN(SIZE(3), m_cOuterRelation.getSize());
	if (dblOuterRelations > 0) {
		dblCount *= Os::Math::pow(100.0, dblOuterRelations + 1);
	}
	cCost_.setRetrieveCost(dblProcessCost * dblCount);

	double dblRate = (getType() == Tree::Node::NotExists) ? 1.0 - 1.0 / dblCount
		: 1.0 / dblCount;

	if (cCost_.isSetRate()) {
		cCost_.setRate(dblRate);
	}
	if (cCost_.isSetCount()) {
		AccessPlan::Cost::Value cCount = cCost_.getTableCount();
		if (cCount.isInfinity()) {
			// use default
			cCount = 100000;
		}
		cCost_.setTupleCount(cCount * dblRate);
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Estimate rate: " << cCost_.getRate();
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif

	return true;
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::getUsedTable -- check used tables
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExistsImpl::
getUsedTable(Utility::RelationSet& cResult_)
{
	cResult_.merge(m_cOuterRelation);
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::getUsedField -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::FieldSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExistsImpl::
getUsedField(Utility::FieldSet& cResult_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::getUnknownKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Predicate::CheckUnknownArgument& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
Impl::ExistsImpl::
getUnknownKey(Opt::Environment& cEnvironment_,
			  Predicate::CheckUnknownArgument& cResult_)
{
	;
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::explain -- 
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
Impl::ExistsImpl::
explain(Opt::Environment* pEnvironment_,
		Opt::Explain& cExplain_)
{
	cExplain_.put(_pszOperatorName);
}



// FUNCTION public
//	Predicate::Impl::ExistsImpl::explain -- 
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
STRING
Impl::ExistsImpl::
toSQLStatement(Opt::Environment& cEnvironment_,
			   const Plan::Sql::QueryArgument& cArgument_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);	
}


// FUNCTION public
//	Predicate::Impl::ExistsImpl::setParameter
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
Impl::ExistsImpl::
setParameter(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 DExecution::Action::StatementConstruction& cExec_,
			 const Plan::Sql::QueryArgument& cArgument_)
{
	if (getType() == Tree::Node::NotExists) {
		cExec_.append("not exists(");
	} else {
		cExec_.append("exists(");		
	}
	Plan::Sql::Query* pQuery = m_pOperand->generateSQL(cEnvironment_);
	pQuery->setParameter(cEnvironment_,
						 cProgram_,
						 pIterator_,
						 cExec_,
						 cArgument_);
	cExec_.append(")");
}

// FUNCTION public
//	Predicate::Impl::ExistsImpl::generate -- 
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
Impl::ExistsImpl::
generate(Opt::Environment& cEnvironment_,
		 Execution::Interface::IProgram& cProgram_,
		 Execution::Interface::IIterator* pIterator_,
		 Candidate::AdoptArgument& cArgument_)
{
	// get variable ID corresponding to the predicate
	int iID = getNodeVariable(pIterator_, cArgument_);
	if (iID < 0) {
		/////////////
		// generate subquery
		/////////////

		AccessPlan::Source cSource(cArgument_.m_vecPrecedingCandidate, 0, 0);
		if (cArgument_.m_pCandidate) {
			cSource.addPrecedingCandidate(cArgument_.m_pCandidate);
		}

		Interface::ICandidate* pCandidate =
			m_pOperand->createAccessPlan(cEnvironment_, 
										 cSource);
		if (pCandidate == 0) {
			// can't optimize
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		Candidate::AdoptArgument cMyArgument(cArgument_);
		cMyArgument.m_pInput = pIterator_;
		cMyArgument.setCandidate(pCandidate);

		Execution::Interface::IIterator* pIterator =
			pCandidate->adopt(cEnvironment_, cProgram_, cMyArgument);
		if (pIterator == 0) {
			_SYDNEY_THROW0(Exception::NotSupported);
		}

		// generate predicate
		Execution::Interface::IPredicate* pResult =
			Execution::Predicate::IsEmpty::Iterator::create(cProgram_,
															pIterator_,
															pIterator->getID());
		if (getType() == Tree::Node::Exists) {
			// exists <=> iterator is not empty
			pResult = Execution::Predicate::Combinator::Not::create(cProgram_,
																	pIterator_,
																	pResult->getID());
		}
		iID = pResult->getID();
		setNodeVariable(pIterator_, cArgument_, iID);
	}
	return iID;
}

// FUNCTION private
//	Predicate::Impl::ExistsImpl::checkOuterRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	Opt::Environment& cEnvironment_
//	const CheckArgument& cArgument_
//	Check::Value* pValue_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::ExistsImpl::
checkOuterRelation(Interface::IRelation* pRelation_,
				   Opt::Environment& cEnvironment_,
				   const CheckArgument& cArgument_,
				   Check::Value* pValue_)
{
	if (cArgument_.m_pCandidate
		&& cArgument_.m_pCandidate->isReferingRelation(pRelation_)) {
		Check::mergeValue(pValue_, Check::Referred);
		// set as requried in the candidate
		require(cEnvironment_,
				cArgument_.m_pCandidate);

	} else if (cArgument_.m_vecPrecedingCandidate.ISEMPTY() == false
			   && Opt::IsAny(cArgument_.m_vecPrecedingCandidate,
							 boost::bind(&Interface::ICandidate::isReferingRelation,
										 _1,
										 pRelation_))) {
		Check::mergeValue(pValue_, Check::Preceding);

	} else {
		Check::mergeValue(pValue_, Check::NotYet);
	}
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2014, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
