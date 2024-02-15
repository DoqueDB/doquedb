// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/WordExtraction.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "DPlan/Candidate/WordExtraction.h"

#include "Plan/Candidate/Argument.h"

#include "Plan/AccessPlan/Cost.h"
#include "Plan/AccessPlan/Source.h"

#include "Common/Assert.h"
#include "Common/DataInstance.h"

#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"
#include "Execution/Operator/Limit.h"
#include "Execution/Operator/Iterate.h"

#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_CANDIDATE_USING

//////////////////////////////////////////////////
//	DPlan::Candidate::WordExtraction

// FUNCTION public
//	Candidate::WordExtraction::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Relation::WordExtraction* pWordExtraction_
//	Interface::ICandidate* pOperand_
//	
// RETURN
//	WordExtraction*
//
// EXCEPTIONS

//static
WordExtraction*
WordExtraction::
create(Opt::Environment& cEnvironment_,
	   Plan::Interface::ICandidate* pOperand_,
	   Predicate::Contains* pContains_,
	   bool bIsNeedScore)
{
	AUTOPOINTER<This> pResult = new WordExtraction(pOperand_,
												   pContains_,
												   bIsNeedScore);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Candidate::WordExtraction::adopt -- 
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
WordExtraction::
adopt(Opt::Environment& cEnvironment_,
	  Execution::Interface::IProgram& cProgram_,
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_pCandidate = getOperand();
	Execution::Interface::IIterator* pResult = m_pContains->adopt(cEnvironment_, cProgram_, cMyArgument);

	// WordをOutDataにセットする
	Plan::Candidate::Row* pRow = getRow(cEnvironment_);
	Plan::Candidate::Row::Iterator ite = pRow->begin();
	Plan::Candidate::Row::Iterator end = pRow->end();
	int iPutDataID = -1;
	Plan::Tree::Node::Type eType;
	for (; ite != end; ite++) {
		if ((*ite)->isField()
			&& (*ite)->getField()->isFunction()) {
			eType = (*ite)->getField()->getFunction()->getType();
			// Word取得の場合はPredicateの結果を返す
			// Scoreの場合は、子サーバーにWordを元に生成したクエリを投げて、Score計算する
			if (eType == Plan::Tree::Node::Word) {
				iPutDataID = (*ite)->generate(cEnvironment_,
											  cProgram_,
											  pResult,
											  cArgument_);
				; _SYDNEY_ASSERT(iPutDataID != -1);
				VECTOR<int> vecData;
				vecData.pushBack(iPutDataID);
				pResult->addAction(cProgram_,
									 _ACTION_ARGUMENT1(OutData,
													   cProgram_.addVariable(vecData)));
				break;
			} else if (eType == Plan::Tree::Node::Score) {
				cMyArgument = cArgument_;
				pResult = getOperand()->adopt(cEnvironment_,
											  cProgram_,
											  cMyArgument);
				break;
			}
		}
	}

	
	return pResult;
}



// FUNCTION public
//	Candidate::WordExtraction::generateSQL
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
WordExtraction::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	return pResult;
}

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
