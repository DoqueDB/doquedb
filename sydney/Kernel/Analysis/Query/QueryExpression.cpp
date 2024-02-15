// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/QueryExpression.cpp --
// 
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Query/QueryExpression.h"
#include "Analysis/Query/Utility.h"

#include "Common/Assert.h"
#include "Common/IntegerData.h"
#include "Common/UnicodeString.h"

#include "Exception/ExceedDegree.h"
#include "Exception/InvalidSubQuery.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Relation/Limit.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Sort.h"

#include "DPlan/Relation/Limit.h"
#include "DPlan/Relation/Sort.h"

#include "Statement/Hint.h"
#include "Statement/HintElement.h"
#include "Statement/LimitSpecification.h"
#include "Statement/Literal.h"
#include "Statement/QueryExpression.h"
#include "Statement/SortSpecification.h"
#include "Statement/SortSpecificationList.h"
#include "Statement/StringValue.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace QueryExpressionImpl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QueryExpressionImpl::Base --
	//		base class for query expression analyzer
	//
	// NOTES
	class Base
		: public Query::QueryExpression
	{
	public:
		typedef Base This;
		typedef Query::QueryExpression Super;

		// constructor
		Base() : Super() {}
		// destructor
		virtual ~Base() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
	//	virtual int getDegree(Opt::Environment& cEnvironment_,
	//						  Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
	protected:
	private:
		// generate by one query expression body
		virtual Plan::Interface::IRelation*
				getQueryRelation(Opt::Environment& cEnvironment_,
								 Statement::QueryExpression* pStatement_) const = 0;

		// parse hint value
		Opt::Environment::Status::Value
				parseHint(Opt::Environment& cEnvironment_,
						  Statement::Hint* pHint_) const;
		// create sorting filter from sort specification
		Plan::Interface::IRelation*
				addSortingFilter(Opt::Environment& cEnvironment_,
								 Plan::Interface::IRelation* pRelation_,
								 Statement::SortSpecificationList* pSSL_) const;
		// get sorting key specified by numeric value
		Plan::Interface::IScalar*
				getNumberedKey(Opt::Environment& cEnvironment_,
							   Statement::ValueExpression* pStatement_,
							   Plan::Interface::IRelation* pQueryBody_) const;
		// create limit filter from limit specification
		Plan::Interface::IRelation*
				addLimitFilter(Opt::Environment& cEnvironment_,
							   Plan::Interface::IRelation* pRelation_,
							   Statement::LimitSpecification* pLS_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QueryExpressionImpl::Primary --
	//		QueryExpression for query primary
	//
	// NOTES
	class Primary
		: public Base
	{
	public:
		typedef Primary This;
		typedef Base Super;

		// constructor
		Primary() : Super() {}
		// destructor
		~Primary() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
		/////////////////
		// Base::
		// generate by one query expression body
		virtual Plan::Interface::IRelation*
				getQueryRelation(Opt::Environment& cEnvironment_,
								 Statement::QueryExpression* pStatement_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QueryExpressionImpl::SetOperator --
	//		QueryExpression for query expression with set operator
	//
	// NOTES
	class SetOperator
		: public Primary
	{
	public:
		typedef SetOperator This;
		typedef Primary Super;

		// constructor
		SetOperator() : Super() {}
		// destructor
		virtual ~SetOperator() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
	protected:
	private:
		/////////////////
		// Base::
		// generate by one query expression body
		virtual Plan::Interface::IRelation*
				getQueryRelation(Opt::Environment& cEnvironment_,
								 Statement::QueryExpression* pStatement_) const;
	};
}

namespace
{
	// CONST local
	//	_pszHintOrder -- constant value for hint
	//
	// NOTES
	const char* const _pszHintOrder = "ORDER";

	// VARIABLE local
	//	_analyzerXXX -- instance
	//
	// NOTES
	const QueryExpressionImpl::Primary _analyzerPrimary;
	const QueryExpressionImpl::SetOperator _analyzerSetOperator;

} // namespace

/////////////////////////////////////////////////////
// Analysis::Query::QueryExpressionImpl::Base
/////////////////////////////////////////////////////

// FUNCTION public
//	QueryExpressionImpl::Base::getRelation -- generate Plan::Tree::Node from Statement::Object
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
QueryExpressionImpl::Base::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Plan::Interface::IRelation* pResult = 0;

	// sub query can't be written as an argument of set function
	if (cEnvironment_.checkStatus(Opt::Environment::Status::SetFunction)) {
		_SYDNEY_THROW0(Exception::InvalidSubQuery);
	}

	Statement::QueryExpression* pQE =
		_SYDNEY_DYNAMIC_CAST(Statement::QueryExpression*, pStatement_);
	; _SYDNEY_ASSERT(pQE);

	// get hint before analyzing query expression
	Opt::Environment::Status::Value iStatus = Opt::Environment::Status::Normal;
	if (Statement::Hint* pHint = pQE->getHint()) {
		iStatus = parseHint(cEnvironment_, pHint);
	}
	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.pushStatus(iStatus);

	// create relation from query expression body
	pResult = getQueryRelation(cEnvironment_, pQE);

	Statement::SortSpecificationList* pSSL = pQE->getSortSpecification();
	Statement::LimitSpecification* pLS = pQE->getLimitSpecification();
	if (pSSL) {
		pResult = addSortingFilter(cEnvironment_,
								   pResult,
								   pSSL);
	}
	if (pLS) {
		pResult = addLimitFilter(cEnvironment_,
								 pResult,
								 pLS);
	}
	return pResult;
}

// FUNCTION public
//	Query::QueryExpressionImpl::Base::getDistributeRelation -- 
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
QueryExpressionImpl::Base::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION private
//	QueryExpressionImpl::Base::parseHint -- parse hint
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Hint* pHint_
//	
// RETURN
//	Opt::Environment::Status::Value
//
// EXCEPTIONS

Opt::Environment::Status::Value
QueryExpressionImpl::Base::
parseHint(Opt::Environment& cEnvironment_, Statement::Hint* pHint_) const
{
	Opt::Environment::Status::Value iResult = Opt::Environment::Status::Normal;
	int n = pHint_->getHintElementCount();
	for (int i = 0; i < n; ++i) {
		const Statement::HintElement* pElement = pHint_->getHintElementAt(i);
		; _SYDNEY_ASSERT(pElement);
		int m = pElement->getHintPrimaryCount();
		for (int j = 0; j < m; ++j) {
			const Statement::Object* pPrimary = pElement->getHintPrimaryAt(j);
			; _SYDNEY_ASSERT(pPrimary);
			switch (pPrimary->getType()) {
			case Statement::ObjectType::StringValue:
				{
					const Statement::StringValue* pString =
						_SYDNEY_DYNAMIC_CAST(const Statement::StringValue*, pPrimary);
					if (pString->getValue()->compare(_TRMEISTER_U_STRING(_pszHintOrder), ModFalse) == 0) {
						// Do not change join order
						iResult |= Opt::Environment::Status::FixJoinOrder;
					}
					break;
				}
			default:
				{
					// ignore
					break;
				}
			}
		}
	}
	return iResult;
}

// FUNCTION private
//	QueryExpressionImpl::Base::addSortingFilter -- create sorting filter from sort specification
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::SortSpecificationList* pSSL_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

Plan::Interface::IRelation*
QueryExpressionImpl::Base::
addSortingFilter(Opt::Environment& cEnvironment_,
				 Plan::Interface::IRelation* pRelation_,
				 Statement::SortSpecificationList* pSSL_) const
{
	// create sort specification
	int n = pSSL_->getCount();
	; _SYDNEY_ASSERT(n);

	VECTOR<Plan::Order::Key*> vecKey;
	vecKey.reserve(n);

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.pushStatus(Opt::Environment::Status::OrderBy);

	for (int i = 0; i < n; ++i) {
		Statement::SortSpecification* pSS = pSSL_->getSortSpecificationAt(i);
		// Convert key specification into scalar
		Statement::ValueExpression* pVE = pSS->getSortKey();
		Plan::Interface::IScalar* pScalar = 0;
		if (pVE->isNumberLiteral()) {
			// if sort key is specified by a numeric literal,
			// it means position of key column in the result row
			pScalar = getNumberedKey(cEnvironment_,
									 pVE,
									 pRelation_);
		} else {
			// convert statement into scalar
			; _SYDNEY_ASSERT(pVE->getAnalyzer2());
			pScalar = pVE->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pVE);
		}

		// create key specification
		vecKey.PUSHBACK(Plan::Order::Key::create(cEnvironment_,
												 pScalar,
												 Utility::getOrdering(pSS->getOrderingSpecification())));
	}
	Plan::Order::Specification* pSortSpecification =
		Plan::Order::Specification::create(cEnvironment_, vecKey, false);

	if (Statement::ValueExpression* pPB = pSSL_->getPartitionBy()) {
		// Partition By
		VECTOR<Plan::Interface::IScalar*> vecKey;
		pPB->getAnalyzer2()->addScalar(cEnvironment_,
									   pRelation_,
									   pPB,
									   vecKey);
		// add partition key to order specification
		pSortSpecification->setPartitionKey(vecKey);
	}

	// create sorting filter
	if (cEnvironment_.hasCascade()) {
		return DPlan::Relation::Sort::create(cEnvironment_,
											 pSortSpecification,
											 pRelation_);
	} else {
		return Plan::Relation::Sort::create(cEnvironment_,
											pSortSpecification,
											pRelation_);
	}
}

// FUNCTION private
//	QueryExpressionImpl::Base::getNumberedKey -- get numbered sorting key
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::ValueExpression* pStatement_
//	Plan::Interface::IRelation* pQueryBody_
//	
// RETURN
//	Plan::Interface::IScalar*
//
// EXCEPTIONS

//get numbered sorting key
Plan::Interface::IScalar*
QueryExpressionImpl::Base::
getNumberedKey(Opt::Environment& cEnvironment_,
			   Statement::ValueExpression* pStatement_,
			   Plan::Interface::IRelation* pQueryBody_) const
{
	; _SYDNEY_ASSERT(pStatement_->isNumberLiteral());
	Statement::Literal* pLiteral = _SYDNEY_DYNAMIC_CAST(Statement::Literal*,
														pStatement_->getPrimary());
	Common::Data::Pointer pData = pLiteral->createData();
	Common::IntegerData* pIntData = _SYDNEY_DYNAMIC_CAST(Common::IntegerData*,
														 pData.get());

	int iNum = pIntData ? pIntData->getValue() : 0;

	if (iNum <= 0 || iNum > pQueryBody_->getDegree(cEnvironment_)) {
		// specified number exceeds the degree of the relation
		_SYDNEY_THROW2(Exception::ExceedDegree,
					   iNum,
					   pQueryBody_->getDegree(cEnvironment_));
	}
	// 1-base -> 0-base
	--iNum;

	return pQueryBody_->getScalar(cEnvironment_, iNum);
}

// FUNCTION private
//	Query::QueryExpressionImpl::Base::addLimitFilter -- create limit filter from limit specification
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::LimitSpecification* pLS_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

Plan::Interface::IRelation*
QueryExpressionImpl::Base::
addLimitFilter(Opt::Environment& cEnvironment_,
			   Plan::Interface::IRelation* pRelation_,
			   Statement::LimitSpecification* pLS_) const
{
	Statement::ValueExpression* pLVE = pLS_->getLimit();
	Statement::ValueExpression* pOVE = pLS_->getOffset();
	Plan::Interface::IScalar* pLimit = pLVE->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pLVE);
	Plan::Interface::IScalar* pOffset = pOVE ? pOVE->getAnalyzer2()->getScalar(cEnvironment_, pRelation_, pOVE) : 0;
	if (cEnvironment_.hasCascade()) {
		return DPlan::Relation::Limit::create(cEnvironment_,
											  pLimit,
											  pOffset,
											  pRelation_);
	} else {
		return Plan::Relation::Limit::create(cEnvironment_,
											 pLimit,
											 pOffset,
											 pRelation_);
	}
}

