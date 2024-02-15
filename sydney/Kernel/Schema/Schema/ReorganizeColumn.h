// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeColumn.h -- Declaration of classes concerning with column reorganization
// 
// Copyright (c) 2006, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_COLUMN_H
#define	__SYDNEY_SCHEMA_REORGANIZE_COLUMN_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeTable.h"
#include "Schema/Column.h"
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
		namespace ReorganizeColumn
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizeColumn::Base --
			//			base class for column reorganization
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
			// Schema::Manager::SystemTable::ReorganizeColumn::Add --
			//			alter table add column executor
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

				// drop a file
				void drop(const File::Pointer& pFile_);
				// cancel drop operation
				void cancelDrop(const File::Pointer& pFile_, bool bStore_);

				// rename prev/post files
				void rename(const File::Pointer& pFile_,
							const Object::Name& cPrevName_, const Object::Name& cPostName_);
				// revert rename operation of prev/post files
				void cancelRename(const File::Pointer& pFile_,
								  const Object::Name& cPrevName_, const Object::Name& cPostName_,
								  bool bStored_);

				// replace source-destination connection of center files
				void replace(const File::Pointer& pPrevFile_, const File::Pointer& pPostFile_);
				// cancel replacing source-destination connection of center files
				void cancelReplace(const File::Pointer& pPrevFile_, const File::Pointer& pPostFile_,
								   bool bStored_);

				// data member set in constructor
				const Statement::AlterTableAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				int m_iImportedFile;
				int m_iReplacedFile;
				int m_iDroppedFile;
				int m_iStoredFile;
				ModVector<File::Pointer> m_vecPrevFiles;
				ModVector<File::Pointer> m_vecPostFiles;
				ModVector<Column::Pointer> m_vecNewColumns;
				ModVector<Object::Name> m_vecPrevName;
				ModVector<Object::Name> m_vecPostName;
				ModVector<Object::Name> m_vecTmpName;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeColumn::Alter --
			//			alter table alter column executor
			// NOTES
			class Alter : public Base
			{
			public:
				// constructor
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  Table* pTable_,
					  const Statement::AlterTableAction* pStatement_);
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  Table* pTable_,
					  const Schema::LogData* pLogData_);
				// destructor
				~Alter();

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
				Column::Pointer m_pPrevColumn;
				Column::Pointer m_pPostColumn;
			};
		} // namespace ReorganizeColumn
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_COLUMN_H

//
// Copyright (c) 2006, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
