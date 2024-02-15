// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/UnionImpl.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Plan/Candidate/Impl/UnionImpl.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Predicate/Argument.h"
#include "Plan/Predicate/CheckedInterface.h"
#include "Plan/Predicate/Combinator.h"
#include "Plan/Relation/Union.h"
#include "Plan/Utility/ObjectSet.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Execution/Collection/Distinct.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/CascadeInput.h"
#include "Execution/Iterator/MergeSort.h"
#include "Execution/Predicate/CollectionCheck.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

namespace
{
	// cost calculator
	class _CostCalculator
	{
	public:
		_CostCalculator(Opt::Environment& cEnvironment_,
						const AccessPlan::Source& cPlanSource_,
						AccessPlan::Cost* pCost_)
			: m_cEnvironment(cEnvironment_),
			  m_cPlanSource(cPlanSource_),
			  m_pCost(pCost_)
		{}

		void operator()(Interface::ICandidate* pCandidate_)
		{
			if (m_pCost) {
				pCandidate_->createCost(m_cEnvironment,
										m_cPlanSource);
				const AccessPlan::Cost& cMyCost = pCandidate_->getCost();

				if (m_pCost->isInfinity()) {
					*m_pCost = cMyCost;

				} else {
					AccessPlan::Cost::Value cOverhead = m_pCost->getOverhead() + cMyCost.getOverhead();
					AccessPlan::Cost::Value cTotalCost = m_pCost->getTotalCost() + cMyCost.getTotalCost();
					AccessPlan::Cost::Value cTupleCount = m_pCost->getTupleCount() + cMyCost.getTupleCount();
					m_pCost->setOverhead(cOverhead);
					m_pCost->setTotalCost(cTotalCost);
					m_pCost->setTupleCount(cTupleCount);

					AccessPlan::Cost::Value cEstimateLimit =
						m_cPlanSource.getEstimateLimit() - cMyCost.getTupleCount();
					if (cEstimateLimit <= 0) {
						// no more calculation
						m_pCost = 0;
					} else {
						m_cPlanSource.setEstimateLimit(cEstimateLimit);
					}
				}
			}
		}
	protected:
	private:
		Opt::Environment& m_cEnvironment;
		AccessPlan::Source m_cPlanSource;

		AccessPlan::Cost* m_pCost;
	};

	class _Inquirer
	{
	public:
		_Inquirer(Opt::Environment& cEnvironment_,
				  const InquiryArgument& cArgument_)
			: m_cEnvironment(cEnvironment_),
			  m_cArgument(cArgument_),
			  m_bReferTable(cArgument_.m_iTarget & InquiryArgument::Target::ReferTable),
			  m_iResult(0)
		{}

		void operator()(Interface::ICandidate* pCandidate_);

		Interface::ICandidate::InquiryResult getResult()
		{return m_iResult;}

	protected:
	private:
		Opt::Environment& m_cEnvironment;
		const InquiryArgument& m_cArgument;
		bool m_bReferTable;
		Interface::IRelation::InquiryResult m_iResult;
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
//	Interface::ICandidate* pCandidate_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_Inquirer::
operator()(Interface::ICandidate* pCandidate_)
{
	if (m_bReferTable) {
		Interface::ICandidate::InquiryResult iTmp =
			pCandidate_->inquiry(m_cEnvironment, m_cArgument);
		// if any operand matches inquiry, it succeed
		m_iResult |= iTmp;
		if (iTmp & InquiryArgument::Target::ReferTable) {
			// no more check
			m_bReferTable = false;
		}
	}
}

//////////////////////////////////////////
// Plan::Candidate::UnionImpl::Base

// FUNCTION public
//	Candidate::UnionImpl::Base::delay -- add retrieved columns as delayable
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
UnionImpl::Base::
delay(Opt::Environment& cEnvironment_,
	  Scalar::Field* pField_,
	  Scalar::DelayArgument& cArgument_)
{
	Scalar::DelayArgument cTmpArgument(cArgument_);
	if (isAll(boost::bind(&Interface::ICandidate::delay,
						  _1,
						  boost::ref(cEnvironment_),
						  pField_,
						  boost::ref(cTmpArgument)))) {
		// all operands can delay this field
		cArgument_.m_cKey = cTmpArgument.m_cKey;
		return true;
	}
	// any operand cannot delay this field -> set this field as required
	require(cEnvironment_,
			pField_);
	return false;
}

// FUNCTION public
//	Candidate::UnionImpl::Base::inquiry -- 
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
UnionImpl::Base::
inquiry(Opt::Environment& cEnvironment_,
		const InquiryArgument& cArgument_)
{
	return foreachOperand(_Inquirer(cEnvironment_, cArgument_)).getResult();
}

// FUNCTION private
//	Candidate::UnionImpl::Base::createCost -- 
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
UnionImpl::Base::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		// not calculated yet
		foreachOperand(_CostCalculator(cEnvironment_,
									   cPlanSource_,
									   &cCost_));
	}
}

