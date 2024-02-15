// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Sort.cpp --
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
const char moduleName[] = "DPlan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "DPlan/Relation/Impl/SortImpl.h"
#include "DPlan/Candidate/Sort.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Sort.h"
#include "Plan/Relation/Argument.h"


#include "Exception/NotSupported.h"

#include "Opt/Configuration.h"
#include "Opt/Environment.h"

#include "Schema/Field.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

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
Plan::Interface::ICandidate*
Impl::SortImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{

	Plan::AccessPlan::Source cSource1(cPlanSource_, m_pSpecification);
	Plan::Interface::ICandidate* pOperandCandidate =
			getOperand()->createAccessPlan(cEnvironment_, cSource1);
	
	Plan::Relation::InquiryArgument cArgument = 0;
	cArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Distributed;
	InquiryResult iResult = getOperand()->inquiry(cEnvironment_, cArgument);
	
	Plan::Interface::ICandidate* pResult = 0;
	if (iResult &
		Plan::Relation::InquiryArgument::Target::Distributed) {
		if (pOperandCandidate->isMergeSortAvailable()) {
			// Distributionの場合は、各子サーバーの結果をマージしてソートする.
			pResult = DPlan::Candidate::Sort::create(cEnvironment_,
													 m_pSpecification,
													 pOperandCandidate);
		} else {
			// 別のDistributionSortがOperandで発生する場合は、
			// 分散マネージャー上でSortを実施する
			pResult = Plan::Candidate::Sort::create(cEnvironment_,
													m_pSpecification,
													pOperandCandidate);
		}
	} else {
		// Replicationの場合は、子サーバーでのソート結果をそのまま返す.
		pResult = pOperandCandidate;
	}
	
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
		Plan::Interface::ICandidate* pCandidate_)
{
	m_pSpecification->require(cEnvironment_,
							  pCandidate_);
	getOperand()->require(cEnvironment_,
						  pCandidate_);
}


// FUNCTION public
//	Relation::TableImpl::Delete::generateSQL -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//
//	
// RETURN
//	Plan::Interface::ISqlNode*
//
// EXCEPTIONS

//virtual
Plan::Sql::Query*
Impl::SortImpl::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pQuery = getOperand()->generateSQL(cEnvironment_);
	pQuery->setOrderBy(m_pSpecification);

	return pQuery;
}

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
