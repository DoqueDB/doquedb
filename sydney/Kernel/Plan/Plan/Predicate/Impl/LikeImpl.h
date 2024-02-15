// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/LikeImpl.h --
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_LIKEIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_LIKEIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/Like.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Nadic.h"
#include "Plan/Tree/Option.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::LikeImpl -- likeImpl predicate
	//
	// NOTES
	class LikeImpl
		: public BaseWithOption< Tree::Dyadic<Predicate::Like, Interface::IScalar>,
								 Tree::Nadic<Tree::Option, Interface::IScalar> >
	{
	public:
		typedef BaseWithOption< Tree::Dyadic<Predicate::Like, Interface::IScalar>,
								Tree::Nadic<Tree::Option, Interface::IScalar> > Super;
		typedef LikeImpl This;

		LikeImpl(const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
				 const VECTOR<Interface::IScalar*>& vecOption_,
				 bool bIsNot_)
			: Super(cOperand_, vecOption_),
			  m_bIsNot(bIsNot_)
		{
			if (m_bIsNot) setNodeType(Tree::Node::NotLike);
		}
		~LikeImpl() {}

	////////////////////////////
	// Predicate::Like::

	///////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate::RewriteResult
							rewrite(Opt::Environment& cEnvironment_,
									Interface::IRelation* pRelation_,
									Predicate::RewriteArgument& cArgument_);
		virtual Interface::IPredicate* convertNot(Opt::Environment& cEnvironment_);
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);

	///////////////////////////
	// Interface::IScalar::
		virtual bool hasParameter();
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

	protected:
	private:
		bool m_bIsNot;
	};

} // namespace Impl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_LIKEIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
