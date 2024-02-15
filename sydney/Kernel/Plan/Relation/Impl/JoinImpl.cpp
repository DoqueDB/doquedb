// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/JoinImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Relation";
}

#include "boost/function.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Plan/Relation/Impl/JoinImpl.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Join.h"
#include "Plan/Candidate/Distinct.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Order/Specification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/Trace.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_RELATION_USING

namespace
{
	class _Inquirer
	{
	public:
		_Inquirer(Opt::Environment& cEnvironment_,
				  const InquiryArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(cArgument_),
			  m_bPropagate(cArgument_.m_iTarget & (InquiryArgument::Target::Depending
												   | InquiryArgument::Target::Refering)),
			  m_iResult(0)
		{}

		void operator()(Interface::IRelation* pRelation_);

		Interface::IRelation::InquiryResult getResult()
		{return m_iResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		const InquiryArgument& m_cArgument;
		bool m_bPropagate;
		Interface::IRelation::InquiryResult m_iResult;
	};

	class _Sigma
	{
	public:
		_Sigma(Opt::Environment& cEnvironment_,
			   boost::function<int(Interface::IRelation*,
								   Opt::Environment&)> function_)
			: m_cEnvironment(cEnvironment_),
			  m_function(function_),
			  m_iResult(0)
		{}

		void operator()(Interface::IRelation* pRelation_);

		int getResult() {return m_iResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		boost::function<int(Interface::IRelation*,
							Opt::Environment&)> m_function;
		int m_iResult;
	};
	class _Pi
	{
	public:
		_Pi(Opt::Environment& cEnvironment_,
			boost::function<int(Interface::IRelation*,
								Opt::Environment&)> function_)
			: m_cEnvironment(cEnvironment_),
			  m_function(function_),
			  m_iResult(1)
		{}

		void operator()(Interface::IRelation* pRelation_);

		int getResult() {return m_iResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		boost::function<int(Interface::IRelation*,
							Opt::Environment&)> m_function;
		int m_iResult;
	};
}

////////////////////////
// $$$::_Inquirer

// FUNCTION local
//	$$$::_Inquirer::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Inquirer::
operator()(Interface::IRelation* pRelation_)
{
	if (m_bPropagate) {
		m_iResult |= pRelation_->inquiry(m_cEnvironment,
										 m_cArgument);
	}
}

////////////////////////
// $$$::_Sigma

// FUNCTION local
//	$$$::_Sigma::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Sigma::
operator()(Interface::IRelation* pRelation_)
{
	m_iResult += m_function(pRelation_, m_cEnvironment);
}

////////////////////////
// $$$::_Pi

// FUNCTION local
//	$$$::_Pi::operator() -- 
//
// NOTES
//
// ARGUMENTS
//	Interface::IRelation* pRelation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Pi::
operator()(Interface::IRelation* pRelation_)
{
	m_iResult *= m_function(pRelation_, m_cEnvironment);
}

///////////////////////////////////////
// Relation::JoinImpl::Dyadic

// FUNCTION public
//	Relation::JoinImpl::Dyadic::estimateUnwind -- 
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
JoinImpl::Dyadic::
estimateUnwind(Opt::Environment& cEnvironment_)
{
	return foreachOperand(_Sigma(cEnvironment_,
								 boost::bind(&Interface::IRelation::estimateUnwind,
											 _1,
											 _2))).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::Dyadic::estimateUnion -- 
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
JoinImpl::Dyadic::
estimateUnion(Opt::Environment& cEnvironment_)
{
	return foreachOperand(_Pi(cEnvironment_,
							  boost::bind(&Interface::IRelation::estimateUnion,
										  _1,
										  _2))).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::Dyadic::createAccessPlan -- create access plan candidate
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
JoinImpl::Dyadic::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Process)) {
		_OPT_OPTIMIZATION_MESSAGE << "Join: create access plan with predicate:"
								  << Utility::Trace::toString(cEnvironment_, cPlanSource_.getPredicate())
								  << ModEndl;
	}
#endif
	// step1: create first operand's plan for each candidate
	Interface::IRelation* pOperand0 = getOperand0();
	Interface::IRelation* pOperand1 = getOperand1();

	// if operands can't swap -> use 0->1
	bool bFixedOrder =
		Relation::Inquiry::isRefering(cEnvironment_,
									  pOperand1,
									  pOperand0);

	if (!bFixedOrder && cPlanSource_.getPredicate() != 0) {
		Utility::RelationSet cTable0;
		Utility::RelationSet cTable1;
		Utility::RelationSet cPrecedingTable;

		// get relations processed by preceding candidates
		FOREACH(cPlanSource_.getPrecedingCandidate(),
				boost::bind(&Interface::ICandidate::createReferingRelation,
							_1,
							boost::ref(cPrecedingTable)));

		// get relations underlying each operand
		pOperand0->getUsedTable(cEnvironment_, cTable0);
		pOperand1->getUsedTable(cEnvironment_, cTable1);

		cTable0.merge(cPrecedingTable);
		cTable1.merge(cPrecedingTable);

		// check effective predicates for each operand
		AccessPlan::Cost::Value cRate0 =
			cPlanSource_.getPredicate()->checkRate(cEnvironment_,
												   cTable0);
		AccessPlan::Cost::Value cRate1 =
			cPlanSource_.getPredicate()->checkRate(cEnvironment_,
												   cTable1);

		if (cRate1 < cRate0) {
			// operand1 is more likely to be reduced by predicates
			// -> swap
			SWAP(pOperand0, pOperand1);
		}
	}

	// check the number of candidates
	AccessPlan::Source cOperandSource(cPlanSource_);
	bFixedOrder = bFixedOrder
		|| cOperandSource.checkJoinMaxCandidates();

	// for first operand, limit specification has no meaning
	cOperandSource.addPredicate(cEnvironment_,
								getJoinPredicate());
	cOperandSource.eraseLimit();	 // erase only limit specification

	Interface::ICandidate* pOperandCandidate0 =
		pOperand0->createAccessPlan(cEnvironment_,
 									cOperandSource);

	// if operand0 can process sort, use 0->1
	bFixedOrder = bFixedOrder
		|| (cPlanSource_.getOrder()
			&& Order::Specification::isCompatible(pOperandCandidate0->getOrder(),
												  cPlanSource_.getOrder()));

	// set estimate limit again
	cOperandSource.setEstimateLimit(cPlanSource_.getEstimateLimit());

	Interface::ICandidate* pOperandCandidate1 =
		bFixedOrder ? 0 :
		pOperand1->createAccessPlan(cEnvironment_,
									cOperandSource);

	if (!bFixedOrder
		&& (cPlanSource_.getOrder()
			&& Order::Specification::isCompatible(pOperandCandidate1->getOrder(),
												  cPlanSource_.getOrder()))) {
		// if operand1 can process sort, use 1->0
		SWAP(pOperandCandidate0, pOperandCandidate1);
		SWAP(pOperand0, pOperand1);
		bFixedOrder = true;
		Interface::ICandidate::erase(cEnvironment_, pOperandCandidate1);
	}

	pOperandCandidate0->createCost(cEnvironment_,
								   cOperandSource);
	if (!bFixedOrder) {
		pOperandCandidate1->createCost(cEnvironment_,
									   cOperandSource);

		if (pOperandCandidate1->getCost() < pOperandCandidate0->getCost()) {
			// let Candidate0 have smaller cost
			SWAP(pOperandCandidate0, pOperandCandidate1);
			SWAP(pOperand0, pOperand1);
		}
	}

	// step2: create rest plan
	// 0->1
	// currently, only nested-loop joinImpl is considered

	Candidate::Join* pCandidate0 =
		Candidate::Join::NestedLoop::create(cEnvironment_,
											this);
	pCandidate0->setFirstPlan(pOperandCandidate0);
	pCandidate0->createPlan(cEnvironment_,
							cOperandSource,
							pOperand1);
	if (!bFixedOrder) {
		pCandidate0->createCost(cEnvironment_,
								cOperandSource);
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
			OSTRSTREAM stream;
			stream << "Join cost(0->1):" << pCandidate0->getCost();
			_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
		}
#endif
	}

	// 1->0
	// This case is skipped if it can be known as larger cost
	if (bFixedOrder
		|| pCandidate0->getCost() < pOperandCandidate1->getCost()) {
		// use 0->1
		return pCandidate0;
	} else {
		// calculate 1->0 case
		Candidate::Join* pCandidate1 =
			Candidate::Join::NestedLoop::create(cEnvironment_,
												this);
		pCandidate1->setFirstPlan(pOperandCandidate1);
		pCandidate1->createPlan(cEnvironment_,
								cOperandSource,
								pOperand0);
		pCandidate1->createCost(cEnvironment_,
								cOperandSource);
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
			OSTRSTREAM stream;
			stream << "Join cost(1->0):" << pCandidate1->getCost();
			_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
		}
#endif
		// take smaller plan
		if (pCandidate1->getCost() < pCandidate0->getCost()) {
			SWAP(pCandidate1, pCandidate0);
		}
		// erase unused object
		Interface::ICandidate::erase(cEnvironment_, pCandidate1);
		return pCandidate0;
	}
}

// FUNCTION public
//	Relation::JoinImpl::Dyadic::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
JoinImpl::Dyadic::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return foreachOperand(_Inquirer(cEnvironment_, cArgument_)).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::Dyadic::require -- 
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
JoinImpl::Dyadic::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Relation::JoinImpl::Dyadic::getUsedTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Dyadic::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}

