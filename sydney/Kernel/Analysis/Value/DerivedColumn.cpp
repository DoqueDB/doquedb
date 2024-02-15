// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Predicate/DerivedColumn.cpp --
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
const char moduleName[] = "Analysis::Value";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Value/DerivedColumn.h"

#include "Common/Assert.h"

#include "Exception/InvalidDerivedName.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Opt/Environment.h"
#include "Opt/NameMap.h"

#include "Plan/Relation/RowElement.h"
#include "Plan/Relation/RowInfo.h"

#include "Statement/ColumnName.h"
#include "Statement/DerivedColumn.h"
#include "Statement/ValueExpressionList.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_VALUE_BEGIN

namespace DerivedColumnImpl
{
	///////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::DerivedColumnImpl::Single --
	//
	// NOTES
	class Single
		: public Value::DerivedColumn
	{
	public:
		typedef Single This;
		typedef Value::DerivedColumn Super;

		// constructor
		Single() : Super() {}
		// destructor
		virtual ~Single() {}

	//////////////////////////////////
	// Analysis::Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
		virtual void addColumns(Opt::Environment& cEnvironment_,
								Plan::Relation::RowInfo* pRowInfo_,
								Plan::Interface::IRelation* pRelation_,
								Statement::Object* pStatement_) const;
	protected:
	private:
	};

	////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::DerivedColumnImpl::Row --
	//
	// NOTES
	class Row
		: public Value::DerivedColumn
	{
	public:
		typedef Row This;
		typedef Value::DerivedColumn Super;

		// constructor
		Row() : Super() {}
		// destructor
		virtual ~Row() {}

	//////////////////////////////////
	// Analysis::Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
		virtual void addColumns(Opt::Environment& cEnvironment_,
								Plan::Relation::RowInfo* pRowInfo_,
								Plan::Interface::IRelation* pRelation_,
								Statement::Object* pStatement_) const;
	protected:
	private:
	};

	////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Analysis::Value::DerivedColumnImpl::Subquery --
	//
	// NOTES
	class Subquery
		: public Value::DerivedColumn
	{
	public:
		typedef Subquery This;
		typedef Value::DerivedColumn Super;

		// constructor
		Subquery() : Super() {}
		// destructor
		virtual ~Subquery() {}

	//////////////////////////////////
	// Analysis::Interface::Analyzer::
		virtual int getDegree(Opt::Environment& cEnvironment_,
							  Statement::Object* pStatement_) const;
		virtual void addColumns(Opt::Environment& cEnvironment_,
								Plan::Relation::RowInfo* pRowInfo_,
								Plan::Interface::IRelation* pRelation_,
								Statement::Object* pStatement_) const;
	protected:
	private:
	};
}

namespace
{
	// VARIABLE local
	//	$$$::_analyzerXXX -- instance
	//
	// NOTES

	const DerivedColumnImpl::Single _analyzerSingle;
	const DerivedColumnImpl::Row _analyzerRow;
	const DerivedColumnImpl::Subquery _analyzerSubquery;
}

////////////////////////////////////////////////
// Analysis::Value::DerivedColumnImpl::Single

// FUNCTION public
//	Value::DerivedColumnImpl::Single::getDegree -- 
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
DerivedColumnImpl::Single::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	return 1;
}

// FUNCTION public
//	Value::DerivedColumnImpl::Single::addColumns -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo* pRowInfo_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
DerivedColumnImpl::Single::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::DerivedColumn* pDC = _SYDNEY_DYNAMIC_CAST(Statement::DerivedColumn*, pStatement_);
	; _SYDNEY_ASSERT(pDC);
	Statement::ValueExpression* pVE = pDC->getValueExpression();
	; _SYDNEY_ASSERT(pVE);

	if (pRelation_ == 0) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// single value
	Plan::Relation::RowElement* pElement =
		pVE->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pVE);

	if (Statement::ColumnName* pCN = pDC->getColumnName()) {
		; _SYDNEY_ASSERT(pCN->getIdentifierString());

		// register name
		const ModUnicodeString& cstrName = *pCN->getIdentifierString();
		Opt::NameMap* pNameMap = cEnvironment_.getNameMap();
		Opt::NameMap::NameScalarMap& cScalarMap = pNameMap->getMap(pRelation_);
		cScalarMap.insert(cstrName, pElement);
		pRowInfo_->PUSHBACK(Plan::Relation::RowInfo::Element(cstrName, pElement));

	} else {
		pRowInfo_->PUSHBACK(Plan::Relation::RowInfo::Element(STRING(), pElement));
	}
}

////////////////////////////////////////////////
// Analysis::Value::DerivedColumnImpl::Row

