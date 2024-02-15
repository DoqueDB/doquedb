// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Combinator.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Plan::Predicate";
}

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Predicate/Combinator.h"
#include "Plan/Predicate/Impl/CombinatorImpl.h"
#include "Plan/Tree/Dyadic.h"
#include "Plan/Tree/Nadic.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::Combinator

// FUNCTION public
//	Predicate::Combinator::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Interface::Type eOperator_
//	const VECTOR<Interface::IPredicate*>& vecOperand_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const VECTOR<Interface::IPredicate*>& vecOperand_)
{
	switch (vecOperand_.GETSIZE()) {
	case 0:
		{
			return 0;
		}
	case 1:
		{
			return create(cEnvironment_,
						  eOperator_,
						  *vecOperand_.begin());
		}
	case 2:
		{
			PAIR<Interface::IPredicate*, Interface::IPredicate*>
				cOperand(vecOperand_[0],
						 vecOperand_[1]);
			return create(cEnvironment_,
						  eOperator_,
						  cOperand);
		}
	default:
		{
			AUTOPOINTER<This> pResult;

			switch (eOperator_) {
			case Tree::Node::And:
				{
					pResult = new Impl::AndImpl< Tree::Nadic<This, Interface::IPredicate> >(vecOperand_);
					break;
				}
			case Tree::Node::Or:
				{
					pResult = new Impl::OrImpl< Tree::Nadic<This, Interface::IPredicate> >(vecOperand_);
					break;
				}
			case Tree::Node::Choice:
				{
					pResult = new Impl::ChoiceImpl< Tree::Nadic<This, Interface::IPredicate> >(vecOperand_);
					break;
				}
			default:
				{
					break;
				}
			}
			; _SYDNEY_ASSERT(pResult.get());

			pResult->registerToEnvironment(cEnvironment_);
			return pResult.release();
		}
	}
}

// FUNCTION public
//	Predicate::Combinator::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IPredicate*, Interface::IPredicate*>& cOperand_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IPredicate*, Interface::IPredicate*>& cOperand_)
{
	AUTOPOINTER<This> pResult;
	switch (eOperator_) {
	case Tree::Node::And:
		{
			pResult = new Impl::AndImpl< Tree::Dyadic<This, Interface::IPredicate> >(cOperand_);
			break;
		}
	case Tree::Node::Or:
		{
			pResult = new Impl::OrImpl< Tree::Dyadic<This, Interface::IPredicate> >(cOperand_);
			break;
		}
	case Tree::Node::Choice:
		{
			pResult = new Impl::ChoiceImpl< Tree::Dyadic<This, Interface::IPredicate> >(cOperand_);
			break;
		}
	default:
		{
			break;
		}
	}
	; _SYDNEY_ASSERT(pResult.get());

	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Combinator::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IPredicate* pOperand_
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Combinator::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IPredicate* pOperand_)
{
	AUTOPOINTER<This> pResult;

	switch (eOperator_) {
	case Tree::Node::And:
	case Tree::Node::Or:
	case Tree::Node::Choice:
		{
			return pOperand_;
		}
	case Tree::Node::Not:
		{
			// try to convert NOT into another predicate
			Plan::Interface::IPredicate* pAlternative = pOperand_->convertNot(cEnvironment_);
			if (pAlternative) {
				return pAlternative;
			}
			// no conversion -> use simple combinator
			pResult = new Impl::NotImpl(pOperand_);
			break;
		}
	default:
		{
			break;
		}
	}
	; _SYDNEY_ASSERT(pResult.get());

	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
