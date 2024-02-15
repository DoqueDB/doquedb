// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/JoinImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Candidate::Impl";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Impl/JoinImpl.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Order/Specification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/Combinator.h"
#include "Plan/Relation/Join.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/EmptyNull.h"
#include "Execution/Iterator/Exists.h"
#include "Execution/Iterator/NestedLoop.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

namespace
{
#ifndef NO_TRACE
	void _addString(OSTRSTREAM& stream_,
					const STRING& string_)
	{
		if (!stream_.isEmpty()) {
			stream_ << ',';
		}
		stream_ << string_;
	}
	STRING _getCorrelationName(Opt::Environment& cEnvironment_,
							   Interface::ICandidate* pCandidate_)
	{
		OSTRSTREAM stream;

		Utility::RelationSet cRelation;
		pCandidate_->createReferingRelation(cRelation);

		cRelation.foreachElement(boost::bind(&_addString,
											 boost::ref(stream),
											 boost::bind(&Interface::IRelation::getCorrelationName,
														 _1,
														 boost::ref(cEnvironment_))));
		return stream.getString();
	}
#endif
}

//////////////////////////////////////////
// Plan::Candidate::JoinImpl::Base

// FUNCTION public
//	Candidate::JoinImpl::Base::require -- add retrieved columns
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Base::
require(Opt::Environment& cEnvironment_,
		Scalar::Field* pField_)
{
	m_cResult.first->require(cEnvironment_,
							 pField_);
	m_cResult.second->require(cEnvironment_,
							  pField_);
}

// FUNCTION public
//	Candidate::JoinImpl::Base::retrieve -- add retrieved columns
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Base::
retrieve(Opt::Environment& cEnvironment_,
		 Scalar::Field* pField_)
{
	m_cResult.first->retrieve(cEnvironment_,
							  pField_);
	m_cResult.second->retrieve(cEnvironment_,
							   pField_);
}

// FUNCTION public
//	Candidate::JoinImpl::Base::use -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Base::
use(Opt::Environment& cEnvironment_,
	Scalar::Field* pField_)
{
	m_cResult.first->use(cEnvironment_,
						 pField_);
	m_cResult.second->use(cEnvironment_,
						  pField_);
}

// FUNCTION public
//	Candidate::JoinImpl::Base::delay -- add retrieved columns as delayable
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Scalar::Field* pField_
//	Scalar::DelayArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
JoinImpl::Base::
delay(Opt::Environment& cEnvironment_,
	  Scalar::Field* pField_,
	  Scalar::DelayArgument& cArgument_)
{
	return m_cResult.first->delay(cEnvironment_, pField_, cArgument_)
		|| m_cResult.second->delay(cEnvironment_, pField_, cArgument_);
}

// FUNCTION public
//	Candidate::JoinImpl::Base::isReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
JoinImpl::Base::
isReferingRelation(Interface::IRelation* pRelation_)
{
	if (m_cReferingRelation.isEmpty()) {
		createReferingRelation(m_cReferingRelation);
	}
	return m_cReferingRelation.isContaining(pRelation_);
}

// FUNCTION public
//	Candidate::JoinImpl::Base::createReferingRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Utility::RelationSet& cRelationSet_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Base::
createReferingRelation(Utility::RelationSet& cRelationSet_)
{
	if (m_cReferingRelation.isEmpty()) {
		m_cResult.first->createReferingRelation(m_cReferingRelation);
		m_cResult.second->createReferingRelation(m_cReferingRelation);
	}
	cRelationSet_.merge(m_cReferingRelation);
}

// FUNCTION public
//	Candidate::JoinImpl::Base::getRelation -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Candidate::Table*
//
// EXCEPTIONS

//virtual
Candidate::Table*
JoinImpl::Base::
getCandidate(Opt::Environment& cEnvironment_,
			 Interface::IRelation* pRelation_)
{
	TableCandidateMap::Iterator found = m_mapTableCandidate.find(pRelation_);
	if (found != m_mapTableCandidate.end()) {
		return (*found).second;
	}
	Candidate::Table* pResult = 0;
	if ((pResult = m_cResult.first->getCandidate(cEnvironment_, pRelation_)) == 0) {
		pResult = m_cResult.second->getCandidate(cEnvironment_, pRelation_);
	}
	// cache the result
	m_mapTableCandidate.insert(pRelation_, pResult);

	return pResult;
}

