// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeExecutor.h -- Declaration of classes concerning with executor reorganization
// 
// Copyright (c) 2006, 2011, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REORGANIZE_EXECUTOR_H
#define	__SYDNEY_SCHEMA_REORGANIZE_EXECUTOR_H

#include "Schema/Module.h"
#include "Schema/Manager.h"

#include "Opt/Declaration.h"

_SYDNEY_BEGIN

namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Database;
class LogData;

namespace Manager
{
	namespace SystemTable
	{
		// CLASS
		// Schema::Manager::SystemTable::ReorganizeExecutor -- base class for reorganize executor
		//
		// NOTES
		class ReorganizeExecutor
		{
		public:
			// constructor
			ReorganizeExecutor(Trans::Transaction& cTrans_)
				: m_cTrans(cTrans_)
			{}
			// destructor
			virtual ~ReorganizeExecutor() {}
		protected:
			// accessor
			Trans::Transaction& getTransaction()
			{return m_cTrans;}

			// log operation
			Trans::Log::LSN storeSystemLog(Schema::LogData& cLogData_);
			Trans::Log::LSN storeDatabaseLog(Schema::LogData& cLogData_,
											 Schema::Database* pDatabase_);

			// execute using plan module
			void executeProgram(Opt::ImportArgument& cArgument_, Schema::Database* pDatabase_);

			// configurations
			bool isCauseCheckpoint();
			void objectCheckWait();
			void alterDatabaseWait();
			void retryWait();
			void fileReflectWait();

		private:
			// log operation
			Trans::Log::LSN storeLog(Schema::LogData& cLogData_,
									 Trans::Log::File::Category::Value eLogCategory_,
									 Schema::Database* pDatabase_ = 0);

			Trans::Transaction& m_cTrans;
		};
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REORGANIZE_EXECUTOR_H

//
// Copyright (c) 2006, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
