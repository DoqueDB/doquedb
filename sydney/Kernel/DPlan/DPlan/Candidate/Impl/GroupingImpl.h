// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/GroupingImpl.h --
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

#ifndef __SYDNEY_DPLAN_CANDIDATE_GROUPINGIMPL_H
#define __SYDNEY_DPLAN_CANDIDATE_GROUPINGIMPL_H

#include "DPlan/Module.h"
#include "DPlan/Declaration.h"
#include "DPlan/Candidate/Grouping.h"

#include "Plan/Candidate/Base.h"
#include "Plan/Candidate/Monadic.h"


_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN
namespace GroupingImpl
{
////////////////////////////////////////////////////
// CLASS local
//	Plan::Candidate::GroupingImpl -- implementation class of Interface::ICandidate for grouping
//
// NOTES
	class Normal
		: public Plan::Candidate::Monadic<Candidate::Grouping>
	{
	public:
		typedef Normal This;
		typedef Plan::Candidate::Monadic<Candidate::Grouping> Super;

		// constructor		
		Normal(Plan::Order::Specification* pSpecification_,
			   const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
			   const Plan::Utility::ScalarSet& cAggregationOperand_,
			   Plan::Interface::ICandidate* pOperand_)
			: Super(pOperand_),
			  m_pSpecification(pSpecification_),
			  m_vecAggregation(vecAggregation_),
			  m_cAggregationOperand(cAggregationOperand_)
			{}

		// destructor
		virtual ~Normal() {}

		/////////////////////////////
		// Interface::ICandidate::
		//	virtual void createCost(Opt::Environment& cEnvironment_,
		//							const AccessPlan::Source& cPlanSource_);
		//	virtual const AccessPlan::Cost& getCost();
		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Plan::Candidate::AdoptArgument& cArgument_);

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

	protected:

		VECTOR<Plan::Interface::IScalar*>& getAggregation() {return m_vecAggregation;}

		Plan::Order::Specification*  getSpecification() {return m_pSpecification;}		
	private:
		/////////////////////////////
		// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const Plan::AccessPlan::Source& cPlanSource_,
								Plan::AccessPlan::Cost& cCost_);
		virtual Plan::Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Plan::Candidate::Row* createKey(Opt::Environment& cEnvironment_);

		Plan::Order::Specification* m_pSpecification;
		VECTOR<Plan::Interface::IScalar*> m_vecAggregation;
		Plan::Utility::ScalarSet m_cAggregationOperand;
	};

	class Simple
		: public Plan::Candidate::Monadic<Candidate::Grouping>
	{
	public:
		typedef Simple This;
		typedef Plan::Candidate::Monadic<Candidate::Grouping> Super;
		
		Simple(const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
			   const Plan::Utility::ScalarSet& cAggregationOperand_,
			   Plan::Interface::ICandidate* pOperand_)
			: Super(pOperand_),
			  m_vecAggregation(vecAggregation_),
			  m_cAggregationOperand(cAggregationOperand_)
		{}		

		virtual ~Simple() {}
		/////////////////////////////
		// Interface::ICandidate::
		//	virtual void createCost(Opt::Environment& cEnvironment_,
		//							const AccessPlan::Source& cPlanSource_);
		//	virtual const AccessPlan::Cost& getCost();
		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Plan::Candidate::AdoptArgument& cArgument_);

		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);
		

	protected:
		// constructor		


	private:
		/////////////////////////////
		// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const Plan::AccessPlan::Source& cPlanSource_,
								Plan::AccessPlan::Cost& cCost_);
		virtual Plan::Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Plan::Candidate::Row* createKey(Opt::Environment& cEnvironment_);
		
		virtual void adoptOperand(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Plan::Candidate::AdoptArgument& cArgument_,
								  Execution::Interface::IIterator* pIterator_,
								  Plan::Candidate::Row* pRow_);
		
		VECTOR<Plan::Interface::IScalar*> m_vecAggregation;
		Plan::Utility::ScalarSet m_cAggregationOperand;
	};

	class Replicate
		:public Normal
	{
	public:
		typedef Replicate This;
		typedef Normal Super;
		// constructor		
		Replicate(Plan::Order::Specification* pSpecification_,
				  const VECTOR<Plan::Interface::IScalar*>& vecAggregation_,
				  const Plan::Utility::ScalarSet& cAggregationOperand_,
				  Plan::Interface::ICandidate* pOperand_)
			: Super(pSpecification_,
					vecAggregation_,
					cAggregationOperand_,
					pOperand_)
			{}
		
		virtual ~Replicate() {};
		
		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Plan::Candidate::AdoptArgument& cArgument_);
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

	protected:


	private:
	};

} // namepspace GroupingImpl
_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_GROUPINGIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
