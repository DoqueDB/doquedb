// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/SortImpl.h --
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

#ifndef __SYDNEY_DPLAN_CANDIDATE_IMPL_SORTIMPL_H
#define __SYDNEY_DPLAN_CANDIDATE_IMPL_SORTIMPL_H

#include "DPlan/Candidate/Sort.h"

#include "DPlan/Declaration.h"
#include "Plan/Candidate/Monadic.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_CANDIDATE_BEGIN

namespace SortImpl
{
	////////////////////////////////////////////////////
	// CLASS
	//	DPlan::Candidate::SortImpl::Base -- base class of implementation classes for sort
	//
	// NOTES
	class Base
		: public Plan::Candidate::Monadic<Candidate::Sort>
	{
	public:
		typedef Plan::Candidate::Monadic<Candidate::Sort> Super;
		typedef Base This;

		// destructor
		virtual ~Base() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();
		virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Plan::Candidate::AdoptArgument& cArgument_);
		
		virtual Plan::Sql::Query* generateSQL(Opt::Environment& cEnvironment_);

		virtual bool isMergeSortAvailable() {return false;}
		

	protected:
		// constructor
		Base(Plan::Order::Specification* pSpecification_,
			 Plan::Interface::ICandidate* pOperand_)
			: Super(pSpecification_, pOperand_)
		{}

	private:
		void adoptCascade(Opt::Environment& cEnvironment_,
						  Execution::Interface::IProgram& cProgram_,
						  Plan::Candidate::AdoptArgument& cArgument_,
						  Schema::Cascade* pCascade_,
						  Plan::Candidate::Row* pRow_,
						  Execution::Interface::IIterator* pIterator_);
	}; // class Base

	////////////////////////////////////////////////////
	// CLASS
	//	DPlan::Candidate::SortImpl::Normal -- implementation class of Interface::ICandidate for sort
	//
	// NOTES
	class Normal
		: public Base
	{
	public:
		typedef Base Super;
		typedef Normal This;

		// constructor
		Normal(Plan::Order::Specification* pSpecification_,
			   Plan::Interface::ICandidate* pOperand_)
			: Super(pSpecification_, pOperand_)
		{}

		// destructor
		virtual ~Normal() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();
	//	virtual Execution::Interface::IIterator*
	//					adopt(Opt::Environment& cEnvironment_,
	//						  Execution::Interface::IProgram& cProgram_,
	//						  Candidate::AdoptArgument& cArgument_);

	protected:
	private:

	}; // class Normal

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::SortImpl::Partial -- implementation class of Interface::ICandidate for sort
	//
	// NOTES
	class Partial
		: public Base
	{
	public:
		typedef Base Super;
		typedef Partial This;

		// constructor
		Partial(Plan::Order::Specification* pSpecification_,
				const Plan::AccessPlan::Limit& cLimit_,
				Plan::Interface::ICandidate* pOperand_)
			: Super(pSpecification_, pOperand_),
			  m_cLimit(cLimit_)
		{}

		// destructor
		virtual ~Partial() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();
	//	virtual Execution::Interface::IIterator*
	//					adopt(Opt::Environment& cEnvironment_,
	//						  Execution::Interface::IProgram& cProgram_,
	//						  Candidate::AdoptArgument& cArgument_);

	protected:
	private:


		Plan::AccessPlan::Limit m_cLimit;
	}; // class Partial

} // namespace SortImpl

_SYDNEY_DPLAN_CANDIDATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_SORTIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