// FUNCTION public
//	Candidate::JoinImpl::Base::setFirstPlan -- set first operand's plan
//
// NOTES
//
// ARGUMENTS
//	Interface::ICandidate* pPlan_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Base::
setFirstPlan(Interface::ICandidate* pPlan_)
{
	m_cResult.first = pPlan_;
}

// FUNCTION public
//	Candidate::JoinImpl::Base::createPlan -- create rest operand's plan
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Base::
createPlan(Opt::Environment& cEnvironment_,
		   AccessPlan::Source& cPlanSource_,
		   Interface::IRelation* pOperand_)
{
	// merge join predicate with other predicate
	// when join predicate is specified from createaccessplan, use it instead
	Interface::IPredicate* pPredicate = 0;
	VECTOR<Interface::IPredicate*> vecNotChecked;

	Interface::IPredicate* pNotChecked =
		m_cResult.first->getNotCheckedPredicate();

	if (getJoin() && getJoin()->isOuter()) {
		// check predicate for first operand here
		pPredicate = getJoin()->getJoinPredicate();
		if (pPredicate && !pPredicate->isChecked()) {
			Predicate::CheckArgument cCheckArgument(m_cResult.first,
													cPlanSource_.getPrecedingCandidate());
			pPredicate = pPredicate->check(cEnvironment_,
										   cCheckArgument);
		}
		if (pNotChecked) {
			// record notchecked predicate separatedly
			vecNotChecked.PUSHBACK(pNotChecked);
		}
	} else {
		pPredicate = pNotChecked;
	}

	// create plan using this candidate as source candidate
	AccessPlan::Source cSource(cPlanSource_,
							   pPredicate);
	cSource.eraseOrder();
	cSource.eraseLimit();
	cSource.eraseEstimateLimit();

	cSource.addPrecedingCandidate(m_cResult.first);

	m_cResult.second = pOperand_->createAccessPlan(cEnvironment_,
												   cSource);
	// calculate cost
	m_cResult.second->createCost(cEnvironment_, cSource);

	// set predicate for the join candidate if not checked
	if (Interface::IPredicate* pNotChecked2 =
		m_cResult.second->getNotCheckedPredicate()) {
		vecNotChecked.PUSHBACK(pNotChecked2);
	}

	if (vecNotChecked.ISEMPTY() == false) {
		Opt::ForEach(vecNotChecked,
					 boost::bind(&Interface::IPredicate::require,
								 _1,
								 boost::ref(cEnvironment_),
								 this));
		setPredicate(Predicate::Combinator::create(cEnvironment_,
												   Tree::Node::And,
												   vecNotChecked));
	}
}

// FUNCTION public
//	Candidate::JoinImpl::Base::getFirstPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
JoinImpl::Base::
getFirstPlan()
{
	return m_cResult.first;
}

// FUNCTION public
//	Candidate::JoinImpl::Base::getSecondPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
JoinImpl::Base::
getSecondPlan()
{
	return m_cResult.second;
}

// FUNCTION private
//	Candidate::JoinImpl::Base::createCost -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	AccessPlan::Cost& cCost_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Base::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		// not calculated yet
		calculateCost(cEnvironment_,
					  cPlanSource_,
					  m_cResult.first->getCost(),
					  m_cResult.second->getCost(),
					  cCost_);
	}
}

// FUNCTION private
//	Candidate::JoinImpl::Base::createRow -- merge operand row
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
JoinImpl::Base::
createRow(Opt::Environment& cEnvironment_)
{
	if (getJoin() && getJoin()->isExists()) {
		// join is exists but candidate is base -> nested loop + distinct plan
		// -> take only second row
		return m_cResult.second->getRow(cEnvironment_);
	}

	Candidate::Row* pRow = Candidate::Row::create(cEnvironment_);

	; _SYDNEY_ASSERT(m_cResult.first);
	; _SYDNEY_ASSERT(m_cResult.second);

	pRow->addRow(*(m_cResult.first->getRow(cEnvironment_)));
	pRow->addRow(*(m_cResult.second->getRow(cEnvironment_)));

	return pRow;
}