////////////////////////////////////
// QueryExpressionImpl::Primary
////////////////////////////////////

// FUNCTION public
//	QueryExpressionImpl::Primary::getQueryRelation -- generate by one query expression body
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::QueryExpression* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QueryExpressionImpl::Primary::
getQueryRelation(Opt::Environment& cEnvironment_,
				 Statement::QueryExpression* pStatement_) const
{
	// convert using query term
	Statement::Object* pQT = pStatement_->getQueryTerm();
	; _SYDNEY_ASSERT(pQT);

	if (cEnvironment_.hasCascade()) {
		return pQT->getAnalyzer2()->getDistributeRelation(cEnvironment_, pQT);
	} else {
		return pQT->getAnalyzer2()->getRelation(cEnvironment_, pQT);
	}
}

// FUNCTION public
//	Query::QueryExpressionImpl::Primary::getDegree -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
QueryExpressionImpl::Primary::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::QueryExpression* pQE =
		_SYDNEY_DYNAMIC_CAST(Statement::QueryExpression*, pStatement_);
	; _SYDNEY_ASSERT(pQE);

	// query term
	Statement::Object* pQT = pQE->getQueryTerm();
	; _SYDNEY_ASSERT(pQT);
	return pQT->getAnalyzer2()->getDegree(cEnvironment_, pQT);
}

////////////////////////////////////////
// QueryExpressionImpl::SetOperator
////////////////////////////////////////

// FUNCTION private
//	QueryExpressionImpl::SetOperator::getQueryRelation -- generate by one query expression body
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::QueryExpression* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QueryExpressionImpl::SetOperator::
getQueryRelation(Opt::Environment& cEnvironment_,
				 Statement::QueryExpression* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION public
//	Query::QueryExpressionImpl::SetOperator::getDegree -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	
// RETURN
//	int
//
// EXCEPTIONS

//virtual
int
QueryExpressionImpl::SetOperator::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

////////////////////////////////////////
// Query::QueryExpression
////////////////////////////////////////

// FUNCTION public
//	Query::QueryExpression::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::QueryExpression* pStatement_
//	
// RETURN
//	const QueryExpression*
//
// EXCEPTIONS

const QueryExpression*
QueryExpression::
create(const Statement::QueryExpression* pStatement_)
{
	if (pStatement_->getOperator()) {
		return &_analyzerSetOperator;
	} else {
		return &_analyzerPrimary;
	}
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
