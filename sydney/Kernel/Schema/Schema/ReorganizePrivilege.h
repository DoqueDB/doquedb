// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizePrivilege.h -- Declaration of classes concerning with privilege reorganization
// 
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_PRIVILEGE_H
#define	__SYDNEY_SCHEMA_REORGANIZE_PRIVILEGE_H

#include "Schema/Module.h"
#include "Schema/Hold.h"
#include "Schema/Manager.h"
#include "Schema/ReorganizeExecutor.h"
#include "Schema/Privilege.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Server
{
	class Session;
}
namespace Statement
{
	class GrantStatement;
	class RevokeStatement;
	class IdentifierList;
}

_SYDNEY_SCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		namespace ReorganizePrivilege
		{
			// CLASS
			// Schema::Manager::SystemTable::ReorganizePrivilege::Base --
			//			base class for privilege reorganization
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
			// Schema::Manager::SystemTable::ReorganizePrivilege::Create --
			//			create privilege executor
			// NOTES
			class Create : public Base
			{
			public:
				// constructor
				Create(Trans::Transaction& cTrans_,
					   Database* pDatabase_,
					   Server::Session* pSession_);
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
				Server::Session* m_pSession;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Privilege::Pointer m_pPrivilege;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizePrivilege::Alter --
			//			grant/revoke privilege executor
			// NOTES
			class Alter : public Base
			{
			public:
				// constructor
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Statement::GrantStatement* pStatement_);
				Alter(Trans::Transaction& cTrans_,
					  Database* pDatabase_,
					  const Statement::RevokeStatement* pStatement_);
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
				const Statement::IdentifierList* m_pRoles;
				const Statement::IdentifierList* m_pGrantee;
				bool m_bGrant;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Privilege::Pointer m_pPrivilege;
				ModVector<Common::Privilege::Value> m_vecPrevValue;
				ModVector<Common::Privilege::Value> m_vecPostValue;
			};

			// CLASS
			// Schema::Manager::SystemTable::ReorganizePrivilege::Drop --
			//			drop privilege executor
			// NOTES
			//	drop is called when a user is dropped
			class Drop : public Base
			{
			public:
				// constructor
				Drop(Trans::Transaction& cTrans_,
					 Database* pDatabase_,
					 int iUserID_);
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
				int m_iUserID;
				const Schema::LogData* m_pLogData;

				// data member set in process
				int m_iStatus;
				Privilege::Pointer m_pPrivilege;
			};
		} // namespace ReorganizePrivilege
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_PRIVILEGE_H

//
// Copyright (c) 2007, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
