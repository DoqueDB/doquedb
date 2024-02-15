// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/Update.cpp --
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
const char moduleName[] = "Analysis::Operation";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Analysis/Operation/Update.h"

#include "Common/Assert.h"

#include "Exception/InvalidInsertSource.h"
#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/Projection.h"
#include "Plan/Relation/RowInfo.h"
#include "Plan/Relation/Table.h"
#include "DPlan/Relation/Table.h"

#include "Schema/Partition.h"
#include "Schema/Table.h"

#include "Statement/ColumnName.h"
#include "Statement/ColumnNameList.h"
#include "Statement/Identifier.h"
#include "Statement/UpdateSetClause.h"
#include "Statement/UpdateSetClauseList.h"
#include "Statement/UpdateStatement.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::Impl::UpdateImpl --
	//		implementation classes for update statement analyzer
	//
	// NOTES
	class UpdateImpl
		: public Operation::Update
	{
	public:
		typedef UpdateImpl This;
		typedef Operation::Update Super;

		// constructor
		UpdateImpl() : Super() {}
		// destructor
		virtual ~UpdateImpl() {}

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
		// add set data correspondence
		void addInput(Opt::Environment& cEnvironment_,
					  Plan::Relation::Table* pTable_,
					  Plan::Interface::IRelation* pInput_,
					  Statement::UpdateSetClauseList* pStatement_) const;

		// add set data correspondence for one set clause
		void addInputSetClause(Opt::Environment& cEnvironment_,
							   Plan::Relation::Table* pTable_,
							   Plan::Relation::RowInfo* pInputRow_,
							   int* piInputRow_,
							   Statement::UpdateSetClause* pStatement_) const;

		// add set data correspondence for one column name
		void addInputColumn(Opt::Environment& cEnvironment_,
							Plan::Relation::Table* pTable_,
							Plan::Relation::RowInfo* pInputRow_,
							int iInputRow_,
							Statement::ColumnName* pStatement_) const;
		void addInputForRelocate(Opt::Environment& cEnvironment_,
								 Plan::Relation::Table* pTable_,
								 Plan::Interface::IRelation* pInput_,
								 Plan::Relation::RowInfo* pRowInfo_,
								 Statement::UpdateSetClauseList* pStatement_) const;
		// add set data correspondence for one column name
		bool isNeedToRelocate(Opt::Environment& cEnvironment_,
							  Plan::Relation::Table* pTable_,
							  Statement::UpdateSetClauseList* pStatement_) const;

	};
}

namespace
{
	// VARIABLE local
	//	_analyzer -- instance
	//
	// NOTES
	const Impl::UpdateImpl _analyzer;

} // namespace

//////////////////////////////////////////
// Operation::Impl::UpdateImpl
//////////////////////////////////////////

// FUNCTION public
//	Operation::Impl::UpdateImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
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
Impl::UpdateImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::UpdateStatement* pUS =
		_SYDNEY_DYNAMIC_CAST(Statement::UpdateStatement*, pStatement_);

	Statement::Identifier* pTarget = pUS->getTargetTable();
	; _SYDNEY_ASSERT(pTarget->getIdentifier());
	Statement::Identifier* pCorrelationName = pUS->getCorrelationName();
	Statement::UpdateSetClauseList* pUSCL = pUS->getSetClauseList();
	; _SYDNEY_ASSERT(pUSCL);
	; _SYDNEY_ASSERT(pUSCL->getCount());

	Plan::Interface::IRelation* pInput = 0;
	Plan::Relation::Table* pTable = 0;
	bool bRelocate = false;
	Plan::Relation::RowInfo* pRowInfo = 0;
	{
		// create new namescope
		Opt::Environment::AutoPop cAutoPop1 = cEnvironment_.pushNameScope();
		Opt::Environment::AutoPop cAutoPop2 =
			cEnvironment_.pushStatus(Opt::Environment::Status::Reset);

		if (cEnvironment_.hasCascade()) {
			// create Interface from schema table
			pTable = DPlan::Relation::Table::Retrieve::create(cEnvironment_,
															  *(pTarget->getIdentifier()));
		} else {
			pTable = Plan::Relation::Table::Retrieve::create(cEnvironment_,
															 *(pTarget->getIdentifier()));			
		}

		if (pCorrelationName) {
			VECTOR<STRING> vecColumnName;
			pTable->setCorrelationName(cEnvironment_,
									   *(pCorrelationName->getIdentifier()),
									   vecColumnName);
		} else {
			pTable->setCorrelationName(cEnvironment_,
									   STRING(),
									   VECTOR<STRING>());
		}

		// default input relation is table
		pInput = pTable;

		if (Statement::ValueExpression* pCondition = pUS->getSearchCondition()) {
			// add search filter
			pInput = pCondition->getAnalyzer2()->getFilter(cEnvironment_,
														   pInput,
														   pCondition);
		}

		pRowInfo = Plan::Relation::RowInfo::create(cEnvironment_);
			// add to rowinfo
		pUSCL->getAnalyzer2()->addColumns(cEnvironment_,
										  pRowInfo,
										  pInput,
										  pUSCL);
		bRelocate = isNeedToRelocate(cEnvironment_, pTable, pUSCL);
		if (cEnvironment_.hasCascade()
			&& bRelocate) {
			pInput = Plan::Relation::Projection::create(cEnvironment_,
														pInput->getRowInfo(cEnvironment_),
														pInput);
		} else {
			// get source data as rowinfo
			pInput = Plan::Relation::Projection::create(cEnvironment_,
														pRowInfo,
														pInput);
		}
	}

	// create result relation
	Plan::Relation::Table* pResult = 0;
	if (cEnvironment_.hasCascade()) {
		if (bRelocate) {
			// 分散キーが更新される場合は再配置する
			pResult = DPlan::Relation::Table::Insert::create(cEnvironment_,
															 *(pTarget->getIdentifier()),
															 pInput,
															 0,
															 true /* relocate update */);
		} else {
			pResult = DPlan::Relation::Table::Update::create(cEnvironment_,
															 pTable,
															 pInput);
		}
	} else {
		pResult = Plan::Relation::Table::Update::create(cEnvironment_,
														pTable,
														pInput);
	}
	if (cEnvironment_.hasCascade() && bRelocate) {
		addInputForRelocate(cEnvironment_,
							pResult,
							pInput,
							pRowInfo,
							pUSCL);
	} else {
		addInput(cEnvironment_,
				 pResult,
				 pInput,
				 pUSCL);
	}

	// create result row
	Plan::Relation::RowInfo* pResultRow = pResult->getGeneratedColumn(cEnvironment_);
	if (pResultRow) {
		return Plan::Relation::Projection::create(cEnvironment_,
												  pResultRow,
												  pResult);
	}
	// return table itself

	return pResult;
}

