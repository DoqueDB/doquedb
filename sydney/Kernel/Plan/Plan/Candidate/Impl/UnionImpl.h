// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/UnionImpl.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_IMPL_UNION_H
#define __SYDNEY_PLAN_CANDIDATE_IMPL_UNION_H

#include "Plan/Candidate/Union.h"
#include "Plan/Candidate/Nadic.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Utility/ObjectSet.h"
#include "Plan/Order/Specification.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace UnionImpl
{
	//////////////////////////////////
	// CLASS
	//	Candidate::UnionImpl::Base -- base class of implementation class of union candidates
	//
	// NOTES
	class Base
		: public Nadic<Union>
	{
	public:
		typedef Base This;
		typedef Nadic<Union> Super;

		// destructor
		virtual ~Base() {}

	/////////////////////////////
	// Interface::ICandidate::
	//	virtual void createCost(Opt::Environment& cEnvironment_,
	//							const AccessPlan::Source& cPlanSource_);
	//	virtual const AccessPlan::Cost& getCost();

	//	virtual Candidate::Row* getRow(Opt::Environment& cEnvironment_);

	//	virtual void require(Scalar::Field* pField_);
	//	virtual void retrieve(Scalar::Field* pField_);
		virtual bool delay(Opt::Environment& cEnvironment_,
						   Scalar::Field* pField_,
						   Scalar::DelayArgument& cArgument_);
	//	virtual void setPredicate(Interface::IPredicate* pPredicate_);
	//	virtual void setOrder(Order::Specification* pOrder_);

	//	virtual Interface::IPredicate* getPredicate();
	//	virtual Order::Specification* getOrder();

	//	virtual bool isReferingRelation(Interface::IRelation* pRelation_);
	//	virtual void createReferingRelation(Utility::RelationSet& cRelationSet_);
	//	virtual Candidate::Table* getCandidate(Opt::Environment& cEnvironment_,
	//										   Interface::IRelation* pRelation_);

	//	virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
	//												   Execution::Interface::IProgram& cProgram_,
	//												   Candidate::AdoptArgument& cArgument_);
	//	virtual void generateDelayed(Opt::Environment& cEnvironment_,
	//								 Execution::Interface::IProgram& cProgram_,
	//								 Execution::Interface::IIterator* pIterator_);
		virtual InquiryResult inquiry(Opt::Environment& cEnvironment_,
									  const InquiryArgument& cArgument_);

	protected:
		// constructor
		Base(Relation::Union* pUnion_,
			 const VECTOR<Interface::ICandidate*>& vecOperand_)
			: Super(pUnion_, vecOperand_),
			  m_cDelayedColumn()
		{}

	private:
	/////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);
		virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);

		Utility::FieldSet m_cDelayedColumn; // delayable columns
	};

	/////////////////////////////////////////
	// CLASS
	//	Candidate::UnionImpl::Cascade -- implementation class for cascade union
	//
	// NOTES
	class Cascade
		: public Base
	{
	public:
		typedef Cascade This;
		typedef Base Super;

		// constructor
		Cascade(Relation::Union* pUnion_,
				const VECTOR<Interface::ICandidate*>& vecOperand_)
			: Super(pUnion_, vecOperand_)
		{}

		// destructor
		~Cascade() {}

	/////////////////////////////
	// Interface::ICandidate::
		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Candidate::AdoptArgument& cArgument_);
		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_);

	protected:
	private:
		void addOperand(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						Candidate::AdoptArgument& cArgument_,
						Interface::ICandidate* pOperand_,
						Candidate::Row* pRow_,
						int iResultID_,
						Execution::Interface::IIterator* pIterator_,
						Execution::Interface::ICollection* pDistinct_);
	};

		/////////////////////////////////////////
	// CLASS
	//	Candidate::UnionImpl::Sort -- implementation class for cascade union
	//
	// NOTES
	class Sort
		: public Base
	{
	public:
		typedef Sort This;
		typedef Base Super;

		// constructor
		Sort(Relation::Union* pUnion_,
			 const VECTOR<Interface::ICandidate*>& vecOperand_,
			 Order::Specification* pOrder_)
			: Super(pUnion_, vecOperand_)
		{
			m_pOrder = pOrder_;
		}

		// destructor
		~Sort() {}

	/////////////////////////////
	// Interface::ICandidate::
		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Candidate::AdoptArgument& cArgument_);
		virtual void generateDelayed(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_);
		virtual Order::Specification* getOrder() {return m_pOrder;}

	protected:
	private:
		void addOperand(Opt::Environment& cEnvironment_,
						Execution::Interface::IProgram& cProgram_,
						Candidate::AdoptArgument& cArgument_,
						Interface::ICandidate* pOperand_,
						Candidate::Row* pRow_,
						int iResultID_,
						Execution::Interface::IIterator* pIterator_,
						Execution::Interface::ICollection* pDistinct_);
		
		Order::Specification* m_pOrder;
	};

} // namespace UnionImpl

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_UNION_H

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
