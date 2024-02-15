// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Comparison.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Analysis::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Predicate/Comparison.h"

#include "Common/Assert.h"

#include "Exception/InvalidRowValue.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/Comparison.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::Dyadic --
	//
	// NOTES
	class Dyadic
		: public Predicate::Comparison
	{
	public:
		typedef Dyadic This;
		typedef Predicate::Comparison Super;

		// constructor
		Dyadic() : Super() {}
		// destructor
		virtual ~Dyadic() {}

		// generate Predicate from Statement::Object
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const;
	protected:
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Plan::Tree::Node::Type eOperator_,
							 Statement::ValueExpression* pLeft_,
							 Statement::ValueExpression* pRight_) const;
	private:
	};

	//////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::Monadic --
	//
	// NOTES
	class Monadic
		: public Predicate::Comparison
	{
	public:
		typedef Monadic This;
		typedef Predicate::Comparison Super;

		// constructor
		Monadic() : Super() {}
		// destructor
		virtual ~Monadic() {}

		// generate Predicate from Statement::Object
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Statement::Object* pStatement_) const;
	protected:
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Plan::Tree::Node::Type eOperator_,
							 Statement::Object* pPrimary_) const;
	private:
	};

	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::DyadicRow --
	//
	// NOTES
	class DyadicRow
		: public Dyadic
	{
	public:
		typedef DyadicRow This;
		typedef Dyadic Super;

		// constructor
		DyadicRow() : Super() {}
		// destructor
		~DyadicRow() {}

	protected:
	/////////////////////
	// Dyadic::
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Plan::Tree::Node::Type eOperator_,
							 Statement::ValueExpression* pLeft_,
							 Statement::ValueExpression* pRight_) const;
	private:
	};

	//////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::MonadicRow --
	//
	// NOTES
	class MonadicRow
		: public Monadic
	{
	public:
		typedef MonadicRow This;
		typedef Monadic Super;

		// constructor
		MonadicRow() : Super() {}
		// destructor
		~MonadicRow() {}

	protected:
	////////////////////
	// Monadic::
		virtual Plan::Interface::IPredicate*
				getPredicate(Opt::Environment& cEnvironment_,
							 Plan::Interface::IRelation* pRelation_,
							 Plan::Tree::Node::Type eOperator_,
							 Statement::Object* pPrimary_) const;
	private:
	};
}

namespace
{
	// FUNCTION local
	//	$$$::_getOperator -- 
	//
	// NOTES
	//
	// ARGUMENTS
	//	int iOperator_
	//	
	// RETURN
	//	Plan::Predicate::Interface::Type
	//
	// EXCEPTIONS

	Plan::Tree::Node::Type
	_getOperator(int iOperator_)
	{
		switch (iOperator_) {
		case Statement::ValueExpression::op_Eq:
			{
				return Plan::Tree::Node::Equals;
			}
		case Statement::ValueExpression::op_Ne:
			{
				return Plan::Tree::Node::NotEquals;
			}
		case Statement::ValueExpression::op_Le:
			{
				return Plan::Tree::Node::LessThanEquals;
			}
		case Statement::ValueExpression::op_Lt:
			{
				return Plan::Tree::Node::LessThan;
			}
		case Statement::ValueExpression::op_Ge:
			{
				return Plan::Tree::Node::GreaterThanEquals;
			}
		case Statement::ValueExpression::op_Gt:
			{
				return Plan::Tree::Node::GreaterThan;
			}
		case Statement::ValueExpression::op_IsNull:
			{
				return Plan::Tree::Node::EqualsToNull;
			}
		case Statement::ValueExpression::op_IsNotNull:
			{
				return Plan::Tree::Node::NotNull;
			}
		default:
			{
				break;
			}
		}
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES
	const Impl::DyadicRow _analyzerDyadic;
	const Impl::MonadicRow _analyzerMonadic;
}

//////////////////////////////////////////////////
// Analysis::Predicate::Impl::Dyadic

// FUNCTION public
//	Predicate::Impl::Dyadic::getPredicate -- 
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
Impl::Dyadic::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	STRING cName = pVE->toSQLStatement();
	Plan::Interface::IPredicate* pResult = cEnvironment_.searchPredicate(cName);
	if (pResult) {
		return pResult;
	}

	pResult = getPredicate(cEnvironment_,
						   pRelation_,
						   _getOperator(pVE->getOperator()),
						   pVE->getLeft(),
						   pVE->getRight());

	if (pResult->hasParameter() == false) {
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}

	return pResult;
}

// FUNCTION protected
//	Predicate::Impl::Dyadic::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Plan::Tree::Node::Type eOperator_
//	Statement::ValueExpression* pLeft_
//	Statement::ValueExpression* pRight_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
Impl::Dyadic::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Plan::Tree::Node::Type eOperator_,
			 Statement::ValueExpression* pLeft_,
			 Statement::ValueExpression* pRight_) const
{
	// operands can be regarded as not null if it is top predicate
	Opt::Environment::AutoPop cAutoPop0(0, 0);
	if (cEnvironment_.checkStatus(Opt::Environment::Status::NoTopPredicate) == false) {
		Opt::Environment::AutoPop cAutoPopTmp = cEnvironment_.pushStatus(Opt::Environment::Status::KnownNotNull);
		cAutoPop0 = cAutoPopTmp; // to avoid compile error in GCC
	}
	

	Plan::Interface::IScalar* pOperand0 = 0;
	{
		// arbitrary array element is allowed in the left operand
		Opt::Environment::AutoPop cAutoPop1 =
			cEnvironment_.pushStatus(Opt::Environment::Status::ArbitraryElementAllowed);
		pOperand0 = 
			pLeft_->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLeft_);
	}
	Plan::Interface::IScalar* pOperand1 =
		pRight_->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pRight_);

	return Plan::Predicate::Comparison::create(cEnvironment_,
											   eOperator_,
											   MAKEPAIR(pOperand0, pOperand1));
}