// FUNCTION public
//	Operation::Impl::DeleteImpl::getDistributeRelation -- 
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
Impl::UpdateImpl::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

// FUNCTION private
//	Operation::Impl::UpdateImpl::addInput -- add input data correspondence
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pTable_
//	Plan::Interface::IRelation* pInput_
//	Statement::UpdateSetClauseList* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::UpdateImpl::
addInput(Opt::Environment& cEnvironment_,
		 Plan::Relation::Table* pTable_,
		 Plan::Interface::IRelation* pInput_,
		 Statement::UpdateSetClauseList* pStatement_) const
{
		
	// create target columns <-> input data correspondence
	int n = pStatement_->getCount();
	; _SYDNEY_ASSERT(n);

	Plan::Relation::RowInfo* pInputRow = pInput_->getRowInfo(cEnvironment_);
	int nInputRow = pInputRow->getSize();
	int iInputRow = 0;


	for (int i = 0; i < n; ++i) {
		Statement::UpdateSetClause* pUSC = pStatement_->getSetClauseAt(i);
		; _SYDNEY_ASSERT(pUSC);

		addInputSetClause(cEnvironment_,
						  pTable_,
						  pInputRow,
						  &iInputRow,
						  pUSC);
	}
	if (iInputRow != nInputRow
		&& !cEnvironment_.hasCascade()) {
		// update source and column specification does not match
		_SYDNEY_THROW2(Exception::InvalidInsertSource,
					   nInputRow,
					   iInputRow);
	}
}

// FUNCTION private
//	Operation::Impl::UpdateImpl::addInputSetClause -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pTable_
//	Plan::Interface::IRelation* pInput_
//	Statement::UpdateSetClause* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::UpdateImpl::
addInputSetClause(Opt::Environment& cEnvironment_,
				  Plan::Relation::Table* pTable_,
				  Plan::Relation::RowInfo* pInputRow_,
				  int* piInputRow_,
				  Statement::UpdateSetClause* pStatement_) const
{
	if (Statement::ColumnName* pCN = pStatement_->getColumnName()) {
		if (*piInputRow_ < pInputRow_->getSize()) {
			// single column assignment
			addInputColumn(cEnvironment_,
						   pTable_,
						   pInputRow_,
						   *piInputRow_,
						   pCN);
		}
		++*piInputRow_;

	} else {
		// multiple column assignment
		Statement::ColumnNameList* pCNL = pStatement_->getColumnNameList();
		int n = pCNL->getCount();
		if (*piInputRow_ + n - 1 < pInputRow_->getSize()) {
			for (int i = 0; i < n; ++i) {
				Statement::ColumnName* pCN = pCNL->getColumnNameAt(i);
				addInputColumn(cEnvironment_,
							   pTable_,
							   pInputRow_,
							   *piInputRow_ + i,
							   pCN);
			}
		}
		*piInputRow_ += n;
	}
}

