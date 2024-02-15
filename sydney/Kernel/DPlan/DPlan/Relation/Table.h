// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Relation/Table.h --
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

#ifndef __SYDNEY_DPLAN_RELATION_TABLE_H
#define __SYDNEY_DPLAN_RELATION_TABLE_H

#include "DPlan/Relation/Module.h"

#include "Plan/Relation/Table.h"

_SYDNEY_BEGIN
_SYDNEY_DPLAN_BEGIN
_SYDNEY_DPLAN_RELATION_BEGIN

////////////////////////////////////
//	CLASS
//	DPlan::Relation::Table -- Plan::Interface::IRelation implementation for schema table
//
//	NOTES
class Table
{
public:
	typedef Plan::Relation::Table This;

	struct Retrieve
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							const STRING& cstrTableName_);
	};
	struct Insert
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							const STRING& cstrTableName_,
							Plan::Interface::IRelation* pOperand_,
							Plan::Relation::Table* pLocalInsert_,
							bool bRelocateUpdate_);
	};
	struct Update
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							This* pRetrieve_,
							Plan::Interface::IRelation* pOperand_);
	};

	struct Delete
	{
		// costructor
		static This* create(Opt::Environment& cEnvironment_,
							This* pRetrieve_,
							Plan::Interface::IRelation* pOperand_);
	};

protected:
private:
	// never constructed
	Table();
	~Table();
};

_SYDNEY_DPLAN_RELATION_END
_SYDNEY_DPLAN_END
_SYDNEY_END

#endif // __SYDNEY_DPLAN_RELATION_TABLE_H

//
//	Copyright (c) 2012, 2013, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
