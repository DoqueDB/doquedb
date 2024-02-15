// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/UnnestImpl.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_IMPL_UNNESTIMPL_H
#define __SYDNEY_PLAN_CANDIDATE_IMPL_UNNESTIMPL_H

#include "Plan/Candidate/Unnest.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace Impl
{
	/////////////////////////////////////////////////
	// CLASS
	//	Candidate::Impl::UnnestImpl --
	//
	// NOTES
	class UnnestImpl
		: public Candidate::Unnest
	{
	public:
		typedef Candidate::Unnest Super;
		typedef UnnestImpl This;

		// constructor
		UnnestImpl(Relation::Unnest* pRelation_)
			: Super(),
			  m_pRelation(pRelation_)
		{}
		// destructor
		~UnnestImpl() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();
	//	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_);
		virtual void require(Opt::Environment& cEnvironment_,
							 Scalar::Field* pField_) {; /* do nothing */}
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_) {; /* do nothing */}
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_,
						   Scalar::DelayArgument& cArgument_)
		{return false;}
		virtual bool isReferingRelation(Interface::IRelation* pRelation_);
		virtual void createReferingRelation(Utility::RelationSet& cRelationSet_);
		virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_);
		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_)
		{; /* do nothing */}
	//	virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
	//								  const Candidate::InquiryArgument& cArgument_);

	protected:
	private:
	/////////////////////////////
	// Interface::ICandidate::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);
		virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);

		Relation::Unnest* m_pRelation;
	};

} // namespace Impl

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_UNNESTIMPL_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
