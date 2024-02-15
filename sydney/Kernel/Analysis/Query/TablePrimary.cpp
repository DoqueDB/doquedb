// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Query/TablePrimary.cpp --
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
#include "SyReinterpretCast.h"

#include "Analysis/Query/TablePrimary.h"

#include "Common/Assert.h"

#include "Exception/InvalidDerivedColumn.h"
#include "Exception/NotSupported.h"
#include "Exception/TableNotFound.h"

#include "Opt/Environment.h"

#include "Plan/Relation/Table.h"
#include "Plan/Relation/Unnest.h"

#include "DPlan/Relation/Table.h"

#include "Schema/Table.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Identifier.h"
#include "Statement/TablePrimary.h"
#include "Statement/ValueExpression.h"

#include "ModUnicodeString.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_QUERY_BEGIN

namespace TablePrimaryImpl
{
	/////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TablePrimaryImpl::Base -- base class for table primary analyzer
	//
	// NOTES
	class Base
		: public Query::TablePrimary
	{
	public:
		typedef Base This;
		typedef Query::TablePrimary Super;

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
	//	virtual int getDegree(Opt::Environment& cEnvironment_,
	//						  Statement::Object* pStatement_) const;
	protected:
	private:
		// create table node without correlation spec
		virtual Plan::Interface::IRelation*
				getTableRelation(Opt::Environment& cEnvironment_,
								 Statement::Object* pStatement_) const = 0;
		// create derived table node by correlation spec
		Plan::Interface::IRelation*
				getDerivedTable(Opt::Environment& cEnvironment_,
								Statement::Object* pStatement_,
								Plan::Interface::IRelation* pRelation_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TablePrimaryImpl::BaseTable -- TablePrimary for derived table
	//
	// NOTES
	class BaseTable
		: public Base
	{
	public:
		typedef BaseTable This;
		typedef Base Super;

		// constructor
		BaseTable() {}
		// destructor
		~BaseTable() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;

	protected:
	private:
		// create table node without correlation spec
		virtual Plan::Interface::IRelation*
				getTableRelation(Opt::Environment& cEnvironment_,
								 Statement::Object* pStatement_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TablePrimaryImpl::DerivedTable -- TablePrimary for derived table
	//
	// NOTES
	class DerivedTable
		: public Base
	{
	public:
		typedef DerivedTable This;
		typedef Base Super;

		// constructor
		DerivedTable() {}
		// destructor
		virtual ~DerivedTable() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;

	protected:
		// create table node without correlation spec
		virtual Plan::Interface::IRelation*
				getTableRelation(Opt::Environment& cEnvironment_,
								 Statement::Object* pStatement_) const;
	private:
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TablePrimaryImpl::JoinedTable -- TablePrimary for joined table
	//
	// NOTES
	class JoinedTable
		: public Base
	{
	public:
		typedef JoinedTable This;
		typedef Base Super;

		// constructor
		JoinedTable() {}
		// destructor
		virtual ~JoinedTable() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;

	protected:
		// create table node without correlation spec
		virtual Plan::Interface::IRelation*
				getTableRelation(Opt::Environment& cEnvironment_,
								 Statement::Object* pStatement_) const;
	private:
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TablePrimaryImpl::BulkInput -- TablePrimary for bulk input
	//
	// NOTES
	//	(non-public function)
	//	SELECT * FROM INPUT FOR <table name> FROM <bulk specification>
	class BulkInput
		: public DerivedTable
	{
	public:
		typedef BulkInput This;
		typedef DerivedTable Super;

		// constructor
		BulkInput() {}
		// destructor
		~BulkInput() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;

	protected:
	private:
		// create table node without correlation spec
		virtual Plan::Interface::IRelation*
				getTableRelation(Opt::Environment& cEnvironment_,
								 Statement::Object* pStatement_) const;
	};

	///////////////////////////////////////////////////////////////////
	// CLASS
	//	Query::Impl::TablePrimaryImpl::UnnestTable -- TablePrimary for unnest table
	//
	// NOTES
	class UnnestTable
		: public Base
	{
	public:
		typedef UnnestTable This;
		typedef Base Super;

		// constructor
		UnnestTable() {}
		// destructor
		virtual ~UnnestTable() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;

	protected:
		// create table node without correlation spec
		virtual Plan::Interface::IRelation*
				getTableRelation(Opt::Environment& cEnvironment_,
								 Statement::Object* pStatement_) const;
	private:
	};
}

namespace
{
	// VARIABLE local
	//	_analyzerXXX -- instance
	//
	// NOTES
	const TablePrimaryImpl::BaseTable _analyzerTable;
	const TablePrimaryImpl::DerivedTable _analyzerDerived;
	const TablePrimaryImpl::JoinedTable _analyzerJoined;
	const TablePrimaryImpl::BulkInput _analyzerBulk;
	const TablePrimaryImpl::UnnestTable _analyzerUnnest;

} //namespace

////////////////////////////////
// Query::TablePrimaryImpl::Base
////////////////////////////////

// FUNCTION public
//	Query::TablePrimaryImpl::Base::getRelation -- generate Plan::Tree::Node from Statement::Object
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
TablePrimaryImpl::Base::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Plan::Interface::IRelation* pRelation = getTableRelation(cEnvironment_, pStatement_);
	return getDerivedTable(cEnvironment_, pStatement_, pRelation);
}

// FUNCTION public
//	Query::TablePrimaryImpl::Base::getDistributeRelation -- generate Plan::Tree::Node from Statement::Object
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
TablePrimaryImpl::Base::
getDistributeRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION private
//	Query::TablePrimaryImpl::Base::getDerivedTable -- create derived table node by correlation spec
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Statement::Object* pStatement_
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	Plan::Interface::IRelation*
//
// EXCEPTIONS

Plan::Interface::IRelation*
TablePrimaryImpl::Base::
getDerivedTable(Opt::Environment& cEnvironment_,
				Statement::Object* pStatement_,
				Plan::Interface::IRelation* pRelation_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	; _SYDNEY_ASSERT(pTR);
	Statement::Identifier* pCorr = pTR->getCorrelationName();

	if (pCorr) {
		const ModUnicodeString* pCorrelationName = pCorr->getIdentifier();
		Statement::ColumnNameList* pDerivedColumns = pTR->getDerivedColumnList();
		int iDerivedColumns = (pDerivedColumns) ? pDerivedColumns->getCount() : 0;

		// Obtain degree of the table
		int iDegree = pRelation_->getDegree(cEnvironment_);

		if (iDerivedColumns && iDerivedColumns != iDegree) {
			// <derived column list> specfies wrong number of column names
			_SYDNEY_THROW2(Exception::InvalidDerivedColumn, iDerivedColumns, iDegree);
		}

		// add derived table node
		VECTOR<STRING> vecColumnName;
		if (iDerivedColumns) {
			; _SYDNEY_ASSERT(pDerivedColumns);
			vecColumnName.reserve(iDerivedColumns);
			int i = 0;
			do {
				vecColumnName.PUSHBACK(*(pDerivedColumns->getColumnNameAt(i)->getIdentifierString()));
			} while (++i < iDerivedColumns);
		}
		pRelation_->setCorrelationName(cEnvironment_,
									   *pCorrelationName,
									   vecColumnName);
	} else {
		pRelation_->setCorrelationName(cEnvironment_,
									   STRING(),
									   VECTOR<STRING>());
	}

	return pRelation_;
}

//////////////////////////////////
// Query::TablePrimaryImpl::BaseTable
//////////////////////////////////

// FUNCTION public
//	Query::TablePrimaryImpl::BaseTable::getDegree -- 
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
TablePrimaryImpl::BaseTable::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	; _SYDNEY_ASSERT(pTR);
	Statement::Identifier* pRefer = pTR->getReference();

	; _SYDNEY_ASSERT(!(pTR->getQuery()));
	; _SYDNEY_ASSERT(pRefer->getIdentifier());

	// Obtain schema table object from name
	Schema::Table* pSchemaTable =
		Plan::Relation::Table::getSchemaTable(cEnvironment_,
											  *(pRefer->getIdentifier()));
	; _SYDNEY_ASSERT(pSchemaTable);

	return pSchemaTable->getColumn(cEnvironment_.getTransaction()).getSize() - 1;
}

// FUNCTION private
//	Query::TablePrimaryImpl::BaseTable::getTableRelation -- create table node without correlation spec
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
TablePrimaryImpl::BaseTable::
getTableRelation(Opt::Environment& cEnvironment_,
				 Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	; _SYDNEY_ASSERT(pTR);
	Statement::Identifier* pRefer = pTR->getReference();

	; _SYDNEY_ASSERT(!(pTR->getQuery()));
	; _SYDNEY_ASSERT(pRefer->getIdentifier());

	// create Interface from schema table
	Plan::Interface::IRelation* pTable = 0;
	if (cEnvironment_.hasCascade()) {
		pTable = DPlan::Relation::Table::Retrieve::create(cEnvironment_, *(pRefer->getIdentifier()));
	} else {
		pTable = Plan::Relation::Table::Retrieve::create(cEnvironment_, *(pRefer->getIdentifier()));
	}

	return pTable;
}

/////////////////////////////////////////
// Query::TablePrimaryImpl::DerivedTable
/////////////////////////////////////////

// FUNCTION public
//	Query::TablePrimaryImpl::DerivedTable::getDegree -- 
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
TablePrimaryImpl::DerivedTable::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	; _SYDNEY_ASSERT(pTR);
	Statement::Object* pQuery = pTR->getQuery();

	; _SYDNEY_ASSERT(pQuery);
	; _SYDNEY_ASSERT(!(pTR->getReference()));

	return pQuery->getAnalyzer2()->getDegree(cEnvironment_, pQuery);
}

// FUNCTION protected
//	Query::TablePrimaryImpl::DerivedTable::getTableRelation -- create table node without correlation spec
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
TablePrimaryImpl::DerivedTable::
getTableRelation(Opt::Environment& cEnvironment_,
				 Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	; _SYDNEY_ASSERT(pTR);
	Statement::Object* pQuery = pTR->getQuery();

	; _SYDNEY_ASSERT(pQuery);
	; _SYDNEY_ASSERT(!(pTR->getReference()));

	// push name scope
	Opt::Environment::AutoPop cAutoPop1 =
		cEnvironment_.pushNameScope();
	Opt::Environment::AutoPop cAutoPop2 =
		cEnvironment_.pushStatus(Opt::Environment::Status::Reset);

	// Obtain subquery relation
	if (cEnvironment_.hasCascade()) {
		return pQuery->getAnalyzer2()->getDistributeRelation(cEnvironment_, pQuery);
	} else {
		return pQuery->getAnalyzer2()->getRelation(cEnvironment_, pQuery);
	}
}

/////////////////////////////////////////
// Query::TablePrimaryImpl::JoinedTable
/////////////////////////////////////////

// FUNCTION public
//	Query::TablePrimaryImpl::JoinedTable::getDegree -- 
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
TablePrimaryImpl::JoinedTable::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	Statement::Object* pQuery = pTR->getQuery();

	; _SYDNEY_ASSERT(pQuery);

	return pQuery->getAnalyzer2()->getDegree(cEnvironment_, pQuery);
}

// FUNCTION protected
//	Query::TablePrimaryImpl::JoinedTable::getTableRelation -- create table node without correlation spec
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
TablePrimaryImpl::JoinedTable::
getTableRelation(Opt::Environment& cEnvironment_,
				 Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	Statement::Object* pQuery = pTR->getQuery();

	; _SYDNEY_ASSERT(pQuery);

	if (cEnvironment_.hasCascade()) {
		return pQuery->getAnalyzer2()->getDistributeRelation(cEnvironment_, pQuery);
	} else {
		return pQuery->getAnalyzer2()->getRelation(cEnvironment_, pQuery);
	}
}

////////////////////////////////////
// Query::TablePrimaryImpl::BulkInput
////////////////////////////////////

// FUNCTION public
//	Query::TablePrimaryImpl::BulkInput::getDegree -- 
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
TablePrimaryImpl::BulkInput::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	_SYDNEY_THROW0(Exception::NotSupported);
}

// FUNCTION private
//	Query::TablePrimaryImpl::BulkInput::getTableRelation -- create table node without correlation spec
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
TablePrimaryImpl::BulkInput::
getTableRelation(Opt::Environment& cEnvironment_,
				 Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	; _SYDNEY_ASSERT(pTR);
	Statement::Identifier* pCorr = pTR->getCorrelationName();
	; _SYDNEY_ASSERT(pCorr);
	const ModUnicodeString* pCorrelationName = pCorr->getIdentifier();

	// create relation using superclass implementation
	Plan::Interface::IRelation* pRelation = Super::getTableRelation(cEnvironment_, pStatement_);

	// obtain schema table from correlation name
	Trans::Transaction& cTrans = cEnvironment_.getTransaction();
	const Schema::Table* pSchemaTable = cEnvironment_.getDatabase()->getTable(*pCorrelationName, cTrans);
	if (!pSchemaTable) {
		// table not found
		_SYDNEY_THROW2(Exception::TableNotFound,
					   *pCorrelationName, cEnvironment_.getDatabase()->getName());
	}

	// create bulk input relation
	//...
	return 0;
}

/////////////////////////////////////////
// Query::TablePrimaryImpl::UnnestTable
/////////////////////////////////////////

// FUNCTION public
//	Query::TablePrimaryImpl::UnnestTable::getDegree -- 
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
TablePrimaryImpl::UnnestTable::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}

