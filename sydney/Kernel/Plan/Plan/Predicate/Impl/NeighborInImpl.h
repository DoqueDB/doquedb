// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/NeighborInImpl.h --
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_NEIGHBORINIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_NEIGHBORINIMPL_H

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/NeighborIn.h"

#include "Exception/NotSupported.h"

#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace NeighborInImpl
{
	///////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::NeighborInImpl::Nadic -- neighborIn predicate
	//
	// NOTES
	class Nadic
		: public Impl::Base< Tree::Nadic<Predicate::NeighborIn, Interface::IScalar> >
	{
	public:
		typedef Impl::Base< Tree::Nadic<Predicate::NeighborIn, Interface::IScalar> > Super;
		typedef Nadic This;

		Nadic(const VECTOR<Interface::IScalar*>& vecOperand_)
			: Super(vecOperand_)
		{}
		virtual ~Nadic() {}

	///////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
	///////////////////////////
	// Interface::IScalar::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_);
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;		
	protected:
		virtual void explainKey(Opt::Environment* pEnvironment_,
								Opt::Explain& cExplain_);
		virtual void explainOperand(Opt::Environment* pEnvironment_,
									Opt::Explain& cExplain_);
	private:
	};

	///////////////////////////////////////////////////////
	// CLASS
	//	Plan::Predicate::NeighborInImpl::NadicWithOption -- neighborIn predicate
	//
	// NOTES
	class NadicWithOption
		: public Tree::MonadicOption< Nadic, Interface::IScalar >
	{
	public:
		typedef Tree::MonadicOption< Nadic, Interface::IScalar > Super;
		typedef NadicWithOption This;

		NadicWithOption(const VECTOR<Interface::IScalar*>& vecOperand_,
						Interface::IScalar* pOption_)
			: Super(vecOperand_, pOption_)
		{}
		~NadicWithOption() {}

	///////////////////////////////
	// Interface::IPredicate::
		virtual Interface::IPredicate* check(Opt::Environment& cEnvironment_,
											 const CheckArgument& cArgument_);
	///////////////////////////
	// Interface::IScalar::
	//	virtual void explain(Opt::Environment* pEnvironment_,
	//						 Opt::Explain& cExplain_);
	//	virtual int generate(Opt::Environment& cEnvironment_,
	//						 Execution::Interface::IProgram& cProgram_,
	//						 Execution::Interface::IIterator* pIterator_,
	//						 Candidate::AdoptArgument& cArgument_);
		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const;		
	};

} // namespace NeighborInImpl

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_NEIGHBORINIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
