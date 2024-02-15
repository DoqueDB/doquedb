// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Grouping.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Grouping.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"
#include "Plan/Candidate/Table.h"

#include "Plan/AccessPlan/Cost.h"
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

#include "Execution/Action/Argument.h"
#include "Execution/Collection/Grouping.h"
#include "Execution/Collection/BitsetDisintegration.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Filter.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Grouping

// FUNCTION public
//	Candidate::Grouping::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSpecification_
//	const VECTOR<Interface::IScalar*>& vecAggregation_
//	const Utility::ScalarSet& cAggregationOperand_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Grouping*
//
// EXCEPTIONS

//static
Grouping*
Grouping::
create(Opt::Environment& cEnvironment_,
	   Order::Specification* pSpecification_,
	   const VECTOR<Interface::IScalar*>& vecAggregation_,
	   const Utility::ScalarSet& cAggregationOperand_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new Grouping(pSpecification_,
											 vecAggregation_,
											 cAggregationOperand_,
											 pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Grouping::adopt -- 
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
Grouping::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	// adopt operand
	Candidate::AdoptArgument cMyArgument(cArgument_);
	


	// count(*)以外の集約関数がある場合は、
	// RowIDをBitSetからunsigned shortに変換する
	if (getOperand()->isGetByBitSetRowID()
		&& (m_vecAggregation.getSize() > 1
			|| (m_vecAggregation.getSize() == 1
				&& m_vecAggregation.getFront()->getName() != "count(*)"))) {
		cMyArgument.m_bForceRowID = true;
	}
	
	Execution::Interface::IIterator* pOperandIterator =
		getOperand()->adopt(cEnvironment_, cProgram_, cMyArgument);

	

	// RowIDがBitSetで返ってくる場合は、
	// Grouping Filterは挟まずに終了
	if (getOperand()->isGetByBitSetRowID()
	   && cMyArgument.m_bForceRowIDSet == false) {
		cArgument_.m_pCandidate = cMyArgument.m_pCandidate;
		cArgument_.m_pTable = cMyArgument.m_pTable;
		return pOperandIterator;
	} else {
		cArgument_.m_bForceRowIDSet = true;
	}

	Order::Specification* pOperandOrder = getOperand()->getOrder();
	Order::GeneratedSpecification* pGenerated = 0;
	if (pOperandOrder == 0 && m_pSpecification->getKeySize() > 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}
	if (pOperandOrder && pOperandOrder->isGenerated()
		&& Order::Specification::isCompatible(pOperandOrder,
											  m_pSpecification)) {
		// operand is sorted by executor
		pGenerated = pOperandOrder->getGenerated();
	} else {
		RowDelayArgument cDelayArgument;
		Row* pOperandRow = getOperand()->getRow(cEnvironment_);
		bool bDelayed = pOperandRow->delay(cEnvironment_,
										   getOperand(),
										   cDelayArgument);

		pGenerated = m_pSpecification->generate(cEnvironment_,
												cProgram_,
												pOperandIterator,
												cMyArgument,
												pOperandRow,
												cDelayArgument);
	}
	; _SYDNEY_ASSERT(pGenerated);
	; _SYDNEY_ASSERT(pGenerated->getDataID().GETSIZE() == pGenerated->getTuple().GETSIZE());

	VECTOR<Interface::IScalar*> vecTuple(pGenerated->getTuple());
	VECTOR<int> vecPutDataID(pGenerated->getDataID());

	// add aggregation operands if not included yet
	Utility::ScalarSet cAddRow(m_cAggregationOperand);
	cAddRow.remove(pGenerated->getTuple());
	if (cAddRow.isEmpty() == false) {
		vecTuple.insert(vecTuple.end(), cAddRow.begin(), cAddRow.end());
		cAddRow.mapElement(vecPutDataID,
						   boost::bind(&Interface::IScalar::generate,
									   _1,
									   boost::ref(cEnvironment_),
									   boost::ref(cProgram_),
									   pOperandIterator,
									   boost::ref(cMyArgument)));
	}
	; _SYDNEY_ASSERT(vecTuple.GETSIZE() == vecPutDataID.GETSIZE());

	int iPutDataID = cProgram_.addVariable(vecPutDataID);
	
	// collection on memory with grouping
	Execution::Collection::Grouping* pGrouping =
		Execution::Collection::Grouping::create(cProgram_,
												pGenerated->getPosition(),
												false);

	///////////////////
	///////////////////

	// main iterator is filtering iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Filter::create(cProgram_,
											pGrouping->getID());

	// filter input is operand result
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pOperandIterator->getID(),
										iPutDataID));

	int iGetDataID = -1;
	if (pGenerated->getPosition().ISEMPTY()) {
		// not grouping -> use same data to get
		Opt::ForEach(vecTuple,
				 boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
							 pResult,
							 pOperandIterator,
							 boost::bind(&Interface::IScalar::getID,
										 _1),
							 true /* collection */));
		iGetDataID = iPutDataID;

	} else {
		// create variable to obtain filtered data
		VECTOR<int> vecGetDataID;
		Opt::MapContainer(vecTuple,
						  vecGetDataID,
						  boost::bind(&Interface::IScalar::generateFromType,
									  _1,
									  boost::ref(cEnvironment_),
									  boost::ref(cProgram_),
									  pResult,
									  boost::ref(cArgument_)));
		iGetDataID = cProgram_.addVariable(vecGetDataID);
	}

	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iGetDataID));
	
	// from here, use adoptargument passed from parent candidate
	// add candidate for possible subquery
	cArgument_.setCandidate(this);

	// gerenate aggretations
	FOREACH(m_vecAggregation,
			boost::bind(&Interface::IScalar::generate,
						_1,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						pResult,
						boost::ref(cArgument_)));

	// add cancel poling
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(CheckCancel,
										   cArgument_.m_eTarget));

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);

		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	return pResult;
}

// FUNCTION private
//	Candidate::Grouping::createCost -- 
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
Grouping::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		getOperand()->createCost(cEnvironment_, cPlanSource_);
		cCost_ = getOperand()->getCost();
	}
}

// FUNCTION private
//	Candidate::Grouping::createRow -- 
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
Grouping::
createRow(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pResult = createKey(cEnvironment_);
	FOREACH(m_vecAggregation,
			boost::bind(&Candidate::Row::addScalar,
						pResult,
						_1));
	return pResult;
}

// FUNCTION private
//	Candidate::Grouping::createKey -- 
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
Grouping::
createKey(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pResult = Candidate::Row::create(cEnvironment_);
	SIZE n = m_pSpecification->getKeySize();
	for (SIZE i = 0; i < n; ++i) {
		pResult->addScalar(m_pSpecification->getKey(i)->getScalar());
	}
	return pResult;
}

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
