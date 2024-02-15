// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeArea.h -- Declaration of classes concerning with area reorganization
// 
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_AREA_H
#define	__SYDNEY_SCHEMA_REORGANIZE_AREA_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeExecutor.h"
#include "Schema/Area.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Statement
{
	class AreaDefinition;
	class DropAreaStatement;
}

_SYDNEY_SCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeArea
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizeArea::Base --
			//			base class for area reorganization
			// NOTES
			class Base : public ReorganizeExecutor
			{
			public:
				// constructor
				Base(Trans::Transaction& cTrans_, Database* pDatabase_,
					 bool bRedo_);
				// destructor
				virtual ~Base();
			protected:
				// accessor
				Database* getDatabase() {return m_pDatabase;}

				// lock operation
				void lock(Schema::Hold::Operation::Value eOperation_,
						  bool bWeaken_ = false, bool bLockDatabase_ = false);

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
				Database* m_pDatabase;
				bool m_bRedo;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeArea::Create --
			//			create area executor
			// NOTES
			class Create : public Base
			{
			public:
				// constructor
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   const Statement::AreaDefinition* pStatement_);
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
				const Statement::AreaDefinition* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Area::Pointer m_pArea;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeArea::Drop --
			//			drop area executor
			// NOTES
			class Drop : public Base
			{
			public:
				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 const Statement::DropAreaStatement* pStatement_);
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
				const Statement::DropAreaStatement* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Area::Pointer m_pArea;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeArea::Alter --
			//			alter area executor
			// NOTES
			class Alter : public Base
			{
			public:
				// constructor
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Statement::AlterAreaStatement* pStatement_);
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
				const Statement::AlterAreaStatement* m_pStatement;
				const Schema::LogData* m_pLogData;

				// data member set in process
				ModVector<ModUnicodeString> m_vecPrevPath;
				ModVector<ModUnicodeString> m_vecPostPath;
				int m_iStatus;
				Area::Pointer m_pArea;
			};
		} // namespace ReorganizeArea
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_AREA_H

//
// Copyright (c) 2006, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
