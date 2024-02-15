// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Function/Case.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Analysis::Function";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Function/Case.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/Combinator.h"
#include "Plan/Predicate/Comparison.h"
#include "Plan/Scalar/Case.h"

#include "Statement/ValueExpression.h"
#include "Statement/ValueExpressionList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_FUNCTION_BEGIN

namespace Impl
{
	// CLASS local
	//	Function::Impl::SimpleCase -- implementation class for case expression analyzer
	//
	// NOTES
	class SimpleCase
		: public Function::Case
	{
	public:
		typedef SimpleCase This;
		typedef Function::Case Super;

		// constructor
		SimpleCase() : Super() {}
		// destructor
		~SimpleCase() {}

	protected:
	private:
		void addCase(Opt::Environment& cEnvironment_,
					 Plan::Interface::IRelation* pRelation_,
					 Plan::Interface::IScalar* pOperand_,
					 Statement::Expression::SimpleWhen* pWhen_,
					 VECTOR<Plan::Interface::IPredicate*>& vecCaseList_,
					 VECTOR<Plan::Interface::IScalar*>& vecResultList_) const;

	////////////////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};

	// CLASS local
	//	Function::Impl::SearchedCase -- implementation class for case expression analyzer
	//
	// NOTES
	class SearchedCase
		: public Function::Case
	{
	public:
		typedef SearchedCase This;
		typedef Function::Case Super;

		// constructor
		SearchedCase() : Super() {}
		// destructor
		~SearchedCase() {}

		// generate Scalar from Statement::Object
		virtual Plan::Interface::IScalar*
					getScalar(Opt::Environment& cEnvironment_,
							  Plan::Interface::IRelation* pRelation_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
		void addCase(Opt::Environment& cEnvironment_,
					 Plan::Interface::IRelation* pRelation_,
					 Statement::Expression::SearchedWhen* pWhen_,
					 VECTOR<Plan::Interface::IPredicate*>& vecCaseList_,
					 VECTOR<Plan::Interface::IScalar*>& vecResultList_) const;

	////////////////////////////////
	// Base::
		virtual Plan::Interface::IScalar*
					createScalar(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::Object* pStatement_) const;

	};
} // namespace Impl

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instances
	//
	// NOTES
	const Impl::SimpleCase _analyzerSimple;
	const Impl::SearchedCase _analyzerSearched;

} // namespace

//////////////////////////////////////
//	Function::Impl::SimpleCase

// FUNCTION private
//	Function::Impl::SimpleCase::addCase -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Plan::Interface::IScalar* pOperand_
//	Statement::Expression::SimpleWhen* pWhen_
//	VECTOR<Plan::Interface::IPredicate*>& vecCaseList_
//	VECTOR<Plan::Interface::IScalar*>& vecResultList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::SimpleCase::
addCase(Opt::Environment& cEnvironment_,
		Plan::Interface::IRelation* pRelation_,
		Plan::Interface::IScalar* pOperand_,
		Statement::Expression::SimpleWhen* pWhen_,
		VECTOR<Plan::Interface::IPredicate*>& vecCaseList_,
		VECTOR<Plan::Interface::IScalar*>& vecResultList_) const
{
	Statement::ValueExpressionList* pOperandList = pWhen_->getWhenOperandList();
	Statement::ValueExpression* pResult = pWhen_->getResult();

	VECTOR<Plan::Interface::IPredicate*> vecOrOperand;
	int n = pOperandList->getCount();
	vecOrOperand.reserve(n);
	for (int i = 0; i < n; ++i) {
		Statement::ValueExpression* pElement = pOperandList->getValueExpressionAt(i);
		Plan::Interface::IScalar* pOperand1 =
			pElement->getAnalyzer2()->getScalar(cEnvironment_,
												pRelation_,
												pElement);
		vecOrOperand.PUSHBACK(Plan::Predicate::Comparison::create(
											  cEnvironment_,
											  Plan::Tree::Node::Equals,
											  MAKEPAIR(pOperand_, pOperand1)));
	}
	vecCaseList_.PUSHBACK(Plan::Predicate::Combinator::create(cEnvironment_,
															  Plan::Tree::Node::Or,
															  vecOrOperand));
	vecResultList_.PUSHBACK(pResult->getAnalyzer2()->getScalar(cEnvironment_,
															   pRelation_,
															   pResult));
}

// FUNCTION private
//	Function::Impl::SimpleCase::createScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::SimpleCase::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::Expression::SimpleCase* pSC =
		_SYDNEY_DYNAMIC_CAST(Statement::Expression::SimpleCase*, pStatement_);
	; _SYDNEY_ASSERT(pSC);

	Statement::ValueExpression* pOperand = pSC->getOperand();
	Statement::ValueExpressionList* pWhenList = pSC->getWhenList();
	Statement::ValueExpression* pElse = pSC->getElse();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);
	Opt::Environment::AutoPop cAutoPop1 =
		cEnvironment_.pushStatus(Opt::Environment::Status::NoTopPredicate);

	Plan::Interface::IScalar* pOperand0 =
		pOperand->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pOperand);
	int n = pWhenList->getCount();
	VECTOR<Plan::Interface::IPredicate*> vecCaseList;
	VECTOR<Plan::Interface::IScalar*> vecResultList;
	vecCaseList.reserve(n);
	vecResultList.reserve(n+1);
	for (int i = 0; i < n; ++i) {
		Statement::Expression::SimpleWhen* pElement =
			_SYDNEY_DYNAMIC_CAST(Statement::Expression::SimpleWhen*,
								 pWhenList->getValueExpressionAt(i));
		addCase(cEnvironment_,
				pRelation_,
				pOperand0,
				pElement,
				vecCaseList,
				vecResultList);
	}

	if (pElse) {
		vecResultList.PUSHBACK(pElse->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pElse));
	}
	return Plan::Scalar::Case::create(cEnvironment_,
									  vecCaseList,
									  vecResultList,
									  pSC->toSQLStatement());
}

