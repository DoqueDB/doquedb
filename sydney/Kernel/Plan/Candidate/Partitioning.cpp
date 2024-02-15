// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Partitioning.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#include "Plan/Candidate/Partitioning.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Collection/Partitioning.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Iterator/Filter.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Partitioning

// FUNCTION public
//	Candidate::Partitioning::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const VECTOR<Interface::IScalar*>& vecPartitionKey_
//	const AccessPlan::Limit& cLimit_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Partitioning*
//
// EXCEPTIONS

//static
Partitioning*
Partitioning::
create(Opt::Environment& cEnvironment_,
	   const VECTOR<Interface::IScalar*>& vecPartitionKey_,
	   const AccessPlan::Limit& cLimit_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new Partitioning(vecPartitionKey_,
												 cLimit_,
												 pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Partitioning::adopt -- 
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
Partitioning::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	RowDelayArgument cDelayArgument;
	// unused operand data is delayed if delayable
	bool bDelayed = getRow(cEnvironment_)->delay(cEnvironment_,
												 getOperand(),
												 cDelayArgument);

	Candidate::AdoptArgument cAdoptArgument(cArgument_);
	cAdoptArgument.m_bCollecting = true;

	// adopt operand
	Execution::Interface::IIterator* pOperandIterator =
		getOperand()->adopt(cEnvironment_, cProgram_, cAdoptArgument);

	// generate limit
	PAIR<int, int> cLimitPair(-1, -1);
	if (m_cLimit.isSpecified()) {
		cLimitPair = m_cLimit.generate(cEnvironment_,
									   cProgram_,
									   pOperandIterator,
									   cArgument_);
	}
	// generate partition key
	VECTOR<int> vecKeyID;
	Opt::MapContainer(m_vecPartitionKey,
					  vecKeyID,
					  boost::bind(&Interface::IScalar::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pOperandIterator,
								  boost::ref(cArgument_)));

	int iKeyID = cProgram_.addVariable(vecKeyID);

	// collection on memory with partitioning
	Execution::Collection::Partitioning* pPartitioning =
		Execution::Collection::Partitioning::create(cProgram_,
													iKeyID,
													cLimitPair);

	///////////////////
	///////////////////

	// main iterator is filtering iterator
	Execution::Interface::IIterator* pResult =
		Execution::Iterator::Filter::create(cProgram_,
											pPartitioning->getID());

	// create variable to put data
 	Utility::ScalarSet cTuple;
 	cTuple.merge(cDelayArgument.m_cKey);
 	cTuple.merge(cDelayArgument.m_cNotDelayed);

	VECTOR<int> vecDataID;
	cTuple.mapElement(vecDataID,
					  boost::bind(&Interface::IScalar::generate,
								  _1,
								  boost::ref(cEnvironment_),
								  boost::ref(cProgram_),
								  pOperandIterator,
								  boost::ref(cArgument_)));

	int iDataID = cProgram_.addVariable(vecDataID);

	// filter input is operand result
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT(Input,
										pOperandIterator->getID(),
										iDataID));

	// Input uses same data set as operanditerator
	cTuple.foreachElement(boost::bind(&Execution::Interface::IIterator::copyNodeVariable,
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

	return pResult;
}

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
