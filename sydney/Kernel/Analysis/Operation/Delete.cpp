// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Operation/Delete.cpp --
// 
// Copyright (c) 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Analysis/Operation/Delete.h"

#include "Common/Assert.h"

#include "DPlan/Relation/Table.h"

#include "Exception/NotSupported.h"

#include "Opt/Environment.h"

#include "Plan/Relation/Projection.h"
#include "Plan/Relation/Table.h"

#include "Statement/Identifier.h"
#include "Statement/DeleteStatement.h"
#include "Statement/ValueExpression.h"

_SYDNEY_BEGIN
_SYDNEY_ANALYSIS_BEGIN
_SYDNEY_ANALYSIS_OPERATION_BEGIN

namespace Impl
{
	//////////////////////////////////////////////////////////////////////////////////
	// CLASS
	//	Operation::Expression::Impl::DeleteImpl --
	//		implementation classes for delete statement analyzer
	//
	// NOTES
	class DeleteImpl
		: public Operation::Delete
	{
	public:
		typedef DeleteImpl This;
		typedef Operation::Delete Super;

		// constructor
		DeleteImpl() : Super() {}
		// destructor
		virtual ~DeleteImpl() {}

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
	};
}

namespace
{
	// VARIABLE local
	//	_analyzer -- instance
	//
	// NOTES
	const Impl::DeleteImpl _analyzer;

} // namespace

//////////////////////////////////////////
// Operation::Impl::DeleteImpl
//////////////////////////////////////////

// FUNCTION public
//	Operation::Impl::DeleteImpl::getRelation -- generate Plan::Tree::Node from Statement::Object
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
Impl::DeleteImpl::
getRelation(Opt::Environment& cEnvironment_,
			Statement::Object* pStatement_) const
{
	Statement::DeleteStatement* pDS =
		_SYDNEY_DYNAMIC_CAST(Statement::DeleteStatement*, pStatement_);

	Statement::Identifier* pTarget = pDS->getTableName();
	; _SYDNEY_ASSERT(pTarget->getIdentifier());
	Statement::Identifier* pCorrelationName = pDS->getCorrelationName();

	Plan::Interface::IRelation* pInput = 0;
	Plan::Relation::Table* pTable = 0;
	{
		// create new namescope
		Opt::Environment::AutoPop cAutoPop1 = cEnvironment_.pushNameScope();
		Opt::Environment::AutoPop cAutoPop2 =
			cEnvironment_.pushStatus(Opt::Environment::Status::Reset);

		// create Interface from schema table
		if (cEnvironment_.hasCascade()) {
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

		if (Statement::ValueExpression* pCondition = pDS->getSearchCondition()) {
			// add search filter
			pInput = pCondition->getAnalyzer2()->getFilter(cEnvironment_,
														   pInput,
														   pCondition);
		}
	}

	// create result relation
	Plan::Relation::Table* pResult = 0;
	if (cEnvironment_.hasCascade()) {
		pResult = DPlan::Relation::Table::Delete::create(cEnvironment_,
														 pTable,
														 pInput);
		
	} else {
		pResult = Plan::Relation::Table::Delete::create(cEnvironment_,
														pTable,
														pInput);
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
Impl::DeleteImpl::
getDistributeRelation(Opt::Environment& cEnvironment_,
					  Statement::Object* pStatement_) const
{
	return getRelation(cEnvironment_, pStatement_);
}

////////////////////////////////////////
// Operation::Delete
////////////////////////////////////////

// FUNCTION public
//	Operation::Delete::create -- constructor
//
// NOTES
//
// ARGUMENTS
//	const Statement::DeleteStatement* pStatement_
//	
// RETURN
//	const Delete*
//
// EXCEPTIONS

const Delete*
Delete::
create(const Statement::DeleteStatement* pStatement_)
{
	return &_analyzer;
}

_SYDNEY_ANALYSIS_OPERATION_END
_SYDNEY_ANALYSIS_END
_SYDNEY_END

//
// Copyright (c) 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
