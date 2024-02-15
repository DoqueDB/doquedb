// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/SelectList.cpp --
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

#include "boost/lambda/bind.hpp"
#include "boost/lambda/if.hpp"
#include "boost/lambda/lambda.hpp"


#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Query/SelectList.h"
#include "Analysis/Query/Utility.h"

#include "Exception/InvalidDerivedName.h"
#include "Exception/InvalidRankFrom.h"
#include "Exception/NonGroupingColumn.h"

#include "Opt/Environment.h"
#include "Opt/NameMap.h"

#include "Plan/Order/Key.h"
#include "Plan/Order/Specification.h"
#include "Plan/Relation/Argument.h"
#include "Plan/Relation/Grouping.h"
#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"

#include "DPlan/Relation/Grouping.h"

#include "Statement/SelectList.h"
#include "Statement/SelectSubList.h"
#include "Statement/SelectSubListList.h"
#include "Statement/ValueExpression.h"


_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace SelectListImpl
{
	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::SelectListImpl::Base -- Base class of select list analyzer
	//
	// NOTES
	class Base
		: public Query::SelectList
	{
	public:
		typedef Base This;
		typedef Query::SelectList Super;

		// constructor
		Base() : Super() {}
		// destructor
		virtual ~Base() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getFilter(Opt::Environment& cEnvironment_,
						  Plan::Interface::IRelation* pRelation_,
						  Statement::Object* pStatement_) const;

	protected:
	private:
		// generate Plan::Relation::RowInfo
		virtual Plan::Relation::RowInfo*
				getRowInfo(Opt::Environment& cEnvironment_,
						   Plan::Interface::IRelation* pRelation_,
						   Statement::Object* pStatement_) const = 0;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::SelectListImpl::Asterisk -- select list analyzer for asterisk
	//
	// NOTES
	class Asterisk
		: public Base
	{
	public:
		typedef Asterisk This;
		typedef Base Super;

		// constructor
		Asterisk() {}
		// destructor
		~Asterisk() {}

	/////////////////////////////
	//Interface::Analyzer::
		// get element numbers added by following methods
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;	

	protected:
	private:
	/////////////////////////////
	//Base::
		// generate Plan::Relation::RowInfo
		virtual Plan::Relation::RowInfo*
				getRowInfo(Opt::Environment& cEnvironment_,
						   Plan::Interface::IRelation* pRelation_,
						   Statement::Object* pStatement_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::SelectListImpl::List -- select list analyzer for select list
	//
	// NOTES
	class List
		: public Base
	{
	public:
		typedef List This;
		typedef Base Super;

		// constructor
		List() {}
		// destructor
		virtual ~List() {}

	/////////////////////////////
	//Interface::Analyzer::
		// get element numbers added by following methods
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;	

	protected:
	private:
	/////////////////////////////
	//Base::
		// generate Plan::Relation::RowInfo
		virtual Plan::Relation::RowInfo*
				getRowInfo(Opt::Environment& cEnvironment_,
						   Plan::Interface::IRelation* pRelation_,
						   Statement::Object* pStatement_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::SelectListImpl::AggregationList -- select list analyzer for select list
	//
	// NOTES
	class AggregationList
		: public List
	{
	public:
		typedef AggregationList This;
		typedef List Super;

		// constructor
		AggregationList() {}
		// destructor
		~AggregationList() {}

	/////////////////////////////
	//Interface::Analyzer::
		// generate Plan::Tree::Node from Statement::Object
		virtual Plan::Interface::IRelation*
				getFilter(Opt::Environment& cEnvironment_,
						  Plan::Interface::IRelation* pRelation_,
						  Statement::Object* pStatement_) const;

	protected:
	private:
	/////////////////////////////
	//Base::
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES
	const SelectListImpl::Asterisk _analyzerAsterisk;
	const SelectListImpl::List _analyzerList;
	const SelectListImpl::AggregationList _analyzerAggregationList;

} // namespace

///////////////////////////////////////////////////
// Analysis::Query::SelectListImpl::Base
///////////////////////////////////////////////////

// FUNCTION public
//	Query::SelectListImpl::Base::getFilter -- generate Plan::Tree::Node from Statement::Object
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
SelectListImpl::Base::
getFilter(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Plan::Relation::RowInfo* pRowInfo = getRowInfo(cEnvironment_, pRelation_, pStatement_);
	return Plan::Relation::Projection::create(cEnvironment_, pRowInfo, pRelation_);
}

	
///////////////////////////////////////////////////////
// Analysis::Query::SelectListImpl::Asterisk
///////////////////////////////////////////////////////

// FUNCTION public
//	Query::SelectListImpl::Asterisk::getDegree -- get element numbers added by following methods
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
SelectListImpl::Asterisk::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	// -1 means number is same as filtered relation
	return -1;
}

// FUNCTION private
//	Query::SelectListImpl::Asterisk::getRowInfo -- create rowinfo
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Relation::RowInfo*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowInfo*
SelectListImpl::Asterisk::
getRowInfo(Opt::Environment& cEnvironment_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Plan::Relation::RowInfo* pRowInfo = 0;
	if (cEnvironment_.isInExists()) {
		// use empty row
		pRowInfo = Plan::Relation::RowInfo::create(cEnvironment_);
	} else if (cEnvironment_.checkStatus(Opt::Environment::Status::RankFrom)) {
		// use only rowid
		pRowInfo = pRelation_->getKeyInfo(cEnvironment_);
		if (pRowInfo->getSize() != 1) {
			// illegal subquery
			_SYDNEY_THROW0(Exception::InvalidRankFrom);
		}
	} else {
		// use operands result row
		pRowInfo = pRelation_->getRowInfo(cEnvironment_);

		if (cEnvironment_.isGrouping()) {
			using namespace boost::lambda;

			// check grouping column here
			Plan::Relation::RowInfo::Iterator found =
				pRowInfo->find(bind(&Opt::Environment::isGroupingColumn,
									&cEnvironment_,
									bind(&Plan::Relation::RowInfo::Element::second,
										 _1))
							   == false);
			if (found != pRowInfo->end()) {
				// non-grouping column is used
				_SYDNEY_THROW1(Exception::NonGroupingColumn,
							   pRowInfo->getScalarName(cEnvironment_, *found));
			}
		}
	}
	return pRowInfo;
}

////////////////////////////////////////////////////
// Analysis::Query::SelectListImpl::List
////////////////////////////////////////////////////

// FUNCTION public
//	Query::SelectListImpl::List::getDegree -- get element numbers added by following methods
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
SelectListImpl::List::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::SelectList* pSL = _SYDNEY_DYNAMIC_CAST(Statement::SelectList*, pStatement_);
	; _SYDNEY_ASSERT(pSL);
	Statement::SelectSubListList* pSSLL = pSL->getSelectSubListList();
	; _SYDNEY_ASSERT(pSSLL);
	; _SYDNEY_ASSERT(pSSLL->getCount());

	int n = pSSLL->getCount();
	int iResult = 0;
	int i = 0;
	do {
		iResult += pSSLL->getSelectSubListAt(i)->getAnalyzer2()->getDegree(
											 cEnvironment_,
											 pSSLL->getSelectSubListAt(i));
	} while (++i < n);

	return iResult;
}

// FUNCTION private
//	Query::SelectListImpl::List::getRowInfo -- generate Plan::Tree::Node from Statement::Object
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Plan::Relation::RowInfo*
//
// EXCEPTIONS

//virtual
Plan::Relation::RowInfo*
SelectListImpl::List::
getRowInfo(Opt::Environment& cEnvironment_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::SelectList* pSL = _SYDNEY_DYNAMIC_CAST(Statement::SelectList*, pStatement_);
	; _SYDNEY_ASSERT(pSL);
	Statement::SelectSubListList* pSSLL = pSL->getSelectSubListList();
	; _SYDNEY_ASSERT(pSSLL);
	; _SYDNEY_ASSERT(pSSLL->getCount());

	Opt::Environment::AutoPop cAutoPop =
		cEnvironment_.pushStatus(Opt::Environment::Status::SelectList);

	// prepare return value
	Plan::Relation::RowInfo* pRowInfo = Plan::Relation::RowInfo::create(cEnvironment_);

	// add to rowinfo
	int n = pSSLL->getCount();
	int i = 0;
	do {
		pSSLL->getSelectSubListAt(i)->getAnalyzer2()->addColumns(
											 cEnvironment_,
											 pRowInfo,
											 pRelation_,
											 pSSLL->getSelectSubListAt(i));
	} while (++i < n);

	return pRowInfo;
}

////////////////////////////////////////////////////
// Analysis::Query::SelectListImpl::AggregationList
////////////////////////////////////////////////////

// FUNCTION public
//	Query::SelectListImpl::AggregationList::getFilter -- generate Plan::Tree::Node from Statement::Object
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
SelectListImpl::AggregationList::
getFilter(Opt::Environment& cEnvironment_,
		  Plan::Interface::IRelation* pRelation_,
		  Statement::Object* pStatement_) const
{
	Plan::Interface::IRelation* pResult = pRelation_;
	if (cEnvironment_.isGrouping() == false) {
		// create empty sort specification
		Plan::Order::Specification* pSortSpecification =
			Plan::Order::Specification::create(cEnvironment_, VECTOR<Plan::Order::Key*>(), false);

		if (cEnvironment_.hasCascade()) {
			pResult = DPlan::Relation::Grouping::create(cEnvironment_,
														pSortSpecification,
														pResult);
		} else 	{
			if (cEnvironment_.checkStatus(Opt::Environment::Status::SimpleForm)) {
			// create filter trying to find simple aggregation

				pResult = Plan::Relation::Grouping::Simple::create(cEnvironment_,
																   pSortSpecification,
																   pResult);
			
			} else {
				// create grouping filter
				pResult = Plan::Relation::Grouping::create(cEnvironment_,
														   pSortSpecification,
														   pResult);
			}
		}
	
	}
	return Super::getFilter(cEnvironment_, pResult, pStatement_);
	
}

/////////////////////////////
// Query::SelectList
/////////////////////////////

// FUNCTION public
//	Query::SelectList::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::SelectList* pStatement_
//	
// RETURN
//	const SelectList*
//
// EXCEPTIONS

//static
const SelectList*
SelectList::
create(const Statement::SelectList* pStatement_)
{
	if (pStatement_->isAsterisk()) {
		return &_analyzerAsterisk;
	} else if (pStatement_->getSelectSubListList()->getExpressionType()
			   == Statement::ValueExpression::type_Aggregation) {
		return &_analyzerAggregationList;
	} else {
		return &_analyzerList;
	}
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
