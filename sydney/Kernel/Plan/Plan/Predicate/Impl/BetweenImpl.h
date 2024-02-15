// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/BetweenImpl.h --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_BETWEENIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_BETWEENIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/Between.h"

#include "Plan/Tree/Nadic.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::BetweenImpl -- betweenImpl predicate
	//
	// NOTES
	class BetweenImpl
		: public Base< Tree::Nadic<Predicate::Between, Interface::IScalar> >
	{
	public:
		typedef Base< Tree::Nadic<Predicate::Between, Interface::IScalar> > Super;
		typedef BetweenImpl This;

		BetweenImpl(const VECTOR<Interface::IScalar*>& vecOperand_,
					bool bIsNot_)
			: Super(vecOperand_),
			  m_bIsNot(bIsNot_)
		{}
		~BetweenImpl() {}

		bool isNot() {return m_bIsNot;}

	////////////////////////////
	// Predicate::Between::

	///////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate::RewriteResult
							rewrite(Opt::Environment& cEnvironment_,
									Interface::IRelation* pRelation_,
									Predicate::RewriteArgument& cArgument_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);

	///////////////////////////
	// Interface::IScalar::
		virtual bool isArbitraryElement();
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

	//////////////////////////////////////////
	// Tree::Node::Super::
	protected:
	private:
		bool m_bIsNot;
	};

} // namespace Impl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_BETWEENIMPL_H

//
//	Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
