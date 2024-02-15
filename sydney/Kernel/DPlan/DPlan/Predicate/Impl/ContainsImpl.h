// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ContainsImpl.h --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_DPLAN_PREDICATE_IMPL_CONTAINSIMPL_H
#define __SYDNEY_DPLAN_PREDICATE_IMPL_CONTAINSIMPL_H

#include "DPlan/Predicate/Module.h"
#include "DPlan/Predicate/Contains.h"
#include "Plan/Predicate/Impl/Base.h"


#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Tree/Option.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_PREDICATE_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::ContainsImpl -- containsImpl predicate
	//
	// NOTES
	class ContainsImpl
		: public Plan::Predicate::Impl::BaseWithOption< Plan::Tree::Dyadic<Predicate::Contains, Plan::Interface::IScalar>,
														Plan::Tree::Nadic<Plan::Tree::Option, Plan::Interface::IScalar> >
	{
	public:
		typedef Plan::Predicate::Impl::BaseWithOption< Plan::Tree::Dyadic<Predicate::Contains, Plan::Interface::IScalar>,
													   Plan::Tree::Nadic<Plan::Tree::Option, Plan::Interface::IScalar> > Super;
		typedef ContainsImpl This;

		ContainsImpl(const PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*>& cOperand_,
					 const VECTOR<Plan::Interface::IScalar*>& vecOption_)
			: Super(cOperand_, vecOption_),
			  m_pExpand(0),
			  m_pRankFrom(0),
			  m_pKwicOption(0),
			  m_pAdoptResult(0),
			  m_iOutData(-1)
		{}
		~ContainsImpl() {}

	////////////////////////////
	// Predicate::Contains::
		virtual void setExpand(Plan::Interface::IRelation* pQuery_)
		{m_pExpand = pQuery_;}
		virtual void setRankFrom(Plan::Interface::IRelation* pQuery_)
		{m_pRankFrom = pQuery_;}

		virtual void createKwicOption(Opt::Environment& cEnvironment_,
									  Plan::Interface::IScalar* pKwicSize_);
		virtual Plan::File::KwicOption* getKwicOption() {return m_pKwicOption;}

	///////////////////////////////
	// Interface::IPredicate::

		virtual Plan::Interface::ICandidate* createDistributePlan(Opt::Environment& cEnvironment_,
																  Plan::Interface::ICandidate* pOperand_,
																  Plan::Utility::FieldSet& cPlanSource_);

	///////////////////////////
	// Interface::IScalar::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	//////////////////////////////////////////
	// Tree::Node::Super::
		virtual ModSize getOptionSize() const;
		virtual const Plan::Tree::Node::Super* getOptionAt(ModInt32 iPosition_) const;


	//////////////////////////////////////////
	// DPlan::Predicate::Contains
		virtual Schema::File* getFulltextIndex(Opt::Environment& cEnvironment_);
		
		virtual Plan::Interface::IScalar* getColumn();
		virtual Plan::Interface::IRelation* getExpand() { return m_pExpand; }

		virtual Execution::Interface::IIterator* adopt(Opt::Environment& cEnvironment_,
													   Execution::Interface::IProgram& cProgram_,
													   Plan::Candidate::AdoptArgument& cArgument_);

	protected:
	private:

	/////////////////////////////////////////
	// Predicate::Contains::
		virtual void addToEnvironment(Opt::Environment& cEnvironment_);

		bool isNeedWordExtraction(const Plan::Utility::FieldSet& cFieldSet__);

	/////////////////////////////////////////
	// DPlan::Predicate::Impl::ContainsImpl::
		Execution::Interface::IIterator* adoptExpand(Opt::Environment& cEnvironment_,
													 Execution::Interface::IProgram& cProgram_,
													 Plan::Candidate::AdoptArgument& cArgument_,
													 Execution::Interface::IIterator* pIterator_);
		Execution::Interface::IIterator* addDFCountIterator(Opt::Environment& cEnvironment_,
															Execution::Interface::IProgram& cProgram_,
															Plan::Candidate::AdoptArgument& cArgument_,
															Execution::Interface::IIterator* pIterator_,
															int iTermDataID_,
															int iFulltextID_,
															VECTOR<int>& vecData);
		Execution::Interface::IIterator*
		addDFCountIteratorByWordList(Opt::Environment& cEnvironment_,
									 Execution::Interface::IProgram& cProgram_,
									 Plan::Candidate::AdoptArgument& cArgument_,
									 Execution::Interface::IIterator* pIterator_,
									 int iTermDataID_,
									 int iFulltextID_,
									 VECTOR<int>& vecData);
		
		
		Plan::Interface::IRelation* m_pExpand;
		Plan::Interface::IRelation* m_pRankFrom;
		Plan::File::KwicOption* m_pKwicOption;
		Execution::Interface::IIterator* m_pAdoptResult;
		int m_iOutData;
	};

} // namespace Impl

_SYDNEY_DPLAN_PREDICATE_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_PREDICATE_IMPL_CONTAINSIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