// FUNCTION public
//	Value::DerivedColumnImpl::Row::getDegree -- 
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
DerivedColumnImpl::Row::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::DerivedColumn* pDC = _SYDNEY_DYNAMIC_CAST(Statement::DerivedColumn*, pStatement_);
	; _SYDNEY_ASSERT(pDC);
	Statement::ValueExpression* pVE = pDC->getValueExpression();
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Rowconst);

	return pVE->getAnalyzer2()->getDegree(cEnvironment_,
										  pVE);
}

// FUNCTION public
//	Value::DerivedColumnImpl::Row::addColumns -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo* pRowInfo_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
DerivedColumnImpl::Row::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::DerivedColumn* pDC = _SYDNEY_DYNAMIC_CAST(Statement::DerivedColumn*, pStatement_);
	; _SYDNEY_ASSERT(pDC);
	Statement::ValueExpression* pVE = pDC->getValueExpression();
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_Rowconst);

	// multi element values
	if (pDC->getColumnName()) {
		// multi element can't be assigned by derived column name
		_SYDNEY_THROW0(Exception::InvalidDerivedName);
	}

	pVE->getAnalyzer2()->addColumns(cEnvironment_,
									pRowInfo_,
									pRelation_,
									pVE);
}

////////////////////////////////////////////////
// Analysis::Value::DerivedColumnImpl::Subquery

// FUNCTION public
//	Value::DerivedColumnImpl::Subquery::getDegree -- 
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
DerivedColumnImpl::Subquery::
getDegree(Opt::Environment& cEnvironment_,
		  Statement::Object* pStatement_) const
{
	Statement::DerivedColumn* pDC = _SYDNEY_DYNAMIC_CAST(Statement::DerivedColumn*, pStatement_);
	; _SYDNEY_ASSERT(pDC);
	Statement::ValueExpression* pVE = pDC->getValueExpression();
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_RowSubquery);

	// get number of columns
	return pVE->getAnalyzer2()->getDegree(cEnvironment_,
										  pVE);
}

// FUNCTION public
//	Value::DerivedColumnImpl::Subquery::addColumns -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::RowInfo* pRowInfo_
//	Plan::Interface::IRelation* pRelation_
//	Statement::Object* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//virtual
void
DerivedColumnImpl::Subquery::
addColumns(Opt::Environment& cEnvironment_,
		   Plan::Relation::RowInfo* pRowInfo_,
		   Plan::Interface::IRelation* pRelation_,
		   Statement::Object* pStatement_) const
{
	Statement::DerivedColumn* pDC = _SYDNEY_DYNAMIC_CAST(Statement::DerivedColumn*, pStatement_);
	; _SYDNEY_ASSERT(pDC);
	Statement::ValueExpression* pVE = pDC->getValueExpression();
	; _SYDNEY_ASSERT(pVE);
	; _SYDNEY_ASSERT(pVE->getOperator() == Statement::ValueExpression::op_RowSubquery);

	// get number of columns
	int n = pVE->getAnalyzer2()->getDegree(cEnvironment_,
										   pVE);

	// multi element values
	if (Statement::ColumnName* pCN = pDC->getColumnName()) {
		if (n != 1) {
			// multi element can't be assigned by derived column name
			_SYDNEY_THROW0(Exception::InvalidDerivedName);
		} else {
			; _SYDNEY_ASSERT(pCN->getIdentifierString());

			Plan::Relation::RowElement* pElement =
				pVE->getAnalyzer2()->getRowElement(cEnvironment_, pRelation_, pVE);

			// register name
			const ModUnicodeString& cstrName = *pCN->getIdentifierString();
			Opt::NameMap* pNameMap = cEnvironment_.getNameMap();
			Opt::NameMap::NameScalarMap& cScalarMap = pNameMap->getMap(pRelation_);
			cScalarMap.insert(cstrName, pElement);
			pRowInfo_->PUSHBACK(Plan::Relation::RowInfo::Element(cstrName, pElement));
		}
	} else {
		pVE->getAnalyzer2()->addColumns(cEnvironment_,
										pRowInfo_,
										pRelation_,
										pVE);
	}
}

//////////////////////////////
// Value::DerivedColumn::

// FUNCTION public
//	Value::DerivedColumn::create -- constuctor
//
// NOTES
//
// ARGUMENTS
//	const Statement::DerivedColumn* pStatement_
//	
// RETURN
//	const DerivedColumn*
//
// EXCEPTIONS

//static
const DerivedColumn*
DerivedColumn::
create(const Statement::DerivedColumn* pStatement_)
{
	Statement::ValueExpression* pVE = pStatement_->getValueExpression();
	; _SYDNEY_ASSERT(pVE);
	switch (pVE->getOperator()) {
	case Statement::ValueExpression::op_Rowconst:
		{
			return &_analyzerRow;
		}
	case Statement::ValueExpression::op_RowSubquery:
		{
			return &_analyzerSubquery;
		}
	default:
		{
			return &_analyzerSingle;
		}
	}
	_SYDNEY_THROW0(Exception::NotSupported);
}

_SYDNEY_ANALYSIS_VALUE_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
