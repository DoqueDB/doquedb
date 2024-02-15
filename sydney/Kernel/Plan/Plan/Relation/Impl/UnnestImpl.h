// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/UnnestImpl.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_UNNESTIMPL_H
#define __SYDNEY_PLAN_RELATION_UNNESTIMPL_H

#include "Plan/Relation/Unnest.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace Impl
{
	//////////////////////////////////
	// CLASS local
	//	Impl::UnnestImpl --
	//
	// NOTES
	class UnnestImpl
		: public Relation::Unnest
	{
	public:
		typedef UnnestImpl This;
		typedef Relation::Unnest Super;

		UnnestImpl()
			: Super(),
			  m_pValue(0)
		{}
		UnnestImpl(Interface::IScalar* pValue_)
			: Super(),
			  m_pValue(pValue_)
		{}
		~UnnestImpl() {}

	//////////////////////////
	// Relation::Unnest
		virtual Interface::IScalar* getValue() {return m_pValue;}

	//////////////////////////
	// Interface::IRelation
		virtual Interface::ICandidate* createAccessPlan(Opt::Environment& cEnvironment_,
														AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
	//////////////////////////
	// Interface::IRelation
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);
		virtual void createScalarName(Opt::Environment& cEnvironment_,
									  VECTOR<STRING>& vecName_,
									  Position iPosition_);
		virtual void createScalar(Opt::Environment& cEnvironment_,
								  VECTOR<Interface::IScalar*>& vecScalar_,
								  Position iPosition_);
		virtual void createScalarType(Opt::Environment& cEnvironment_,
									  VECTOR<Tree::Node::Type>& vecType_,
									  Position iPosition_);
		virtual void setRetrieved(Opt::Environment& cEnvironment_,
								  Position iPosition_);
		virtual int addAggregation(Opt::Environment& cEnvironment_,
								   Interface::IScalar* pScalar_,
								   Interface::IScalar* pOperand_);

		Interface::IScalar* m_pValue;
	};

} // namespace Impl

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_UNNESTIMPL_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
