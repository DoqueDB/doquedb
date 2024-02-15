// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/Argument.h --
// 
// Copyright (c) 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_ARGUMENT_H
#define __SYDNEY_OPT_ARGUMENT_H

#include "Opt/Module.h"
#include "Opt/Declaration.h"

#include "Execution/Declaration.h"
#include "Plan/Declaration.h"
#include "Schema/Field.h"

class ModArchive;

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}
namespace Communication
{
	class Connection;
}
namespace Schema
{
	class Database;
	class Table;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_OPT_BEGIN

////////////////////////////////////////////////////////////////////
// STRUCT
//	Opt::EnvironmentArgument -- arguments for Environment constructor
//
// NOTES
//	This structure is used to reduce infruence of argument changing
struct EnvironmentArgument
{
	// constructor
	EnvironmentArgument(Schema::Database* pDatabase_,
						Trans::Transaction& cTransaction_,
						Execution::Program* pProgram_,
						Communication::Connection* pConnection_,
						Common::DataArrayData* pParameter_,
						bool bPrepare_,
						bool bRecovery_ = false,
						bool bRollback_ = false,
						bool bUndo_ = false)
		: m_pDatabase(pDatabase_),
		  m_cTransaction(cTransaction_),
		  m_pProgram(pProgram_),
		  m_pConnection(pConnection_),
		  m_pParameter(pParameter_),
		  m_bPrepare(bPrepare_),
		  m_bRecovery(bRecovery_),
		  m_bRollback(bRollback_),
		  m_bUndo(bUndo_)
	{}
	EnvironmentArgument(const EnvironmentArgument& cOther_)
		: m_pDatabase(cOther_.m_pDatabase),
		  m_cTransaction(cOther_.m_cTransaction),
		  m_pProgram(cOther_.m_pProgram),
		  m_pConnection(cOther_.m_pConnection),
		  m_pParameter(cOther_.m_pParameter),
		  m_bPrepare(cOther_.m_bPrepare),
		  m_bRecovery(cOther_.m_bRecovery),
		  m_bRollback(cOther_.m_bRollback),
		  m_bUndo(cOther_.m_bUndo)
	{}

	// destructor
	~EnvironmentArgument() {}

	Schema::Database* m_pDatabase;
	Trans::Transaction& m_cTransaction;
	Execution::Program* m_pProgram;
	Communication::Connection* m_pConnection;
	Common::DataArrayData* m_pParameter;
	bool m_bPrepare;
	bool m_bRecovery;
	bool m_bRollback;
	bool m_bUndo;
};

// STRUCT
//	Opt::ExplainFileArgument -- argument for execution::file::explain method
//
// NOTES
struct ExplainFileArgument
{
	Plan::Interface::IRelation* m_pTable;
	Plan::File::Parameter* m_pParameter;

	bool m_bIsSearch;
	bool m_bIsGetByBitSet;
	bool m_bIsSearchByBitSet;
	bool m_bIsLimited;
	bool m_bIsSimple;

	ExplainFileArgument()
		: m_pTable(0),
		  m_pParameter(0),
		  m_bIsSearch(false),
		  m_bIsGetByBitSet(false),
		  m_bIsSearchByBitSet(false),
		  m_bIsLimited(false),
		  m_bIsSimple(false)
	{}
	ExplainFileArgument(Plan::File::Parameter* pParameter_)
		: m_pTable(0),
		  m_pParameter(pParameter_),
		  m_bIsSearch(false),
		  m_bIsGetByBitSet(false),
		  m_bIsSearchByBitSet(false),
		  m_bIsLimited(false),
		  m_bIsSimple(false)
	{}
	ExplainFileArgument(const ExplainFileArgument& cOther_)
		: m_pTable(cOther_.m_pTable),
		  m_pParameter(cOther_.m_pParameter),
		  m_bIsSearch(cOther_.m_bIsSearch),
		  m_bIsGetByBitSet(cOther_.m_bIsGetByBitSet),
		  m_bIsSearchByBitSet(cOther_.m_bIsSearchByBitSet),
		  m_bIsLimited(cOther_.m_bIsLimited),
		  m_bIsSimple(cOther_.m_bIsSimple)
	{}

	void setValues();
	void serialize(ModArchive& cArchive_);
};

// STRUCT
//	Opt::ImportArgument -- argument for reorganize::import method
//
// NOTES
struct ImportArgument
{
	Schema::Table* m_pTargetTable;
	ModVector<Schema::Field*> m_vecSourceField;
	ModVector<Schema::Field*> m_vecTargetField;
	bool m_bCheckConstraint;
	bool m_bRowIDOrder;

	ImportArgument()
		: m_pTargetTable(0),
		  m_vecSourceField(),
		  m_vecTargetField(),
		  m_bCheckConstraint(false),
		  m_bRowIDOrder(false)
	{}
	ImportArgument(Schema::Table* pTargetTable_,
				   const ModVector<Schema::Field*>& vecSourceField_,
				   const ModVector<Schema::Field*>& vecTargetField_,
				   bool bCheckConstraint_,
				   bool bRowIDOrder_)
		: m_pTargetTable(pTargetTable_),
		  m_vecSourceField(vecSourceField_),
		  m_vecTargetField(vecTargetField_),
		  m_bCheckConstraint(bCheckConstraint_),
		  m_bRowIDOrder(bRowIDOrder_)
	{}
	ImportArgument(const ImportArgument& cOther_)
		: m_pTargetTable(cOther_.m_pTargetTable),
		  m_vecSourceField(cOther_.m_vecSourceField),
		  m_vecTargetField(cOther_.m_vecTargetField),
		  m_bCheckConstraint(cOther_.m_bCheckConstraint),
		  m_bRowIDOrder(cOther_.m_bRowIDOrder)
	{}
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_ARGUMENT_H

//
//	Copyright (c) 2010, 2011, 2015, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
