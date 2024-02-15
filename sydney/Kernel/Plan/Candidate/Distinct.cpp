// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Distinct.cpp --
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
const char moduleName[] = "Plan::Candidate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Candidate/Distinct.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Row.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"
#include "Plan/Interface/IPredicate.h"
#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/CheckedSpecification.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Relation/Distinct.h"
#include "Plan/Utility/Algorithm.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Collection/Distinct.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Interface/IIterator.h"
#include "Execution/Predicate/CollectionCheck.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_PLAN_USING
_SYDNEY_PLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	Plan::Candidate::Distinct

// FUNCTION public
//	Candidate::Distinct::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::Row* pKey_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	Distinct*
//
// EXCEPTIONS

//static
Distinct*
Distinct::
create(Opt::Environment& cEnvironment_,
	   Candidate::Row* pKey_,
	   Interface::ICandidate* pOperand_)
{
	AUTOPOINTER<This> pResult = new Distinct(pKey_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::Distinct::adopt -- 
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
Distinct::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Candidate::AdoptArgument& cArgument_)
{
	// adopt operand -> used as result
	Execution::Interface::IIterator* pResult =
		getOperand()->adopt(cEnvironment_, cProgram_, cArgument_);
	if (isReplicate(cEnvironment_)) { // 分散レプリケーションの場合は何もしない.
		return pResult;
	}

	if (getPredicate() && getPredicate()->isChecked()) {
		// create delayed retrieving program if needed
		generateDelayed(cEnvironment_,
						cProgram_,
						pResult);
		generateCheckPredicate(cEnvironment_, cProgram_, pResult, cArgument_, getPredicate());
	}

	// generate distinct key
	int iKeyID = m_pKey->generate(cEnvironment_, cProgram_, pResult, cArgument_);

	// collection on memory with distinct
	Execution::Collection::Distinct* pDistinct =
		Execution::Collection::Distinct::create(cProgram_);

	// add predicate checking duplication
	pResult->addPredicate(cProgram_,
						  Execution::Predicate::CollectionCheck::create(
								cProgram_,
								pResult,
								pDistinct->getID(),
								iKeyID),
						  cArgument_.m_eTarget);

	return pResult;
}

// FUNCTION private
//	Candidate::Distinct::createCost -- 
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
Distinct::
createCost(Opt::Environment& cEnvironment_,
		   const AccessPlan::Source& cPlanSource_,
		   AccessPlan::Cost& cCost_)
{
	if (cCost_.isInfinity()) {
		getOperand()->createCost(cEnvironment_, cPlanSource_);
		cCost_ = getOperand()->getCost();
		cCost_.addDistinctCost();
	}
}


// FUNCTION public
//	Relation::Impl::DistinctImpl::isReplicate -- 
//
// NOTES
//
// ARGUMENTS
//	
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
Distinct::
isReplicate(Opt::Environment& cEnvironment_)
{

	if (cEnvironment_.hasCascade()) {
		Plan::Candidate::InquiryArgument cArgument = 0;
		cArgument.m_iTarget |= Plan::Candidate::InquiryArgument::Target::Distributed;
		InquiryResult iResult = getOperand()->inquiry(cEnvironment_, cArgument);
		return !(iResult & Plan::Candidate::InquiryArgument::Target::Distributed);
	}
	return false;
}

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
