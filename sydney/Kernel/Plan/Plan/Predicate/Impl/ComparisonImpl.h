// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/ComparisonImpl.h --
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_COMPARISONIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_COMPARISONIMPL_H

#include "boost/bind.hpp"

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/Comparison.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Monadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	////////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::DyadicComparison -- comparison predicate with two operands
	//
	// NOTES
	class DyadicComparison
		: public Impl::Base< Tree::Dyadic<Predicate::Comparison, Interface::IScalar> >
	{
	public:
		typedef Impl::Base< Tree::Dyadic<Predicate::Comparison, Interface::IScalar> > Super;
		typedef DyadicComparison This;

		DyadicComparison(Tree::Node::Type eType_,
						 const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cOperand_)
		{
			setArgument(eType_);
		}
#else
			: Super(eType_, cOperand_)
		{}
#endif
		~DyadicComparison() {}

	/////////////////////////////
	// Interface::IPredicate::
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_,
	//						  Interface::ICandidate* pCandidate_);
	//	virtual bool delay(Opt::Environment& cEnvironment_,
	//					   Interface::ICandidate* pCandidate_,
	//					   Scalar::DelayArgument& cArgument_);
		virtual Interface::IPredicate* convertNot(Opt::Environment& cEnvironment_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual Interface::IPredicate* createFetch(Opt::Environment& cEnvironment_,
												   Utility::ScalarSet& cFetchKey_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);

	///////////////////////////
	// Interface::IScalar::
	//	virtual bool hasParameter();
		virtual bool isArbitraryElement();
	//	virtual void getUsedTable(Utility::RelationSet& cResult_);
	//	virtual void getUsedField(Utility::FieldSet& cResult_);
	//	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
	//							   Predicate::CheckUnknownArgument& cResult_);
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;

		virtual void setParameter(Opt::Environment& cEnvironment_,
								  Execution::Interface::IProgram& cProgram_,
								  Execution::Interface::IIterator* pIterator_,
								  DExecution::Action::StatementConstruction& cExec,
								  const Plan::Sql::QueryArgument& cArgument_);

	protected:
	private:
	};

	////////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::DyadicRowComparison -- comparison predicate with two operands
	//
	// NOTES
	class DyadicRowComparison
		: public Impl::Base< Tree::Dyadic<Predicate::Comparison, Interface::IRow> >
	{
	public:
		typedef Impl::Base< Tree::Dyadic<Predicate::Comparison, Interface::IRow> > Super;
		typedef DyadicComparison This;

		DyadicRowComparison(Tree::Node::Type eType_,
							const PAIR<Interface::IRow*, Interface::IRow*>& cOperand_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(cOperand_)
		{
			setArgument(eType_);
		}
#else
			: Super(eType_, cOperand_)
		{}
#endif
		~DyadicRowComparison() {}

	/////////////////////////////
	// Interface::IPredicate::
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_,
	//						  Interface::ICandidate* pCandidate_);
	//	virtual bool delay(Opt::Environment& cEnvironment_,
	//					   Interface::ICandidate* pCandidate_,
	//					   Scalar::DelayArgument& cArgument_);
		virtual Interface::IPredicate* convertNot(Opt::Environment& cEnvironment_);
	//	virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
	//										 const CheckArgument& cArgument_);
	//	virtual Interface::IPredicate* createFetch(Opt::Environment& cEnvironment_,
	//											   Utility::ScalarSet& cFetchKey_);

	///////////////////////////
	// Interface::IScalar::
	//	virtual bool hasParameter();
		virtual bool isArbitraryElement();
	//	virtual void getUsedTable(Utility::RelationSet& cResult_);
	//	virtual void getUsedField(Utility::FieldSet& cResult_);
	//	virtual void getUnknownKey(Opt::Environment& cEnvironment_,
	//							   Predicate::CheckUnknownArgument& cResult_);
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;		

	protected:
	private:
	};

	/////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::MonadicComparison -- comparison predicate with two operands
	//
	// NOTES
	class MonadicComparison
		: public Impl::Base< Tree::Monadic<Predicate::Comparison, Interface::IScalar> >
	{
	public:
		typedef Impl::Base< Tree::Monadic<Predicate::Comparison, Interface::IScalar> > Super;
		typedef MonadicComparison This;

		MonadicComparison(Tree::Node::Type eType_,
						  Interface::IScalar* pOperand_)
#ifdef SYD_CC_NO_IMPLICIT_INHERIT_FUNCTION
			: Super(pOperand_)
		{
			setArgument(eType_);
		}
#else
			: Super(eType_, pOperand_)
		{}
#endif
		~MonadicComparison() {}

	////////////////////////////////
	// Interface::IPredicate::
	//	virtual void require(Opt::Environment& cEnvironment_,
	//						 Interface::ICandidate* pCandidate_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_);
	//	virtual void retrieve(Opt::Environment& cEnvironment_,
	//						  Interface::ICandidate* pCandidate_);
	//	virtual bool delay(Opt::Environment& cEnvironment_,
	//					   Interface::ICandidate* pCandidate_,
	//					   Scalar::DelayArgument& cArgument_);
		virtual Interface::IPredicate* convertNot(Opt::Environment& cEnvironment_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);

	///////////////////////////
	// Interface::IScalar::
	//	virtual bool hasParameter();
		virtual bool isArbitraryElement();
	//	virtual void getUsedTable(Utility::RelationSet& cResult_);
	//	virtual void getUsedField(Utility::FieldSet& cResult_);
		virtual void getUnknownKey(Opt::Environment& cEnvironment_,
								   Predicate::CheckUnknownArgument& cResult_);
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;


	protected:
	private:
	};
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_COMPARISONIMPL_H

//
//	Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
