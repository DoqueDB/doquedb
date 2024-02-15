// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Order/Argument.h --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_ORDER_ARGUMENT_H
#define __SYDNEY_PLAN_ORDER_ARGUMENT_H

#include "Plan/Order/Specification.h"

#include "Plan/Declaration.h"
#include "Plan/AccessPlan/Limit.h"
#include "Plan/Scalar/Argument.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_ORDER_BEGIN

// STRUCT
//	Plan::Order::CheckArgument -- argument for Specification::check method
//
// NOTES

struct CheckArgument
	: public Scalar::CheckArgument
{
	typedef CheckArgument This;
	typedef Scalar::CheckArgument Super;

	bool m_bCheckPartial;
	bool m_bGrouping;
	
	CheckArgument(Interface::ICandidate* pCandidate_,
				  const VECTOR<Interface::ICandidate*>& vecPrecedingCandidate_,
				  bool bCheckPartial_,
				  bool bGrouping_)
		: Super(pCandidate_, vecPrecedingCandidate_),
		m_bCheckPartial(bCheckPartial_),
		  m_bGrouping(bGrouping_)
	{}
	CheckArgument(const CheckArgument& cArgument_)
		: Super(cArgument_),
		m_bCheckPartial(cArgument_.m_bCheckPartial),
		  m_bGrouping(cArgument_.m_bGrouping)
	{}
};

// STRUCT
//	Plan::Order::ChooseArgument -- argument for CheckedSpecification::choose method
//
// NOTES

struct ChooseArgument
{
	Candidate::Table* m_pTable;
	Interface::IPredicate* m_pPredicate;
	AccessPlan::Limit m_cLimit;

	ChooseArgument(Candidate::Table* pTable_,
				   Interface::IPredicate* pPredicate_,
				   const AccessPlan::Limit& cLimit_ = AccessPlan::Limit())
		: m_pTable(pTable_),
		  m_pPredicate(pPredicate_),
		  m_cLimit(cLimit_)
	{}
	ChooseArgument(ChooseArgument& cArgument_)
		: m_pTable(cArgument_.m_pTable),
		  m_pPredicate(cArgument_.m_pPredicate),
		  m_cLimit(cArgument_.m_cLimit)
	{}
};

_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_ORDER_ARGUMENT_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
