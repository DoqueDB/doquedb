// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/JoinImpl.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_IMPL_JOIN_H
#define __SYDNEY_PLAN_CANDIDATE_IMPL_JOIN_H

#include "Plan/Candidate/Join.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace JoinImpl
{
	//////////////////////////////////
	// CLASS
	//	Candidate::JoinImpl::Base -- base class of implementation class of join candidates
	//
	// NOTES
	class Base
		: public Join
	{
	public:
		typedef Base This;
		typedef Join Super;

		// destructor
		virtual ~Base() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();

	//	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_);

		virtual void require(Opt::Environment& cEnvironment_,
							 Scalar::Field* pField_);
		virtual void retrieve(Opt::Environment& cEnvironment_,
							  Scalar::Field* pField_);
		virtual void use(Opt::Environment& cEnvironment_,
						 Scalar::Field* pField_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_,
						   Scalar::DelayArgument& cArgument_);
	//	virtual void setPredicate(Interface::IPredicate* pPredicate_);
	//	virtual void setOrder(Order::Specification* pOrder_);

	//	virtual Interface::IPredicate* getPredicate();
	//	virtual Order::Specification* getOrder();

		virtual bool isReferingRelation(Interface::IRelation* pRelation_);
		virtual void createReferingRelation(Utility::RelationSet& cRelationSet_);
		virtual Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
											   Interface::IRelation* pRelation_);

	//	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
	//												   Execution::Interface::IProgram& cProgram_,
	//												   Candidate::AdoptArgument& cArgument_);
	//	virtual void generateDelayed(Opt::Environment& cEnvironment_,
	//								 Execution::Interface::IProgram& cProgram_,
	//								 Execution::Interface::IIterator* pIterator_);

	///////////////////////
	// Candidate::Join::
		virtual void setFirstPlan(Interface::ICandidate* pPlan_);
		virtual void createPlan(Opt::Environment& cEnvironment_,
								AccessPlan::Source& cPlanSource_,
								Interface::IRelation* pRelation_);

		virtual Interface::ICandidate* getFirstPlan();
		virtual Interface::ICandidate* getSecondPlan();

	protected:
		// constructor
		Base(Relation::Join* pJoin_)
			: Super(pJoin_),
			  m_cReferingRelation(),
			  m_mapTableCandidate(),
			  m_cResult(0, 0)
		{}

		// accessor
		PAIR<Interface::ICandidate*, Interface::ICandidate*>& getResult() {return m_cResult;}
	private:

		typedef MAP<Interface::IRelation*,
					Candidate::Table*,
					Utility::RelationSet::Comparator> TableCandidateMap;

		// calculate cost
		virtual void calculateCost(Opt::Environment& cEnvironment_,
								   const AccessPlan::Source& cPlanSource_,
								   const AccessPlan::Cost& cCost0_,
								   const AccessPlan::Cost& cCost1_,
								   AccessPlan::Cost& cResult_) = 0;

		// is exists join?
		virtual bool isExists() {return false;}

	/////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);
		virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);

		Utility::RelationSet m_cReferingRelation; // refering relation cache
		TableCandidateMap m_mapTableCandidate; // table candidate cache

		// candidate result
		PAIR<Interface::ICandidate*, Interface::ICandidate*> m_cResult;
	};

	/////////////////////////////////////////
	// CLASS
	//	Candidate::JoinImpl::NestedLoop -- implementation class for nested-loop join
	//
	// NOTES
	class NestedLoop
		: public Base
	{
	public:
		typedef NestedLoop This;
		typedef Base Super;

		// constructor
		NestedLoop(Relation::Join* pJoin_)
			: Super(pJoin_)
		{}

		// destructor
		~NestedLoop() {}

	/////////////////////////////
	// Interface::ICandidate::
		virtual Order::Specification* getOrder();
		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Candidate::AdoptArgument& cArgument_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_);

	protected:
	private:
	/////////////////////////////
	// Base::
		virtual void calculateCost(Opt::Environment& cEnvironment_,
								   const AccessPlan::Source& cPlanSource_,
								   const AccessPlan::Cost& cCost0_,
								   const AccessPlan::Cost& cCost1_,
								   AccessPlan::Cost& cResult_);
	};

	/////////////////////////////////////////
	// CLASS
	//	Candidate::JoinImpl::Exists -- implementation class for exists join
	//
	// NOTES
	class Exists
		: public Base
	{
	public:
		typedef Exists This;
		typedef Base Super;

		// constructor
		Exists(Relation::Join* pJoin_)
			: Super(pJoin_)
		{}

		// destructor
		~Exists() {}

	/////////////////////////////
	// Interface::ICandidate::
		virtual Order::Specification* getOrder();
		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Candidate::AdoptArgument& cArgument_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);
		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_);

	protected:
	private:
	/////////////////////////////
	// Base::
		virtual void calculateCost(Opt::Environment& cEnvironment_,
								   const AccessPlan::Source& cPlanSource_,
								   const AccessPlan::Cost& cCost0_,
								   const AccessPlan::Cost& cCost1_,
								   AccessPlan::Cost& cResult_);
		virtual bool isExists() {return true;}
	/////////////////////////
	// Candidate::Base::
		virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);
	};

} // namespace JoinImpl

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_JOIN_H

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
