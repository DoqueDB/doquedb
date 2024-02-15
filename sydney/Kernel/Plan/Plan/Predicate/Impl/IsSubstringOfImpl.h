// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/IsSubstringOfImpl.h --
// 
// Copyright (c) 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_ISSUBSTRINGOFIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_ISSUBSTRINGOFIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/IsSubstringOf.h"

#include "Plan/Tree/Dyadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::Impl::IsSubstringOfImpl -- isSubstringOfImpl predicate
	//
	// NOTES
	class IsSubstringOfImpl
		: public Base< Tree::Dyadic<Predicate::IsSubstringOf, Interface::IScalar> >
	{
	public:
		typedef Base< Tree::Dyadic<Predicate::IsSubstringOf, Interface::IScalar> > Super;
		typedef IsSubstringOfImpl This;

		IsSubstringOfImpl(const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_)
			: Super(cOperand_)
		{}
		~IsSubstringOfImpl() {}

	////////////////////////////
	// Predicate::IsSubstringOf::

	///////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
		virtual bool estimateCost(Opt::Environment& cEnvironment_,
								  AccessPlan::Cost& cResult_);

	///////////////////////////
	// Interface::IScalar::
		virtual bool hasParameter();
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

} // namespace Impl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_ISSUBSTRINGOFIMPL_H

//
//	Copyright (c) 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