// FUNCTION private
//	Candidate::UnionImpl::Base::createRow -- merge operand row
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
UnionImpl::Base::
createRow(Opt::Environment& cEnvironment_)
{
	return getOperandi(0)->getRow(cEnvironment_);
}

// FUNCTION private
//	Candidate::UnionImpl::Base::createKey -- 
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
UnionImpl::Base::
createKey(Opt::Environment& cEnvironment_)
{
	return getOperandi(0)->getKey(cEnvironment_);
}

//////////////////////////////////////////
// Plan::Candidate::UnionImpl::Cascade

// FUNCTION public
//	Candidate::UnionImpl::Cascade::adopt -- 
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
UnionImpl::Cascade::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::CascadeInput::create(cProgram_);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	// distinct collection to check distinct
	Execution::Collection::Distinct* pDistinct = 0;
	if (getUnion()->isAll()) {
		// no check
	} else {
		pDistinct = Execution::Collection::Distinct::create(cProgram_);
	}

	// get non-delayable subset of result row
	Candidate::Row* pRow = getRow(cEnvironment_);
	if (getUnion()->isDistinct() == false) {
		pRow = pRow->delay(cEnvironment_,
						   this);
	}

	// result data
	int iResultID = pRow->generateFromType(cEnvironment_,
										   cProgram_,
										   pResult,
										   cArgument_);
	// set outdata
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iResultID));

	Candidate::AdoptArgument cMyArgument(cArgument_);

	foreachOperand(boost::bind(&This::addOperand,
							   this,
							   boost::ref(cEnvironment_),
							   boost::ref(cProgram_),
							   boost::ref(cMyArgument),
							   _1,
							   pRow,
							   iResultID,
							   pResult,
							   pDistinct));

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cMyArgument, getPredicate());
	}

	cArgument_.setCandidate(this);

	return pResult;
}

// FUNCTION public
//	Candidate::UnionImpl::Cascade::generateDelayed -- generate additional action to obtain delayed scalars
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
UnionImpl::Cascade::
generateDelayed(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_)
{
	// use first operand
	getOperandi(0)->generateDelayed(cEnvironment_,
									cProgram_,
									pIterator_);
}

// FUNCTION private
//	Candidate::UnionImpl::Cascade::addOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Interface::ICandidate* pOperand_
//	Candidate::Row* pRow_
//	int iResultID_
//	Execution::Interface::IIterator* pIterator_
//	Execution::Interface::ICollection* pDistinct_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UnionImpl::Cascade::
addOperand(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Interface::ICandidate* pOperand_,
		   Candidate::Row* pRow_,
		   int iResultID_,
		   Execution::Interface::IIterator* pIterator_,
		   Execution::Interface::ICollection* pDistinct_)
{
	// set require for operand
	pRow_->foreachElement(boost::bind(&Interface::IScalar::require,
									  _1,
									  boost::ref(cEnvironment_),
									  pOperand_));
	// adopt operand
	Execution::Interface::IIterator* pOperandIterator =
		pOperand_->adopt(cEnvironment_, cProgram_, cArgument_);

	int iDataID = pRow_->generate(cEnvironment_,
								  cProgram_,
								  pOperandIterator,
								  cArgument_);
	if (pDistinct_) {
		// add check collection predicate
		int iKeyID = -1;
		if (getUnion()->isDistinctByKey()) {
			// use relation key to take distinct
			iKeyID = pOperand_->getKey(cEnvironment_)->generate(cEnvironment_,
																cProgram_,
																pOperandIterator,
																cArgument_);
		} else {
			// use value itself
			iKeyID = iDataID;
		}
		pOperandIterator->addPredicate(cProgram_,
									   Execution::Predicate::CollectionCheck::create(
												 cProgram_,
												 pOperandIterator,
												 pDistinct_->getID(),
												 iKeyID),
									   cArgument_.m_eTarget);
	}
	// add operand as input
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT2(Input,
											pOperandIterator->getID(),
											iDataID));

}



