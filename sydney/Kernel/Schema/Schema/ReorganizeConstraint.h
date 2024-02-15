// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeConstraint.h -- Declaration of classes concerning with constraint reorganization
// 
// Copyright (c) 2006, 2008, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_CONSTRAINT_H
#define	__SYDNEY_SCHEMA_REORGANIZE_CONSTRAINT_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeTable.h"
#include "Schema/Constraint.h"
#include "Schema/File.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class AlterTableAction;
}

_SYDNEY_SCHEMA_BEGIN

class Table;

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeConstraint
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizeConstraint::Base --
			//			base class for constraint reorganization
			// NOTES
			class Base : public ReorganizeTable::TableElement
			{
			public:
				typedef ReorganizeTable::TableElement Super;

				// constructor
				Base(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 Table* pTable_,
					 bool bRedo_);
				// destructor
				virtual ~Base();
			protected:
				// drop corresponding index if needed
				void dropIndex(Constraint* pConstraint_,
							   bool bRecovery_,
							   bool bNoUnset_);

			///////////////////////////////////////
			// ReorganizeTable::TableElement::
			//	Table* getTable() {return m_pTable;}
			//	void setTable(Table* pTable_) {m_pTable = pTable_;}
			//	void lock(Schema::Hold::Operation::Value eOperation_);
			//	void import(const ModVector<Schema::Field*>& vecSourceFields_,
			//				const ModVector<Schema::Field*>& vecTargetFields_,
			//				bool bCheckConstraint_,
			//				bool bRowIDOrder_);
			//	void getImportField(const File* pFile_,
			//						ModVector<Schema::Field*>& vecSourceFields_,
			//						ModVector<Schema::Field*>& vecTargetFields_);

			//////////////
			// ReorganizeExecutor::
			//	Trans::Transaction& getTransaction()
			//	void storeSystemLog(Schema::LogData& cLogData_);
			//	void storeDatabaseLog(Schema::LogData& cLogData_, Schema::Database* pDatabase_);
			//	void executeProgram(Analysis::Analyzer* pAnalyzer_, Schema::Database* pDatabase_);
			//	void objectCheckDelay();
			//	bool isCauseCheckpoint();

			private:
				// data member set in constructor
				// (none)
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeConstraint::Add --
			//			alter table add constraint executor
			// NOTES
			class Add : public Base
			{
			public:
				// constructor
				Add(Trans::Transaction& cTrans_,
					Database* pDatabase_,
					Table* pTable_,
					const Statement::AlterTableAction* pStatement_);
				Add(Trans::Transaction& cTrans_,
					Database* pDatabase_,
					Table* pTable_,
					const Schema::LogData* pLogData_);
				// destructor
				~Add();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undo();

				// data member set in constructor
				const Statement::AlterTableAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Constraint::Pointer m_pConstraint;
				ModVector<Table*> m_vecReferencedTable;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeConstraint::Alter --
			//			alter table alter constraint executor
			// NOTES
			class Drop : public Base
			{
			public:
				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 Table* pTable_,
					 const Statement::AlterTableAction* pStatement_);
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 Table* pTable_,
					 const Schema::LogData* pLogData_);
				// destructor
				~Drop();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undo();

				// data member set in constructor
				const Statement::AlterTableAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
			};
		} // namespace ReorganizeConstraint
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_CONSTRAINT_H

//
// Copyright (c) 2006, 2008, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
