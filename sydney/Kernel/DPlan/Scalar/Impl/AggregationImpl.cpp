// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Scalar/Impl/AggregationImpl.cpp --
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
const char moduleName[] = "DPlan::Scalar::Impl";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "DPlan/Scalar/Impl/AggregationImpl.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Relation/Argument.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Action/Argument.h"
#include "Execution/Function/Aggregation.h"
#include "Execution/Function/Factory.h"
#include "Execution/Interface/IIterator.h"

#include "Opt/Environment.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_SCALAR_BEGIN

// FUNCTION protected
//	Scalar::AggregationImpl::generateThis -- generate
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Execution::Interface::IIterator* pIterator_
//	Candidate::AdoptArgument& cArgument_
//	int iDataID_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual

int
AggregationImpl::Monadic::
generateThis(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Execution::Interface::IIterator* pIterator_,
			 Plan::Candidate::AdoptArgument& cArgument_,
			 int iDataID_)
{
	// レプリケーションの場合は子サーバーの結果をそのまま返す
	if (cArgument_.m_pCandidate) {
		Plan::Candidate::InquiryArgument cInqArg(Plan::Candidate::InquiryArgument::Target::Distributed);
		Plan::Interface::ICandidate::InquiryResult iResult = 
			cArgument_.m_pCandidate->inquiry(cEnvironment_, cInqArg);
		if (!(iResult & Plan::Candidate::InquiryArgument::Target::Distributed)) {
			int iOperandID = getOperand()->generateFromType(cEnvironment_, cProgram_, pIterator_, cArgument_);
			pIterator_->addCalculation(cProgram_,
									   Execution::Function::Factory::create(cProgram_,
																			pIterator_,
																			Plan::Tree::Node::Copy,
																			iOperandID,
																			iDataID_));
			return iDataID_;
		}
	}
	
	// Avg(Fulltext_length())の場合のみオペランドの演算を分散マネージャー上で行う。
	int iOperandID = 0;
	if (getType() == Plan::Tree::Node::Sum
		&& getOperand()->getType() == Plan::Tree::Node::Multiply 
		&& getOperand()->getOperandAt(0)->getType() == Plan::Tree::Node::Avg 
		&& getOperand()->getOperandAt(0)->getOperandAt(0)->getType() == Plan::Tree::Node::FullTextLength) {
		iOperandID = getOperand()->generate(cEnvironment_, cProgram_, pIterator_, cArgument_);
	} else {
		iOperandID = getOperand()->generateFromType(cEnvironment_, cProgram_, pIterator_, cArgument_);
	}
	
	Execution::Interface::IAction* pResult = 0;	
	switch (getType()) {
	case Plan::Tree::Node::Count:
		{
			pResult = Execution::Function::Aggregation::Count::create(cProgram_,
																	  pIterator_,
																	  iOperandID,
																	  iDataID_);
			break;
		}
	case Plan::Tree::Node::Max:
		{
			pResult = Execution::Function::Aggregation::Max::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Plan::Tree::Node::Min:
		{
			pResult = Execution::Function::Aggregation::Min::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Plan::Tree::Node::Sum:
		{
			pResult = Execution::Function::Aggregation::Sum::create(cProgram_,
																	pIterator_,
																	iOperandID,
																	iDataID_);
			break;
		}
	case Plan::Tree::Node::Word:
		{
			pResult = Execution::Function::Aggregation::Word::create(cProgram_,
																	 pIterator_,
																	 iOperandID,
																	 iDataID_);
			break;
		}

	case Plan::Tree::Node::Distinct:
		{
			pResult = Execution::Function::Aggregation::Distinct::create(cProgram_,
																		 pIterator_,
																		 iOperandID,
																		 iDataID_);
			break;
		}
	default:
		{
			_SYDNEY_THROW0(Exception::Unexpected);
		}
	}

	// add aggregate action
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT1(Aggregation,
											pResult->getID()));

	return iDataID_;
}

_SYDNEY_DPLAN_SCALAR_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

