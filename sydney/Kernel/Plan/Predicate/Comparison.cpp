// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Comparison.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
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

#include "Plan/Predicate/Comparison.h"
#include "Plan/Predicate/Impl/ComparisonImpl.h"
#include "Plan/Predicate/Combinator.h"
#include "Plan/Interface/IRow.h"
#include "Plan/Scalar/DataType.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Exception/NotComparable.h"
#include "Exception/NotSupported.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_PREDICATE_BEGIN

////////////////////////////////////
//	Plan::Predicate::Comparison

// FUNCTION public
//	Predicate::Comparison::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_
//	bool bCheckComparability_ = true
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IScalar*, Interface::IScalar*>& cOperand_,
	   bool bCheckComparability_ /* = true */)
{
	; _SYDNEY_ASSERT(isDyadic(eOperator_));

	PAIR<Interface::IScalar*, Interface::IScalar*> cOperand(cOperand_);
	if (bCheckComparability_) {
		checkComparability(cEnvironment_,
						   eOperator_,
						   &cOperand.first,
						   &cOperand.second);
	}

	AUTOPOINTER<This> pResult =
		new Impl::DyadicComparison(eOperator_, cOperand);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar* pOperand_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Interface::IScalar* pOperand_)
{
	; _SYDNEY_ASSERT(isMonadic(eOperator_));

	AUTOPOINTER<This> pResult =
		new Impl::MonadicComparison(eOperator_, pOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR< VECTOR<Interface::IScalar*>, VECTOR<Interface::IScalar*> >& cOperand_
//	bool bCheckComparability_ = true
//	
// RETURN
//	Interface::IPredicate*
//
// EXCEPTIONS

//static
Interface::IPredicate*
Comparison::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR< VECTOR<Interface::IScalar*>, VECTOR<Interface::IScalar*> >& cOperand_,
	   bool bCheckComparability_ /* = true */)
{
	; _SYDNEY_ASSERT(isDyadic(eOperator_));
	if (cOperand_.first.GETSIZE() != cOperand_.second.GETSIZE()) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	if (cOperand_.first.GETSIZE() == 1) {
		return create(cEnvironment_,
					  eOperator_,
					  MAKEPAIR(cOperand_.first[0], cOperand_.second[0]),
					  bCheckComparability_);
	}
	if (eOperator_ == Tree::Node::Equals) {
		VECTOR<Interface::IPredicate*> vecOperand;
		vecOperand.reserve(cOperand_.first.GETSIZE());
		VECTOR<Interface::IScalar*>::CONSTITERATOR iterator0 = cOperand_.first.begin();
		const VECTOR<Interface::IScalar*>::CONSTITERATOR last0 = cOperand_.first.end();
		VECTOR<Interface::IScalar*>::CONSTITERATOR iterator1 = cOperand_.second.begin();
		const VECTOR<Interface::IScalar*>::CONSTITERATOR last1 = cOperand_.second.end();
		for (; iterator0 != last0; ++iterator0, ++iterator1) {
			_SYDNEY_ASSERT(iterator1 != last1);
			vecOperand.PUSHBACK(Comparison::create(cEnvironment_,
												   eOperator_,
												   MAKEPAIR(*iterator0, *iterator1),
												   bCheckComparability_));
		}
		return Combinator::create(cEnvironment_,
								  Tree::Node::And,
								  vecOperand);
	}

	VECTOR<Interface::IScalar*> vecOperand0(cOperand_.first);
	VECTOR<Interface::IScalar*> vecOperand1(cOperand_.second);
	if (bCheckComparability_) {
		VECTOR<Interface::IScalar*>::ITERATOR iterator0 = vecOperand0.begin();
		const VECTOR<Interface::IScalar*>::ITERATOR last0 = vecOperand0.end();
		VECTOR<Interface::IScalar*>::ITERATOR iterator1 = vecOperand1.begin();
		const VECTOR<Interface::IScalar*>::ITERATOR last1 = vecOperand1.end();
		for (; iterator0 != last0; ++iterator0, ++iterator1) {
			_SYDNEY_ASSERT(iterator1 != last1);

			checkComparability(cEnvironment_,
							   eOperator_,
							   &(*iterator0),
							   &(*iterator1));
		}
	}
	Interface::IRow* pOperand0 = Interface::IRow::create(cEnvironment_,
														 vecOperand0);
	Interface::IRow* pOperand1 = Interface::IRow::create(cEnvironment_,
														 vecOperand1);

	return create(cEnvironment_,
				  eOperator_,
				  MAKEPAIR(pOperand0, pOperand1));
}

// FUNCTION public
//	Predicate::Comparison::create -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	const PAIR<Interface::IRow*
//	Interface::IRow*>& cOperand_
//	
// RETURN
//	Comparison*
//
// EXCEPTIONS

//static
Comparison*
Comparison::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   const PAIR<Interface::IRow*, Interface::IRow*>& cOperand_)
{
	AUTOPOINTER<This> pResult =
		new Impl::DyadicRowComparison(eOperator_,
									  cOperand_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}

// FUNCTION public
//	Predicate::Comparison::isDyadic -- check availability
//
// NOTES
//
// ARGUMENTS
//	Tree::Node::Type eOperator_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Comparison::
isDyadic(Tree::Node::Type eOperator_)
{
	return (eOperator_ == Tree::Node::Equals
			|| eOperator_ == Tree::Node::LessThanEquals
			|| eOperator_ == Tree::Node::GreaterThanEquals
			|| eOperator_ == Tree::Node::LessThan
			|| eOperator_ == Tree::Node::GreaterThan
			|| eOperator_ == Tree::Node::NotEquals);
}

// FUNCTION public
//	Predicate::Comparison::isMonadic -- check availability
//
// NOTES
//
// ARGUMENTS
//	Tree::Node::Type eOperator_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Comparison::
isMonadic(Tree::Node::Type eOperator_)
{
	return (eOperator_ == Tree::Node::NotNull
			|| eOperator_ == Tree::Node::EqualsToNull);
}

// FUNCTION public
//	Predicate::Comparison::checkComparability -- check comparability
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Interface::IScalar** ppOperand0_
//	Interface::IScalar** ppOperand1_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Comparison::
checkComparability(Opt::Environment& cEnvironment_,
				   Tree::Node::Type eOperator_,
				   Interface::IScalar** ppOperand0_,
				   Interface::IScalar** ppOperand1_)
{
	bool bResult = false;

	// dyadic comparison has to have comparable operands
	if (!Scalar::DataType::isComparable((**ppOperand0_).getDataType(),
										(**ppOperand1_).getDataType())) {
		Scalar::DataType cMostType;
		if (!Scalar::DataType::getCompatibleType((**ppOperand0_).getDataType(),
												 (**ppOperand1_).getDataType(),
												 cMostType,
												 true /* for comparison */)) {
			// Can't compared each other
			_SYDNEY_THROW0(Exception::NotComparable);
		}
		// replace operands to casted scalars if needed
		if (!Scalar::DataType::isComparable((**ppOperand0_).getDataType(),
											cMostType)) {
			*ppOperand0_ = (*ppOperand0_)->createCast(cEnvironment_,
													  cMostType,
													  true /* for comparison */);
			bResult = true;
		}
		if (!Scalar::DataType::isComparable((**ppOperand1_).getDataType(),
											cMostType)) {
			*ppOperand1_ = (*ppOperand1_)->createCast(cEnvironment_,
													  cMostType,
													  true /* for comparison */);
			bResult = true;
		}
	}
	return bResult;
}

_SYDNEY_PLAN_PREDICATE_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2008, 2009, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
