// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/HavingClause.cpp --
// 
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Query/HavingClause.h"

#include "Common/Assert.h"

#include "Opt/Environment.h"

#include "Plan/Order/Specification.h"
#include "Plan/Relation/Grouping.h"

#include "Statement/HavingClause.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::HavingClauseImpl -- implementation class of having clause analyzer
	//
	// NOTES
	class HavingClauseImpl
		: public Query::HavingClause
	{
	public:
		typedef HavingClauseImpl This;
		typedef Query::HavingClause Super;

		// constructor
		HavingClauseImpl() : Super() {}
		// destructor
		~HavingClauseImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getFilter(Opt::Environment& cEnvironment_,
						  Plan::Interface::IRelation* pRelation_,
						  Statement::Object* pStatement_) const;

		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getDistributeFilter(Opt::Environment& cEnvironment_,
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
	const Impl::HavingClauseImpl _analyzer;

} // namespace

///////////////////////////////////////////////////
// Analysis::Query::Impl::HavingClauseImpl
///////////////////////////////////////////////////

// FUNCTION public
//	Query::Impl::HavingClauseImpl::getFilter -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
Impl::HavingClauseImpl::
getFilter(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::HavingClause* pHC =
		_SYDNEY_DYNAMIC_CAST(Statement::HavingClause*, pStatement_);
	; _SYDNEY_ASSERT(pHC);

	if (cEnvironment_.isGrouping() == false) {
		// create empty sort specification
		Plan::Order::Specification* pSortSpecification =
			Plan::Order::Specification::create(cEnvironment_, VECTOR<Plan::Order::Key*>(), false);
		// create grouping filter
		pRelation_ = Plan::Relation::Grouping::create(cEnvironment_,
													  pSortSpecification,
													  pRelation_);
	}

	// set status as group by clause
	Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushStatus(Opt::Environment::Status::Having);

	Statement::ValueExpression* pCondition = pHC->getCondition();
	return pCondition->getAnalyzer2()->getFilter(cEnvironment_, pRelation_, pCondition);
}


// FUNCTION public
//	Query::GroupByClauseImpl::Primitive::getDistributeFilter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual

Plan::Interface::IRelation*
Impl::HavingClauseImpl::
getDistributeFilter(Opt::Environment& cEnvironment_,
					Plan::Interface::IRelation* pRelation_,
					Statement::Object* pStatement_) const
{
	return getFilter(cEnvironment_, pRelation_, pStatement_);
}
/////////////////////////////
// Query::HavingClause
/////////////////////////////

// FUNCTION public
//	Query::HavingClause::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::HavingClause* pStatement_
//	
// RETURN
//	const HavingClause*
//
// EXCEPTIONS

//static
const HavingClause*
HavingClause::
create(const Statement::HavingClause* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