// FUNCTION protected
//	Query::TablePrimaryImpl::UnnestTable::getTableRelation -- create table node without correlation spec
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
TablePrimaryImpl::UnnestTable::
getTableRelation(Opt::Environment& cEnvironment_,
				 Statement::Object* pStatement_) const
{
	Statement::TablePrimary* pTR =
		_SYDNEY_DYNAMIC_CAST(Statement::TablePrimary*, pStatement_);
	; _SYDNEY_ASSERT(pTR);
	Statement::ValueExpression* pValue = pTR->getCollection();

	; _SYDNEY_ASSERT(pValue);
	; _SYDNEY_ASSERT(!(pTR->getReference()));

	Plan::Interface::IScalar* pScalar = pValue->getAnalyzer2()->getScalar(cEnvironment_,
																		  0,
																		  pValue);
	return Plan::Relation::Unnest::create(cEnvironment_,
										  pScalar);
}

//////////////////////////////////
// Query::TablePrimary
//////////////////////////////////

// FUNCTION public
//	Query::TablePrimary::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::TablePrimary* pStatement_
//	
// RETURN
//	const TablePrimary*
//
// EXCEPTIONS

//static
const TablePrimary*
TablePrimary::
create(const Statement::TablePrimary* pStatement_)
{
	switch (pStatement_->getTablePrimaryType()) {
	case Statement::TablePrimary::Type::Table:
		{
			return &_analyzerTable;
		}
	case Statement::TablePrimary::Type::DerivedTable:
		{
			return &_analyzerDerived;
		}
	case Statement::TablePrimary::Type::JoinedTable:
		{
			return &_analyzerJoined;
		}
	case Statement::TablePrimary::Type::Bulk:
		{
			return &_analyzerBulk;
		}
	case Statement::TablePrimary::Type::UnnestTable:
		{
			return &_analyzerUnnest;
		}
	default:
		{
			break;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_QUERY_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
