// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Similar.cpp --
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Analysis::Predicate";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Predicate/Similar.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/Similar.h"

#include "Statement/SimilarPredicate.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::SimilarImpl --
	//
	// NOTES
	class SimilarImpl
		: public Predicate::Similar
	{
	public:
		typedef SimilarImpl This;
		typedef Predicate::Similar Super;

		// constructor
		SimilarImpl() : Super() {}
		// destructor
		virtual ~SimilarImpl() {}

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
	//	$$$::_analyzer -- instance
	//
	// NOTES
	const Impl::SimilarImpl _analyzer;
}

////////////////////////////////////////////
// Predicate::Impl::SimilarImpl

// FUNCTION public
//	Predicate::Impl::SimilarImpl::getPredicate -- 
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
Impl::SimilarImpl::
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

	Statement::SimilarPredicate* pSP =
		_SYDNEY_DYNAMIC_CAST(Statement::SimilarPredicate*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pSP);

	Statement::ValueExpression* pTarget = pSP->getString();
	Statement::ValueExpression* pPattern = pSP->getPattern();
	Statement::ValueExpression* pEscape = pSP->getEscape();
	Statement::ValueExpression* pLanguage = pSP->getLanguage();

	if (pEscape || pLanguage) {
		// escape and language is not supported
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*> cOperand;
	{
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
			
		cOperand.first = pTarget->getAnalyzer2()->getScalar(cEnvironment_,
															pRelation_,
															pTarget);
	}
	cOperand.second = pPattern->getAnalyzer2()->getScalar(cEnvironment_,
														  pRelation_,
														  pPattern)
		->setExpectedType(cEnvironment_,
						  Plan::Scalar::DataType::getStringType());

	pResult =
		Plan::Predicate::Similar::create(cEnvironment_,
										 cOperand,
										 pVE->getOperator() == Statement::ValueExpression::op_NotSimilar);

	if (pResult->hasParameter() == false) {
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}

	return pResult;
}

//////////////////////////////
// Predicate::Similar::

// FUNCTION public
//	Predicate::Similar::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Similar*
//
// EXCEPTIONS

//static
const Similar*
Similar::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
