// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/GroupByClause.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#include "boost/bind.hpp"

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Query/GroupByClause.h"

#include "Common/Assert.h"

#include "DPlan/Relation/Grouping.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"


#include "Plan/Interface/IScalar.h"
#include "Plan/Scalar/Field.h"
#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Relation/Grouping.h"
#include "Plan/Relation/Sort.h"

#include "Statement/GroupByClause.h"
#include "Statement/GroupingElementList.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace GroupByClauseImpl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::GroupByClauseImpl::Primitive -- implementation class of group by clause analyzer
	//
	// NOTES
	class Primitive
		: public Query::GroupByClause
	{
	public:
		typedef Primitive This;
		typedef Query::GroupByClause Super;

		// constructor
		Primitive() : Super() {}
		// destructor
		~Primitive() {}

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
		bool isAbleToBitSetSort(Statement::GroupByClause* pGBC,
								VECTOR<Plan::Interface::IScalar*>& vecScalar) const;
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES
	const GroupByClauseImpl::Primitive _analyzerPrimitive;

} // namespace

///////////////////////////////////////////////////
// Analysis::Query::GroupByClauseImpl::Primitive
///////////////////////////////////////////////////

// FUNCTION public
//	Query::GroupByClauseImpl::Primitive::getFilter -- generate Plan::Tree::Node from Statement::Object
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
GroupByClauseImpl::Primitive::
getFilter(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Statement::GroupByClause* pGBC =
		_SYDNEY_DYNAMIC_CAST(Statement::GroupByClause*, pStatement_);
	; _SYDNEY_ASSERT(pGBC);

	Statement::GroupingElementList* pElementList = pGBC->getGroupingElementList();
	; _SYDNEY_ASSERT(pElementList);

	// create scalar node for group by key
	VECTOR<Plan::Interface::IScalar*> vecScalar;
	{
		// set status as group by clause
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushStatus(Opt::Environment::Status::GroupBy
																	  |Opt::Environment::Status::ExpandElementAllowed);
		int n = pElementList->getCount();
		for (int i = 0; i < n; ++i) {
			Statement::Object* pElement = pElementList->getAt(i);
			pElement->getAnalyzer2()->addScalar(cEnvironment_, pRelation_, pElement, vecScalar);
		}
	}
	// convert keys into ordering key with unknown ordering
	VECTOR<Plan::Order::Key*> vecKey;
	vecKey.reserve(vecScalar.GETSIZE());
	Opt::MapContainer(vecScalar, vecKey,
					  boost::bind(&Plan::Order::Key::create,
								  boost::ref(cEnvironment_),
								  _1,
								  Plan::Order::Direction::Unknown));

	Plan::Order::Specification* pSortSpecification =
		Plan::Order::Specification::create(cEnvironment_, vecKey, isAbleToBitSetSort(pGBC, vecScalar));

	// create grouping filter
	if (cEnvironment_.hasCascade()) {
		return DPlan::Relation::Grouping::create(cEnvironment_,
												 pSortSpecification,
												 pRelation_);
	} else {
		return Plan::Relation::Grouping::create(cEnvironment_,
												pSortSpecification,
												pRelation_);
	}
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
GroupByClauseImpl::Primitive::
getDistributeFilter(Opt::Environment& cEnvironment_,
					Plan::Interface::IRelation* pRelation_,
					Statement::Object* pStatement_) const
{
	return getFilter(cEnvironment_, pRelation_, pStatement_);
}




// FUNCTION public
//	Query::GroupByClauseImpl::Primitive::isAbleToBitSetSort -- 
//
// NOTES
//
// ARGUMENTS
//
//	Statement::GroupByClause* pGBC_
//	VECTOR<Plan::Interface::IScalar*>& vecScalar
//	
// RETURN
//	bool
//
// EXCEPTIONS

//virtual
bool
GroupByClauseImpl::Primitive::
isAbleToBitSetSort(Statement::GroupByClause* pGBC_,
				   VECTOR<Plan::Interface::IScalar*>& vecScalar) const
{
	if (!pGBC_->hasHavingClause()
		&& vecScalar.getSize() == 1
		&& vecScalar[0]->isField()) {
		Plan::Scalar::Field* pField = vecScalar[0]->getField();
		
		// 配列の場合で、要素を展開しない場合は、使用不可とする
		if (pField->getDataType().isArrayType()
			&& !pField->isExpandElement()) {
				return false;
		}
		
		return true;
	}
	return false;
}
/////////////////////////////
// Query::GroupByClause
/////////////////////////////

// FUNCTION public
//	Query::GroupByClause::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::GroupByClause* pStatement_
//	
// RETURN
//	const GroupByClause*
//
// EXCEPTIONS

//static
const GroupByClause*
GroupByClause::
create(const Statement::GroupByClause* pStatement_)
{
	return &_analyzerPrimitive;
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