// FUNCTION private
//	Candidate::JoinImpl::Base::createKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
JoinImpl::Base::
createKey(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pRow = Candidate::Row::create(cEnvironment_);

	// get key from relation
	if (getJoin()) {
		Relation::RowInfo* pKeyInfo = getJoin()->getKeyInfo(cEnvironment_);
		pKeyInfo->foreachElement(boost::bind(&Candidate::Row::addScalar,
											 pRow,
											 boost::bind(&Relation::RowElement::getScalar,
														 _1,
														 boost::ref(cEnvironment_))));
	} else {
		; _SYDNEY_ASSERT(m_cResult.first);
		; _SYDNEY_ASSERT(m_cResult.second);

		pRow->addRow(*(m_cResult.first->getKey(cEnvironment_)));
		pRow->addRow(*(m_cResult.second->getKey(cEnvironment_)));
	}
	return pRow;
}

//////////////////////////////////////////
// Plan::Candidate::JoinImpl::NestedLoop

// FUNCTION public
//	Candidate::JoinImpl::NestedLoop::getOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Order::Specification*
//
// EXCEPTIONS

//virtual
Order::Specification*
JoinImpl::NestedLoop::
getOrder()
{
	return getResult().first->getOrder();
}

// FUNCTION public
//	Candidate::JoinImpl::NestedLoop::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
JoinImpl::NestedLoop::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	bool bOuter = getJoin() && getJoin()->isOuter();

	// main iterator is nested-loop
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::NestedLoop::create(cProgram_);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	// set first operand's row as delayed if can
	Candidate::Row* pRow0 =
		getResult().first->getRow(cEnvironment_)->delay(cEnvironment_,
														getResult().first);

	// adopt first operand
	Execution::Interface::IIterator* pIterator0 =
		getResult().first->adopt(cEnvironment_, cProgram_, cArgument_);

	Execution::Interface::IIterator* pInputSave = cArgument_.m_pInput;
	bool bCollectingSave = cArgument_.m_bCollecting;
	cArgument_.setCandidate(getResult().first);

	// generate first operand's row
	pRow0->generate(cEnvironment_, cProgram_, pIterator0, cArgument_);

	// set last iterator to adoptargument
	cArgument_.m_pInput = pIterator0;

	// set second operand's row as delayed if can
	Candidate::Row* pRow1 =
		getResult().second->getRow(cEnvironment_)->delay(cEnvironment_,
														 getResult().second);

	// adopt second operand
	Execution::Interface::IIterator* pIterator1 =
		getResult().second->adopt(cEnvironment_, cProgram_, cArgument_);

	cArgument_.setCandidate(getResult().second);

	// generate key fields for delayed retrieval
	pRow0->generate(cEnvironment_, cProgram_, pIterator1, cArgument_);
	pRow1->generate(cEnvironment_, cProgram_, pIterator1, cArgument_);

	// set last iterator to adoptargument
	cArgument_.m_pInput = pIterator1;
	cArgument_.m_bCollecting = bOuter; // if outer join, it works same as collecting

	// generate key fields for delayed retrieval
	int iDataID0 = pRow0->generate(cEnvironment_, cProgram_, pResult, cArgument_);
	int iDataID1 = pRow1->generate(cEnvironment_, cProgram_, pResult, cArgument_);

	if (bOuter) {
		// Outer join iterator
		Execution::Interface::IIterator* pIterator2 =
			Execution::Iterator::EmptyNull::create(cProgram_);
		pIterator2->addAction(cProgram_,
							  _ACTION_ARGUMENT2(Input,
												pIterator1->getID(),
												iDataID1));
		pIterator2->copyNodeVariable(pIterator1);
		pIterator1 = pIterator2;
	}

	getResult().first->getKey(cEnvironment_)->foreachElement(
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pIterator0,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 false /* not collection */));
	getResult().first->getRow(cEnvironment_)->foreachElement(
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pIterator0,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 false /* not collection */));
	getResult().second->getKey(cEnvironment_)->foreachElement(
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pIterator1,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 false /* not collection */));
	getResult().second->getRow(cEnvironment_)->foreachElement(
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pIterator1,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 false /* not collection */));

	//////////////////////////////
	//////////////////////////////

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(Input,
										 pIterator0->getID()));
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(Input,
										 pIterator1->getID()));

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0(CheckCancel));

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);

		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}
	cArgument_.m_pInput = pInputSave;
	cArgument_.m_bCollecting = bCollectingSave;

	return pResult;
}

