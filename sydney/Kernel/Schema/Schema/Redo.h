// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Redo.h -- Managerに属する関数のうちリカバリー関連のクラス定義、関数宣言
// 
// Copyright (c) 2001, 2002, 2005, 2006, 2007, 2009, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef	__SYDNEY_SCHEMA_REDO_H
#define	__SYDNEY_SCHEMA_REDO_H

#include "Schema/Module.h"
#include "Schema/ObjectID.h"

#include "Lock/Mode.h"
#include "Lock/Duration.h"

#include "Trans/LogFile.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace Admin
{
	namespace Recovery
	{
		class Database;
	}
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_SCHEMA_BEGIN

class Area;
class Database;
class Index;
class LogData;
class ObjectName;
class Table;

namespace Manager
{
	namespace SystemTable
	{
		namespace RedoUtility
		{
			Admin::Recovery::Database*
			mount(Trans::Transaction& trans,
				  const LogData& logData,
				  const Schema::ObjectName& dbName,
				  bool bRollforward_);

			void		unmount(Trans::Transaction& cTrans_,
								const Schema::LogData& cLogData_,
								const Schema::ObjectName& cDatabaseName_,
								bool bRollforward_);

			void		startBackup(Trans::Transaction& cTrans_,
									const Schema::LogData& cLogData_,
									const Schema::ObjectName& cDatabaseName_,
									bool bRollforward_);

			void		endBackup(Trans::Transaction& cTrans_,
								  const Schema::LogData& cLogData_,
								  const Schema::ObjectName& cDatabaseName_,
								  bool bRollforward_);

			Admin::Recovery::Database*
			createDatabase(Trans::Transaction& trans,
						   const LogData& logData,
						   const Schema::ObjectName& dbName,
						   bool bRollforward_);

			void		dropDatabase(Trans::Transaction& cTrans_,
									 const Schema::LogData& cLogData_,
									 const Schema::ObjectName& cDatabaseName_,
									 bool bRollforward_);
			void		moveDatabase(Trans::Transaction& cTrans_,
								 	 const Schema::LogData& cLogData_,
								 	 const Schema::ObjectName& cDatabaseName_,
									 bool bRollforward_);
			void		alterDatabase(Trans::Transaction& cTrans_,
								 	  const Schema::LogData& cLogData_,
								 	  const Schema::ObjectName& cDatabaseName_,
									  bool bRollforward_);

			void		createArea(Trans::Transaction& cTrans_,
								   const Schema::LogData& cLogData_,
								   const Schema::ObjectName& cDatabaseName_,
								   bool bRollforward_);
			void		dropArea(Trans::Transaction& cTrans_,
								 const Schema::LogData& cLogData_,
								 const Schema::ObjectName& cDatabaseName_,
								 bool bRollforward_);

			// Admin::Mountでも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			void		dropArea(Trans::Transaction& cTrans_,
								 ObjectID::Value iID_,
								 const Schema::ObjectName& cDatabaseName_,
								 Database* pDatabase_ = 0,
								 bool bMount_ = false);

			void		alterArea(Trans::Transaction& cTrans_,
								  const Schema::LogData& cLogData_,
								  const Schema::ObjectName& cDatabaseName_,
								  bool bRollforward_);
			// Admin::Mountでも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			void		alterArea(Trans::Transaction& cTrans_,
								  ObjectID::Value id_,
								  const ModVector<ModUnicodeString>& vecPrevPath_,
								  const ModVector<ModUnicodeString>& vecPostPath_,
								  const Schema::ObjectName& cDatabaseName_,
								  Area& cArea_, bool bMount_ = false);

			void		createTable(Trans::Transaction& cTrans_,
									const Schema::LogData& cLogData_,
									const Schema::ObjectName& cDatabaseName_,
									bool bRollforward_);
			void		dropTable(Trans::Transaction& cTrans_,
								  const Schema::LogData& cLogData_,
								  const Schema::ObjectName& cDatabaseName_,
								  bool bRollforward_);
			void		alterTable(Trans::Transaction& cTrans_,
								   const Schema::LogData& cLogData_,
								   const Schema::ObjectName& cDatabaseName_,
								   bool bRollforward_);
			void		alterTableArea(Trans::Transaction& cTrans_,
									   ObjectID::Value id_,
									   const Schema::LogData& cLogData_,
									   Schema::Database* cDatabase_,
									   bool bRollforward_);
			// Admin::Mountでも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			void		alterTable(Trans::Transaction& cTrans_,
								   ObjectID::Value iID_,
								   const ModVector<ObjectID::Value>& vecPrevAreaID_,
								   const ModVector<ObjectID::Value>& vecPostAreaID_,
								   const Schema::ObjectName& cDatabaseName_,
								   Table& cTable_,
								   bool bMount_ = false);
			void		createIndex(Trans::Transaction& cTrans_,
									const Schema::LogData& cLogData_,
									const Schema::ObjectName& cDatabaseName_,
									bool bRollforward_);
			void		dropIndex(Trans::Transaction& cTrans_,
								  const Schema::LogData& cLogData_,
								  const Schema::ObjectName& cDatabaseName_,
								  bool bRollforward_);
			void		alterIndex(Trans::Transaction& cTrans_,
								   const Schema::LogData& cLogData_,
								   const Schema::ObjectName& cDatabaseName_,
								   bool bRollforward_);
			// Admin::Mountでも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			void		alterIndex(Trans::Transaction& cTrans_,
								   ObjectID::Value iID_,
								   const ModVector<ObjectID::Value>& vecPrevAreaID_,
								   const ModVector<ObjectID::Value>& vecPostAreaID_,
								   const Schema::ObjectName& cDatabaseName_,
								   Index& cIndex_,
								   bool bMount_ = false);
			void		renameIndex(Trans::Transaction& cTrans_,
									const Schema::LogData& cLogData_,
									const Schema::ObjectName& cDatabaseName_,
									bool bRollforward_);

			void		createPrivilege(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::ObjectName& cDatabaseName_,
										bool bRollforward_);
			void		dropPrivilege(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::ObjectName& cDatabaseName_,
									  bool bRollforward_);
			void		alterPrivilege(Trans::Transaction& cTrans_,
									   const Schema::LogData& cLogData_,
									   const Schema::ObjectName& cDatabaseName_,
									   bool bRollforward_);

			void		createCascade(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::ObjectName& cDatabaseName_,
										bool bRollforward_);
			void		dropCascade(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::ObjectName& cDatabaseName_,
									  bool bRollforward_);
			void		alterCascade(Trans::Transaction& cTrans_,
									   const Schema::LogData& cLogData_,
									   const Schema::ObjectName& cDatabaseName_,
									   bool bRollforward_);

			void		createPartition(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::ObjectName& cDatabaseName_,
										bool bRollforward_);
			void		dropPartition(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::ObjectName& cDatabaseName_,
									  bool bRollforward_);
			void		alterPartition(Trans::Transaction& cTrans_,
									   const Schema::LogData& cLogData_,
									   const Schema::ObjectName& cDatabaseName_,
									   bool bRollforward_);

			void		createFunction(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::ObjectName& cDatabaseName_,
										bool bRollforward_);
			void		dropFunction(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::ObjectName& cDatabaseName_,
									  bool bRollforward_);
		}
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_REDO_H

//
// Copyright (c) 2001, 2002, 2005, 2006, 2007, 2009, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
