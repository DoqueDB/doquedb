// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Impl/CheckUnknownImpl.h --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_PREDICATE_IMPL_CHECKUNKNOWNIMPL_H
#define __SYDNEY_PLAN_PREDICATE_IMPL_CHECKUNKNOWNIMPL_H

#include "boost/bind.hpp"

#include "Exception/NotSupported.h"

#include "Plan/Predicate/Impl/Base.h"
#include "Plan/Predicate/CheckUnknown.h"

#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Monadic.h"
#include "Plan/Tree/Nadic.h"

#include "Opt/Explain.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

namespace CheckUnknownImpl
{
	////////////////////////////////////////////////////////////
	// TEMPLATE CLASS
	//	Predicate::CheckUnknownImpl::Base -- base class of checkUnknown implementation
	//
	// TEMPLATE ARGUMENT
	//	class Handle_
	//
	// NOTES
	template <class Handle_>
	class Base
		: public Impl::Base<Handle_>
	{
	public:
		typedef Impl::Base<Handle_> Super;
		typedef Base<Handle_> This;

		typedef typename Handle_::Operand Operand;

		virtual ~Base() {}

	///////////////////////////////
	// Interface::IPredicate::

	///////////////////////////
	// Interface::IScalar::
		virtual void explain(Opt::Environment* pEnvironment_,
							 Opt::Explain& cExplain_)
		{
			cExplain_.put("check unknown(");
			joinOperand(boost::bind(&Operand::explain,
									_1,
									pEnvironment_,
									boost::ref(cExplain_)),
						boost::bind(&Opt::Explain::putChar,
									&cExplain_,
									','));
			cExplain_.put(")");
		}

		virtual STRING toSQLStatement(Opt::Environment& cEnvironment_,
									  const Plan::Sql::QueryArgument& cArgument_) const
		{
			_SYDNEY_THROW0(Exception::NotSupported);
		}
			
		using Handle_::joinOperand;


	/////////////////////////////
	// Tree::Node::Interface::
	protected:
		// constructor
		Base(typename Handle_::Argument cArgument_,
			 bool bArray_)
			: Super(cArgument_),
			  m_bArray(bArray_)
		{}

		// accessor
		bool isArray() {return m_bArray;}

	private:
		bool m_bArray;
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Predicate::CheckUnknownImpl::Monadic -- monadic checkUnknown implementation
	//
	// NOTES
	class Monadic
		: public Base< Tree::Monadic<CheckUnknown, Interface::IScalar> >
	{
	public:
		typedef Base< Tree::Monadic<CheckUnknown, Interface::IScalar> > Super;
		typedef Monadic This;

		Monadic(Argument pOperand_, bool bArray_)
			: Super(pOperand_, bArray_)
		{}
		virtual ~Monadic() {}

	///////////////////////////////
	// Interface::IPredicate::

	///////////////////////////
	// Interface::IScalar::
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

	/////////////////////////////
	// Tree::Node::Interface::
	protected:
	private:
	};

	////////////////////////////////////////////////////////////
	// CLASS
	//	Predicate::CheckUnknownImpl::Nadic -- N-adic checkUnknown implementation
	//
	// NOTES
	class Nadic
		: public Base< Tree::Nadic<CheckUnknown, Interface::IScalar> >
	{
	public:
		typedef Base< Tree::Nadic<CheckUnknown, Interface::IScalar> > Super;
		typedef Nadic This;

		Nadic(Argument vecOperand_, bool bArray_)
			: Super(vecOperand_, bArray_)
		{}
		virtual ~Nadic() {}

	///////////////////////////////
	// Interface::IPredicate::

	///////////////////////////
	// Interface::IScalar::
		virtual int generate(Opt::Environment& cEnvironment_,
							 Execution::Interface::IProgram& cProgram_,
							 Execution::Interface::IIterator* pIterator_,
							 Candidate::AdoptArgument& cArgument_);

	/////////////////////////////
	// Tree::Node::Interface::
	protected:
	private:
	};
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_PREDICATE_IMPL_CHECKUNKNOWNIMPL_H

//
//	Copyright (c) 2010, 2011, 2012, 2013, 2023, 2024 Ricoh Company, Ltd.
//	All rights reserved.
//
