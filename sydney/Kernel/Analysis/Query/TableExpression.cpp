// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/TableExpression.cpp --
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

#include "Analysis/Query/TableExpression.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Interface/IRelation.h"

#include "Statement/GroupByClause.h"
#include "Statement/HavingClause.h"
#include "Statement/TableExpression.h"
#include "Statement/TableReferenceList.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Expression::Impl::TableExpressionImpl --
	//		implementation class for table expression analyzer
	//
	// NOTES
	class TableExpressionImpl
		: public Query::TableExpression
	{
	public:
		typedef TableExpressionImpl This;
		typedef Query::TableExpression Super;

		// constructor
		TableExpressionImpl() : Super() {}
		// destructor
		virtual ~TableExpressionImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
		virtual int getDegree(Opt::Environment& cEnvironment_,
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
	const Impl::TableExpressionImpl _analyzer;

} // namespace

//////////////////////////////////////////
// Query::Impl::TableExpressionImpl
//////////////////////////////////////////

// FUNCTION public
//	Query::Impl::TableExpressionImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
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
Impl::TableExpressionImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::TableExpression* pTE =
		_SYDNEY_DYNAMIC_CAST(Statement::TableExpression*, pStatement_);

	Statement::TableReferenceList* pTRL = pTE->getFromClause();

	// create Relation from TableReferenceList
	Plan::Interface::IRelation* pResult = 0;
	bool bHasCascade = cEnvironment_.hasCascade();
	if (bHasCascade) {
		pResult = pTRL->getAnalyzer2()->getDistributeRelation(cEnvironment_, pTRL);
	} else {
		pResult = pTRL->getAnalyzer2()->getRelation(cEnvironment_, pTRL);
	}
	bool bSimple = (pResult->getType() == Plan::Tree::Node::Table);

	if (bHasCascade) {
		if (Statement::ValueExpression* pVE = pTE->getWhereClause()) {
			pResult = pVE->getAnalyzer2()->getDistributeFilter(cEnvironment_,
															   pResult,
															   pVE);
			bSimple = false;
		}
		if (Statement::GroupByClause* pGC = pTE->getGroupByClause()) {
			pGC->setHasHavingClause(pTE->getHavingClause() != 0);
			pResult = pGC->getAnalyzer2()->getDistributeFilter(cEnvironment_,
															   pResult,
															   pGC);
			bSimple = false;
		}
		if (Statement::HavingClause* pHC = pTE->getHavingClause()) {
			pResult = pHC->getAnalyzer2()->getDistributeFilter(cEnvironment_,
															   pResult,
															   pHC);
			bSimple = false;
		}
	} else {
		if (Statement::ValueExpression* pVE = pTE->getWhereClause()) {
			pResult = pVE->getAnalyzer2()->getFilter(cEnvironment_,
													 pResult,
													 pVE);
			bSimple = false;
		}
		if (Statement::GroupByClause* pGC = pTE->getGroupByClause()) {
			pGC->setHasHavingClause(pTE->getHavingClause() != 0);
			pResult = pGC->getAnalyzer2()->getFilter(cEnvironment_,
													 pResult,
													 pGC);
			bSimple = false;
		}
		if (Statement::HavingClause* pHC = pTE->getHavingClause()) {
			pResult = pHC->getAnalyzer2()->getFilter(cEnvironment_,
													 pResult,
													 pHC);
			bSimple = false;
		}
	}

	if (bSimple) {
		// set status as simple form
		cEnvironment_.addStatus(Opt::Environment::Status::SimpleForm);
	}

	return pResult;
}

// FUNCTION public
//	Query::Impl::TableExpressionImpl::getDistributeRelation -- 
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
Impl::TableExpressionImpl::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION public
//	Query::Impl::TableExpressionImpl::getDegree -- 
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
Impl::TableExpressionImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::TableExpression* pTE =
		_SYDNEY_DYNAMIC_CAST(Statement::TableExpression*, pStatement_);

	Statement::TableReferenceList* pTRL = pTE->getFromClause();

	return pTRL->getAnalyzer2()->getDegree(cEnvironment_, pTRL);
}

////////////////////////////////////////
// Query::TableExpression
////////////////////////////////////////

// FUNCTION public
//	Query::TableExpression::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::TableExpression* pStatement_
//	
// RETURN
//	const TableExpression*
//
// EXCEPTIONS

const TableExpression*
TableExpression::
create(const Statement::TableExpression* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