// FUNCTION private
//	Operation::Impl::UpdateImpl::addInputColumn -- add set data correspondence for one column name
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pTable_
//	Plan::Relation::RowInfo* pInputRow_
//	int iInputRow_
//	Statement::ColumnName* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::UpdateImpl::
addInputColumn(Opt::Environment& cEnvironment_,
			   Plan::Relation::Table* pTable_,
			   Plan::Relation::RowInfo* pInputRow_,
			   int iInputRow_,
			   Statement::ColumnName* pStatement_) const
{
	Plan::Interface::IScalar* pInputData =
		(*pInputRow_)[iInputRow_].second->getScalar(cEnvironment_);

	Plan::Relation::RowElement* pRowElement =
		pStatement_->getAnalyzer2()->getRowElement(cEnvironment_,
												   pTable_,
												   pStatement_);

	; _SYDNEY_ASSERT(pRowElement->isElementOf(pTable_) == true);

	pTable_->addInput(cEnvironment_,
					  pRowElement->getPosition(),
					  pInputData);
}


// FUNCTION private
//	Operation::Impl::UpdateImpl::addInput -- add set data correspondence for one column name
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pTable_
//	Plan::Interface::IRelation* pInput
//	Statement::UpdateSetClauseList* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Impl::UpdateImpl::
addInputForRelocate(Opt::Environment& cEnvironment_,
					Plan::Relation::Table* pTable_,
					Plan::Interface::IRelation* pInput_,
					Plan::Relation::RowInfo* pRowInfo_,
					Statement::UpdateSetClauseList* pStatement_) const
{


	MAP<int, Plan::Interface::IScalar*, LESS<int> > mapRow;

	for (int i = 0; i < pRowInfo_->getSize(); ++i) {
		Statement::UpdateSetClause* pUSC = pStatement_->getSetClauseAt(i);
		; _SYDNEY_ASSERT(pUSC);
		Statement::ColumnName* pCN = pUSC->getColumnName();
		if (pCN) {
			Plan::Relation::RowElement* pRowElement =
				pCN->getAnalyzer2()->getRowElement(cEnvironment_,
												   pTable_,
												   pCN);
			mapRow.insert(pRowElement->getPosition(),
						  (*pRowInfo_)[i].second->getScalar(cEnvironment_));
		}
	}

	Plan::Relation::RowInfo* pRow = pTable_->getRowInfo(cEnvironment_);
	int n = pRow->getSize();
	if (pInput_->getRowInfo(cEnvironment_) &&
		pInput_->getRowInfo(cEnvironment_)->getSize() != pRow->getSize())
		_SYDNEY_THROW2(Exception::InvalidInsertSource,
					   pInput_->getRowInfo(cEnvironment_)->getSize(),
					   n);
	for (int i = 0; i < n; ++i) {
		if (mapRow.find((*pRow)[i].second->getPosition()) == mapRow.end())
			mapRow.insert((*pRow)[i].second->getPosition(),
						  (*pInput_->getRowInfo(cEnvironment_))[i].second->getScalar(cEnvironment_));
	}
	
	MAP<int, Plan::Interface::IScalar*, LESS<int> >::ConstIterator ite = mapRow.begin();
	for (; ite != mapRow.end(); ++ite) {
		pTable_->addInput(cEnvironment_, (*ite).first, (*ite).second);
	}
}
// FUNCTION private
//	Operation::Impl::UpdateImpl::addInputColumn -- add set data correspondence for one column name
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Relation::Table* pTable_
//	Plan::Relation::RowInfo* pInputRow_
//	int iInputRow_
//	Statement::ColumnName* pStatement_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

bool
Impl::UpdateImpl::
isNeedToRelocate(Opt::Environment& cEnvironment_,
				 Plan::Relation::Table* pTable_,
				 Statement::UpdateSetClauseList* pStatement_) const
{
	
	Schema::Partition* pRule =
		pTable_->getSchemaTable()->getPartition(cEnvironment_.getTransaction());
	if (!pRule) return false;


	const ModVector<Schema::Object::ID::Value>& vecColumn = pRule->getTarget().m_vecColumn;
	ModVector<Schema::Object::ID::Value>::ConstIterator ite = vecColumn.begin();
	for (; ite != vecColumn.end(); ++ite) {
		const STRING& cstrRuleColumn = pTable_->getSchemaTable()->
			getColumnByID((*ite), cEnvironment_.getTransaction())->getName();

		int n = pStatement_->getCount();
		for (int i = 0; i < n; ++i) {
			Statement::ColumnName* pCN =
				pStatement_->getSetClauseAt(i)->getColumnName();
			if (pCN && cstrRuleColumn.compare(*(pCN->getIdentifierString()), ModFalse) == 0)
				return true;
		}
	}

	return false;
}

////////////////////////////////////////
// Operation::Update
////////////////////////////////////////

// FUNCTION public
//	Operation::Update::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::UpdateStatement* pStatement_
//	
// RETURN
//	const Update*
//
// EXCEPTIONS

const Update*
Update::
create(const Statement::UpdateStatement* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
