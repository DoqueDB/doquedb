// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Between.cpp --
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
const char moduleName[] = "Analysis::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Predicate/Between.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/Between.h"
#include "Plan/Interface/IRelation.h"

#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::BetweenImpl --
	//
	// NOTES
	class BetweenImpl
		: public Predicate::Between
	{
	public:
		typedef BetweenImpl This;
		typedef Predicate::Between Super;

		// constructor
		BetweenImpl() : Super() {}
		// destructor
		~BetweenImpl() {}

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
	//	$$$::_analyzer -- instances
	//
	// NOTES
	const Impl::BetweenImpl _analyzer;
}

////////////////////////////////////////////
// Predicate::Impl::BetweenImpl

// FUNCTION public
//	Predicate::Impl::BetweenImpl::getPredicate -- 
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

//virtual
Plan::Interface::IPredicate*
Impl::BetweenImpl::
getPredicate(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_,
			 Statement::Object* pStatement_) const
{
	Statement::ValueExpression* pVE =
		_SYDNEY_DYNAMIC_CAST(Statement::ValueExpression*, pStatement_);

	STRING cName = pVE->toSQLStatement();
	Plan::Interface::IPredicate* pResult = cEnvironment_.searchPredicate(cName);
	if (pResult) {
		return pResult;
	}

	Statement::ValueExpression* pLeft = pVE->getLeft();
	Statement::ValueExpression* pRight = pVE->getRight();
	Statement::ValueExpression* pOption = pVE->getOption();

	VECTOR<Plan::Interface::IScalar*> vecScalar;

	{
		// arbitrary array element is allowed in the left operand
		Opt::Environment::AutoPop cAutoPop(0, 0);
		if (cEnvironment_.checkStatus(Opt::Environment::Status::NoTopPredicate) == false) {
			Opt::Environment::AutoPop cAutoPopTmp =
				cEnvironment_.pushStatus(Opt::Environment::Status::ArbitraryElementAllowed
										 | Opt::Environment::Status::KnownNotNull);
			cAutoPop = cAutoPopTmp; // to avoid compile error in GCC
		} else {
			Opt::Environment::AutoPop cAutoPopTmp =
				cEnvironment_.pushStatus(Opt::Environment::Status::ArbitraryElementAllowed);
			cAutoPop = cAutoPopTmp;  // to avoid compile error in GCC
		}
		
		pLeft->getAnalyzer2()->addScalar(cEnvironment_,
										 pRelation_,
										 pLeft,
										 vecScalar);
	}
	pRight->getAnalyzer2()->addScalar(cEnvironment_,
									  pRelation_,
									  pRight,
									  vecScalar);
	pOption->getAnalyzer2()->addScalar(cEnvironment_,
									   pRelation_,
									   pOption,
									   vecScalar);

	pResult =
		Plan::Predicate::Between::create(cEnvironment_,
										 vecScalar,
										 pVE->getOperator() == Statement::ValueExpression::op_NotBetween);

	if (pResult->hasParameter() == false) {
		// add to namemap
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}
	return pResult;
}

//////////////////////////////
// Predicate::Between::

// FUNCTION public
//	Predicate::Between::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Between*
//
// EXCEPTIONS

//static
const Between*
Between::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
