// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/Expand.cpp --
// 
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Query/Expand.h"
#include "Analysis/Query/Utility.h"

#include "Common/Assert.h"

#include "Exception/InvalidExpandDegree.h"
#include "Exception/InvalidExpandLimit.h"
#include "Exception/InvalidExpandOrder.h"
#include "Exception/NotSupported.h"
#include "Exception/OuterReferenceNotAllowed.h"

#include "Opt/Environment.h"

#include "Plan/Interface/IScalar.h"
#include "Plan/Order/Specification.h"
#include "Plan/Relation/Expand.h"

#include "Statement/Expand.h"
#include "Statement/LimitSpecification.h"
#include "Statement/QueryExpression.h"
#include "Statement/SortSpecification.h"
#include "Statement/SortSpecificationList.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::ExpandImpl -- implementation class of Expand
	//
	// NOTES
	class ExpandImpl
		: public Query::Expand
	{
	public:
		typedef ExpandImpl This;
		typedef Query::Expand Super;

		// constructor
		ExpandImpl() {}
		// destructor
		virtual ~ExpandImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	_analyzer -- instance
	//
	// NOTES
	const Impl::ExpandImpl _analyzer;

} //namespace

////////////////////////////////
// Query::Impl::ExpandImpl
////////////////////////////////

// FUNCTION public
//	Query::Impl::ExpandImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::ExpandImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::Expand* pE =
		_SYDNEY_DYNAMIC_CAST(Statement::Expand*, pStatement_);
	; _SYDNEY_ASSERT(pE);

	// generate relation from subquery
	Statement::QueryExpression* pSubQuery = pE->getSubQuery();

	Plan::Interface::IRelation* pSubRelation = 0;
	{
		Opt::Environment::AutoPop cAutoPop1 =
			cEnvironment_.pushNameScope();
		Opt::Environment::AutoPop cAutoPop2 =
			cEnvironment_.pushStatus(Opt::Environment::Status::Subquery
									 | Opt::Environment::Status::Reset);

		pSubRelation = pSubQuery->getAnalyzer2()->getRelation(cEnvironment_, pSubQuery);

		// Subquery for expand should have 1 to 2 row elements
		Plan::Interface::IRelation::Size iDegree = pSubRelation->getDegree(cEnvironment_);
		if (iDegree > 2 || iDegree == 0) {
			_SYDNEY_THROW1(Exception::InvalidExpandDegree, iDegree);
		}

		if (cEnvironment_.hasOuterReference()) {
			_SYDNEY_THROW0(Exception::OuterReferenceNotAllowed);
		}
	}

	Plan::Order::Specification* pOrderSpecification = 0;

	if (Statement::SortSpecificationList* pSSL = pE->getSortSpecification()) {
		// ORDER BY for keyword candidates
		int n = pSSL->getCount();
		if (n != 1) {
			_SYDNEY_THROW0(Exception::InvalidExpandOrder);
		}

		// convert sort key into scalar
		Statement::SortSpecification* pSS = pSSL->getSortSpecificationAt(0);
		Statement::ValueExpression* pVE = pSS->getSortKey();

		if (pVE->getOperator() != Statement::ValueExpression::op_Func
			|| (pVE->getFunction() != Statement::ValueExpression::func_WordDf
				&& pVE->getFunction() != Statement::ValueExpression::func_WordScale)) {
			_SYDNEY_THROW0(Exception::InvalidExpandOrder);
		}
		
		Plan::Interface::IScalar* pScalar =
			pVE->getAnalyzer2()->getScalar(cEnvironment_, pSubRelation, pVE);
		pOrderSpecification =
			Plan::Order::Specification::create(
					   cEnvironment_,
					   Plan::Order::Key::create(cEnvironment_,
												pScalar,
												Utility::getOrdering(pSS->getOrderingSpecification())));
	}
	Plan::Interface::IScalar* pLimit = 0;
	if (Statement::LimitSpecification* pLS = pE->getLimitSpecification()) {
		if (pLS->getOffset()) {
			_SYDNEY_THROW0(Exception::InvalidExpandLimit);
		}
		Statement::ValueExpression* pLVE = pLS->getLimit();
		pLimit = pLVE->getAnalyzer2()->getScalar(cEnvironment_, pSubRelation, pLVE)
			->setNodeType(Plan::Tree::Node::Limit);
	}

	// create result
	return Plan::Relation::Expand::create(cEnvironment_,
										  pSubRelation,
										  pOrderSpecification,
										  pLimit);
}

//////////////////////////////////
// Query::Expand
//////////////////////////////////

// FUNCTION public
//	Query::Expand::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::Expand* pStatement_
//	
// RETURN
//	const Expand*
//
// EXCEPTIONS

//static
const Expand*
Expand::
create(const Statement::Expand* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
