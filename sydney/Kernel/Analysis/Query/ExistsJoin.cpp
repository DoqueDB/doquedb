// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/ExistsJoin.cpp --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Query/ExistsJoin.h"

#include "Common/Assert.h"
#include "Common/Message.h"

#include "Exception/CommonColumnNotFound.h"

#include "Opt/Environment.h"
#include "Opt/NameMap.h"

#include "Plan/Relation/Join.h"
#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/RowElement.h"
#include "Plan/Scalar/Function.h"
#include "Plan/Utility/ObjectSet.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/JoinType.h"
#include "Statement/ExistsJoin.h"
#include "Statement/Type.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	/////////////////////////////////////////////
	// CLASS
	//	Query::Impl::ExistsJoinImpl::Base -- base class for exists join analyzer
	//
	// NOTES
	class ExistsJoinImpl
		: public Query::ExistsJoin
	{
	public:
		typedef ExistsJoinImpl This;
		typedef Query::ExistsJoin Super;

		// constructor
		ExistsJoinImpl() : Super() {}
		// destructor
		~ExistsJoinImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
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
	const Impl::ExistsJoinImpl _analyzer;

} //namespace

///////////////////////////////////
// Query::Impl::ExistsJoinImpl
///////////////////////////////////

// FUNCTION public
//	Query::Impl::ExistsJoinImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
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
Impl::ExistsJoinImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::ExistsJoin* pCJ =
		_SYDNEY_DYNAMIC_CAST(Statement::ExistsJoin*, pStatement_);

	// Join operand
	Statement::Object* pLeft = pCJ->getLeft();
	Statement::Object* pRight = pCJ->getRight();

	Plan::Interface::IRelation* pLeftRelation = 0;
	Plan::Interface::IRelation* pRightRelation = 0;
	Plan::Relation::Join* pResult = 0;
	{
		// create relations from operand
		pLeftRelation = pLeft->getAnalyzer2()->getRelation(cEnvironment_, pLeft);

		// create new namescope for right operand
		Opt::Environment::AutoPop cAutoPop0 =
			cEnvironment_.pushNameScope(Opt::Environment::Scope::Exists);

		Opt::Environment::AutoPop cAutoPop1 =
			cEnvironment_.pushStatus(Opt::Environment::Status::Exists
									 | Opt::Environment::Status::Subquery
									 | Opt::Environment::Status::Reset);

		pRightRelation = pRight->getAnalyzer2()->getRelation(cEnvironment_, pRight);

		Statement::Object* pCondition = pCJ->getCondition();
		Plan::Interface::IPredicate* pJoinCondition =
			pCondition->getAnalyzer2()->getPredicate(cEnvironment_, pResult, pCondition);

		pResult =
			Plan::Relation::Join::create(cEnvironment_,
										 pCJ->getFlag()
										 ? Plan::Interface::IRelation::Exists
										 : Plan::Interface::IRelation::NotExists,
										 0,
										 MAKEPAIR(pLeftRelation, pRightRelation));

		pResult->setJoinPredicate(pJoinCondition);
	}
	return pResult;
}

// FUNCTION public
//	Query::Impl::ExistsJoinImpl::getDegree -- 
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
Impl::ExistsJoinImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::ExistsJoin* pCJ =
		_SYDNEY_DYNAMIC_CAST(Statement::ExistsJoin*, pStatement_);

	// Join operand
	Statement::Object* pLeft = pCJ->getLeft();

	return pLeft->getAnalyzer2()->getDegree(cEnvironment_, pLeft);
}

//////////////////////////////////
// Query::ExistsJoin
//////////////////////////////////

// FUNCTION public
//	Query::ExistsJoin::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::ExistsJoin* pStatement_
//	
// RETURN
//	const ExistsJoin*
//
// EXCEPTIONS

//static
const ExistsJoin*
ExistsJoin::
create(const Statement::ExistsJoin* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