//////////////////////////////////////
//	Function::Impl::SearchedCase

// FUNCTION public
//	Function::Impl::SearchedCase::getScalar -- generate Scalar from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::SearchedCase::
getScalar(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::Expression::SearchedCase* pSC =
		_SYDNEY_DYNAMIC_CAST(Statement::Expression::SearchedCase*, pStatement_);
	; _SYDNEY_ASSERT(pSC);

	Statement::ValueExpressionList* pWhenList = pSC->getWhenList();
	Statement::ValueExpression* pElse = pSC->getElse();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);

	int n = pWhenList->getCount();
	VECTOR<Plan::Interface::IPredicate*> vecCaseList;
	VECTOR<Plan::Interface::IScalar*> vecResultList;
	vecCaseList.reserve(n);
	vecResultList.reserve(n+1);
	for (int i = 0; i < n; ++i) {
		Statement::Expression::SearchedWhen* pElement =
			_SYDNEY_DYNAMIC_CAST(Statement::Expression::SearchedWhen*,
								 pWhenList->getValueExpressionAt(i));
		addCase(cEnvironment_,
				pRelation_,
				pElement,
				vecCaseList,
				vecResultList);
	}

	if (pElse) {
		vecResultList.PUSHBACK(pElse->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pElse));
	}
	return Plan::Scalar::Case::create(cEnvironment_,
									  vecCaseList,
									  vecResultList,
									  pSC->toSQLStatement());
}

// FUNCTION private
//	Function::Impl::SearchedCase::addCase -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Expression::SearchedWhen* pWhen_
//	VECTOR<Plan::Interface::IPredicate*>& vecCaseList_
//	VECTOR<Plan::Interface::IScalar*>& vecResultList_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::SearchedCase::
addCase(Opt::Environment& cEnvironment_,
		Plan::Interface::IRelation* pRelation_,
		Statement::Expression::SearchedWhen* pWhen_,
		VECTOR<Plan::Interface::IPredicate*>& vecCaseList_,
		VECTOR<Plan::Interface::IScalar*>& vecResultList_) const
{
	Statement::ValueExpression* pCondition = pWhen_->getWhenCondition();
	Statement::ValueExpression* pResult = pWhen_->getResult();

	vecCaseList_.PUSHBACK(pCondition->getAnalyzer2()->getPredicate(cEnvironment_,
																   pRelation_,
																   pCondition));
	vecResultList_.PUSHBACK(pResult->getAnalyzer2()->getScalar(cEnvironment_,
															   pRelation_,
															   pResult));
}

// FUNCTION private
//	Function::Impl::SearchedCase::createScalar -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//virtual
Plan::Interface::IScalar*
Impl::SearchedCase::
createScalar(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::Expression::SearchedCase* pSC =
		_SYDNEY_DYNAMIC_CAST(Statement::Expression::SearchedCase*, pStatement_);
	; _SYDNEY_ASSERT(pSC);

	Statement::ValueExpressionList* pWhenList = pSC->getWhenList();
	Statement::ValueExpression* pElse = pSC->getElse();

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.eraseStatus(Opt::Environment::Status::ArbitraryElementAllowed);
	Opt::Environment::AutoPop cAutoPop1 =
		cEnvironment_.pushStatus(Opt::Environment::Status::NoTopPredicate);	

	int n = pWhenList->getCount();
	VECTOR<Plan::Interface::IPredicate*> vecCaseList;
	VECTOR<Plan::Interface::IScalar*> vecResultList;
	vecCaseList.reserve(n);
	vecResultList.reserve(n+1);
	for (int i = 0; i < n; ++i) {
		Statement::Expression::SearchedWhen* pElement =
			_SYDNEY_DYNAMIC_CAST(Statement::Expression::SearchedWhen*,
								 pWhenList->getValueExpressionAt(i));
		addCase(cEnvironment_,
				pRelation_,
				pElement,
				vecCaseList,
				vecResultList);
	}

	if (pElse) {
		vecResultList.PUSHBACK(pElse->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pElse));
	}
	return Plan::Scalar::Case::create(cEnvironment_,
									  vecCaseList,
									  vecResultList,
									  pSC->toSQLStatement());
}

//////////////////////////////
// Function::Case
//////////////////////////////

// FUNCTION public
//	Function::Case::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Case*
//
// EXCEPTIONS

//static
const Case*
Case::
create(const Statement::ValueExpression* pStatement_)
{
	switch (pStatement_->getOperator()) {
	case Statement::ValueExpression::op_Case:
		{
			if (pStatement_->getRight()) {
				return &_analyzerSimple;
			} else {
				return &_analyzerSearched;
			}
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_FUNCTION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
