// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/QuerySpecification.cpp --
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
const char moduleName[] = "Analysis::Query";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Query/QuerySpecification.h"

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/Argument.h"
#include "Plan/Relation/Distinct.h"
#include "Plan/Relation/RowInfo.h"

#include "DPlan/Relation/Distinct.h"

#include "Statement/BulkSpecification.h"
#include "Statement/QuerySpecification.h"
#include "Statement/SelectList.h"
#include "Statement/TableExpression.h"
#include "Statement/ValueExpression.h"
#include "Statement/SelectTargetList.h"
#include "Statement/VariableName.h"

#include "Plan/Utility/ObjectSet.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace QuerySpecificationImpl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QuerySpecification::Base -- base class for query specification analyzer
	//
	// NOTES
	class Base
		: public Query::QuerySpecification
	{
	public:
		typedef Base This;
		typedef Query::QuerySpecification Super;

		// constructor
		Base() {}
		// destructor
		virtual ~Base() {}

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
		virtual Plan::Interface::IRelation* addSelectTargetList(Opt::Environment& cEnvironment_,
																Plan::Interface::IRelation* pRelation_,
																Statement::Object* pStatement_) const;
		
	protected:
		struct _AddFilterArgument
		{
			_AddFilterArgument(Plan::Interface::IRelation* pRelation_,
							   const Statement::QuerySpecification* pQS_)
				: m_pRelation(pRelation_),
				  m_pQS(pQS_)
			{}

			Plan::Interface::IRelation* m_pRelation;
			const Statement::QuerySpecification* m_pQS;
		};
		// add filter part
		virtual Plan::Interface::IRelation*
				addFilter(Opt::Environment& cEnvironment_,
						  const _AddFilterArgument& cArgument_) const = 0;
	private:
	};

	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QuerySpecification::All -- select ALL
	//
	// NOTES
	class All
		: public Base
	{
	public:
		typedef All This;
		typedef Base Super;

		// constructor
		All() : Super() {}
		// destructor
		virtual ~All() {}

	protected:
		// add filter part
		virtual Plan::Interface::IRelation*
				addFilter(Opt::Environment& cEnvironment_,
						  const _AddFilterArgument& cArgument_) const;
	private:
	};

	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QuerySpecification::Distinct -- select DISTINCT
	//
	// NOTES
	class Distinct
		: public All
	{
	public:
		typedef Distinct This;
		typedef All Super;

		// constructor
		Distinct() : Super() {}
		// destructor
		virtual ~Distinct() {}

	protected:
		// add filter part
		virtual Plan::Interface::IRelation*
				addFilter(Opt::Environment& cEnvironment_,
						  const _AddFilterArgument& cArgument_) const;
	private:
	};

	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QuerySpecification::Bulk -- bulk output
	//
	// NOTES
	class Bulk
		: public All
	{
	public:
		typedef Bulk This;
		typedef All Super;

		// constructor
		Bulk() : Super() {}
		// destructor
		virtual ~Bulk() {}

	protected:
		// add filter part
		virtual Plan::Interface::IRelation*
				addFilter(Opt::Environment& cEnvironment_,
						  const _AddFilterArgument& cArgument_) const;
	private:
	};

	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Query::Impl::QuerySpecification::BulkDistinct -- bulk output with distinct
	//
	// NOTES
	class BulkDistinct
		: public Distinct
	{
	public:
		typedef Bulk This;
		typedef Distinct Super;

		// constructor
		BulkDistinct() : Super() {}
		// destructor
		virtual ~BulkDistinct() {}

	protected:
		// add filter part
		virtual Plan::Interface::IRelation*
				addFilter(Opt::Environment& cEnvironment_,
						  const _AddFilterArgument& cArgument_) const;
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instances
	//
	// NOTES
	const QuerySpecificationImpl::All _analyzerAll;
	const QuerySpecificationImpl::Distinct _analyzerDistinct;
	const QuerySpecificationImpl::Bulk _analyzerBulk;
	const QuerySpecificationImpl::BulkDistinct _analyzerBulkDistinct;

	// CONST local
	//	$$$::_cDispatchTable --
	//
	// NOTES
	const QuerySpecification* const _cDispatchTable[] =
	{
		&_analyzerAll,
		&_analyzerDistinct,
		&_analyzerBulk,
		&_analyzerBulkDistinct
	};
} // namespace

/////////////////////////////////////////////////////
// Analysis::Query::Impl::QuerySpecificationImpl::Base
/////////////////////////////////////////////////////

// FUNCTION public
//	Query::QuerySpecificationImpl::Base::getRelation -- generate Plan::Tree::Node from Statement::Object
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
QuerySpecificationImpl::Base::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::QuerySpecification* pQS =
		_SYDNEY_DYNAMIC_CAST(Statement::QuerySpecification*, pStatement_);

	Statement::TableExpression* pTable = pQS->getTable();

	// Get relation from TableExpression
	Plan::Interface::IRelation* pResult =
		pTable->getAnalyzer2()->getRelation(cEnvironment_,
											pTable);
	; _SYDNEY_ASSERT(pResult);

	// add filter and return an result
	pResult = addFilter(cEnvironment_, _AddFilterArgument(pResult, pQS));
	if (pQS->getSelectTargetList()) {
		pResult = addSelectTargetList(cEnvironment_, pResult, pQS->getSelectTargetList());
	}
	return pResult;
}