////////////////////////////////////////////////
// Analysis::Predicate::Impl::Monadic

// FUNCTION public
//	Predicate::Impl::Monadic::getPredicate -- 
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
Impl::Monadic::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);
	; _SYDNEY_ASSERT(pVE);

	STRING cName = pVE->toSQLStatement();
	Plan::Interface::IPredicate* pResult = cEnvironment_.searchPredicate(cName);
	if (pResult) {
		return pResult;
	}

	pResult = getPredicate(cEnvironment_,
						   pRelation_,
						   _getOperator(pVE->getOperator()),
						   pVE->getPrimary());

	if (pResult->hasParameter() == false) {
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}

	return pResult;
}

// FUNCTION protected
//	Predicate::Impl::Monadic::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Plan::Tree::Node::Type eOperator_
//	Statement::ValueExpression* pPrimary_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
Impl::Monadic::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Plan::Tree::Node::Type eOperator_,
			 Statement::Object* pPrimary_) const
{
	Opt::Environment::Status::Value eKnown =
		(eOperator_ == Plan::Tree::Node::EqualsToNull)
		? Opt::Environment::Status::KnownNull
		: Opt::Environment::Status::KnownNotNull;

	// arbitrary array element is allowed in the operand
	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.pushStatus(Opt::Environment::Status::ArbitraryElementAllowed
								 | eKnown);
	Plan::Interface::IScalar* pOperand =
		pPrimary_->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pPrimary_);

	return Plan::Predicate::Comparison::create(cEnvironment_,
											   eOperator_,
											   pOperand);
}

////////////////////////////////////////////
// Analysis::Predicate::Impl::DyadicRow

// FUNCTION protected
//	Predicate::Impl::DyadicRow::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Plan::Tree::Node::Type eOperator_
//	Statement::ValueExpression* pLeft_
//	Statement::ValueExpression* pRight_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
Impl::DyadicRow::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Plan::Tree::Node::Type eOperator_,
			 Statement::ValueExpression* pLeft_,
			 Statement::ValueExpression* pRight_) const
{
	int iLeftDegree =
		pLeft_->getAnalyzer2()->getDegree(cEnvironment_,
										  pLeft_);
	int iRightDegree =
		pRight_->getAnalyzer2()->getDegree(cEnvironment_,
										   pRight_);

	if (iLeftDegree != iRightDegree) {
		_SYDNEY_THROW0(Exception::InvalidRowValue);
	}

	if (iLeftDegree == 1) {
		return Super::getPredicate(cEnvironment_,
								   pRelation_,
								   eOperator_,
								   pLeft_,
								   pRight_);
	}
	// operands can be regarded as not null
	Opt::Environment::AutoPop cAutoPop0(0, 0);
	if (cEnvironment_.checkStatus(Opt::Environment::Status::NoTopPredicate) == false) {
		Opt::Environment::AutoPop cAutoPopTmp =
			cEnvironment_.pushStatus(Opt::Environment::Status::KnownNotNull);
		cAutoPop0 = cAutoPopTmp; // to avoid compile error in GCC
	}	
	
	VECTOR<Plan::Interface::IScalar*> vecOperand0;
	{
		// arbitrary array element is allowed in the left operand
		Opt::Environment::AutoPop cAutoPop1 =
			cEnvironment_.pushStatus(Opt::Environment::Status::ArbitraryElementAllowed);

		pLeft_->getAnalyzer2()->addScalar(cEnvironment_,
										  pRelation_,
										  pLeft_,
										  vecOperand0);
	}

	VECTOR<Plan::Interface::IScalar*> vecOperand1;
	pRight_->getAnalyzer2()->addScalar(cEnvironment_,
									   pRelation_,
									   pRight_,
									   vecOperand1);

	return Plan::Predicate::Comparison::create(cEnvironment_,
											   eOperator_,
											   MAKEPAIR(vecOperand0,
														vecOperand1));
}

///////////////////////////////////////////
// Analysis::Predicate::Impl::MonadicRow

// FUNCTION public
//	Predicate::Impl::MonadicRow::getPredicate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Plan::Tree::Node::Type eOperator_
//	Statement::Object* pPrimary_
//	
// RETURN
//	Plan::Interface::IPredicate*
//
// EXCEPTIONS

//virtual
Plan::Interface::IPredicate*
Impl::MonadicRow::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Plan::Tree::Node::Type eOperator_,
			 Statement::Object* pPrimary_) const
{
	// currently, more than one elements are not allowed in monadic comparison
	// -> use superclass implementation
	// [NOTES]
	// NotSupported is thrown in getScalar if primary has more than one elements

	return Super::getPredicate(cEnvironment_,
							   pRelation_,
							   eOperator_,
							   pPrimary_);
}

//////////////////////////////
// Predicate::Comparison::

// FUNCTION public
//	Predicate::Comparison::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Comparison*
//
// EXCEPTIONS

//static
const Comparison*
Comparison::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_Eq:
	case Statement::ValueExpression::op_Ne:
	case Statement::ValueExpression::op_Le:
	case Statement::ValueExpression::op_Lt:
	case Statement::ValueExpression::op_Ge:
	case Statement::ValueExpression::op_Gt:
		{
			return &_analyzerDyadic;
		}
	case Statement::ValueExpression::op_IsNull:
	case Statement::ValueExpression::op_IsNotNull:
		{
			return &_analyzerMonadic;
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
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
