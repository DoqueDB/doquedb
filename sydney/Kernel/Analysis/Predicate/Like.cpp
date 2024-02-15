// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Like.cpp --
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

#include "Analysis/Predicate/Like.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Interface/IScalar.h"
#include "Plan/Predicate/Like.h"

#include "Statement/LikePredicate.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::LikeImpl --
	//
	// NOTES
	class LikeImpl
		: public Predicate::Like
	{
	public:
		typedef LikeImpl This;
		typedef Predicate::Like Super;

		// constructor
		LikeImpl() : Super() {}
		// destructor
		virtual ~LikeImpl() {}

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
	const Impl::LikeImpl _analyzer;
}

////////////////////////////////////////////
// Predicate::Impl::LikeImpl

// FUNCTION public
//	Predicate::Impl::LikeImpl::getPredicate -- 
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
Impl::LikeImpl::
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

	Statement::LikePredicate* pLP =
		_SYDNEY_DYNAMIC_CAST(Statement::LikePredicate*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pLP);

	Statement::ValueExpression* pTarget = pLP->getString();
	Statement::ValueExpression* pPattern = pLP->getPattern();
	Statement::ValueExpression* pEscape = pLP->getEscape();
	Statement::ValueExpression* pLanguage = pLP->getLanguage();

	PAIR<Plan::Interface::IScalar*, Plan::Interface::IScalar*> cOperand;
	VECTOR<Plan::Interface::IScalar*> vecOption;
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

	if (pEscape) {
		vecOption.PUSHBACK(pEscape->getAnalyzer2()->getScalar(cEnvironment_,
															  pRelation_,
															  pEscape)
						   ->setExpectedType(cEnvironment_,
											 Plan::Scalar::DataType::getStringType()));
	}
	if (pLanguage) {
		vecOption.PUSHBACK(pLanguage->getAnalyzer2()->getScalar(cEnvironment_,
																pRelation_,
																pLanguage)
						   ->setExpectedType(cEnvironment_,
											 Plan::Scalar::DataType::getLanguageType())
						   ->setNodeType(Plan::Tree::Node::Language));
	}

	pResult =
		Plan::Predicate::Like::create(cEnvironment_,
									  cOperand,
									  vecOption,
									  pVE->getOperator() == Statement::ValueExpression::op_NotLike);

	if (pResult->hasParameter() == false) {
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}

	return pResult;
}

//////////////////////////////
// Predicate::Like::

// FUNCTION public
//	Predicate::Like::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Like*
//
// EXCEPTIONS

//static
const Like*
Like::
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
