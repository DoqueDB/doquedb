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

#include "DPlan/Relation/Sort.h"
#include "Plan/Relation/Filter.h"
#include "Plan/Order/Specification.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

namespace Impl
{
	// CLASS
	//	Relation::Impl::SortImpl --implementation class of Sort
	//
	// NOTES

	class SortImpl
		: public Plan::Relation::Filter<Relation::Sort>
	{
	public:
		typedef Plan::Relation::Filter<Relation::Sort> Super;
		typedef SortImpl This;

		SortImpl(Plan::Order::Specification* pSortSpecification_,
				 Plan::Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_pSpecification(pSortSpecification_)
		{}
		~SortImpl() {}

		////////////////////////
		// Relation::Sort
		// accessor
		virtual Plan::Order::Specification* getSpecification() {return m_pSpecification;}

	////////////////////////
	// Interface::IRelation
		virtual Plan::Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 Plan::AccessPlan::Source& cPlanSource_);
	//	virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
	//								  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Plan::Interface::ICandidate* pCandidate_);
		
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

	protected:
	private:
		Plan::Order::Specification* m_pSpecification;
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
