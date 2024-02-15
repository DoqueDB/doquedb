// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/CrossJoin.cpp --
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

#include "Analysis/Query/CrossJoin.h"

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
#include "Statement/CrossJoin.h"
#include "Statement/Type.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace Impl
{
	/////////////////////////////////////////////
	// CLASS
	//	Query::Impl::CrossJoinImpl::Base -- base class for cross join analyzer
	//
	// NOTES
	class CrossJoinImpl
		: public Query::CrossJoin
	{
	public:
		typedef CrossJoinImpl This;
		typedef Query::CrossJoin Super;

		// constructor
		CrossJoinImpl() : Super() {}
		// destructor
		~CrossJoinImpl() {}

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
	const Impl::CrossJoinImpl _analyzer;

} //namespace

///////////////////////////////////
// Query::Impl::CrossJoinImpl
///////////////////////////////////

// FUNCTION public
//	Query::Impl::CrossJoinImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
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
Impl::CrossJoinImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::CrossJoin* pCJ =
		_SYDNEY_DYNAMIC_CAST(Statement::CrossJoin*, pStatement_);

	// Join operand
	Statement::Object* pLeft = pCJ->getLeft();
	Statement::Object* pRight = pCJ->getRight();

	Plan::Interface::IRelation* pLeftRelation = 0;
	Plan::Interface::IRelation* pRightRelation = 0;

	{
		// create new namescope if current scope is not joined table
		Opt::Environment::AutoPop cAutoPop =
			cEnvironment_.pushNameScope(Opt::Environment::Scope::JoinedTable);

		// create relations from operand
		pLeftRelation = pLeft->getAnalyzer2()->getRelation(cEnvironment_, pLeft);
		pRightRelation = pRight->getAnalyzer2()->getRelation(cEnvironment_, pRight);
	}

	return Plan::Relation::Join::create(cEnvironment_,
										Plan::Interface::IRelation::SimpleJoin,
										0,
										MAKEPAIR(pLeftRelation, pRightRelation));
}

// FUNCTION public
//	Query::Impl::CrossJoinImpl::getDegree -- 
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
Impl::CrossJoinImpl::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::CrossJoin* pCJ =
		_SYDNEY_DYNAMIC_CAST(Statement::CrossJoin*, pStatement_);

	// Join operand
	Statement::Object* pLeft = pCJ->getLeft();
	Statement::Object* pRight = pCJ->getRight();

	return pLeft->getAnalyzer2()->getDegree(cEnvironment_, pLeft)
		+ pRight->getAnalyzer2()->getDegree(cEnvironment_, pRight);
}

//////////////////////////////////
// Query::CrossJoin
//////////////////////////////////

// FUNCTION public
//	Query::CrossJoin::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::CrossJoin* pStatement_
//	
// RETURN
//	const CrossJoin*
//
// EXCEPTIONS

//static
const CrossJoin*
CrossJoin::
create(const Statement::CrossJoin* pStatement_)
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