// FUNCTION public
//	Candidate::JoinImpl::NestedLoop::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::ICandidate::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::ICandidate::InquiryResult
JoinImpl::NestedLoop::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	PAIR<InquiryResult, InquiryResult> cResult =
		MAKEPAIR(getResult().first->inquiry(cEnvironment_, cArgument_),
				 getResult().second->inquiry(cEnvironment_, cArgument_));

	InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & InquiryArgument::Target::ReferTable) {
		iResult |= (cResult.first & InquiryArgument::Target::ReferTable
					| cResult.second & InquiryArgument::Target::ReferTable);
	}
	return iResult;
}

// FUNCTION public
//	Candidate::JoinImpl::NestedLoop::generateDelayed -- generate additional action to obtain delayed scalars
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::NestedLoop::
generateDelayed(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_)
{
	getResult().first->generateDelayed(cEnvironment_,
									   cProgram_,
									   pIterator_);
	getResult().second->generateDelayed(cEnvironment_,
										cProgram_,
										pIterator_);
}

// FUNCTION private
//	Candidate::JoinImpl::NestedLoop::calculateCost -- calculate cost
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	const AccessPlan::Cost& cCost0_
//	const AccessPlan::Cost& cCost1_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
JoinImpl::NestedLoop::
calculateCost(Opt::Environment& cEnvironment_,
			  const AccessPlan::Source& cPlanSource_,
			  const AccessPlan::Cost& cCost0_,
			  const AccessPlan::Cost& cCost1_,
			  AccessPlan::Cost& cResult_)
{
	AccessPlan::Cost cCost0(cCost0_);
	AccessPlan::Cost cCost1(cCost1_);

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Calculate join cost:"
			   << "\noperand0("
			   << _getCorrelationName(cEnvironment_, getFirstPlan())
			   << "):\n  " << cCost0
			   << "\noperand1("
			   << _getCorrelationName(cEnvironment_, getSecondPlan())
			   << "):\n  " << cCost1;
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
	AccessPlan::Cost::Value cOverhead = cCost0.getOverhead() + cCost1.getOverhead();
	AccessPlan::Cost::Value cTupleCount = cCost0.getTupleCount();

	if ((getJoin() && getJoin()->isOuter() == false)
		|| cCost1.getTupleCount() > 1) {
		cTupleCount *= cCost1.getTupleCount();
	}
	if (cCost0.getRate().isInfinity() == false
		&& cCost0.getRate() > 0) {
		cTupleCount *= cCost0.getRate();
	}
	if (cCost1.getRate().isInfinity() == false
		&& cCost1.getRate() > 0) {
		cTupleCount *= cCost1.getRate();
	}

	AccessPlan::Cost::Value cRepeatCount(1);
	if (cCost1.isFetch() == false
		&& cCost1.getRate().isInfinity() == false
		&& cCost1.getRate() > 0) {
		cRepeatCount /= cCost1.getRate();
	}
	AccessPlan::Cost::Value cRetrieveCost = cCost0.getRetrieveCost();
	AccessPlan::Cost::Value cTotalCost = (cCost0.getTotalCost()
										  + (cCost1.getRepeatCost() * cCost0.getTupleCount() * cRepeatCount));
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Total cost:\n  "
			   << cCost0.getTotalCost() 
			   << " + ("
			   << cCost1.getRepeatCost()
			   << " * "
			   << cCost0.getTupleCount()
			   << " * "
			   << cRepeatCount
			   << ") = "
			   << cTotalCost;
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
	cResult_.setOverhead(cOverhead);
	cResult_.setTotalCost(cTotalCost);
	cResult_.setTupleCount(cTupleCount);
	cResult_.setRetrieveCost(cRetrieveCost);

	AccessPlan::Cost::Value cRate(1);
	if (Interface::IPredicate* pPredicate = getPredicate()) {
		AccessPlan::Cost cPredicateCost;
		if (pPredicate->estimateCost(cEnvironment_,
									 cPredicateCost)) {
			cRate = cPredicateCost.getRate();
		}
	}
	cResult_.setRate(cRate);

	if (cCost1.isFetch()
		&& (cPlanSource_.getOrder() == 0
			|| Order::Specification::isCompatible(getOrder(),
												  cPlanSource_.getOrder()))) {
		cResult_.setLimitCount(cPlanSource_.getEstimateLimit());
	}
	if (cCost0.isFetch()) {
		cResult_.setIsFetch();
	}

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Result join cost\n"
			   << "("
			   << _getCorrelationName(cEnvironment_, getFirstPlan())
			   <<") x ("
			   << _getCorrelationName(cEnvironment_, getSecondPlan())
			   << "):\n  " << cResult_;
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
}

