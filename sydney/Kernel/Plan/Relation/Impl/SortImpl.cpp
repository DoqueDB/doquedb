// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Sort.cpp --
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Plan/Relation/Impl/SortImpl.h"
#include "Plan/Relation/RowInfo.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Partitioning.h"
#include "Plan/Candidate/Sort.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/ChosenSpecification.h"
#include "Plan/Scalar/Field.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

/////////////////////////////////////
// Relation::Impl::SortImpl

// FUNCTION public
//	Relation::Impl::SortImpl::createAccessPlan -- create access plan candidate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//virtual
Interface::ICandidate*
Impl::SortImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 AccessPlan::Source& cPlanSource_)
{
	Order::Specification* pUsedSpecification =
		Order::Specification::getCompatible(cEnvironment_,
											cPlanSource_.getOrder(),
											m_pSpecification);
	
	if (pUsedSpecification == 0) {
		// if passed order is not compatible to this order, use this order
		pUsedSpecification = m_pSpecification;
	}

	Interface::ICandidate* pResult =
		createSortAccessPlan(cEnvironment_,
							 getOperand(),
							 cPlanSource_,
							 pUsedSpecification);

	return pResult;
}

// FUNCTION public
//	Relation::Impl::SortImpl::require -- 
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
Impl::SortImpl::
require(Opt::Environment& cEnvironment_,
		Interface::ICandidate* pCandidate_)
{
	m_pSpecification->require(cEnvironment_,
							  pCandidate_);
	getOperand()->require(cEnvironment_,
						  pCandidate_);
}

// FUNCTION public
//	Relation::Impl::SortImpl::createSortAccessPlan -- may use same algorithm in other relations
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::IRelation* pRelation_
//	AccessPlan::Source& cPlanSource_
//	Order::Specification* pSpecification_
//	
// RETURN
//	Interface::ICandidate*
//
// EXCEPTIONS

//static
Interface::ICandidate*
Impl::SortImpl::
createSortAccessPlan(Opt::Environment& cEnvironment_,
					 Interface::IRelation* pRelation_,
					 AccessPlan::Source& cPlanSource_,
					 Order::Specification* pSpecification_)
{
	if (pSpecification_ == 0) {
		return pRelation_->createAccessPlan(cEnvironment_,
											cPlanSource_);
	}

	// step1: create operand's candidate using the sorting order
	AccessPlan::Source cSource1(cPlanSource_, pSpecification_);
	cSource1.erasePredicate();

	Interface::ICandidate* pCandidate1 =
		pRelation_->createAccessPlan(cEnvironment_, cSource1);

	Interface::ICandidate* pResult = 0;
	bool bIsPartial = false;
	bool bNeedPartition = (pSpecification_->getPartitionKey().ISEMPTY() == false);

	if (Order::Specification::isCompatible(pCandidate1->getOrder(),
										   pSpecification_)) {
		pCandidate1->createCost(cEnvironment_, cSource1);
		AccessPlan::Cost cCost1(pCandidate1->getCost());

		// operand's order can be replaced to this order (including partial case)
		; _SYDNEY_ASSERT(pCandidate1->getOrder());

		// check no sorting case

		bool bTakeSortedPlan = false;

		if (pCandidate1->getOrder()->isChosen()
			&& pCandidate1->getOrder()->getChosen()->isPartial()) {
			// partial sorting
			; _SYDNEY_ASSERT(cSource1.getLimit().isSpecified());
			; _SYDNEY_ASSERT(cSource1.getEstimateLimit().isInfinity() == false);

			bIsPartial = true;
			cCost1.setLimitCount(cSource1.getEstimateLimit());
			cCost1.addSortingCost();
		}
		if (bNeedPartition
			&& Order::Specification::hasSamePartitionKey(pCandidate1->getOrder(),
														 pSpecification_) == false) {
			// if paritionkey is specified and operand cannot process the partition,
			// limit cannot be passed to operand
			cCost1.setLimitCount(AccessPlan::Cost::Value());
		}

		if (pCandidate1->getOrder()->hasAlternativeValue(cEnvironment_)
			|| pCandidate1->getOrder()->hasExpandElement()
			|| pCandidate1->getOrder()->isBitSetSort()) {
			// order key includes alternative values
			// OR order key includes expand element
			// OR order is processed for bitset sort
			// -> always use index file
			bTakeSortedPlan = true;

		} else {
			// step2: create operand's candidate using no order
			AccessPlan::Source cSource2(cPlanSource_);
			cSource2.erasePredicate();
			cSource2.eraseOrder();
			cSource2.eraseLimit();
			cSource2.eraseEstimateLimit();

			Interface::ICandidate* pCandidate2 =
				pRelation_->createAccessPlan(cEnvironment_, cSource2);

			// step3: compare costs
			pCandidate2->createCost(cEnvironment_, cSource2);
			AccessPlan::Cost cCost2(pCandidate2->getCost());

#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
				OSTRSTREAM stream;
				stream << "sort by collection(retrieve):" << cCost2;
				_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
			}
#endif

			cCost2.addSortingCost();

#ifndef NO_TRACE
			if (_OPT_IS_OUTPUT_OPTIMIZATION_PROCESS(Opt::Configuration::TraceLevel::Cost)) {
				OSTRSTREAM stream;
				stream << "Sort cost:\n"
					   << "sort by file:      " << cCost1 << "\n"
					   << "sort by collection:" << cCost2;
				_OPT_OPTIMIZATION_MESSAGE << stream.getString() << ModEndl;
			}
#endif

			if (cCost1 <= cCost2) {
				// getting sorted results is faster than sorting
				// -> use operand result as the result
				Interface::ICandidate::erase(cEnvironment_,
											 pCandidate2);
				bTakeSortedPlan = true;
			} else {
				// use sorting candidate
				// -> abandon the 1st candidate
				Interface::ICandidate::erase(cEnvironment_,
											 pCandidate1);
				bIsPartial = false;
				// use the second candidate instead
				pCandidate1 = pCandidate2;
				bTakeSortedPlan = false;
			}
		}
		if (bTakeSortedPlan == true
			&& bIsPartial == false) {
			// result is sorted plan
			pResult = pCandidate1;

			// if sorted plan is selected and no other process is needed, check partition key
			if (pSpecification_->getPartitionKey().ISEMPTY() == false
				&& Order::Specification::hasSamePartitionKey(pResult->getOrder(),
															 pSpecification_) == true) {
				// operand processes partitioning
				bNeedPartition = false;
			}
		}
	}

	if (pResult == 0) {
		// set sorting key as required in operand
		pSpecification_->require(cEnvironment_,
								 pCandidate1);
		// create candidate
		if (bIsPartial) {
			pResult = Candidate::Sort::Partial::create(cEnvironment_,
													   pSpecification_,
													   cPlanSource_.getLimit(),
													   pCandidate1);

		} else {
			pResult = Candidate::Sort::create(cEnvironment_,
											  pSpecification_,
											  pCandidate1);
		}
	}

	if (bNeedPartition) {
		// partitioning is needed
		Opt::ForEach(pSpecification_->getPartitionKey(),
					 boost::bind(&Interface::IScalar::require,
								 _1,
								 boost::ref(cEnvironment_),
								 pResult));

		pResult = Candidate::Partitioning::create(cEnvironment_,
												  pSpecification_->getPartitionKey(),
												  cPlanSource_.getLimit(),
												  pResult);
	}

	pResult->checkPredicate(cEnvironment_,
							cPlanSource_);
	return pResult;
}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