//////////////////////////////////////////
// Plan::Candidate::UnionImpl::Sort

// FUNCTION public
//	Candidate::UnionImpl::Sort::adopt -- 
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
UnionImpl::Sort::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	Execution::Iterator::MergeSort* pResult =
		Execution::Iterator::MergeSort::create(cProgram_);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput);
	}

	// distinct collection to check distinct
	Execution::Collection::Distinct* pDistinct = 0;
	if (getUnion()->isAll()) {
		// no check
	} else {
		pDistinct = Execution::Collection::Distinct::create(cProgram_);
	}

	// get non-delayable subset of result row
	Candidate::Row* pRow = getRow(cEnvironment_);

	if (getUnion()->isDistinct() == false) {
		pRow = pRow->delay(cEnvironment_,
						   this);
	}


	// result data
	int iResultID = pRow->generateFromType(cEnvironment_,
										   cProgram_,
										   pResult,
										   cArgument_);

	Candidate::AdoptArgument cMyArgument(cArgument_);

	foreachOperand(boost::bind(&This::addOperand,
							   this,
							   boost::ref(cEnvironment_),
							   boost::ref(cProgram_),
							   boost::ref(cMyArgument),
							   _1,
							   pRow,
							   iResultID,
							   pResult,
							   pDistinct));
	
	// set outdata
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iResultID));
	if (m_pOrder) {
		RowDelayArgument cDelayArgument;
		pRow->delay(cEnvironment_,
					this,
					cDelayArgument);
		
		Order::GeneratedSpecification* pGenerated =
			m_pOrder->generate(cEnvironment_,
							   cProgram_,
							   pResult,
							   cArgument_,
							   pRow,
							   cDelayArgument);
		
		pResult->setSortParameter(pGenerated->getPosition(), pGenerated->getDirection());
	}
	

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cMyArgument, getPredicate());
	}

	cArgument_.setCandidate(this);

	return pResult;
}

// FUNCTION public
//	Candidate::UnionImpl::Sort::generateDelayed -- generate additional action to obtain delayed scalars
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
UnionImpl::Sort::
generateDelayed(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Execution::Interface::IIterator* pIterator_)
{
	// use first operand
	getOperandi(0)->generateDelayed(cEnvironment_,
									cProgram_,
									pIterator_);
}

// FUNCTION private
//	Candidate::UnionImpl::Sort::addOperand -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Candidate::AdoptArgument& cArgument_
//	Interface::ICandidate* pOperand_
//	Candidate::Row* pRow_
//	int iResultID_
//	Execution::Interface::IIterator* pIterator_
//	Execution::Interface::ICollection* pDistinct_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
UnionImpl::Sort::
addOperand(Opt::Environment& cEnvironment_,
		   Execution::Interface::IProgram& cProgram_,
		   Candidate::AdoptArgument& cArgument_,
		   Interface::ICandidate* pOperand_,
		   Candidate::Row* pRow_,
		   int iResultID_,
		   Execution::Interface::IIterator* pIterator_,
		   Execution::Interface::ICollection* pDistinct_)
{
	// set require for operand
	pRow_->foreachElement(boost::bind(&Interface::IScalar::require,
									  _1,
									  boost::ref(cEnvironment_),
									  pOperand_));
	// adopt operand
	Execution::Interface::IIterator* pOperandIterator =
		pOperand_->adopt(cEnvironment_, cProgram_, cArgument_);

	int iDataID = pRow_->generate(cEnvironment_,
								  cProgram_,
								  pOperandIterator,
								  cArgument_);
	if (pDistinct_) {
		// add check collection predicate
		int iKeyID = -1;
		if (getUnion()->isDistinctByKey()) {
			// use relation key to take distinct
			iKeyID = pOperand_->getKey(cEnvironment_)->generate(cEnvironment_,
																cProgram_,
																pOperandIterator,
																cArgument_);
		} else {
			// use value itself
			iKeyID = iDataID;
		}
		
		pOperandIterator->addPredicate(cProgram_,
									   Execution::Predicate::CollectionCheck::create(
												 cProgram_,
												 pOperandIterator,
												 pDistinct_->getID(),
												 iKeyID),
									   cArgument_.m_eTarget);
	}
	
	// add operand as input
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT2(Input,
											pOperandIterator->getID(),
											iDataID));
}



//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
