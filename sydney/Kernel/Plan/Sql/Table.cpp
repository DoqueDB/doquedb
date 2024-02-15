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

#include "Plan/Sql/Table.h"

#include "Plan/Sql/Impl/TableImpl.h"
#include "Common/Assert.h"
#include "Exception/NotSupported.h"
#include "Opt/Environment.h"


_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_SQL_BEGIN



/////////////////////////////////////
// Relation::SimpleTable

// FUNCTION public
//	Sql::Table::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	STRING cstrTable_
//	
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::
create(Opt::Environment& cEnvironment_,
	   const STRING& cstrTable_,
	    const STRING& cstrCorrelationName)
{
	AUTOPOINTER<Table> pResult = new TableImpl::SimpleTable(cstrTable_, cstrCorrelationName);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}


// FUNCTION public
//	Sql::Table::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Tree::Node::Type eOperator_
//	Sql::Table* pOperand0_
//	Sql::Table* pOperand1_
//
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::
create(Opt::Environment& cEnvironment_,
	   Tree::Node::Type eOperator_,
	   Plan::Interface::ISqlNode* pPredicate_,
	   Plan::Interface::ISqlNode* pLeft_,
	   Plan::Interface::ISqlNode* pRight_)
{
	AUTOPOINTER<Table> pResult = new TableImpl::DyadicJoin(eOperator_, pPredicate_, pLeft_, pRight_);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}



// FUNCTION public
//	Sql::Table::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	VECTOR<Table*> pOperand
//
// RETURN
//	Table*
//
// EXCEPTIONS

//static
Table*
Table::
create(Opt::Environment& cEnvironment_,
	   VECTOR<Interface::ISqlNode*>& cVecOperands) 
{
	AUTOPOINTER<Table> pResult = new TableImpl::NadicJoin(cVecOperands);
	pResult->registerToEnvironment(cEnvironment_);
	return pResult.release();
}




// FUNCTION private
//	Relation::SimpleTable::create -- costructor
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
Table::
registerToEnvironment(Opt::Environment& cEnvironment_)
{
	cEnvironment_.addObject(this);
}
	
	
_SYDNEY_PLAN_ORDER_END
_SYDNEY_PLAN_END
_SYDNEY_END

//
// Copyright (c) 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//

