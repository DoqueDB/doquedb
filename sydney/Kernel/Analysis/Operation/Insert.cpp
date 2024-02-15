// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/Insert.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Analysis::Operation";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"

#include "Analysis/Operation/Insert.h"

#include "Common/Assert.h"

#include "Exception/InvalidInsertSource.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"

#include "DPlan/Relation/Table.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/InsertStatement.h"
#include "Statement/QueryExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::Impl::InsertImpl --
	//		implementation classes for insert statement analyzer
	//
	// NOTES
	class InsertImpl
		: public Operation::Insert
	{
	public:
		typedef InsertImpl This;
		typedef Operation::Insert Super;

		// constructor
		InsertImpl() : Super() {}
		// destructor
		virtual ~InsertImpl() {}

	/////////////////////////////
	//Interface::Analyzer::
		virtual Plan::Interface::IRelation*
				getRelation(Opt::Environment& cEnvironment_,
							Statement::Object* pStatement_) const;
		virtual Plan::Interface::IRelation*
				getDistributeRelation(Opt::Environment& cEnvironment_,
									  Statement::Object* pStatement_) const;
	protected:
	private:
		// add input data correspondence
		void addInput(Opt::Environment& cEnvironment_,
					  Plan::Relation::Table* pTable_,
					  Plan::Interface::IRelation* pInput_,
					  Statement::InsertStatement* pStatement_) const;
	};
}

namespace
{
	// VARIABLE local
	//	_analyzer -- instance
	//
	// NOTES
	const Impl::InsertImpl _analyzer;

} // namespace

//////////////////////////////////////////
// Operation::Impl::InsertImpl
//////////////////////////////////////////

// FUNCTION public
//	Operation::Impl::InsertImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
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
Impl::InsertImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::InsertStatement* pIS =
		_SYDNEY_DYNAMIC_CAST(Statement::InsertStatement*, pStatement_);

	Statement::QueryExpression* pQE = pIS->getQueryExpression();

	Plan::Interface::IRelation* pInput = 0;
	if (pQE) {
		// create new namescope
		Opt::Environment::AutoPop cAutoPop1 = cEnvironment_.pushNameScope();
		Opt::Environment::AutoPop cAutoPop2 =
			cEnvironment_.pushStatus(Opt::Environment::Status::Reset);

		// convert subquery into relation node
		pInput = pQE->getAnalyzer2()->getRelation(cEnvironment_, pQE);
	}

	// create result relation
	Plan::Relation::Table* pResult = 0;
	if (cEnvironment_.hasCascade()) {
		{
			Opt::Environment::AutoPop cAutoPop1 = cEnvironment_.pushNameScope();
			pResult = Plan::Relation::Table::Insert::create(cEnvironment_,
															*(pIS->getTableNameString()),
															0);
		}
		
		pResult = DPlan::Relation::Table::Insert::create(cEnvironment_,
														 *(pIS->getTableNameString()),
														 pInput,
														 pResult,
														 false /* relocate update */);
	} else {
		pResult = Plan::Relation::Table::Insert::create(cEnvironment_,
														*(pIS->getTableNameString()),
														pInput);
	}
	
	if (pInput) {
		addInput(cEnvironment_,
				 pResult,
				 pInput,
				 pIS);
	}

	// create result row
	Plan::Relation::RowInfo* pResultRow = pResult->getGeneratedColumn(cEnvironment_);
	if (pResultRow) {
		// add projection
		return Plan::Relation::Projection::create(cEnvironment_,
												  pResultRow,
												  pResult);
	}
	// return table itself
	return pResult;
}

// FUNCTION public
//	Operation::Impl::InsertImpl::getDistributeRelation -- 
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
Impl::InsertImpl::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION private
//	Operation::Impl::InsertImpl::addInput -- add input data correspondence
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pTable_
//	Plan::Interface::IRelation* pInput_
//	Statement::InsertStatement* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::InsertImpl::
addInput(Opt::Environment& cEnvironment_,
		 Plan::Relation::Table* pTable_,
		 Plan::Interface::IRelation* pInput_,
		 Statement::InsertStatement* pStatement_) const
{
	// create target columns <-> input data correspondence
	Plan::Relation::RowInfo* pInputRow = pInput_->getRowInfo(cEnvironment_);
	Plan::Relation::RowInfo* pRow = pTable_->getRowInfo(cEnvironment_);

	if (Statement::ColumnNameList* pCNL = pStatement_->getColumnNameList()) {
		/////////////////////////////////
		// with column name list case
		/////////////////////////////////
		int n = pCNL->getCount();
		if (pInputRow && n != pInputRow->getSize()) {
			// Insert source and column specification does not match
			_SYDNEY_THROW2(Exception::InvalidInsertSource,
						   pInputRow->getSize(),
						   n);
		}
		if (pInputRow == 0) {
			// set input row here
			pInputRow = Plan::Relation::RowInfo::create(cEnvironment_,
														pInput_,
														0,
														n);
			pInput_->setRowInfo(cEnvironment_,
								pInputRow);
		}
		for (int i = 0; i < n; ++i) {
			Statement::ColumnName* pCN = pCNL->getColumnNameAt(i);
			Plan::Relation::RowElement* pRowElement =
				pCN->getAnalyzer2()->getRowElement(cEnvironment_,
												   pTable_,
												   pCN);

			; _SYDNEY_ASSERT(pRowElement->isElementOf(pTable_) == true);

			pTable_->addInput(cEnvironment_,
							  pRowElement->getPosition(),
							  (*pInputRow)[i].second->getScalar(cEnvironment_));
		}
	} else {
		/////////////////////////////////
		// without column name list case
		/////////////////////////////////
		int n = pRow->getSize();
		if (pInputRow && n != pInputRow->getSize()) {
			// Insert source and column specification does not match
			_SYDNEY_THROW2(Exception::InvalidInsertSource,
						   pInputRow->getSize(),
						   n);
		}
		if (pInputRow == 0) {
			// set input row here
			pInputRow = Plan::Relation::RowInfo::create(cEnvironment_,
														pInput_,
														0,
														n);
			pInput_->setRowInfo(cEnvironment_,
								pInputRow);
		}
		for (int i = 0; i < n; ++i) {
			pTable_->addInput(cEnvironment_,
							  (*pRow)[i].second->getPosition(),
							  (*pInputRow)[i].second->getScalar(cEnvironment_));
		}
	}
}

////////////////////////////////////////
// Operation::Insert
////////////////////////////////////////

// FUNCTION public
//	Operation::Insert::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::InsertStatement* pStatement_
//	
// RETURN
//	const Insert*
//
// EXCEPTIONS

const Insert*
Insert::
create(const Statement::InsertStatement* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
