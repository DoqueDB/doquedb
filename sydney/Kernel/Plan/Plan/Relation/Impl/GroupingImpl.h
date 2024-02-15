// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/GroupingImpl.h --
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

#ifndef __SYDNEY_PLAN_RELATION_GROUPINGIMPL_H
#define __SYDNEY_PLAN_RELATION_GROUPINGIMPL_H

#include "boost/bind.hpp"

#include "Plan/Relation/Grouping.h"
#include "Plan/Relation/Filter.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace GroupingImpl
{
	// CLASS
	//	Relation::GroupingImpl::Normal --implementation class of Grouping
	//
	// NOTES

	class Normal
		: public Relation::Filter<Relation::Grouping>
	{
	public:
		typedef Relation::Filter<Relation::Grouping> Super;
		typedef Normal This;

		Normal(Order::Specification* pSortSpecification_,
			   Interface::IRelation* pOperand_)
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
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);

	protected:
		// accessor
		VECTOR<Interface::IScalar*>& getAggregation() {return m_vecAggregation;}

	private:
	////////////////////////
	// Interface::IRelation
	//	virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
	//	virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
		virtual void createScalarName(Opt::Environment& cEnvironment_,
									  VECTOR<STRING>& vecName_,
									  Interface::IRelation::Position iPosition_);
		virtual void createScalar(Opt::Environment& cEnvironment_,
								  VECTOR<Interface::IScalar*>& vecScalar_,
								  Interface::IRelation::Position iPosition_);
		virtual void createScalarType(Opt::Environment& cEnvironment_,
									  VECTOR<Tree::Node::Type>& vecType_,
									  Interface::IRelation::Position iPosition_);
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Interface::IRelation::Position iPosition_);
		virtual int addAggregation(Opt::Environment& cEnvironment_,
								   Interface::IScalar* pScalar_,
								   Interface::IScalar* pOperand_);

		Order::Specification* m_pSortSpecification;
		VECTOR<Interface::IScalar*> m_vecAggregation;
		Utility::ScalarSet m_cAggregationOperand;
	};

	// CLASS
	//	Relation::GroupingImpl::Simple --implementation class of Grouping
	//
	// NOTES

	class Simple
		: public Normal
	{
	public:
		typedef Normal Super;
		typedef Simple This;

		Simple(Order::Specification* pSortSpecification_,
			   Interface::IRelation* pOperand_)
			: Super(pSortSpecification_, pOperand_)
		{}
		~Simple() {}

	////////////////////////
	// Relation::Grouping

	////////////////////////
	// Interface::IRelation
		virtual Interface::ICandidate*
						createAccessPlan(Opt::Environment& cEnvironment_,
										 AccessPlan::Source& cPlanSource_);
	protected:
	private:
		bool addConvertFunction(Opt::Environment& cEnvironment_,
								Interface::IScalar* pFunction_,
								VECTOR<Interface::IScalar*>& vecResult_);
	};
} // namespace Impl

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_GROUPINGIMPL_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
