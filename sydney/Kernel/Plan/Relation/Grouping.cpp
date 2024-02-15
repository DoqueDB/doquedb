// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Grouping.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Relation";
}

#include "boost/bind.hpp"

#include "SyDefault.h"

#include "Plan/Relation/Impl/GroupingImpl.h"
#include "Plan/Relation/Sort.h"
#include "Plan/Order/Specification.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

/////////////////////////////////////
// Relation::Grouping::Simple::

// FUNCTION public
//	Relation::Grouping::Simple::create -- costructor
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
Grouping::Simple::
create(Opt::Environment& cEnvironment_,
	   Order::Specification* pSortSpecification_,
	   Interface::IRelation* pOperand_)
{
	; _SYDNEY_ASSERT(pSortSpecification_->getKeySize() == 0);
	pOperand_->setGrouping();
	// create grouping relation
	AUTOPOINTER<This> pResult = new GroupingImpl::Simple(pSortSpecification_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

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
	   Order::Specification* pSortSpecification_,
	   Interface::IRelation* pOperand_)
{
	pOperand_->setGrouping();
	// create operand relation for sorting
	Interface::IRelation* pSort =
		(pSortSpecification_->getKeySize() == 0)
		? pOperand_
		: Sort::create(cEnvironment_, pSortSpecification_, pOperand_);

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
	: Super(Tree::Node::GroupBy)
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
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
