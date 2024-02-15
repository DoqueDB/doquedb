// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeDatabase.h -- Declaration of classes concerning with database reorganization
// 
// Copyright (c) 2006, 2007, 2008, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_DATABASE_H
#define	__SYDNEY_SCHEMA_REORGANIZE_DATABASE_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeExecutor.h"
#include "Schema/Database.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Server
{
	class Session;
}
namespace Statement
{
	class DatabaseDefinition;
	class DropDatabaseStatement;
	class AlterDatabaseStatement;
}

_SYDNEY_SCHEMA_BEGIN

class LogData;

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizeDatabase
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizeDatabase::Base --
			//			base class for database reorganization
			// NOTES
			class Base : public ReorganizeExecutor
			{
			public:
				// constructor
				Base(Trans::Transaction& cTrans_, bool bRedo_);
				// destructor
				virtual ~Base();
			protected:
				// lock operation
				void lock(Schema::Hold::Operation::Value eOperation_, Schema::ObjectID::Value iID_,
						  bool bCreating_ = false, bool bLockDatabase_ = true,
						  Schema::Database* pDatabase_ = 0);

				// is redo?
				bool isRedo() {return m_bRedo;}

			//////////////
			// ReorganizeExecutor::
			//	Trans::Transaction& getTransaction()
			//	void storeSystemLog(Schema::LogData& cLogData_);
			//	void storeDatabaseLog(Schema::LogData& cLogData_,
			//						  Schema::Database* pDatabase_);
			//	void executeProgram(Analysis::Analyzer* pAnalyzer_, Schema::Database* pDatabase_);
			//	void objectCheckDelay();
			//	bool isCauseCheckpoint();

			private:
				// data member set in constructor
				bool m_bRedo;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeDatabase::Create --
			//			create database executor
			// NOTES
			class Create : public Base
			{
			public:
				// constructor
				Create(Trans::Transaction& cTrans_,
					   const Statement::DatabaseDefinition* pStatement_,
					   Server::Session* pSession_);
				// destructor
				~Create();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undoAlways();
				void undo();

				// data member set in constructor
				const Statement::DatabaseDefinition* m_pStatement;

				// data member set in process
				Server::Session* m_pSession;
				int m_iStatus;
				Database::Pointer m_pDatabase;
				bool m_bFatal;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeDatabase::Drop --
			//			drop database executor
			// NOTES
			class Drop : public Base
			{
			public:
				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Schema::Database* pDatabase_,
					 const Statement::DropDatabaseStatement* pStatement_);
				// destructor
				~Drop();

				// execute
				Result::Value execute();

			private:
				// error recovery
				bool retry();

				// data member set in constructor
				const Statement::DropDatabaseStatement* m_pStatement;
				Database::Pointer m_pDatabase;

				// data member set in process
				int m_iStatus;
				bool m_bFatal;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeDatabase::Move --
			//			move database executor
			// NOTES
			class Move : public Base
			{
			public:
				// constructor
				Move(Trans::Transaction& cTrans_,
					 Schema::Database* pDatabase_,
					 const Statement::MoveDatabaseStatement* pStatement_);
				// destructor
				~Move();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undoAlways();
				void undo();

				// data member set in constructor
				const Statement::MoveDatabaseStatement* m_pStatement;
				Database::Pointer m_pDatabase;

				// data member set in process
				ModVector<ModUnicodeString> m_vecPrevPath;
				ModVector<ModUnicodeString> m_vecPostPath;
				int m_iStatus;
				bool m_bFatal;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizeDatabase::Alter --
			//			alter database executor
			// NOTES
			class Alter : public Base
			{
			public:
				// constructor
				Alter(Trans::Transaction& cTrans_,
					  Schema::Database* pDatabase_,
					  const Statement::AlterDatabaseStatement* pStatement_);
				// destructor
				~Alter();

				// execute
				Result::Value execute();

			private:
				// error recovery
				void undoAlways();
				void undo();

				// data member set in constructor
				const Statement::AlterDatabaseStatement* m_pStatement;
				Database::Pointer m_pDatabase;

				// data member set in process
				Database::Attribute m_cPrevAttribute;
				bool m_bPropagate;
				int m_iStatus;
				bool m_bFatal;
				bool m_bRecoveryChange;
			};

		} // namespace ReorganizeDatabase
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_DATABASE_H

//
// Copyright (c) 2006, 2007, 2008, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
