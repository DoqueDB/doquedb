// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Combinator.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Predicate/Combinator.h"

#include "Common/Assert.h"

#include "Exception/InvalidRowValue.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/Combinator.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::AndImpl --
	//
	// NOTES
	class AndImpl
		: public Predicate::Combinator
	{
	public:
		typedef AndImpl This;
		typedef Predicate::Combinator Super;

		// constructor
		AndImpl() : Super() {}
		// destructor
		virtual ~AndImpl() {}

		// generate Predicate from Statement::Object
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const;
	protected:
	private:
	};

	//////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::OrImpl --
	//
	// NOTES
	class OrImpl
		: public Predicate::Combinator
	{
	public:
		typedef OrImpl This;
		typedef Predicate::Combinator Super;

		// constructor
		OrImpl() : Super() {}
		// destructor
		virtual ~OrImpl() {}

		// generate Predicate from Statement::Object
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const;
	protected:
	private:
	};

	//////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::NotImpl --
	//
	// NOTES
	class NotImpl
		: public Predicate::Combinator
	{
	public:
		typedef NotImpl This;
		typedef Predicate::Combinator Super;

		// constructor
		NotImpl() : Super() {}
		// destructor
		virtual ~NotImpl() {}

		// generate Predicate from Statement::Object
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES
	const Impl::AndImpl _analyzerAnd;
	const Impl::OrImpl _analyzerOr;
	const Impl::NotImpl _analyzerNot;
}

//////////////////////////////////////////////////
// Analysis::Predicate::Impl::AndImpl

// FUNCTION public
//	Predicate::Impl::AndImpl::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

Plan::Interface::IPredicate*
Impl::AndImpl::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::ValueExpressionList* pVEL = pVE->getOperandList();
	int n = pVEL->getCount();

	VECTOR<Plan::Interface::IPredicate*> vecOperand(n);
	bool bHasOr = false;
	for (int i = 0; i < n; ++i) {
		Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(i);
		if (pElement->getOperator() == Statement::ValueExpression::op_Or) {
			bHasOr = true;
		}
		vecOperand[i] = pElement->getAnalyzer2()->getPredicate(cEnvironment_,
															   pRelation_,
															   pElement);
	}
	return Plan::Predicate::Combinator::create(cEnvironment_,
											   Plan::Tree::Node::And,
											   vecOperand);
}

//////////////////////////////////////////////////
// Analysis::Predicate::Impl::OrImpl

// FUNCTION public
//	Predicate::Impl::OrImpl::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

Plan::Interface::IPredicate*
Impl::OrImpl::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::ValueExpressionList* pVEL = pVE->getOperandList();
	int n = pVEL->getCount();

	VECTOR<Plan::Interface::IPredicate*> vecOperand(n);

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.pushStatus(Opt::Environment::Status::NoTopPredicate);

	bool bHasOr = false;
	for (int i = 0; i < n; ++i) {
		Statement::ValueExpression* pElement = pVEL->getValueExpressionAt(i);
		vecOperand[i] = pElement->getAnalyzer2()->getPredicate(cEnvironment_,
															   pRelation_,
															   pElement);
	}
	return Plan::Predicate::Combinator::create(cEnvironment_,
											   Plan::Tree::Node::Or,
											   vecOperand);
}

//////////////////////////////////////////////////
// Analysis::Predicate::Impl::NotImpl

// FUNCTION public
//	Predicate::Impl::NotImpl::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

Plan::Interface::IPredicate*
Impl::NotImpl::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	Statement::ValueExpression* pElement = pVE->getLeft();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.pushStatus(Opt::Environment::Status::NoTopPredicate);

	Plan::Interface::IPredicate* pOperand =
		pElement->getAnalyzer2()->getPredicate(cEnvironment_,
											   pRelation_,
											   pElement);
	return Plan::Predicate::Combinator::create(cEnvironment_,
											   Plan::Tree::Node::Not,
											   pOperand);
}

//////////////////////////////
// Predicate::Combinator::

// FUNCTION public
//	Predicate::Combinator::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Combinator*
//
// EXCEPTIONS

//static
const Combinator*
Combinator::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_And:
		{
			return &_analyzerAnd;
		}
	case Statement::ValueExpression::op_Or:
		{
			return &_analyzerOr;
		}
	case Statement::ValueExpression::op_Not:
		{
			return &_analyzerNot;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
