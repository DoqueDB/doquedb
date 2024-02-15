// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeIndex.h -- Declaration of classes concerning with index reorganization
// 
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_DSCHEMA_REORGANIZE_INDEX_H
#define	__SYDNEY_DSCHEMA_REORGANIZE_INDEX_H

#include "DSchema/ReorganizeDistribute.h"

#include "Schema/ReorganizeIndex.h"

_SYDNEY_BEGIN
_SYDNEY_DSCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeIndex
		{

			// CLASS
			// DSchema::Manager::SystemTable::ReorganizeIndex::Create --
			//			create index executor
			// NOTES
			class Create
				: public Schema::Manager::SystemTable::ReorganizeIndex::Create,
				  public ReorganizeDistribute
			{
			public:
				typedef Schema::Manager::SystemTable::ReorganizeIndex::Create Super;
				typedef Create This;

				// constructor
				Create(Trans::Transaction& cTrans_,
					   Schema::Database* pDatabase_,
					   const Statement::IndexDefinition* pStatement_);
				Create(Trans::Transaction& cTrans_,
					   Schema::Database* pDatabase_,
					   const Schema::LogData* pLogData_);
				// destructor
				~Create();

				// do work
				Schema::Manager::SystemTable::Result::Value execute();

			private:
				// cancellation
				void undo();

				// get sql for execution
				ModUnicodeString getSQL();
				// get sql for cancelation
				ModUnicodeString getCancelSQL();

				int m_iStatus;
			};

			// CLASS
			// DSchema::Manager::SystemTable::ReorganizeIndex::Drop --
			//			drop index executor
			// NOTES
			class Drop
				: public Schema::Manager::SystemTable::ReorganizeIndex::Drop,
				  public ReorganizeDistribute
			{
			public:
				typedef Schema::Manager::SystemTable::ReorganizeIndex::Drop Super;
				typedef Drop This;

				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Schema::Database* pDatabase_,
					 const Statement::DropIndexStatement* pStatement_);
				Drop(Trans::Transaction& cTrans_,
					 Schema::Database* pDatabase_,
					 Schema::Table* pTable_,
					 const Schema::LogData* pLogData_);
				// destructor
				~Drop();

				// do work
				Schema::Manager::SystemTable::Result::Value execute();

			private:
				// retrying
				Schema::Manager::SystemTable::Result::Value retry();

				// get sql for execution
				ModUnicodeString getSQL();
				// get sql for cancelation
				ModUnicodeString getCancelSQL();

				int m_iStatus;
			};
		} // namespace ReorganizeIndex
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_DSCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_DSCHEMA_REORGANIZE_INDEX_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
