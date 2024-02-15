// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Candidate/Impl/BulkImpl.h --
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

#ifndef __SYDNEY_PLAN_CANDIDATE_IMPL_BULKIMPL_H
#define __SYDNEY_PLAN_CANDIDATE_IMPL_BULKIMPL_H

#include "Plan/Candidate/Bulk.h"
#include "Plan/Candidate/Monadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_CANDIDATE_BEGIN

namespace BulkImpl
{
	/////////////////////////////////////////////////
	// CLASS
	//	Candidate::BulkImpl::Base --
	//
	// NOTES
	class Base
		: public Candidate::Bulk
	{
	public:
		typedef Candidate::Bulk Super;
		typedef Base This;

		// destructor
		virtual ~Base() {}

	protected:
		// constructor
		Base(Relation::Bulk* pRelation_)
			: Super(),
			  m_pRelation(pRelation_)
		{}
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
		Base()
			: Super(),
			  m_pRelation(0)
		{}
		void setArgument(Relation::Bulk* pRelation_) {m_pRelation = pRelation_;}
#endif

		// accessor
		Relation::Bulk* getRelation() {return m_pRelation;}

		// create bulk collection
		Execution::Interface::ICollection*
				createBulkCollection(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Execution::Interface::IIterator* pIterator_,
									 Candidate::AdoptArgument& cArgument_,
									 bool bInput_);
	private:
		Relation::Bulk* m_pRelation;
	};

	/////////////////////////////////////////////////
	// CLASS
	//	Candidate::BulkImpl::Input --
	//
	// NOTES
	class Input
		: public Base
	{
	public:
		typedef Base Super;
		typedef Input This;

		// constructor
		Input(Relation::Bulk* pRelation_)
			: Super(pRelation_)
		{}
		// destructor
		~Input() {}

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
	// 	virtual bool isReferingRelation(Interface::IRelation* pRelation_);
	//	virtual void createReferingRelation(Utility::RelationSet& cRelationSet_);
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
		AccessPlan::Cost::Value getTupleSize(Opt::Environment& cEnvironment_);
		AccessPlan::Cost::Value getTupleCount(Opt::Environment& cEnvironment_);

	/////////////////////////////
	// Candidate::Base::
		virtual void createCost(Opt::Environment& cEnvironment_,
								const AccessPlan::Source& cPlanSource_,
								AccessPlan::Cost& cCost_);
		virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);
	};

	/////////////////////////////////////////////////
	// CLASS
	//	Candidate::BulkImpl::Output --
	//
	// NOTES
	class Output
		: public Monadic<Base>
	{
	public:
		typedef Monadic<Base> Super;
		typedef Output This;

		// constructor
		Output(Relation::Bulk* pRelation_,
			   Relation::RowInfo* pRowInfo_,
			   Interface::ICandidate* pOperand_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pOperand_),
			  m_pRowInfo(pRowInfo_)
		{
			setArgument(pRelation_);
		}
#else
			: Super(pRelation_, pOperand_),
			  m_pRowInfo(pRowInfo_)
		{}
#endif
		// destructor
		~Output() {}

	/////////////////////////////
	// Interface::ICandidate::
		virtual Execution::Interface::IIterator*
						adopt(Opt::Environment& cEnvironment_,
							  Execution::Interface::IProgram& cProgram_,
							  Candidate::AdoptArgument& cArgument_);
	protected:
	private:
	/////////////////////////////
	// Interface::ICandidate::
		virtual Candidate::Row* createRow(Opt::Environment& cEnvironment_);
		virtual Candidate::Row* createKey(Opt::Environment& cEnvironment_);

		Relation::RowInfo* m_pRowInfo;
	};

} // namespace BulkImpl

_SYDNEY_PLAN_CANDIDATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_CANDIDATE_IMPL_BULKIMPL_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
