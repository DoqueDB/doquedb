// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Projection.cpp --
// 
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Sql";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Common/Assert.h"

#include "Plan/Sql/Query.h"
#include "Plan/Sql/Impl/QueryImpl.h"
#include "Plan/Sql/Table.h"

#include "Plan/Interface/ISqlNode.h"
#include "Plan/Interface/IScalar.h"

#include "Opt/Environment.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN


/////////////////////////////////////
// Sql::Query

// FUNCTION public
//	Sql::Query::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Query*
//
// EXCEPTIONS

//static
Query*
Query::
create(Opt::Environment& cEnvironment_,
	   Type eType,
	   Interface::ISqlNode* pTable_,
	   bool isDistribution)
{
	AUTOPOINTER<This> pResult;
	switch(eType) {
	case SELECT:
	{
		;_SYDNEY_ASSERT(pTable_->isTable());
		pResult = new QueryImpl::SelectImpl(pTable_, isDistribution);
		break;
	}
	case INSERT:
	{
		;_SYDNEY_ASSERT(pTable_->isTable());
		pResult = new QueryImpl::InsertImpl(pTable_, isDistribution);
		break;
	}
	case UPDATE:
	{
		if (pTable_->isQuery()) {
			Query* pQuery = pTable_->getQuery();
			pResult = new QueryImpl::UpdateImpl(pQuery->getTable(),
												pQuery->getPredicate(),
												isDistribution);			
		} else if (pTable_->isTable()) {
			pResult = new QueryImpl::UpdateImpl(pTable_, isDistribution);
		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		break;
			
	}	
	case DEL:
	{
		if (pTable_->isQuery()) {
			Query* pQuery = pTable_->getQuery();
			pResult = new QueryImpl::DeleteImpl(pQuery->getTable(),
												pQuery->getPredicate(),
												isDistribution);		
		} else if (pTable_->isTable()) {
			pResult = new QueryImpl::DeleteImpl(pTable_,
												isDistribution);
		} else {
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		break;
			
	}
	default:
	{
		_SYDNEY_THROW0(Exception::NotSupported);
	}
	}
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Sql::Query::join -- join
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Query*
//
// EXCEPTIONS

//static
Query*
Query::
join(Opt::Environment& cEnvironment_,
	 Tree::Node::Type eOperator_,
	 Plan::Interface::ISqlNode* pPredicate_,
	 Query* pLeft_,
	 Query* pRight_)
{
	Table* pJoinTable = Table::create(cEnvironment_,
									  eOperator_,
									  pPredicate_,
									  pLeft_->isTable() ? pLeft_->getTable() : pLeft_,
									  pRight_->isTable() ? pRight_->getTable() : pRight_);
	return create(cEnvironment_,
				  SELECT,
				  pJoinTable,
				  (pLeft_->isDistribution() & pRight_->isDistribution()));
}



// FUNCTION public
//	Sql::Query::join -- join
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Query*
//
// EXCEPTIONS

//static
Query*
Query::
join(Opt::Environment& cEnvironment_,
	 VECTOR<Query*>& cVecOperands_)
{
	VECTOR<Plan::Interface::ISqlNode*> cVecTables;
	VECTOR<Query*>::ConstIterator iterator = cVecOperands_.begin();
	VECTOR<Query*>::ConstIterator last = cVecOperands_.end();

	bool bDistribution = false;
	for(; iterator != last; ++iterator) {
		if ((*iterator)->isTable()) {
			bDistribution = bDistribution & ((*iterator)->isDistribution());
			cVecTables.PUSHBACK((*iterator)->getTable());
		} else {
			cVecTables.PUSHBACK(*iterator);
		}
	}

	Table* pJoinTable = Table::create(cEnvironment_, cVecTables);
	return create(cEnvironment_,
				  SELECT,
				  pJoinTable,
				  bDistribution);
}



// FUNCTION private
//	Sql::Query::registerToEnvironment -- regiter to environment
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	
//
// EXCEPTIONS
void
Query::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addObject(this);
}
	
	

_SYDNEY_PLAN_END
_SYDNEY_PLAN_SQL_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