////////////////////////////////////////
// Relation::JoinImpl::Nadic

// FUNCTION public
//	Relation::JoinImpl::Nadic::estimateUnwind -- 
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
JoinImpl::Nadic::
estimateUnwind(Opt::Environment& cEnvironment_)
{
	return foreachOperand(_Sigma(cEnvironment_,
								 boost::bind(&Interface::IRelation::estimateUnwind,
											 _1,
											 _2))).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::estimateUnion -- 
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
JoinImpl::Nadic::
estimateUnion(Opt::Environment& cEnvironment_)
{
	return foreachOperand(_Pi(cEnvironment_,
							  boost::bind(&Interface::IRelation::estimateUnion,
										  _1,
										  _2))).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::createAccessPlan -- create access plan candidate
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
JoinImpl::Nadic::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Process)) {
		_OPT_OPTIMIZATION_MESSAGE << "NadicJoin: create access plan with predicate:"
								  << Utility::Trace::toString(cEnvironment_, cPlanSource_.getPredicate())
								  << ModEndl;
	}
#endif
	/////////////////////////////////////////////////////////
	// step0: create N candidates for each operands
	/////////////////////////////////////////////////////////
	OperandMinCandidateMap cMap0;

	// for first operand, limit specification has no meaning
	AccessPlan::Source cInitialSource(cPlanSource_);
	cInitialSource.addPredicate(cEnvironment_,
								getJoinPredicate());
	cInitialSource.eraseLimit();		// erase only limit specification

	foreachOperand_i(boost::bind(&This::createInitialOperandPlan,
								 boost::ref(cEnvironment_),
								 boost::ref(cInitialSource),
								 _1,
								 _2,
								 boost::ref(cMap0)));
	; _SYDNEY_ASSERT(cMap0.ISEMPTY() == false);

	////////////////////////////////////////////////
	// step1~N: accumulate processed relations
	////////////////////////////////////////////////
	int iStep = 0;
	++iStep;
	while (iStep < getSize()) {
		AccessPlan::Source cOperandSource(cPlanSource_);
		OperandMinCandidateMap cMap1;
		int iPosition = 0;
		foreachOperand_i(boost::bind(&This::accumulateOperandPlan,
									 boost::ref(cEnvironment_),
									 boost::ref(cOperandSource),
									 _1,
									 _2,
									 boost::cref(cMap0),
									 boost::ref(cMap1)));
		; _SYDNEY_ASSERT(cMap1.ISEMPTY() == false);
		cMap0 = cMap1;
		++iStep;
	}
	; _SYDNEY_ASSERT(cMap0.GETSIZE() == 1);

	Interface::ICandidate* pResult = (*cMap0.begin()).second;

	return pResult;
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
JoinImpl::Nadic::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return foreachOperand(_Inquirer(cEnvironment_, cArgument_)).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::require -- 
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
JoinImpl::Nadic::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::getUsedTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Nadic::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}