//////////////////////////////////////////
// Plan::Candidate::JoinImpl::Exists

// FUNCTION public
//	Candidate::JoinImpl::Exists::getOrder -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Order::Specification*
//
// EXCEPTIONS

//virtual
Order::Specification*
JoinImpl::Exists::
getOrder()
{
	return getResult().first->getOrder();
}

// FUNCTION public
//	Candidate::JoinImpl::Exists::adopt -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual
Execution::Interface::IIterator*
JoinImpl::Exists::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	; _SYDNEY_ASSERT(getJoin());

	// main iterator is exists-loop
	Execution::Interface::IIterator* pResult =
		getJoin()->getType() == Tree::Node::Exists
		? Execution::Iterator::Exists::create(cProgram_)
		: Execution::Iterator::Exists::Not::create(cProgram_);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	// set first operand's row as delayed if can
	Candidate::Row* pRow0 =
		getResult().first->getRow(cEnvironment_)->delay(cEnvironment_,
														getResult().first);

	// adopt first operand
	Execution::Interface::IIterator* pIterator0 =
		getResult().first->adopt(cEnvironment_, cProgram_, cArgument_);

	Execution::Interface::IIterator* pInputSave = cArgument_.m_pInput;
	cArgument_.setCandidate(getResult().first);

	// generate first operand's row
	pRow0->generate(cEnvironment_, cProgram_, pIterator0, cArgument_);

	// set last iterator to adoptargument
	cArgument_.m_pInput = pIterator0;

	// adopt second operand
	Execution::Interface::IIterator* pIterator1 =
		getResult().second->adopt(cEnvironment_, cProgram_, cArgument_);

	cArgument_.setCandidate(getResult().second);

	Candidate::Row* pRow1 =
		getResult().second->getRow(cEnvironment_)->delay(cEnvironment_,
														 getResult().second);

	// generate key fields for delayed retrieval
	pRow0->generate(cEnvironment_, cProgram_, pIterator1, cArgument_);
	pRow1->generate(cEnvironment_, cProgram_, pIterator1, cArgument_);

	// set last iterator to adoptargument
	cArgument_.m_pInput = pIterator1;

	// generate key fields for delayed retrieval
	pRow0->generate(cEnvironment_, cProgram_, pResult, cArgument_);

	getResult().first->getKey(cEnvironment_)->foreachElement(
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pIterator0,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 false /* not collection */));
	getResult().first->getRow(cEnvironment_)->foreachElement(
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pIterator0,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 false /* not collection */));

	//////////////////////////////
	//////////////////////////////

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(Input,
										 pIterator0->getID()));
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(Input,
										 pIterator1->getID()));

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0(CheckCancel));

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}
	cArgument_.m_pInput = pInputSave;

	return pResult;
}

// FUNCTION public
//	Candidate::JoinImpl::Exists::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::ICandidate::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::ICandidate::InquiryResult
JoinImpl::Exists::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	PAIR<InquiryResult, InquiryResult> cResult =
		MAKEPAIR(getResult().first->inquiry(cEnvironment_, cArgument_),
				 getResult().second->inquiry(cEnvironment_, cArgument_));

	InquiryResult iResult = 0;
	if (cArgument_.m_iTarget & InquiryArgument::Target::ReferTable) {
		iResult |= (cResult.first & InquiryArgument::Target::ReferTable
					| cResult.second & InquiryArgument::Target::ReferTable);
	}
	return iResult;
}

// FUNCTION public
//	Candidate::JoinImpl::Exists::generateDelayed -- generate additional action to obtain delayed scalars
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Exists::
generateDelayed(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_)
{
	getResult().first->generateDelayed(cEnvironment_,
									   cProgram_,
									   pIterator_);
}

