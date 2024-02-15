// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeTable.h -- Declaration of classes concerning with table reorganization
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

#ifndef	__SYDNEY_DSCHEMA_REORGANIZE_TABLE_H
#define	__SYDNEY_DSCHEMA_REORGANIZE_TABLE_H

#include "DSchema/ReorganizeDistribute.h"

#include "Schema/ReorganizeTable.h"

_SYDNEY_BEGIN
_SYDNEY_DSCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeTable
		{

			// CLASS
			// DSchema::Manager::SystemTable::ReorganizeTable::Create --
			//			create table executor
			// NOTES
			class Create
				: public Schema::Manager::SystemTable::ReorganizeTable::Create,
				  public ReorganizeDistribute
			{
			public:
				typedef Schema::Manager::SystemTable::ReorganizeTable::Create Super;
				typedef Create This;

				// constructor
				Create(Trans::Transaction& cTrans_,
					   Schema::Database* pDatabase_,
					   const Statement::TableDefinition* pStatement_,
					   bool bIsTemporary_);
				Create(Trans::Transaction& cTrans_,
					   Schema::Database* pDatabase_,
					   const Schema::LogData* pLogData_,
					   bool bIsTemporary_);
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
			// DSchema::Manager::SystemTable::ReorganizeTable::Drop --
			//			drop table executor
			// NOTES
			class Drop
				: public Schema::Manager::SystemTable::ReorganizeTable::Drop,
				  public ReorganizeDistribute
			{
			public:
				typedef Schema::Manager::SystemTable::ReorganizeTable::Drop Super;
				typedef Drop This;

				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Schema::Database* pDatabase_,
					 const Statement::DropTableStatement* pStatement_,
					 bool bIsTemporary_);
				Drop(Trans::Transaction& cTrans_,
					 Schema::Database* pDatabase_,
					 const Schema::LogData* pLogData_,
					 bool bIsTemporary_,
					 bool bRollForward_);
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

#ifdef YET
			// CLASS
			// DSchema::Manager::SystemTable::ReorganizeTable::Alter --
			//			alter table executor (HUB class)
			// NOTES
			class Alter
				: public Schema::Manager::SystemTable::ReorganizeTable::Alter,
				  public ReorganizeDistribute
			{
			public:
				// constructor
				Alter(Trans::Transaction& cTrans_,
					  Schema::Database* pDatabase_,
					  const Statement::AlterTableStatement* pStatement_);
				Alter(Trans::Transaction& cTrans_,
					  Schema::Database* pDatabase_,
					  const Schema::LogData* pLogData_,
					  bool bRollfoward_);
				// destructor
				~Alter();

				// do work
				Schema::Manager::SystemTable::Result::Value execute();

			private:
				// get sql for execution
				ModUnicodeString getSQL();
				// get sql for cancelation
				ModUnicodeString getCancelSQL();

				int m_iStatus;
			};

			// CLASS
			// DSchema::Manager::SystemTable::ReorganizeTable::AlterArea --
			//			alter table set/drop path executor
			// NOTES
			class AlterArea
				: public Schema::Manager::SystemTable::ReorganizeTable::AlterArea,
				  public ReorganizeDistribute
			{
			public:
				// constructor
				AlterArea(Trans::Transaction& cTrans_,
						  Schema::Database* pDatabase_,
						  Table* pTable_,
						  const Statement::AlterTableAction* pStatement_);
				AlterArea(Trans::Transaction& cTrans_,
						  Schema::Database* pDatabase_,
						  Table* pTable_,
						  const Schema::LogData* pLogData_);
				// destructor
				~AlterArea();

				// do work
				Schema::Manager::SystemTable::Result::Value execute();

			private:
				// get sql for execution
				ModUnicodeString getSQL();
				// get sql for cancelation
				ModUnicodeString getCancelSQL();

				int m_iStatus;
			};

			// CLASS
			// DSchema::Manager::SystemTable::ReorganizeTable::AlterName --
			//			alter table rename executor
			// NOTES
			class AlterName
				: public Schema::Manager::SystemTable::ReorganizeTable::AlterName,
				  public ReorganizeDistribute
			{
			public:
				// constructor
				AlterName(Trans::Transaction& cTrans_,
						  Schema::Database* pDatabase_,
						  Table* pTable_,
						  const Statement::AlterTableAction* pStatement_);
				AlterName(Trans::Transaction& cTrans_,
						  Schema::Database* pDatabase_,
						  Table* pTable_,
						  const Schema::LogData* pLogData_,
						  bool bRollforward_);
				// destructor
				~AlterName();

				// do work
				Schema::Manager::SystemTable::Result::Value execute();

			private:
				// get sql for execution
				ModUnicodeString getSQL();
				// get sql for cancelation
				ModUnicodeString getCancelSQL();

				int m_iStatus;
			};
#endif
		} // namespace ReorganizeTable
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_DSCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_DSCHEMA_REORGANIZE_TABLE_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
