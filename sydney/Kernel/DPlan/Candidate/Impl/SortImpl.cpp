// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/SortImpl.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "DPlan/Candidate/Impl/SortImpl.h"

#include "Common/Assert.h"
#include "Exception/NotSupported.h"

#include "Execution/Iterator/MergeSort.h"
#include "Execution/Interface/IProgram.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Order/Argument.h"
#include "Plan/Order/Specification.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/GeneratedSpecification.h"
#include "Plan/Relation/Sort.h"
#include "Plan/Utility/Algorithm.h"


#include "Opt/Environment.h"

_SYDNEY_USING
_SYDNEY_DPLAN_USING
_SYDNEY_DPLAN_CANDIDATE_USING

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
	  Plan::Candidate::AdoptArgument& cArgument_)
{
	Execution::Iterator::MergeSort* pResult =
		Execution::Iterator::MergeSort::create(cProgram_);
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT0_T(CheckCancel,
										   cArgument_.m_eTarget));
	
	ModVector<Schema::Cascade*> vecSchemaCascade =
		cEnvironment_.getDatabase()->getCascade(cEnvironment_.getTransaction());

	Plan::Candidate::Row* pRow = cArgument_.m_pQuery->createRow(cEnvironment_);
	cArgument_.m_pInput = pResult;
	FOREACH(vecSchemaCascade,
			boost::bind(&This::adoptCascade,
						this,
						boost::ref(cEnvironment_),
						boost::ref(cProgram_),
						boost::ref(cArgument_),
						_1,
						pRow,
						pResult));

	int iResultID = pRow->generateFromType(cEnvironment_,
										   cProgram_,
										   pResult,
										   cArgument_);
	
	Plan::Candidate::RowDelayArgument cDelayArgument;
	Plan::Order::GeneratedSpecification* pGenerated =
		getOrder()->generate(cEnvironment_,
							 cProgram_,
							 pResult,
							 cArgument_,
							 pRow,
							 cDelayArgument);
	
	cArgument_.m_pQuery->addSqlGenerator(cEnvironment_,
										 cProgram_,
										 pResult,
										 cArgument_.m_bConcatinateIterable);
	pResult->addAction(cProgram_,
					   _ACTION_ARGUMENT1(OutData,
										 iResultID));
	pResult->setSortParameter(pGenerated->getPosition(),
							  pGenerated->getDirection());
	return pResult;

}

//////////////////////////////////////////////////
//	Plan::Candidate::SortImpl::Base::

// FUNCTION private
//	Candidate::SortImpl::Base::adoptCascade -- 
//
//
// NOTES
//	Cascade単位でoperandをadoptして、その結果をpIteratorのInputにセットする
//	
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_,
//	Candidate::AdoptArgument& cArgument_
//	Schema::Cascade* pCandidate_,
//	Candidate::Row* pRow_,
//	Execution::Interface::IIterator* pIterator_
//
//	
// RETURN
//	Execution::Interface::IIterator*
//
// EXCEPTIONS

//virtual


void
SortImpl::Base::
adoptCascade(Opt::Environment& cEnvironment_,
			 Execution::Interface::IProgram& cProgram_,
			 Plan::Candidate::AdoptArgument& cArgument_,
			 Schema::Cascade* pCascade_,
			 Plan::Candidate::Row* pRow_,
			 Execution::Interface::IIterator* pIterator_)
{
	Plan::Candidate::AdoptArgument cMyArgument(cArgument_);
	cMyArgument.m_pCascade = pCascade_;
	
	Execution::Interface::IIterator* pOperandIterator =
		getOperand()->adopt(cEnvironment_, cProgram_, cMyArgument);

	Plan::Candidate::RowDelayArgument cDelayArgument;	
	Plan::Order::GeneratedSpecification* pGenerated =
		getOrder()->generate(cEnvironment_,
							 cProgram_,
							 pOperandIterator,
							 cArgument_,
							 pRow_,
							 cDelayArgument);
	
	int iDataID = cProgram_.addVariable(pGenerated->getDataID());
		
	// add operand as input
	pIterator_->addAction(cProgram_,
						  _ACTION_ARGUMENT2(Input,
											pOperandIterator->getID(),
											iDataID));
}


// FUNCTION public
//	Candidate::SortImpl::Base::generateSQL
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
SortImpl::Base::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult = getOperand()->generateSQL(cEnvironment_);
	if (pResult == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	Plan::Order::Specification* pOrder = getOrder();
	;_SYDNEY_ASSERT(Plan::Order::Specification::isCompatible(pOrder, pResult->getOrderBy()));
	
	for (int i = 0; i < pOrder->getKeySize(); i++ ) {
		pOrder->getKey(i)->getScalar()->retrieveFromCascade(cEnvironment_, pResult);
	}
	
	return pResult;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