// FUNCTION public
//	Query::QuerySpecificationImpl::Base::getDistributeRelation -- 
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
QuerySpecificationImpl::Base::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION public
//	Query::QuerySpecificationImpl::Base::addSelectTargetList -- generate Plan::Tree::Node from Statement::Object
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
QuerySpecificationImpl::Base::
addSelectTargetList(Opt::Environment& cEnvironment_,
					Plan::Interface::IRelation* pRelation_,
					Statement::Object* pStatement_) const
{
	Statement::SelectTargetList* pSTL =
		_SYDNEY_DYNAMIC_CAST(Statement::SelectTargetList*, pStatement_);

	const STRING* variableName;
	if (pSTL->getCount() == 1) {
		variableName = pSTL->getVariableNameAt(0)->getName();
	} else if (pSTL->getCount() == 0) {
		return pRelation_;
	} else {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	Plan::Utility::RelationSet cRelationSet;
	pRelation_->getUsedTable(cEnvironment_, cRelationSet);
	if (cRelationSet.getSize() !=1) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	
	pRelation_->addOutputVariableName(cEnvironment_, *variableName);
	return pRelation_;
}

	
// FUNCTION public
//	Query::QuerySpecificationImpl::Base::getDegree -- 
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
QuerySpecificationImpl::Base::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::QuerySpecification* pQS =
		_SYDNEY_DYNAMIC_CAST(Statement::QuerySpecification*, pStatement_);

	Statement::SelectList* pSelect = pQS->getSelectList();
	if (pSelect) {
		int iResult = pSelect->getAnalyzer2()->getDegree(cEnvironment_,
														 pSelect);
		if (iResult < 0) {
			// use table expression
			Statement::TableExpression* pTable = pQS->getTable();

			iResult = pTable->getAnalyzer2()->getDegree(cEnvironment_,
														pTable);
		}
		return iResult;
	} else {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

/////////////////////////////////////////////////////
// Analysis::Query::QuerySpecificationImpl::All
/////////////////////////////////////////////////////

// FUNCTION protected
//	Query::QuerySpecificationImpl::All::addFilter -- add filter part
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const _AddFilterArgument& cArgument_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QuerySpecificationImpl::All::
addFilter(Opt::Environment& cEnvironment_,
		  const _AddFilterArgument& cArgument_) const
{
	Statement::SelectList* pSelect = cArgument_.m_pQS->getSelectList();

	// Create filter from select list 
	return pSelect->getAnalyzer2()->getFilter(cEnvironment_,
											  cArgument_.m_pRelation,
											  pSelect);
}

////////////////////////////////////////////////////////
// Analysis::Query::QuerySpecificationImpl::Distinct
////////////////////////////////////////////////////////

// FUNCTION protected
//	Query::QuerySpecificationImpl::Distinct::addFilter -- add filter part
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const _AddFilterArgument& cArgument_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QuerySpecificationImpl::Distinct::
addFilter(Opt::Environment& cEnvironment_,
		  const _AddFilterArgument& cArgument_) const
{
	// create relation using superclass implementation
	Plan::Interface::IRelation* pRelation = Super::addFilter(cEnvironment_, cArgument_);
	return Plan::Relation::Distinct::create(cEnvironment_,
											pRelation);
}

/////////////////////////////////////////////////////
// Analysis::Query::QuerySpecificationImpl::Bulk
/////////////////////////////////////////////////////

// FUNCTION protected
//	Query::QuerySpecificationImpl::Bulk::addFilter -- add filter part
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const _AddFilterArgument& cArgument_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QuerySpecificationImpl::Bulk::
addFilter(Opt::Environment& cEnvironment_,
		  const _AddFilterArgument& cArgument_) const
{
	Statement::BulkSpecification* pBulk = cArgument_.m_pQS->getOutput();
	; _SYDNEY_ASSERT(pBulk);

	return pBulk->getAnalyzer2()->getFilter(cEnvironment_,
											Super::addFilter(cEnvironment_,
															 cArgument_),
											pBulk);
}

///////////////////////////////////////////////////////////
// Analysis::Query::QuerySpecificationImpl::BulkDistinct
///////////////////////////////////////////////////////////

// FUNCTION protected
//	Query::QuerySpecificationImpl::BulkDistinct::addFilter -- add filter part
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const _AddFilterArgument& cArgument_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

//virtual
Plan::Interface::IRelation*
QuerySpecificationImpl::BulkDistinct::
addFilter(Opt::Environment& cEnvironment_,
		  const _AddFilterArgument& cArgument_) const
{
	Statement::BulkSpecification* pBulk = cArgument_.m_pQS->getOutput();
	; _SYDNEY_ASSERT(pBulk);

	return pBulk->getAnalyzer2()->getFilter(cEnvironment_,
											Super::addFilter(cEnvironment_,
															 cArgument_),
											pBulk);
}

////////////////////////////////////
// Query::QuerySpecification
////////////////////////////////////

// FUNCTION public
//	Query::QuerySpecification::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::QuerySpecification* pStatement_
//	
// RETURN
//	const QuerySpecification*
//
// EXCEPTIONS

const QuerySpecification*
QuerySpecification::
create(const Statement::QuerySpecification* pStatement_)
{
	int iDispatchKey =
		(pStatement_->getQuantifier() == Statement::ValueExpression::quant_Distinct
		 ? 1 : 0)
		+ (pStatement_->getOutput() ? 2 : 0);

	return _cDispatchTable[iDispatchKey];
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
