// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeTable.h -- Declaration of classes concerning with table reorganization
// 
// Copyright (c) 2006, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_TABLE_H
#define	__SYDNEY_SCHEMA_REORGANIZE_TABLE_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeExecutor.h"
#include "Schema/Table.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class TableDefinition;
	class DropTableStatement;
	class AlterTableStatement;
	class AlterTableAction;
}

_SYDNEY_SCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeTable
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizeTable::Base --
			//			base class for table reorganization
			// NOTES
			class Base : public ReorganizeExecutor
			{
			public:
				// constructor
				Base(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 bool bIsTemporary_,
					 bool bRedo_,
					 bool bRollforward_ = false);
				// destructor
				virtual ~Base();
			protected:
				// accessor
				Database* getDatabase() {return m_pDatabase;}
				bool isTemporary() {return m_bIsTemporary;}
				bool isRedo() {return m_bRedo;}
				bool isRollforward() {return m_bRollforward;}

				// lock operation
				void lock(Schema::Hold::Operation::Value eOperation_,
						  Schema::Table* pTable_ = 0);

			//////////////
			// ReorganizeExecutor::
			//	Trans::Transaction& getTransaction()
			//	void storeSystemLog(Schema::LogData& cLogData_);
			//	void storeDatabaseLog(Schema::LogData& cLogData_, Schema::Database* pDatabase_);
			//	void executeProgram(Opt::ImportArgument& cArgument_, Schema::Database* pDatabase_);
			//	void objectCheckDelay();
			//	bool isCauseCheckpoint();

			private:
				// data member set in constructor
				Database* m_pDatabase;
				bool m_bIsTemporary;
				bool m_bRedo;
				bool m_bRollforward;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeTable::TableElement --
			//			base class for schema objects consisting a table
			// NOTES
			class TableElement : public Base
			{
			public:
				// constructor
				TableElement(Trans::Transaction& cTrans_,
							 Database* pDatabase_,
							 Table* pTable_,
							 bool bRedo_);
				// destructor
				virtual ~TableElement();
			protected:
				// accessor
				Table* getTable() {return m_pTable;}
				void setTable(Table* pTable_) {m_pTable = pTable_;}

				// lock operation
				void lock(Schema::Hold::Operation::Value eOperation_);

				// importing
				void import(const ModVector<Schema::Field*>& vecSourceFields_,
							const ModVector<Schema::Field*>& vecTargetFields_,
							bool bCheckConstraint_,
							bool bRowIDOrder_);

				// create importing field pair using source-destination
				void getImportField(const File* pFile_,
									ModVector<Schema::Field*>& vecSourceFields_,
									ModVector<Schema::Field*>& vecTargetFields_);

			private:
				// data member set in constructor
				Table* m_pTable;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeTable::Create --
			//			create table executor
			// NOTES
			class Create : public Base
			{
			public:
				// constructor
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   const Statement::TableDefinition* pStatement_,
					   bool bIsTemporary_);
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   const Schema::LogData* pLogData_,
					   bool bIsTemporary_);
				// destructor
				virtual ~Create();

				// execute
				Result::Value execute();

			protected:
				// accessor
				const Statement::TableDefinition* getStatement() {return m_pStatement;}
				const Schema::LogData* getLogData() {return m_pLogData;}

				// error recovery
				void undo();

			private:
				// data member set in constructor
				const Statement::TableDefinition* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Table::Pointer m_pTable;
				ModVector<Table*> m_vecReferencedTable;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeTable::Drop --
			//			drop table executor
			// NOTES
			class Drop : public Base
			{
			public:
				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 const Statement::DropTableStatement* pStatement_,
					 bool bIsTemporary_);
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 const Schema::LogData* pLogData_,
					 bool bIsTemporary_,
					 bool bRollForward_);
				// destructor
				virtual ~Drop();

				// execute
				Result::Value execute();

			protected:
				// accessor
				const Statement::DropTableStatement* getStatement() {return m_pStatement;}
				const Schema::LogData* getLogData() {return m_pLogData;}

				// error recovery
				bool retry();

			private:
				// data member set in constructor
				const Statement::DropTableStatement* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Table::Pointer m_pTable;
				ModVector<Table*> m_vecReferencedTable;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeTable::Alter --
			//			alter table executor (HUB class)
			// NOTES
			class Alter : public Base
			{
			public:
				// constructor
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Statement::AlterTableStatement* pStatement_);
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Schema::LogData* pLogData_,
					  bool bRollfoward_);
				// destructor
				virtual ~Alter();

				// execute
				Result::Value execute();

			protected:
				// accessor
				const Statement::AlterTableStatement* getStatement() {return m_pStatement;}
				const Schema::LogData* getLogData() {return m_pLogData;}

			private:
				// data member set in constructor
				const Statement::AlterTableStatement* m_pStatement;
				const Schema::LogData* m_pLogData;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeTable::AlterArea --
			//			alter table set/drop path executor
			// NOTES
			class AlterArea : public Base
			{
			public:
				// constructor
				AlterArea(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  const Statement::AlterTableAction* pStatement_);
				AlterArea(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  const Schema::LogData* pLogData_);
				// destructor
				virtual ~AlterArea();

				// execute
				Result::Value execute();

			protected:
				// accessor
				const Statement::AlterTableAction* getStatement() {return m_pStatement;}
				const Schema::LogData* getLogData() {return m_pLogData;}

				// error recovery
				void undo();

			private:
				// data member set in constructor
				const Statement::AlterTableAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Table::Pointer m_pTable;
				ModVector<Object::ID::Value> m_vecPrevAreaID;
				ModVector<Object::ID::Value> m_vecPostAreaID;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeTable::AlterName --
			//			alter table rename executor
			// NOTES
			class AlterName : public Base
			{
			public:
				// constructor
				AlterName(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  const Statement::AlterTableAction* pStatement_);
				AlterName(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  const Schema::LogData* pLogData_,
						  bool bRollforward_);
				// destructor
				virtual ~AlterName();

				// execute
				Result::Value execute();

			protected:
				// accessor
				const Statement::AlterTableAction* getStatement() {return m_pStatement;}
				const Schema::LogData* getLogData() {return m_pLogData;}

				// error recovery
				void undo();

			private:
				// data member set in constructor
				const Statement::AlterTableAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Table::Pointer m_pTable;
				Object::Name m_cPrevName;
				Object::Name m_cPostName;
			};
		} // namespace ReorganizeTable
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_TABLE_H

//
// Copyright (c) 2006, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
