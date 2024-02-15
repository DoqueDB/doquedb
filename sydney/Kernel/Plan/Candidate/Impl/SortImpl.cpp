// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/SortImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Impl/SortImpl.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Relation/Sort.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Collection/Sort.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Filter.h"
#include "Execution/Operator/Limit.h"
#include "Execution/Operator/Output.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::SortImpl::Base::

// FUNCTION public
//	Candidate::SortImpl::Base::adopt -- 
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
SortImpl::Base::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	RowDelayArgument cDelayArgument;
	// unused operand data is delayed if delayable
	bool bDelayed = getRow(cEnvironment_)->delay(cEnvironment_,
												 getOperand(),
												 cDelayArgument);

	bool bCollectingSave = cArgument_.m_bCollecting;
	cArgument_.m_bCollecting = true;

	// adopt operand candidate
	Execution::Interface::IIterator* pOperandIterator =
		adoptOperand(cEnvironment_,
					 cProgram_,
					 cArgument_);

	// generate sorting keys
	Order::GeneratedSpecification* pGenerated =
		getOrder()->generate(cEnvironment_,
							 cProgram_,
							 pOperandIterator,
							 cArgument_,
							 getRow(cEnvironment_),
							 cDelayArgument);
	setOrder(pGenerated);

	// collection on memory with sorting
	Execution::Collection::Sort* pSort =
		Execution::Collection::Sort::create(cProgram_,
											pGenerated->getPosition(),
											pGenerated->getDirection(),
											pGenerated->getWordPosition());

	////////////////////
	////////////////////

	// main iterator for sorting is filter iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Filter::create(cProgram_,
											pSort->getID());

	int iDataID = cProgram_.addVariable(pGenerated->getDataID());

	// input from operand iterator
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pOperandIterator->getID(),
										iDataID));

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0(CheckCancel));

	// Input uses same data set as operanditerator
	Opt::ForEach(pGenerated->getTuple(),
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pOperandIterator,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 true /* collection */));

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iDataID));

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);

		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	cArgument_.m_bCollecting = bCollectingSave;

	return pResult;
}

//////////////////////////////////////////////////
//	Plan::Candidate::SortImpl::Normal::

// FUNCTION private
//	Candidate::SortImpl::Normal::adoptOperand -- 
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
SortImpl::Normal::
adoptOperand(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Candidate::AdoptArgument& cArgument_)
{
	return getOperand()->adopt(cEnvironment_, cProgram_, cArgument_);
}

// FUNCTION private
//	Candidate::SortImpl::Normal::createCost -- 
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
SortImpl::Normal::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		getOperand()->createCost(cEnvironment_, cPlanSource_);
		cCost_ = getOperand()->getCost();
		cCost_.addSortingCost();
	}
}

//////////////////////////////////////////////////
//	Plan::Candidate::SortImpl::Partial::

// FUNCTION private
//	Candidate::SortImpl::Partial::adopt -- 
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
SortImpl::Partial::
adoptOperand(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Candidate::AdoptArgument& cArgument_)
{
	// adopt operand candidate
	Execution::Interface::IIterator* pResult =
		getOperand()->adopt(cEnvironment_, cProgram_, cArgument_);

	// operand sorting order
	Order::Specification* pOperandOrder = getOperand()->getOrder();
	; _SYDNEY_ASSERT(pOperandOrder);

	int iPartialKeyID = pOperandOrder->generateKey(cEnvironment_,
												   cProgram_,
												   pResult,
												   cArgument_);

	PAIR<int, int> cLimitPair = m_cLimit.generate(cEnvironment_,
												  cProgram_,
												  pResult,
												  cArgument_);
	pResult->addCalculation(cProgram_,
							Execution::Operator::Limit::Partial::create(
										   cProgram_,
										   pResult,
										   pResult->getID(),
										   cLimitPair,
										   iPartialKeyID),
							cArgument_.m_eTarget);

	return pResult;
}

// FUNCTION private
//	Candidate::SortImpl::Partial::createCost -- 
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
SortImpl::Partial::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		getOperand()->createCost(cEnvironment_, cPlanSource_);
		cCost_ = getOperand()->getCost();
		AccessPlan::Cost::Value cEstimateLimit = cPlanSource_.getEstimateLimit();
		if (cEstimateLimit.isInfinity()) {
			m_cLimit.estimateCount(cEnvironment_,
								   cEstimateLimit);
		}
		cCost_.setLimitCount(cEstimateLimit);
		cCost_.addSortingCost();
	}
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
