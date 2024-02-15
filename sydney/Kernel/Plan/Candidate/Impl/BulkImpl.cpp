// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Bulk.cpp --
// 
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Impl/BulkImpl.h"
#include "Plan/Candidate/Bulk.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Monadic.h"
#include "Plan/Candidate/Row.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Relation/Bulk.h"
#include "Plan/Relation/RowInfo.h"

#include "Common/Assert.h"

#include "Execution/Collection/Bulk.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Input.h"

#include "LogicalFile/Estimate.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

//////////////////////////////////////////////
// Candidate::BulkImpl::Base

// FUNCTION protected
//	Candidate::BulkImpl::Base::createBulkCollection -- create bulk collection
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	bool bInput_
//	
// RETURN
//	Execution::Interface::ICollection*
//
// EXCEPTIONS

Execution::Interface::ICollection*
BulkImpl::Base::
createBulkCollection(Opt::Environment& cEnvironment_,
					 Execution::Interface::IProgram& cProgram_,
					 Execution::Interface::IIterator* pIterator_,
					 Candidate::AdoptArgument& cArgument_,
					 bool bInput_)
{
	// generate bulk options
	const Relation::Bulk::Argument& cBulkOption = m_pRelation->getBulkOption();
	int iPathID = cBulkOption.m_arg1->generate(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument_);
	int iWithID = -1;
	if (cBulkOption.m_arg2) {
		iWithID = cBulkOption.m_arg2->generate(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument_);
	}
	int iHintID = -1;
	if (cBulkOption.m_arg3) {
		iHintID = cBulkOption.m_arg3->generate(cEnvironment_,
											   cProgram_,
											   pIterator_,
											   cArgument_);
	}
	// get type
	VECTOR<Common::SQLData> vecDataType;
	if (bInput_) {
		Relation::RowInfo* pRowInfo = m_pRelation->getRowInfo(cEnvironment_);
		; _SYDNEY_ASSERT(pRowInfo);

		pRowInfo->mapElement(vecDataType,
							 boost::bind(&Interface::IScalar::getDataType,
										 boost::bind(&Relation::RowElement::getScalar,
													 _1,
													 boost::ref(cEnvironment_))));
	}

	// create bulk collection
	return Execution::Collection::Bulk::create(cProgram_,
											   iPathID,
											   iWithID,
											   iHintID,
											   bInput_,
											   vecDataType);
}

//////////////////////////////////////////////
// Candidate::BulkImpl::Input

// FUNCTION public
//	Candidate::BulkImpl::Input::adopt -- 
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
BulkImpl::Input::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	// create input iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Input::create(cProgram_);

	if (cArgument_.m_pInput) {
		// copy node-variable map from input
		pResult->copyNodeVariable(cArgument_.m_pInput, true /* collection */);
	}

	cArgument_.setCandidate(this);

	// generate bulk collection
	Execution::Interface::ICollection* pBulk =
		createBulkCollection(cEnvironment_,
							 cProgram_,
							 pResult,
							 cArgument_,
							 true /* is input */);

	// create result tuple
	Candidate::Row* pRow = createRow(cEnvironment_);
	int iResultID = pRow->generate(cEnvironment_,
								   cProgram_,
								   pResult,
								   cArgument_);
	// set collection as input
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pBulk->getID(),
										iResultID));

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
//	Candidate::BulkImpl::Input::createCost -- 
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
BulkImpl::Input::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	; // do nothing
}

// FUNCTION private
//	Candidate::BulkImpl::Input::createRow -- 
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
BulkImpl::Input::
createRow(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pResult = Candidate::Row::create(cEnvironment_);
	int n = getRelation()->getDegree(cEnvironment_);
	for (int i = 0; i < n; ++i) {
		pResult->addScalar(getRelation()->getScalar(cEnvironment_,
													i));
	}
	return pResult;
}

// FUNCTION private
//	Candidate::BulkImpl::Input::createKey -- 
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
BulkImpl::Input::
createKey(Opt::Environment& cEnvironment_)
{
	return Candidate::Row::create(cEnvironment_);
}

// FUNCTION private
//	Candidate::BulkImpl::Input::getTupleSize -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

AccessPlan::Cost::Value
BulkImpl::Input::
getTupleSize(Opt::Environment& cEnvironment_)
{
	return getRelation()->getDegree(cEnvironment_) * 8;
}

// FUNCTION private
//	Candidate::BulkImpl::Input::getTupleCount -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
// RETURN
//	AccessPlan::Cost::Value
//
// EXCEPTIONS

//virtual
AccessPlan::Cost::Value
BulkImpl::Input::
getTupleCount(Opt::Environment& cEnvironment_)
{
	return getRelation()->getCardinality(cEnvironment_);
}

//////////////////////////////////////////////
// Candidate::BulkImpl::Output

// FUNCTION public
//	Candidate::BulkImpl::OutPut::adopt -- 
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
BulkImpl::Output::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	RowDelayArgument cDelayArgument;
	// unused operand data is delayed if delayable
	bool bDelayed = getOperand()->getRow(cEnvironment_)->delay(cEnvironment_,
															   getOperand(),
															   cDelayArgument);

	Execution::Interface::IIterator* pResult =
		getOperand()->adopt(cEnvironment_,
							cProgram_,
							cArgument_);

	getOperand()->generateDelayed(cEnvironment_,
								  cProgram_,
								  pResult);

	// generate bulk collection
	Execution::Interface::ICollection* pBulk =
		createBulkCollection(cEnvironment_,
							 cProgram_,
							 pResult,
							 cArgument_,
							 false /* not input */);

	// generate result row
	int iResultID = m_pRowInfo->generate(cEnvironment_,
										 cProgram_,
										 pResult,
										 cArgument_);
	// add output action
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Output,
										pBulk->getID(),
										iResultID));
	return pResult;
}

// FUNCTION private
//	Candidate::BulkImpl::Output::createRow -- 
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
BulkImpl::Output::
createRow(Opt::Environment& cEnvironment_)
{
	return Candidate::Row::create(cEnvironment_);
}

// FUNCTION private
//	Candidate::BulkImpl::Output::createKey -- 
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
BulkImpl::Output::
createKey(Opt::Environment& cEnvironment_)
{
	return Candidate::Row::create(cEnvironment_);
}

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
