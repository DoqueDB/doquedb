// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/SortImpl.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_IMPL_SORTIMPL_H
#define __SYDNEY_PLAN_CANDIDATE_IMPL_SORTIMPL_H

#include "Plan/Candidate/Sort.h"

#include "Plan/Declaration.h"
#include "Plan/Candidate/Monadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace SortImpl
{
	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::SortImpl::Base -- base class of implementation classes for sort
	//
	// NOTES
	class Base
		: public Monadic<Candidate::Sort>
	{
	public:
		typedef Monadic<Candidate::Sort> Super;
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
							  Candidate::AdoptArgument& cArgument_);



	protected:
		// constructor
		Base(Order::Specification* pSpecification_,
			 Interface::ICandidate* pOperand_)
			: Super(pSpecification_, pOperand_)
		{}

	private:
		virtual Execution::Interface::IIterator*
						adoptOperand(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Candidate::AdoptArgument& cArgument_) = 0;
	}; // class Base

	////////////////////////////////////////////////////
	// CLASS
	//	Plan::Candidate::SortImpl::Normal -- implementation class of Interface::ICandidate for sort
	//
	// NOTES
	class Normal
		: public Base
	{
	public:
		typedef Base Super;
		typedef Normal This;

		// constructor
		Normal(Order::Specification* pSpecification_,
			   Interface::ICandidate* pOperand_)
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
	////////////////////////////
	// SortImpl::Base::
		virtual Execution::Interface::IIterator*
						adoptOperand(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Candidate::AdoptArgument& cArgument_);
	////////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);
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
		Partial(Order::Specification* pSpecification_,
				const AccessPlan::Limit& cLimit_,
				Interface::ICandidate* pOperand_)
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
	////////////////////////////
	// SortImpl::Normal::
		virtual Execution::Interface::IIterator*
						adoptOperand(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Candidate::AdoptArgument& cArgument_);

	////////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);

		AccessPlan::Limit m_cLimit;
	}; // class Partial

} // namespace SortImpl

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_SORTIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