// FUNCTION private
//	Relation::JoinImpl::Nadic::createInitialOperandPlan -- create first operand's plan
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	Interface::IRelation* pOperand_
//	int iPosition_
//	OperandMinCandidateMap& cMap_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
JoinImpl::Nadic::
createInitialOperandPlan(Opt::Environment& cEnvironment_,
						 AccessPlan::Source& cPlanSource_,
						 Interface::IRelation* pOperand_,
						 int iPosition_,
						 OperandMinCandidateMap& cMap_)
{
	if (cMap_.ISEMPTY() == false
		&& cPlanSource_.checkJoinMaxCandidates()) {
		// if join candidates reaches the limit, don't create
		return;
	}

	// create access plan for the operand without preceding tables
	Interface::ICandidate* pCandidate = pOperand_->createAccessPlan(cEnvironment_,
																	cPlanSource_);
	pCandidate->createCost(cEnvironment_,
						   cPlanSource_);

	// create key for the map
	Utility::LightBitSet cKey;
	cKey.set(iPosition_);

	// record first result to the map
	cMap_.insert(cKey, pCandidate);
}

// FUNCTION public
//	Relation::JoinImpl::Nadic::accumulateOperandPlan -- accumulate access plan for operands
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	Interface::IRelation* pOperand_
//	int iPosition_
//	const OperandMinCandidateMap& cPreviousMap_
//	OperandMinCandidateMap& cMap_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
JoinImpl::Nadic::
accumulateOperandPlan(Opt::Environment& cEnvironment_,
					  AccessPlan::Source& cPlanSource_,
					  Interface::IRelation* pOperand_,
					  int iPosition_,
					  const OperandMinCandidateMap& cPreviousMap_,
					  OperandMinCandidateMap& cMap_)
{
	// PreviousMap holds a set of candidates for {N-1} combinations of operands.
	// accumulateOperandPlan creates a set of candidates for N combinations.
	// ex.
	//   cPreviousMap : {0,1}=>candidate01, {0,2}=>candidate02, ...
	//   cMap : {0,1,2}=>min({0,1}x2,{0,2}x1,{1,2}x0)), ...

	bool bFixedOrder = cPlanSource_.checkJoinMaxCandidates();

	OperandMinCandidateMap::ConstIterator iterator = cPreviousMap_.begin();
	const OperandMinCandidateMap::ConstIterator last = cPreviousMap_.end();
	for (; iterator != last; ++iterator) {
		if ((*iterator).first.test(iPosition_)) continue;

		// target entry of previous map
		Interface::ICandidate* pPrevCandidate = (*iterator).second;

		// get calculated plan from map
		Utility::LightBitSet cKey((*iterator).first);

#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
			OSTRSTREAM stream;
			stream << "Accumulate join operand:{" << cKey.getString() << "} + " << iPosition_;
			_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
		}
#endif

		cKey.set(iPosition_);
		OperandMinCandidateMap::Iterator found = cMap_.find(cKey);
		AccessPlan::Cost cMinCost;
		if (found != cMap_.end()) {
			cMinCost = (*found).second->getCost();
		}

		// create plan for prev x iposition
		Candidate::Join* pCandidate =
			Candidate::Join::NestedLoop::create(cEnvironment_,
												0);
		pCandidate->setFirstPlan(pPrevCandidate);
		pCandidate->createPlan(cEnvironment_,
							   cPlanSource_,
							   pOperand_);
		pCandidate->createCost(cEnvironment_,
							   cPlanSource_);

		if (pCandidate->getCost() < cMinCost) {
			if (!cMinCost.isInfinity()) {
				// erase previous minimum plan
				Interface::ICandidate::erase(cEnvironment_,
											 cMap_[cKey]);
			}
			cMap_[cKey] = pCandidate;

			if (bFixedOrder) {
				// don't estimate others
				break;
			}
		} else {
			// erase unused object
			Interface::ICandidate::erase(cEnvironment_, pCandidate);
		}
	}
}

