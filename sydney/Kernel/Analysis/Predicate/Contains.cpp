// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/Contains.cpp --
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

#include "Analysis/Predicate/Contains.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Predicate/Contains.h"

#include "Statement/ContainsOperand.h"
#include "Statement/ContainsOption.h"
#include "Statement/ContainsPredicate.h"
#include "Statement/Expand.h"
#include "Statement/QueryExpression.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_PREDICATE_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Predicate::Impl::ContainsImpl --
	//
	// NOTES
	class ContainsImpl
		: public Predicate::Contains
	{
	public:
		typedef ContainsImpl This;
		typedef Predicate::Contains Super;

		// constructor
		ContainsImpl() : Super() {}
		// destructor
		virtual ~ContainsImpl() {}

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
	const Impl::ContainsImpl _analyzer;
}

////////////////////////////////////////////
// Predicate::Impl::ContainsImpl

// FUNCTION public
//	Predicate::Impl::ContainsImpl::getPredicate -- 
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
Impl::ContainsImpl::
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

	Statement::ContainsPredicate* pCP =
		_SYDNEY_DYNAMIC_CAST(Statement::ContainsPredicate*, pVE->getPrimary());
	; _SYDNEY_ASSERT(pCP);

	Statement::ValueExpression* pTarget = pCP->getString();
	Statement::ContainsOperand* pPattern = pCP->getPattern();
	Statement::ContainsOption* pOption = pCP->getOption();

	// convert target and pattern
	VECTOR<Plan::Interface::IScalar*> vecOperand0;
	{
		// arbitrary array element is allowed in the left operand
		// operand can be regarded as not null if it is top predicate
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

		pTarget->getAnalyzer2()->addScalar(cEnvironment_, pRelation_, pTarget, vecOperand0);
	}

	Plan::Interface::IScalar* pOperand1 = 0;
	{
		Opt::Environment::AutoPop cAutoPop2 =
			cEnvironment_.pushStatus(Opt::Environment::Status::ContainsOperand);
		pOperand1 =
			pPattern->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pPattern);
	}

	VECTOR<Plan::Interface::IScalar*> vecOption;

	if (pOption) {
		if (Statement::ValueExpression* pCalculator = pOption->getCalculator()) {
			vecOption.PUSHBACK(pCalculator->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pCalculator)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getStringType())
							   ->setNodeType(Plan::Tree::Node::Calculator));
		}
		if (Statement::ValueExpression* pAverageLength = pOption->getAverageLength()) {
			vecOption.PUSHBACK(pAverageLength->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pAverageLength)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getStringType())
							   ->setNodeType(Plan::Tree::Node::AverageLength));
		}
		if (Statement::ValueExpression* pDf = pOption->getDf()) {
			vecOption.PUSHBACK(pDf->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pDf)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getIntegerType())
							   ->setNodeType(Plan::Tree::Node::Df));
		}
		if (Statement::ValueExpression* pScoreFunction = pOption->getScoreFunction()) {
			vecOption.PUSHBACK(pScoreFunction->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pScoreFunction)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getStringType())
							   ->setNodeType(Plan::Tree::Node::ScoreFunction));
		}
		if (Statement::ValueExpression* pExtractor = pOption->getExtractor()) {
			vecOption.PUSHBACK(pExtractor->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pExtractor)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getStringType())
							   ->setNodeType(Plan::Tree::Node::Extractor));
		}
		if (Statement::ValueExpression* pClusteredLimit = pOption->getClusteredLimit()) {
			vecOption.PUSHBACK(pClusteredLimit->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pClusteredLimit)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getFloatType())
							   ->setNodeType(Plan::Tree::Node::ClusteredLimit));
		}
		if (Statement::ValueExpression* pScoreCombiner = pOption->getScoreCombiner()) {
			vecOption.PUSHBACK(pScoreCombiner->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pScoreCombiner)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getStringType())
							   ->setNodeType(Plan::Tree::Node::ScoreCombiner));
		}
		if (Statement::ValueExpression* pClusteredCombiner = pOption->getClusteredCombiner()) {
			vecOption.PUSHBACK(pClusteredCombiner->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pClusteredCombiner)
							   ->setExpectedType(cEnvironment_,
												 Plan::Scalar::DataType::getStringType())
							   ->setNodeType(Plan::Tree::Node::ClusteredCombiner));
		}
	}

	// create result object
	Plan::Predicate::Contains* pContains =
		Plan::Predicate::Contains::create(cEnvironment_,
										  MAKEPAIR(vecOperand0, pOperand1),
										  vecOption);
	
	// set expand if exists
	if (Statement::Expand* pExpand = pOption ? pOption->getExpand() : 0) {
		pContains->setExpand(pExpand->getAnalyzer2()
							 ->getRelation(cEnvironment_, pExpand));
	}
	
	// set rank from if exists
	if (Statement::QueryExpression* pRankFrom = pOption ? pOption->getRankFrom() : 0) {
		Plan::Interface::IRelation* pSubRelation = 0;
		{
			Opt::Environment::AutoPop cAutoPop1 =
				cEnvironment_.pushNameScope();
			Opt::Environment::AutoPop cAutoPop2 =
				cEnvironment_.pushStatus(Opt::Environment::Status::Subquery
										 | Opt::Environment::Status::RankFrom
										 | Opt::Environment::Status::Reset);
			pSubRelation = pRankFrom->getAnalyzer2()->getRelation(cEnvironment_, pRankFrom);
		}
		pContains->setRankFrom(pSubRelation);
	}

	pResult = pContains;
	if (pResult->hasParameter() == false) {
		cEnvironment_.addPredicate(pRelation_,
								   cName,
								   pResult);
	}

	return pResult;
}

//////////////////////////////
// Predicate::Contains::

// FUNCTION public
//	Predicate::Contains::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ValueExpression* pStatement_
//	
// RETURN
//	const Contains*
//
// EXCEPTIONS

//static
const Contains*
Contains::
create(const Statement::ValueExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_PREDICATE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
