// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeIndex.h -- Declaration of classes concerning with index reorganization
// 
// Copyright (c) 2006, 2008, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_INDEX_H
#define	__SYDNEY_SCHEMA_REORGANIZE_INDEX_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeTable.h"
#include "Schema/Index.h"

#include "Trans/LogFile.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class IndexDefinition;
}

_SYDNEY_SCHEMA_BEGIN

class Table;

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeIndex
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizeIndex::Base --
			//			base class for index reorganization
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
				// lock operations (indivisual version)
				void lockLogicalLog(Schema::Hold::Operation::Value eOperation_);
				void lockDatabase(Schema::Hold::Operation::Value eOperation_);
				void lockTable(Schema::Hold::Operation::Value eOperation_);

				// is temporary index?
				bool isTemporary();

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
			// Schema::Manager::SystemTable::ReorganizeIndex::Create --
			//			create index executor
			// NOTES
			class Create : public Base
			{
			public:
				// constructor
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   const Statement::IndexDefinition* pStatement_);
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   const Schema::LogData* pLogData_);
				// destructor
				~Create();

				// execute
				Result::Value execute();

			protected:
				const Statement::IndexDefinition* getStatement()
				{return m_pStatement;}

				// execute in redo
				Result::Value redo();

				// error recovery
				void undo();

			private:
				// reflect data from logical log data
				void reflect(Trans::Log::LSN& cLSN_);

				// block update operations
				void blockUpdateOperation();
				// wait for all the updating transactions for the database end
				bool tryLockDatabase();
				// wait for all the updating transactions for the database end
				void waitLockDatabase();
				// allow update operations
				void allowUpdateOperation();

				// data member set in constructor
				const Statement::IndexDefinition* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				Index::Pointer m_pIndex;
				File* m_pFile;
				int m_iStatus;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeIndex::Drop --
			//			drop index executor
			// NOTES
			class Drop : public Base
			{
			public:
				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 const Statement::DropIndexStatement* pStatement_);
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 Table* pTable_,
					 const Schema::LogData* pLogData_);
				// destructor
				~Drop();

				// execute
				Result::Value execute();

			protected:
				const Statement::DropIndexStatement* getStatement()
				{return m_pStatement;}

			private:
				// error recovery
				bool retry();

				// data member set in constructor
				const Statement::DropIndexStatement* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Index::Pointer m_pIndex;
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
					  const Statement::AlterIndexStatement* pStatement_);
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Schema::LogData* pLogData_);
				// destructor
				~Alter();

				// execute
				Result::Value execute();

			protected:
				const Statement::AlterIndexStatement* getStatement()
				{return m_pStatement;}

			private:
				// data member set in constructor
				const Statement::AlterIndexStatement* m_pStatement;
				const Schema::LogData* m_pLogData;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeIndex::AlterArea --
			//			alter index set/drop path executor
			// NOTES
			class AlterArea : public Base
			{
			public:
				// constructor
				AlterArea(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  Index* pIndex_,
						  const Statement::AlterIndexAction* pStatement_);
				AlterArea(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  Index* pIndex_,
						  const Schema::LogData* pLogData_);
				// destructor
				~AlterArea();

				// execute
				Result::Value execute();

			protected:
				const Statement::AlterIndexAction* getStatement()
				{return m_pStatement;}

			private:
				// error recovery
				void undo();

				// data member set in constructor
				const Statement::AlterIndexAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Index::Pointer m_pIndex;
				ModVector<Object::ID::Value> m_vecPrevAreaID;
				ModVector<Object::ID::Value> m_vecPostAreaID;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeIndex::AlterName --
			//			alter index rename executor
			// NOTES
			class AlterName : public Base
			{
			public:
				// constructor
				AlterName(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  Index* pIndex_,
						  const Statement::AlterIndexAction* pStatement_);
				AlterName(Trans::Transaction& cTrans_,
						  Database* pDatabase_,
						  Table* pTable_,
						  Index* pIndex_,
						  const Schema::LogData* pLogData_,
						  bool bRollForward_);
				// destructor
				~AlterName();

				// execute
				Result::Value execute();

			protected:
				const Statement::AlterIndexAction* getStatement()
				{return m_pStatement;}

			private:
				// error recovery
				void undo();

				// data member set in constructor
				const Statement::AlterIndexAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Index::Pointer m_pIndex;
				Object::Name m_cPrevName;
				Object::Name m_cPostName;
				bool m_bRollforward;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeIndex::AlterUsage --
			//			alter index online/offline executor
			// NOTES
			class AlterUsage : public Base
			{
			public:
				// constructor
				AlterUsage(Trans::Transaction& cTrans_,
						   Database* pDatabase_,
						   Table* pTable_,
						   Index* pIndex_,
						   const Statement::AlterIndexAction* pStatement_);
				AlterUsage(Trans::Transaction& cTrans_,
						   Database* pDatabase_,
						   Table* pTable_,
						   Index* pIndex_,
						   const Schema::LogData* pLogData_);
				// destructor
				~AlterUsage();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undo();

				// data member set in constructor
				const Statement::AlterIndexAction* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Index::Pointer m_pIndex;
				bool m_bPrevOffline;
				bool m_bPostOffline;
			};
		} // namespace ReorganizeIndex
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_INDEX_H

//
// Copyright (c) 2006, 2008, 2009, 2011, 2012, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
