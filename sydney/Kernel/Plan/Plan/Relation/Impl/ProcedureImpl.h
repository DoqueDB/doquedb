// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Impl/ProcedureImpl.h --
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_RELATION_PROCEDUREIMPL_H
#define __SYDNEY_PLAN_RELATION_PROCEDUREIMPL_H

#include "Plan/Relation/Procedure.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_RELATION_BEGIN

namespace ProcedureImpl
{
	//////////////////////////////////
	// CLASS
	//	ProcedureImpl::Base --
	//
	// NOTES
	class Base
		: public Relation::Procedure
	{
	public:
		typedef Base This;
		typedef Relation::Procedure Super;

		// destructor
		virtual ~Base() {}

	//////////////////////////
	// Relation::Procedure
		virtual void setRoutine(Interface::IScalar* pRoutine_)
		{m_pRoutine = pRoutine_;}

		virtual VECTOR<Interface::IScalar*>& getParam()
		{return m_vecParams;}
		virtual Interface::IScalar* getReturnValue()
		{return 0;}
		virtual Interface::IScalar* getRoutine()
		{return m_pRoutine;}

	//////////////////////////
	// Interface::IRelation
	//	virtual Interface::ICandidate* createAccessPlan(Opt::Environment& cEnvironment_,
	//													AccessPlan::Source& cPlanSource_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Interface::ICandidate* pCandidate_);
		virtual void getUsedTable(Opt::Environment& cEnvironment_,
								  Utility::RelationSet& cResult_);
	protected:
		// constructor
		Base()
			: Super(),
			  m_vecParams(),
			  m_pRoutine(0)
		{}
		Base(const VECTOR<Interface::IScalar*>& vecParams_)
			: Super(),
			  m_vecParams(vecParams_),
			  m_pRoutine(0)
		{}

	private:
	//////////////////////////
	// Interface::IRelation
		virtual RowInfo* createRowInfo(Opt::Environment& cEnvironment_);
		virtual RowInfo* createKeyInfo(Opt::Environment& cEnvironment_);
		virtual Size getCardinality(Opt::Environment& cEnvironment_);
		virtual bool getRow(Opt::Environment& cEnvironment_,
							int iPosition_,
							VECTOR<Interface::IScalar*>& vecResult_);
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

		VECTOR<Interface::IScalar*> m_vecParams;
		Interface::IScalar* m_pRoutine;
	};

	//////////////////////////////////
	// CLASS local
	//	ProcedureImpl::Function --
	//
	// NOTES
	class Function
		: public Base
	{
	public:
		typedef Function This;
		typedef Base Super;

		Function()
			: Super(),
			  m_pReturnValue(0)
		{}
		Function(const VECTOR<Interface::IScalar*>& vecParams_,
				 Interface::IScalar* pReturnValue_)
			: Super(vecParams_),
			  m_pReturnValue(pReturnValue_)
		{}
		~Function() {}
	//////////////////////////
	// Relation::Procedure
		virtual Interface::IScalar* getReturnValue()
		{return m_pReturnValue;}

	//////////////////////////
	// Interface::IRelation
		virtual Interface::ICandidate* createAccessPlan(Opt::Environment& cEnvironment_,
														AccessPlan::Source& cPlanSource_);
	protected:
	private:
		Interface::IScalar* m_pReturnValue;
	};

} // namespace ProcedureImpl

_SYDNEY_PLAN_RELATION_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_RELATION_PROCEDUREIMPL_H

//
//	Copyright (c) 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
