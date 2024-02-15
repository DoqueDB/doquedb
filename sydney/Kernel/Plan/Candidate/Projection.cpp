// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Projection.cpp --
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
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Projection.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/Interface/IPredicate.h"
#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Utility/Algorithm.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Projection

// FUNCTION public
//	Candidate::Projection::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::Projection* pProjection_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Projection*
//
// EXCEPTIONS

//static
Projection*
Projection::
create(Opt::Environment& cEnvironment_,
	   Relation::Projection* pProjection_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new Projection(pProjection_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Projection::adopt -- 
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
Projection::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	RowDelayArgument cDelayArgument;
	// unused operand data is delayed if delayable
	bool bDelayed = getOperand()->getRow(cEnvironment_)->delay(cEnvironment_,
															   getOperand(),
															   cDelayArgument);
	// adopt operand -> used as result
	Execution::Interface::IIterator* pResult =
		getOperand()->adopt(cEnvironment_, cProgram_, cArgument_);

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	return pResult;
}

// FUNCTION public
//	Candidate::Projection::isLimitAvailable -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Projection::
isLimitAvailable(Opt::Environment& cEnvironment_)
{
	return getOperand()->isLimitAvailable(cEnvironment_);
}

// FUNCTION private
//	Candidate::Projection::createRow -- 
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
Projection::
createRow(Opt::Environment& cEnvironment_)
{
	Candidate::Row* pResult = Row::create(cEnvironment_);

	Utility::ScalarSet cRow;
	Candidate::Row* pOperandRow = getOperand()->getRow(cEnvironment_);
	cRow.add(pOperandRow->begin(),
			 pOperandRow->end());

	Relation::RowInfo* pRowInfo = m_pProjection->getRowInfo(cEnvironment_);
	pRowInfo->foreachElement(boost::bind(&Utility::ScalarSet::addObject,
										 &cRow,
										 boost::bind(&Relation::RowElement::getScalar,
													 _1,
													 boost::ref(cEnvironment_))));
	cRow.foreachElement(boost::bind(&Candidate::Row::addScalar,
									pResult,
									_1));
	return pResult;
}

//
// Copyright (c) 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
