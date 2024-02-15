// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/GroupingImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_RELATION_GROUPINGIMPL_H
#define __SYDNEY_DPLAN_RELATION_GROUPINGIMPL_H

#include "boost/bind.hpp"

#include "DPlan/Relation/Grouping.h"
#include "Plan/Relation/Filter.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

namespace GroupingImpl
{
	// CLASS
	//	Relation::GroupingImpl::Normal --implementation class of Grouping
	//
	// NOTES

	class Normal
		: public Plan::Relation::Filter<DPlan::Relation::Grouping>
	{
	public:
		typedef Plan::Relation::Filter<DPlan::Relation::Grouping> Super;
		typedef Normal This;

		Normal(Plan::Order::Specification* pSortSpecification_,
			   Plan::Interface::IRelation* pOperand_)
			: Super(pOperand_),
			  m_pSortSpecification(pSortSpecification_),
			  m_vecAggregation(),
			  m_cAggregationOperand()
		{}
		virtual ~Normal() {}

	////////////////////////
	// Relation::Grouping

	////////////////////////
	// Interface::IRelation
		virtual Plan::Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 Plan::AccessPlan::Source& cPlanSource_);
		virtual Plan::Interface::IRelation::InquiryResult inquiry(Opt::Environment& cEnvironment_,
																  const Plan::Relation::InquiryArgument& cArgument_);
		
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);

	protected:
		// accessor
		VECTOR<Plan::Interface::IScalar*>& getAggregation() {return m_vecAggregation;}

	private:
	////////////////////////
	// Interface::IRelation
	//	virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
	//	virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
		virtual void createScalarName(Opt::Environment& cEnvironment_,
									  VECTOR<STRING>& vecName_,
									  Plan::Interface::IRelation::Position iPosition_);
		virtual void createScalar(Opt::Environment& cEnvironment_,
								  VECTOR<Plan::Interface::IScalar*>& vecScalar_,
								  Plan::Interface::IRelation::Position iPosition_);
		virtual void createScalarType(Opt::Environment& cEnvironment_,
									  VECTOR<Plan::Tree::Node::Type>& vecType_,
									  Plan::Interface::IRelation::Position iPosition_);
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Plan::Interface::IRelation::Position iPosition_);
		virtual int addAggregation(Opt::Environment& cEnvironment_,
								   Plan::Interface::IScalar* pScalar_,
								   Plan::Interface::IScalar* pOperand_);

		Plan::Order::Specification* m_pSortSpecification;
		VECTOR<Plan::Interface::IScalar*> m_vecAggregation;
		Plan::Utility::ScalarSet m_cAggregationOperand;
	};


} // namespace GroupingImpl

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_RELATION_GROUPINGIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