// FUNCTION private
//	Candidate::JoinImpl::Exists::calculateCost -- calculate cost
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const AccessPlan::Source& cPlanSource_
//	const AccessPlan::Cost& cCost0_
//	const AccessPlan::Cost& cCost1_
//	AccessPlan::Cost& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
JoinImpl::Exists::
calculateCost(Opt::Environment& cEnvironment_,
			  const AccessPlan::Source& cPlanSource_,
			  const AccessPlan::Cost& cCost0_,
			  const AccessPlan::Cost& cCost1_,
			  AccessPlan::Cost& cResult_)
{
	AccessPlan::Cost cCost0(cCost0_);
	AccessPlan::Cost cCost1(cCost1_);
	if (cCost1.isFetch()) {
		// set estimate limit 1
		cCost1.setLimitCount(1);
	}

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Calculate exists cost:"
			   << "\noperand0("
			   << _getCorrelationName(cEnvironment_, getFirstPlan())
			   << "):\n  " << cCost0
			   << "\noperand1("
			   << _getCorrelationName(cEnvironment_, getSecondPlan())
			   << "):\n  " << cCost1;
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
	AccessPlan::Cost::Value cOverhead = cCost0.getOverhead() + cCost1.getOverhead();
	AccessPlan::Cost::Value cTupleCount = cCost0.getTupleCount();

	if (cCost0.getRate().isInfinity() == false
		&& cCost0.getRate() > 0) {
		cTupleCount *= cCost0.getRate();
	}
	if (cCost1.getTupleCount() * cCost1.getRate() < 1) {
		cTupleCount *= cCost1.getTupleCount() * cCost1.getRate();
	}

	AccessPlan::Cost::Value cRepeatCount(1);
	if (cCost1.isFetch() == false
		&& cCost1.getRate().isInfinity() == false
		&& cCost1.getRate() > 0) {
		cRepeatCount /= cCost1.getRate();
	}
	AccessPlan::Cost::Value cRetrieveCost = cCost0.getRetrieveCost();
	AccessPlan::Cost::Value cTotalCost = (cCost0.getTotalCost()
										  + (cCost1.getRepeatCost() * cCost0.getTupleCount() * cRepeatCount));
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Total cost:\n  "
			   << cCost0.getTotalCost() 
			   << " + ("
			   << cCost1.getRepeatCost()
			   << " * "
			   << cCost0.getTupleCount()
			   << " * "
			   << cRepeatCount
			   << ") = "
			   << cTotalCost;
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
	cResult_.setOverhead(cOverhead);
	cResult_.setTotalCost(cTotalCost);
	cResult_.setTupleCount(cTupleCount);
	cResult_.setRetrieveCost(cRetrieveCost);

	AccessPlan::Cost::Value cRate(1);
	if (Interface::IPredicate* pPredicate = getPredicate()) {
		AccessPlan::Cost cPredicateCost;
		if (pPredicate->estimateCost(cEnvironment_,
									 cPredicateCost)) {
			cRate = cPredicateCost.getRate();
		}
	}
	cResult_.setRate(cRate);

	if (cCost1.isFetch()
		&& (cPlanSource_.getOrder() == 0
			|| Order::Specification::isCompatible(getOrder(),
												  cPlanSource_.getOrder()))) {
		cResult_.setLimitCount(cPlanSource_.getEstimateLimit());
	}
	if (cCost0.isFetch()) {
		cResult_.setIsFetch();
	}

#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Result exists cost\n"
			   << "("
			   << _getCorrelationName(cEnvironment_, getFirstPlan())
			   <<") x ("
			   << _getCorrelationName(cEnvironment_, getSecondPlan())
			   << "):\n  " << cResult_;
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif
}

// FUNCTION private
//	Candidate::JoinImpl::Exists::createRow -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
JoinImpl::Exists::
createRow(Opt::Environment& cEnvironment_)
{
	return getResult().first->getRow(cEnvironment_);
}

// FUNCTION private
//	Candidate::JoinImpl::Exists::createKey -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Candidate::Row*
//
// EXCEPTIONS

//virtual
Candidate::Row*
JoinImpl::Exists::
createKey(Opt::Environment& cEnvironment_)
{
	return getResult().first->getKey(cEnvironment_);
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
