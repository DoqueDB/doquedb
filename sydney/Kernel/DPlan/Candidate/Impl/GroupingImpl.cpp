// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/GroupingImpl.cpp --
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
const char moduleName[] = "DPlan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DPlan/Candidate/Impl/GroupingImpl.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"
#include "Plan/Candidate/Table.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Relation/Grouping.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Interface/IProgram.h"
#include "Execution/Collection/Grouping.h"
#include "Execution/Iterator/Filter.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::GroupingImpl::Normal


// FUNCTION public
//	Candidate::GroupingImpl::Normal::adopt -- 
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
GroupingImpl::Normal::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	
	// adopt operand
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	Execution::Interface::IIterator* pOperandIterator =
		getOperand()->adopt(cEnvironment_, cProgram_, cMyArgument);

	Plan::Candidate::RowDelayArgument cDelayArgument;
	Plan::Candidate::Row* pOperandRow = cArgument_.m_pQuery->createRow(cEnvironment_);
	
	Plan::Order::GeneratedSpecification* pGenerated =
		m_pSpecification->generate(cEnvironment_,
								   cProgram_,
								   pOperandIterator,
								   cMyArgument,
								   pOperandRow,
								   cDelayArgument);

	; _SYDNEY_ASSERT(pGenerated);
	; _SYDNEY_ASSERT(pGenerated->getDataID().GETSIZE() == pGenerated->getTuple().GETSIZE());

	// collection on memory with grouping
	Execution::Collection::Grouping* pGrouping =
		Execution::Collection::Grouping::create(cProgram_,
												pGenerated->getPosition(),
												true);

	// main iterator is filtering iterator
	Execution::Interface::IIterator* pResult = Execution::Iterator::Filter::create(cProgram_,
																				   pGrouping->getID());

	int iPutDataID = pOperandRow->generate(cEnvironment_,
										   cProgram_,
										   pOperandIterator,
										   cArgument_);

	// filter input is operand result
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pOperandIterator->getID(),
										iPutDataID));

	
	// from here, use adoptargument passed from parent candidate
	// add candidate for possible subquery
	cArgument_.setCandidate(this);
	int iGetDataID = cArgument_.m_pQuery->createOutput(cEnvironment_,
													   cProgram_,
													   pResult,	
													   cArgument_);

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iGetDataID));

	// gerenate aggretations
	cArgument_.m_pInput = pOperandIterator;
	Plan::Utility::ScalarSet::Iterator ite = m_vecAggregation.begin();
	for (; ite != m_vecAggregation.end(); ++ite ) {
		(*ite)->generate(cEnvironment_, cProgram_, pResult, cArgument_);
	}
	
	// add cancel poling
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(CheckCancel,
										   cArgument_.m_eTarget));

	if (getPredicate()) {
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}
	return pResult;
}


// FUNCTION public
//	Candidate::GroupingImpl::Normal::generateSQL
//
//
// NOTES
//	
//
// ARGUMENTS
//	Opt::Environment cEnvironment_
//
//	
// RETURN
//	Sql::Query*
//
// EXCEPTIONS

//virtual


Plan::Sql::Query*
GroupingImpl::Normal::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	if (!m_pSpecification->isWordGrouping()) {
		pResult->setGroupBy(m_pSpecification);
	}
	
	for (int i = 0; i < m_pSpecification->getKeySize(); i++ ) {
		m_pSpecification->getKey(i)->getScalar()->retrieveFromCascade(cEnvironment_, pResult);
	}


	FOREACH(m_vecAggregation,
			boost::bind(&Plan::Interface::IScalar::retrieveFromCascade,
						_1,
						boost::ref(cEnvironment_),
						pResult));
	
	
	return pResult;
}




// FUNCTION private
//	Candidate::GroupingImpl::Normal::createCost -- 
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
GroupingImpl::Normal::
createCost(Opt::Environment& cEnvironment_,
		   const Plan::AccessPlan::Source& cPlanSource_,
		   Plan::AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		getOperand()->createCost(cEnvironment_, cPlanSource_);
		cCost_ = getOperand()->getCost();
	}
}

// FUNCTION private
//	Candidate::GroupingImpl::Normal::createRow -- 
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
Plan::Candidate::Row*
GroupingImpl::Normal::
createRow(Opt::Environment& cEnvironment_)
{
	Plan::Candidate::Row* pResult = createKey(cEnvironment_);
	FOREACH(m_vecAggregation,
			boost::bind(&Plan::Candidate::Row::addScalar,
						pResult,
						_1));	

	return pResult;
}

// FUNCTION private
//	Candidate::GroupingImpl::Normal::createKey -- 
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
Plan::Candidate::Row*
GroupingImpl::Normal::
createKey(Opt::Environment& cEnvironment_)
{
	Plan::Candidate::Row* pResult = Plan::Candidate::Row::create(cEnvironment_);
	SIZE n = m_pSpecification->getKeySize();
	for (SIZE i = 0; i < n; ++i) {
		pResult->addScalar(m_pSpecification->getKey(i)->getScalar());
	}
	return pResult;
}


//////////////////////////////////////////////////
//	Plan::Candidate::GroupingImpl::Simple