///////////////////////////////////////
// Relation::JoinImpl::Exists

// FUNCTION public
//	Relation::JoinImpl::Exists::createAccessPlan -- create access plan candidate
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
JoinImpl::Exists::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	if (getType() == Tree::Node::Exists
		&& cPlanSource_.isExists()) {
		// if exists join is placed under exists subquery,
		// it is treated as normal join with limit 1
		AccessPlan::Source cOperandSource(cPlanSource_);
		cOperandSource.setEstimateLimit(1);
		return Super::createAccessPlan(cEnvironment_,
									   cOperandSource);
	}
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Process)) {
		_OPT_OPTIMIZATION_MESSAGE << "ExistsJoin: create access plan with predicate:"
								  << Utility::Trace::toString(cEnvironment_, cPlanSource_.getPredicate())
								  << ModEndl;
	}
#endif
	// step1: create first operand's plan for each candidate
	Interface::IRelation* pOperand0 = getOperand0();
	Interface::IRelation* pOperand1 = getOperand1();

	AccessPlan::Source cOperandSource(cPlanSource_);
	// check the number of candidates
	bool bFixedOrder = (getType() == Tree::Node::NotExists)
		|| (cOperandSource.checkJoinMaxCandidates())
		|| Relation::Inquiry::isDepending(cEnvironment_,
										  pOperand1,
										  pOperand0);

	

	// for first operand, limit specification has no meaning
	cOperandSource.addPredicate(cEnvironment_,
								getJoinPredicate());
	cOperandSource.eraseLimit();		// erase only limit specification

	Interface::ICandidate* pOperandCandidate0 =
		pOperand0->createAccessPlan(cEnvironment_,
 									cOperandSource);
	pOperandCandidate0->createCost(cEnvironment_,
								   cOperandSource);

	// step2: create rest plan
	// 0->1
	// currently, only nested-loop joinImpl is considered

	// set exists flag
	cOperandSource.setExists();

	Candidate::Join* pCandidate0 =
		Candidate::Join::Exists::create(cEnvironment_,
										this);
	pCandidate0->setFirstPlan(pOperandCandidate0);
	pCandidate0->createPlan(cEnvironment_,
							cOperandSource,
							pOperand1);
	pCandidate0->createCost(cEnvironment_,
							cOperandSource);
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
		OSTRSTREAM stream;
		stream << "Exists cost(0->1):" << pCandidate0->getCost();
		_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
	}
