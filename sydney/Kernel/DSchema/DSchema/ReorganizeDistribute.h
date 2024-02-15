// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeDistribute.h -- Declaration of classes concerning with reorganization
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

#ifndef	__SYDNEY_DSCHEMA_REORGANIZE_DISTRIBUTE_H
#define	__SYDNEY_DSCHEMA_REORGANIZE_DISTRIBUTE_H

#include "DSchema/Module.h"

#include "Common/LargeVector.h"

#include "DServer/OpenMPExecutor.h"

_SYDNEY_BEGIN

namespace DServer
{
	class Session;
}
namespace Schema
{
	class Database;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_DSCHEMA_BEGIN

namespace Manager
{
	namespace SystemTable
	{
		// CLASS
		// DSchema::Manager::SystemTable::ReorganizeDistribute -- base class for reorganize distribute
		//
		// NOTES
		class ReorganizeDistribute
		{
		public:
			typedef ReorganizeDistribute This;

			// constructor
			ReorganizeDistribute()
				: m_vecSession(),
				  m_vecStatus()
			{}
			// destructor
			virtual ~ReorganizeDistribute() {}

		protected:
			// start session
			void open(Schema::Database* pDatabase_,
					  Trans::Transaction& cTrans_);
			// end session
			void close();
			// execute statement
			void executeStatement(const ModUnicodeString& cstrSQL_);
			// calcel statement
			void cancelStatement(const ModUnicodeString& cstrSQL_,
								 Schema::Database* pDatabase_,
								 Trans::Transaction& cTrans_);

		private:
			void setSession(Schema::Database* pDatabase_,
							Trans::Transaction& cTrans_);
			void clearSession();
			void clearStatus();

			Common::LargeVector<DServer::Session*> m_vecSession;
			Common::LargeVector<DServer::OpenMPExecutor::ExecuteStatus::Value> m_vecStatus;
		};
	} // namespace SystemTable
} // namespace Manager

_SYDNEY_DSCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_DSCHEMA_REORGANIZE_DISTRIBUTE_H

//
// Copyright (c) 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
