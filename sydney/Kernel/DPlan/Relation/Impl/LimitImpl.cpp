// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Limit.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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

#include "DPlan/Candidate/Limit.h"

#include "DPlan/Relation/Impl/LimitImpl.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"


_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN


/////////////////////////////////////
// Relation::Impl::LimitImpl

// FUNCTION public
//	Relation::Impl::LimitImpl::createAccessPlan -- create access plan candidate
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
Impl::LimitImpl::
createAccessPlan(Opt::Environment& cEnvironment_,
				 Plan::AccessPlan::Source& cPlanSource_)
{
	Plan::AccessPlan::Source cSource1(cPlanSource_, m_cLimit);

	Plan::Interface::ICandidate* pOperandCandidate =
		getOperand()->createAccessPlan(cEnvironment_, cSource1);
	
	// need limit candidate
	Plan::Interface::ICandidate* pResult =
		DPlan::Candidate::Limit::create(cEnvironment_, this, pOperandCandidate);

	
	return pResult;
}

// FUNCTION public
//	Interface::IRelation::IRelation -- generateSQL
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Sql::Query*
//
// EXCEPTIONS
//
// VIRTUAL
Plan::Sql::Query*
Impl::LimitImpl::
generateSQL(Opt::Environment& cEnvironment_)
{
	Plan::Sql::Query* pResult =  Super::generateSQL(cEnvironment_);
	pResult->setLimit(&m_cLimit);
	return pResult;
}


// FUNCTION public
//	Relation::Impl::LimitImpl::inquiry -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const InquiryArgument& cArgument_
//	
// RETURN
//	Interface::IRelation::InquiryResult
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation::InquiryResult
Impl::LimitImpl::
inquiry(Opt::Environment& cEnvironment_,
		const Plan::Relation::InquiryArgument& cArgument_)
{
	Plan::Relation::InquiryArgument cMyArgument(cArgument_);
	if (cArgument_.m_iTarget & Plan::Relation::InquiryArgument::Target::Depending) {
		cMyArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Refering;
	}
	// inquiry operand
	Plan::Interface::IRelation::InquiryResult iResult = getOperand()->inquiry(cEnvironment_,
												  cMyArgument);

	if (cArgument_.m_iTarget & Plan::Relation::InquiryArgument::Target::Depending
		&& (iResult & Plan::Relation::InquiryArgument::Target::Refering)) {
		iResult |= Plan::Relation::InquiryArgument::Target::Depending;
	}
	return (iResult & cArgument_.m_iTarget);	
}

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
