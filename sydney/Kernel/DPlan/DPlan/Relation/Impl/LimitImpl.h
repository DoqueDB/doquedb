// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/DistinctImpl.h --
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

#ifndef __SYDNEY_DPLAN_RELATION_DISTINCTIMPL_H
#define __SYDNEY_DPLAN_RELATION_DISTINCTIMPL_H

#include "DPlan/Relation/Limit.h"

#include "Plan/AccessPlan/Limit.h"
#include "Plan/AccessPlan/Source.h"

#include "Plan/Relation/Filter.h"

#include "Opt/Environment.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

namespace Impl
{
	// CLASS
	//	Relation::Impl::SortImpl --implementation class of Sort
	//
	// NOTES

	class LimitImpl
		: public Plan::Relation::Filter<Relation::Limit>
	{
	public:
		typedef Plan::Relation::Filter<Relation::Limit> Super;
		typedef LimitImpl This;

		LimitImpl(Plan::Interface::IScalar* pLimit_,
				  Plan::Interface::IScalar* pOffset_,
				  Plan::Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_cLimit(pLimit_, pOffset_)
		{}
		~LimitImpl() {}

	////////////////////////
	// Relation::Limit::
		virtual const Plan::AccessPlan::Limit& getLimit() {return m_cLimit;}

	////////////////////////
	// Interface::IRelation
		virtual Plan::Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 Plan::AccessPlan::Source& cPlanSource_);
		
		virtual Plan::Interface::IRelation::InquiryResult inquiry(Opt::Environment& cEnvironment_,
																 const Plan::Relation::InquiryArgument& cArgument_);

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);
		
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	protected:
	private:
		Plan::AccessPlan::Limit m_cLimit;

	};
} // namespace Impl

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_RELATION_DISTINCTIMPL_H

//
//	Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