#endif

	bFixedOrder = bFixedOrder
		|| (cPlanSource_.getOrder()
			&& Order::Specification::isCompatible(pCandidate0->getOrder(),
												  cPlanSource_.getOrder()));

	if (bFixedOrder) {
		// use 0->1
		return pCandidate0;

	} else {
		// 1->0
		// This case is skipped if it can be known as larger cost

		// set estimate limit again
		cOperandSource.setEstimateLimit(cPlanSource_.getEstimateLimit());
		// reset exists flag
		cOperandSource.eraseExists();

		Interface::ICandidate* pOperandCandidate1 =
			pOperand1->createAccessPlan(cEnvironment_,
										cOperandSource);
		pOperandCandidate1->createCost(cEnvironment_,
									   cOperandSource);

		if (pCandidate0->getCost() < pOperandCandidate1->getCost()) {
			// use 0->1
			return pCandidate0;
		} else {
			// calculate 1->0 case
			// nestedloop + distinct

			Candidate::Join* pJoin1 =
				Candidate::Join::NestedLoop::create(cEnvironment_,
													this);
			pJoin1->setFirstPlan(pOperandCandidate1);
			pJoin1->createPlan(cEnvironment_,
							   cOperandSource,
							   pOperand0);

			// add distinct
			Interface::ICandidate* pCandidate1 =
				Candidate::Distinct::create(cEnvironment_,
											pJoin1->getSecondPlan()->getKey(cEnvironment_),
											pJoin1);												  

			pCandidate1->createCost(cEnvironment_,
									cOperandSource);

			AccessPlan::Cost cCost0(pCandidate0->getCost());
			AccessPlan::Cost cCost1(pCandidate1->getCost());
			// add distinct cost
			if (cCost0.getLimitCount().isInfinity() == false
				&& cCost1.getLimitCount().isInfinity() == false
				&& cCost0.getTupleCount() < cCost1.getTupleCount()
				&& cCost0.getTupleCount() > 0) {
				cCost1.setLimitCount(cCost1.getLimitCount() * (cCost1.getTupleCount() / cCost0.getTupleCount()));
			}

#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
				OSTRSTREAM stream;
				stream << "Exists cost(1->0):" << cCost1;
				_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
			}
#endif

			// take smaller plan
			if (cCost1 < cCost0) {
				// erase unused object
				Interface::ICandidate::erase(cEnvironment_, pCandidate0);
				return pCandidate1;
			} else {
				// erase unused object
				Interface::ICandidate::erase(cEnvironment_, pCandidate1);
				Interface::ICandidate::erase(cEnvironment_, pJoin1);
				return pCandidate0;
			}
		}
	}
}