// FUNCTION public
//	Candidate::GroupingImpl::Simple::adopt -- 
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
GroupingImpl::Simple::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	Execution::Interface::IIterator* pResult = 0;
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);


	Plan::Candidate::Row* pOperandRow =
		cArgument_.m_pQuery->createRow(cEnvironment_);
	
	// operandをgenerateする(現状ではAvg(Fulltext_length())の場合のみ子サーバーで、
	// sumできないため、分散マネージャー上でgenerateが必要
	//	ただし、group byとの併用はNotSupportなので、generateのみとする.
	// Plan::Utility::ScalarSet cAddRow(m_cAggregationOperand);	
	m_cAggregationOperand.foreachElement(boost::bind(&Plan::Candidate::Row::addScalar,
													 pOperandRow,
													 _1));
	if (cArgument_.m_bConcatinateIterable) {
		pResult = Execution::Iterator::Filter::Distribute::create(cProgram_);
		ModVector<Schema::Cascade*> vecSchemaCascade =
			cEnvironment_.getDatabase()->getCascade(cEnvironment_.getTransaction());
		ModVector<Schema::Cascade*>::ConstIterator ite = vecSchemaCascade.begin();
		for (;ite != vecSchemaCascade.end(); ++ite) {
			cMyArgument.m_pCascade = *ite;
			adoptOperand(cEnvironment_, cProgram_, cMyArgument, pResult, pOperandRow);
		}
		
	} else {
		// collection on memory with grouping
		Execution::Collection::Grouping* pGrouping =
			Execution::Collection::Grouping::create(cProgram_,
													VECTOR<int>(),
													true);

		// main iterator is filtering iterator
		pResult = Execution::Iterator::Filter::create(cProgram_,
													  pGrouping->getID());
		adoptOperand(cEnvironment_, cProgram_, cMyArgument, pResult, pOperandRow);
	}

	// from here, use adoptargument passed from parent candidate
	// add candidate for possible subquery
	cArgument_.setCandidate(this);
	int iGetDataID = pOperandRow->generateFromType(cEnvironment_,
												   cProgram_,
												   pResult,
												   cArgument_);
	
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iGetDataID));

	// gerenate aggretations
	Plan::Utility::ScalarSet::Iterator ite = m_vecAggregation.begin();
	for (; ite != m_vecAggregation.end(); ++ite ) {
		(*ite)->generate(cEnvironment_, cProgram_, pResult, cArgument_);
	}
	
	// add cancel poling
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(CheckCancel,
										   cArgument_.m_eTarget));

	return pResult;
}



// FUNCTION public
//	Candidate::GroupingImpl::Simple::generateSQL
//
//
// NOTES
//	
//
// ARGUMENTS
//	Opt::Environment cEnvironment_
//
//	
// RETURN
//	Sql::Query*
//
// EXCEPTIONS

//virtual


Plan::Sql::Query*
GroupingImpl::Simple::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	return pResult;
}

void
GroupingImpl::Simple::
adoptOperand(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Plan::Candidate::AdoptArgument& cArgument_,
			 Execution::Interface::IIterator* pIterator_,
			 Plan::Candidate::Row* pRow_)
{

	Execution::Interface::IIterator* pOperandIterator =
		getOperand()->adopt(cEnvironment_, cProgram_, cArgument_);

	int iPutDataID = pRow_->generate(cEnvironment_,
									 cProgram_,
									 pOperandIterator,
									 cArgument_);



	// filter input is operand result
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT(Input,
										   pOperandIterator->getID(),
										   iPutDataID));
}

// FUNCTION private
//	Candidate::GroupingImpl::Simple::createCost -- 
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
GroupingImpl::Simple::
createCost(Opt::Environment& cEnvironment_,
		   const Plan::AccessPlan::Source& cPlanSource_,
		   Plan::AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		getOperand()->createCost(cEnvironment_, cPlanSource_);
		cCost_ = getOperand()->getCost();
	}
}

// FUNCTION private
//	Candidate::GroupingImpl::Simple::createRow -- 
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
Plan::Candidate::Row*
GroupingImpl::Simple::
createRow(Opt::Environment& cEnvironment_)
{
	Plan::Candidate::Row* pResult = createKey(cEnvironment_);
	FOREACH(m_vecAggregation,
			boost::bind(&Plan::Candidate::Row::addScalar,
						pResult,
						_1));	

	return pResult;
}

// FUNCTION private
//	Candidate::GroupingImpl::Simple::createKey -- 
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
Plan::Candidate::Row*
GroupingImpl::Simple::
createKey(Opt::Environment& cEnvironment_)
{
	return Plan::Candidate::Row::create(cEnvironment_);
}


//////////////////////////////////////////////////
//	Plan::Candidate::GroupingImpl::Replicate


// FUNCTION public
//	Candidate::GroupingImpl::Simple::adopt -- 
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
GroupingImpl::Replicate::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	// Replicationの場合は、子サーバーでの集約結果をそのまま返す.
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	Execution::Interface::IIterator* pOperandIterator =
		getOperand()->adopt(cEnvironment_, cProgram_, cMyArgument);

	
	return pOperandIterator;
}

// FUNCTION public
//	Candidate::GroupingImpl::Normal::generateSQL
//
//
// NOTES
//	
//
// ARGUMENTS
//	Opt::Environment cEnvironment_
//
//	
// RETURN
//	Sql::Query*
//
// EXCEPTIONS

//virtual


Plan::Sql::Query*
GroupingImpl::Replicate::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Plan::Order::Specification* pSpecification = getSpecification();
	if (!pSpecification->isWordGrouping()) {
		pResult->setGroupBy(pSpecification);
	}
	
	for (int i = 0; i < pSpecification->getKeySize(); i++ ) {
		pSpecification->getKey(i)->getScalar()->retrieveFromCascade(cEnvironment_, pResult);
	}

	pResult->setHaving(getPredicate());	
	return pResult;
}

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
