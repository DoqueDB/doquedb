// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizePartition.h -- Declaration of classes concerning with partition reorganization
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_PARTITION_H
#define	__SYDNEY_SCHEMA_REORGANIZE_PARTITION_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeExecutor.h"
#include "Schema/Partition.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class PartitionDefinition;
	class AlterPartitionStatement;
	class DropPartitionStatement;
}

_SYDNEY_SCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizePartition
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizePartition::Base --
			//			base class for partition reorganization
			// NOTES
			class Base : public ReorganizeExecutor
			{
			public:
				// constructor
				Base(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 bool bRedo_);
				// destructor
				virtual ~Base();
			protected:
				// accessor
				Database* getDatabase() {return m_pDatabase;}
				bool isRedo() {return m_bRedo;}

				// lock operation
				void lock(Schema::Hold::Operation::Value eOperation_,
						  bool bWeaken_ = false,
						  bool bLockDatabase_ = false);

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
				bool m_bRedo;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizePartition::Create --
			//			create partition executor
			// NOTES
			class Create : public Base
			{
			public:
				// constructor
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   const Statement::PartitionDefinition* pStatement_);
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   const Schema::LogData* pLogData_);
				// destructor
				~Create();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undo();

				// data member set in constructor
				const Statement::PartitionDefinition* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Partition::Pointer m_pPartition;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizePartition::Drop --
			//			drop partition executor
			// NOTES
			class Drop : public Base
			{
			public:
				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 const Statement::DropPartitionStatement* pStatement_);
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 const Schema::LogData* pLogData_);
				// destructor
				~Drop();

				// execute
				Result::Value execute();

			private:
				// error recovery
				bool retry();

				// data member set in constructor
				const Statement::DropPartitionStatement* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Partition::Pointer m_pPartition;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizePartition::Alter --
			//			alter partition executor
			// NOTES
			class Alter : public Base
			{
			public:
				// constructor
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Statement::AlterPartitionStatement* pStatement_);
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Schema::LogData* pLogData_);
				// destructor
				~Alter();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undo();

				// data member set in constructor
				const Statement::AlterPartitionStatement* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Partition::Pointer m_pPartition;

				Partition::Target m_cPrevTarget;
				Partition::Target m_cPostTarget;
			};
		} // namespace ReorganizePartition
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_PARTITION_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