// FUNCTION public
//	Relation::JoinImpl::Exists::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
JoinImpl::Exists::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	if (cArgument_.m_iTarget & InquiryArgument::Target::Distinct) {
		// check first operand
		return getOperand0()->inquiry(cEnvironment_, cArgument_);

	} else {
		return Super::inquiry(cEnvironment_,
							  cArgument_);
	}
}

// FUNCTION public
//	Relation::JoinImpl::Exists::require -- 
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
JoinImpl::Exists::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Relation::JoinImpl::Exists::getUsedTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::Exists::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}

// FUNCTION private
//	Relation::JoinImpl::Exists::createRowInfo -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	RowInfo*
//
// EXCEPTIONS

//virtual
RowInfo*
JoinImpl::Exists::
createRowInfo(Opt::Environment& cEnvironment_)
{
	return getOperand0()->getRowInfo(cEnvironment_);
}

// FUNCTION private
//	Relation::JoinImpl::Exists::createKeyInfo -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	RowInfo*
//
// EXCEPTIONS

//virtual
RowInfo*
JoinImpl::Exists::
createKeyInfo(Opt::Environment& cEnvironment_)
{
	return getOperand0()->getKeyInfo(cEnvironment_);
}

// FUNCTION private
//	Relation::JoinImpl::Exists::setDegree -- 
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
JoinImpl::Exists::
setDegree(Opt::Environment& cEnvironment_)
{
	return getOperand0()->getDegree(cEnvironment_);
}

// FUNCTION private
//	Relation::JoinImpl::Exists::setMaxPosition -- 
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
JoinImpl::Exists::
setMaxPosition(Opt::Environment& cEnvironment_)
{
	return getOperand0()->getMaxPosition(cEnvironment_);
}

///////////////////////////////////////
// Relation::JoinImpl::DyadicOuter

