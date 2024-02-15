// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Grouping.cpp --
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
const char moduleName[] = "DPlan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "DPlan/Relation/Impl/GroupingImpl.h"
#include "DPlan/Relation/Sort.h"
#include "Plan/Relation/Sort.h"
#include "Plan/Order/Specification.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN


/////////////////////////////////////
// Relation::Grouping

// FUNCTION public
//	Relation::Grouping::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Order::Specification* pSortSpecification_
//	Interface::IRelation* pOperand_
//	
// RETURN
//	Grouping*
//
// EXCEPTIONS

//static
Grouping*
Grouping::
create(Opt::Environment& cEnvironment_,
	   Plan::Order::Specification* pSortSpecification_,
	   Plan::Interface::IRelation* pOperand_)
{

	Plan::Relation::InquiryArgument cArgument = 0;
	cArgument.m_iTarget |= Plan::Relation::InquiryArgument::Target::Distributed;
	InquiryResult iResult = pOperand_->inquiry(cEnvironment_, cArgument);

	Plan::Interface::IRelation* pSort = 0;
	// Groupingキーがない場合または、レプリケーションの場合はSortしない。
	if ((!(iResult & Plan::Relation::InquiryArgument::Target::Distributed))
		|| pSortSpecification_->getKeySize() == 0)
		pSort = pOperand_;
	else
		pSort = Sort::create(cEnvironment_,
							 pSortSpecification_,
							 pOperand_);
	
	
	// create grouping relation
	AUTOPOINTER<This> pResult = new GroupingImpl::Normal(pSortSpecification_, pSort);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION protected
//	Relation::Grouping::Grouping -- constructor
//
// NOTES
//
// ARGUMENTS
//	Nothing
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

Grouping::
Grouping()
	: Super(Plan::Tree::Node::GroupBy)
{}

// FUNCTION private
//	Relation::Grouping::registerToEnvironment -- register to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Grouping::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.setGrouping();
	Super::registerToEnvironment(cEnvironment_);	
}

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
