// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Table.cpp --
// 
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "DPlan::Relation";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "DPlan/Relation/Table.h"
#include "DPlan/Relation/Impl/TableImpl.h"

#include "Plan/Relation/Table.h"
#include "Schema/Table.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

///////////////////////////////////////////
//	DPlan::Relation::Table::Retrieve

// FUNCTION public
//	Relation::Table::Retrieve::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	
// RETURN
//	Plan::Relation::Table*
//
// EXCEPTIONS

//static
Plan::Relation::Table*
Table::Retrieve::
create(Opt::Environment& cEnvironment_, const STRING& cstrTableName_)
{
	Schema::Table* pSchemaTable =
		Plan::Relation::Table::getSchemaTable(cEnvironment_, cstrTableName_);

	if (pSchemaTable->isSystem()
		|| pSchemaTable->isTemporary()) {
		return Plan::Relation::Table::Retrieve::create(cEnvironment_,
													   cstrTableName_);
	}

	AUTOPOINTER<TableImpl::Retrieve> pResult = new TableImpl::Retrieve(pSchemaTable);

	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();
	return pThis;
}

///////////////////////////////////////////
//	DPlan::Relation::Table::Insert

// FUNCTION public
//	Relation::Table::Insert::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	Plan::Interface::IRelation* pOperand_
//	
// RETURN
//	Plan::Relation::Table*
//
// EXCEPTIONS

//static
Plan::Relation::Table*
Table::Insert::
create(Opt::Environment& cEnvironment_,
	   const STRING& cstrTableName_,
	   Plan::Interface::IRelation* pOperand_,
	   Plan::Relation::Table* pLocalInsert_,
	   bool bRelocateUpdate_)
{
	Schema::Table* pSchemaTable =
		Plan::Relation::Table::getSchemaTable(cEnvironment_, cstrTableName_);

	if (pSchemaTable->isSystem()
		|| pSchemaTable->isTemporary()) {
		return Plan::Relation::Table::Insert::create(cEnvironment_,
													 cstrTableName_,
													 pOperand_);
	}

	AUTOPOINTER<TableImpl::Insert> pResult = new TableImpl::Insert(pSchemaTable,
																   pOperand_,
																   pLocalInsert_,
																   bRelocateUpdate_);
	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();

	
	pThis->setCorrelationName(cEnvironment_,
							  STRING(),
							  VECTOR<STRING>());

	return pThis;
}



///////////////////////////////////////////
//	DPlan::Relation::Table::Insert

// FUNCTION public
//	Relation::Table::Update::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	Plan::Interface::IRelation* pOperand_
//	
// RETURN
//	Plan::Relation::Table*
//
// EXCEPTIONS

//static
Plan::Relation::Table*
Table::Update::
create(Opt::Environment& cEnvironment_,
	   Plan::Relation::Table* pTable_,
	   Plan::Interface::IRelation* pOperand_)
{
	Schema::Table* pSchemaTable = pTable_->getSchemaTable();

	if (pSchemaTable->isSystem()
		|| pSchemaTable->isTemporary()) {
		return Plan::Relation::Table::Update::create(cEnvironment_,
													 pTable_,
													 pOperand_);
	}

	AUTOPOINTER<TableImpl::Update> pResult = new TableImpl::Update(pTable_,
																   pOperand_);
	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();

	pThis->setCorrelationName(cEnvironment_,
							  STRING(),
							  VECTOR<STRING>());

	return pThis;
}



///////////////////////////////////////////
//	DPlan::Relation::Table::Delete

// FUNCTION public
//	Relation::Table::Delete::create -- costructor
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const STRING& cstrTableName_
//	Plan::Interface::IRelation* pOperand_
//	
// RETURN
//	Plan::Relation::Table*
//
// EXCEPTIONS

//static
Plan::Relation::Table*
Table::Delete::
create(Opt::Environment& cEnvironment_,
	   Plan::Relation::Table* pTable_,
	   Plan::Interface::IRelation* pOperand_)
{
	Schema::Table* pSchemaTable = pTable_->getSchemaTable();
	if (pSchemaTable->isSystem()
		|| pSchemaTable->isTemporary()) {
		return Plan::Relation::Table::Delete::create(cEnvironment_,
													 pTable_,
													 pOperand_);
	}

	AUTOPOINTER<TableImpl::Delete> pResult = new TableImpl::Delete(pTable_,
																   pOperand_);
	pResult->registerToEnvironment(cEnvironment_);

	This* pThis = pResult.release();

	pThis->setCorrelationName(cEnvironment_,
							  STRING(),
							  VECTOR<STRING>());

	return pThis;
}

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

//
// Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
