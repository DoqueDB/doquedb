// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/ValueListImpl.h --
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

#ifndef __SYDNEY_PLAN_RELATION_VALUELISTIMPL_H
#define __SYDNEY_PLAN_RELATION_VALUELISTIMPL_H

#include "Plan/Relation/ValueList.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace ValueListImpl
{
	//////////////////////////////////
	// CLASS
	//	ValueListImpl::Base --
	//
	// NOTES
	class Base
		: public Relation::ValueList
	{
	public:
		typedef Base This;
		typedef Relation::ValueList Super;

		// destructor
		virtual ~Base() {}

	//////////////////////////
	// Interface::IRelation
	//	virtual Interface::ICandidate* createAccessPlan(Opt::Environment& cEnvironment_,
	//													AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
	//	virtual void getUsedTable(Opt::Environment& cEnvironment_,
	//							  Utility::RelationSet& cResult_);
	protected:
		// constructor
		Base() : Super() {}

	private:
	//////////////////////////
	// Interface::IRelation
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
	//	virtual int setDegree(Opt::Environment& cEnvironment_);
	//	virtual int setMaxPosition(Opt::Environment& cEnvironment_);
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
	};

	//////////////////////////////////
	// CLASS local
	//	ValueListImpl::SingleColumn --
	//
	// NOTES
	class SingleColumn
		: public Base
	{
	public:
		typedef SingleColumn This;
		typedef Base Super;

		SingleColumn()
			: Super(),
			  m_vecValue()
		{}
		SingleColumn(const VECTOR<Interface::IScalar*>& vecScalar_)
			: Super(),
			  m_vecValue(vecScalar_)
		{}
		~SingleColumn() {}

	//////////////////////////
	// Interface::IRelation
		virtual Size getCardinality(Opt::Environment& cEnvironment_);
		virtual bool getRow(Opt::Environment& cEnvironment_,
							int iPosition_,
							VECTOR<Interface::IScalar*>& vecResult_);
		virtual Interface::ICandidate* createAccessPlan(Opt::Environment& cEnvironment_,
														AccessPlan::Source& cPlanSource_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
	//////////////////////////
	// Interface::IRelation
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);

		VECTOR<Interface::IScalar*> m_vecValue;
	};

	//////////////////////////////////
	// CLASS local
	//	ValueListImpl::MultiColumn --
	//
	// NOTES
	class MultiColumn
		: public Base
	{
	public:
		typedef MultiColumn This;
		typedef Base Super;

		MultiColumn()
			: Super(),
			  m_vecvecValue(),
			  m_vecVariable()
		{}
		MultiColumn(const VECTOR< VECTOR<Interface::IScalar*> >& vecvecValue_)
			: Super(),
			  m_vecvecValue(vecvecValue_),
			  m_vecVariable()
		{}
		~MultiColumn() {}

	//////////////////////////
	// Interface::IRelation
		virtual Size getCardinality(Opt::Environment& cEnvironment_);
		virtual bool getRow(Opt::Environment& cEnvironment_,
							int iPosition_,
							VECTOR<Interface::IScalar*>& vecResult_);
		virtual Interface::ICandidate* createAccessPlan(Opt::Environment& cEnvironment_,
														AccessPlan::Source& cPlanSource_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
	private:
	//////////////////////////
	// Interface::IRelation
		virtual int setDegree(Opt::Environment& cEnvironment_);
		virtual int setMaxPosition(Opt::Environment& cEnvironment_);

		VECTOR< VECTOR<Interface::IScalar*> > m_vecvecValue;
		VECTOR<Interface::IScalar*> m_vecVariable;
	};

} // namespace ValueListImpl

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_VALUELISTIMPL_H

//
//	Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