// FUNCTION public
//	Relation::JoinImpl::DyadicOuter::estimateUnwind -- 
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
JoinImpl::DyadicOuter::
estimateUnwind(Opt::Environment& cEnvironment_)
{
	return foreachOperand(_Sigma(cEnvironment_,
								 boost::bind(&Interface::IRelation::estimateUnwind,
											 _1,
											 _2))).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::DyadicOuter::estimateUnion -- 
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
JoinImpl::DyadicOuter::
estimateUnion(Opt::Environment& cEnvironment_)
{
	return foreachOperand(_Pi(cEnvironment_,
							  boost::bind(&Interface::IRelation::estimateUnion,
										  _1,
										  _2))).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::DyadicOuter::createAccessPlan -- create access plan candidate
//	
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	AccessPlan::Source& cPlanSource_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
JoinImpl::DyadicOuter::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
#ifndef NO_TRACE
	if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Process)) {
		_OPT_OPTIMIZATION_MESSAGE << "OuterJoin: create access plan with predicate:"
								  << Utility::Trace::toString(cEnvironment_, cPlanSource_.getPredicate())
								  << ModEndl;
	}
#endif
	// step1: create first operand's plan for each candidate
	Interface::IRelation* pOperand0 = getOperand0();
	Interface::IRelation* pOperand1 = getOperand1();
	
	if (getType() == Tree::Node::RightOuterJoin) {
		if (Relation::Inquiry::isDepending(cEnvironment_,
										   pOperand1,
										   pOperand0)) {
			// can't swap -> can't process
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		SWAP(pOperand0, pOperand1);
	}

	// for first operand, limit specification has no meaning
	AccessPlan::Source cOperandSource(cPlanSource_);
	cOperandSource.eraseLimit();		// erase only limit specification

	// create first operand's candidate
	Interface::ICandidate* pOperandCandidate0 =
		pOperand0->createAccessPlan(cEnvironment_,
 									cOperandSource);

	// calculate cost
	pOperandCandidate0->createCost(cEnvironment_,
								   cOperandSource);

	// step2: create rest plan
	// 0->1
	Candidate::Join* pCandidate0 =
		Candidate::Join::NestedLoop::create(cEnvironment_,
											this);
	// erase predicate (use joinpredicate instead)
	cOperandSource.erasePredicate();

	pCandidate0->setFirstPlan(pOperandCandidate0);
	pCandidate0->createPlan(cEnvironment_,
							cOperandSource,
							pOperand1);

	if (Interface::IPredicate* pPredicate = pCandidate0->getPredicate()) {
		Predicate::CheckArgument cCheckArgument(pCandidate0,
												cPlanSource_.getPrecedingCandidate());
		pCandidate0->setPredicate(pPredicate->check(cEnvironment_,
													cCheckArgument));
	}

	pCandidate0->createCost(cEnvironment_,
							cOperandSource);

	return pCandidate0;
}

// FUNCTION public
//	Relation::JoinImpl::DyadicOuter::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Interface::IRelation::InquiryResult
JoinImpl::DyadicOuter::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return foreachOperand(_Inquirer(cEnvironment_, cArgument_)).getResult();
}

// FUNCTION public
//	Relation::JoinImpl::DyadicOuter::require -- 
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
JoinImpl::DyadicOuter::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	foreachOperand(boost::bind(&Operand::require,
							   _1,
							   boost::ref(cEnvironment_),
							   pCandidate_));
}

// FUNCTION public
//	Relation::JoinImpl::DyadicOuter::getUsedTable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Utility::RelationSet& cResult_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
JoinImpl::DyadicOuter::
getUsedTable(Opt::Environment& cEnvironment_,
			 Utility::RelationSet& cResult_)
{
	foreachOperand(boost::bind(&Operand::getUsedTable,
							   _1,
							   boost::ref(cEnvironment_),
							   boost::ref(cResult_)));
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
