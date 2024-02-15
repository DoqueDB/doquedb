// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Undo.h -- Managerに属する関数のうちリカバリー関連のクラス定義、関数宣言
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

#ifndef	__SYDNEY_SCHEMA_UNDO_H
#define	__SYDNEY_SCHEMA_UNDO_H

#include "Schema/Module.h"
#include "Schema/Object.h"

#include "Lock/Mode.h"
#include "Lock/Duration.h"

#include "Trans/LogFile.h"

#include "ModVector.h"

_SYDNEY_BEGIN

namespace OS
{
	class CriticalSection;
}

namespace Statement
{
	class Object;
}
namespace Trans
{
	class Transaction;
}

namespace Communication
{
	class Connection;
}

_SYDNEY_SCHEMA_BEGIN

class Area;
class AreaContent;
class Database;
class LogData;
class Sequence;

namespace Manager
{
	namespace SystemTable
	{
		namespace UndoUtility
		{
			void		undoMount(Trans::Transaction& cTrans_,
								  const Schema::LogData& cLogData_,
								  const Schema::Object::Name& cDatabaseName_,
								  bool redone, bool bRollforward_);

			void		undoUnmount(Trans::Transaction& cTrans_,
									const Schema::LogData& cLogData_,
									const Schema::Object::Name& cDatabaseName_,
									bool redone, bool bRollforward_);

			void		undoStartBackup(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::Object::Name& cDatabaseName_,
										bool bRollforward_);

			void		undoEndBackup(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::Object::Name& cDatabaseName_,
									  bool bRollforward_);

			void		undoCreateDatabase(Trans::Transaction& cTrans_,
										   const Schema::LogData& cLogData_,
										   const Schema::Object::Name& cDatabaseName,
										   bool redone_, bool bRollforward_);

			void		undoDropDatabase(Trans::Transaction& cTrans_,
										 const Schema::LogData& cLogData_,
										 const Schema::Object::Name& cDatabaseName_,
										 bool redone_, bool bRollforward_);

			void		undoMoveDatabase(Trans::Transaction& cTrans_,
										 const Schema::LogData& cLogData_,
										 const Schema::Object::Name& cDatabaseName_,
										 bool redone_, bool bRollforward_);

			void		undoAlterDatabase(Trans::Transaction& cTrans_,
										  const Schema::LogData& cLogData_,
										  const Schema::Object::Name& cDatabaseName_,
										  bool redone_, bool bRollforward_);

			void		undoCreateArea(Trans::Transaction& cTrans_,
									   const Schema::LogData& cLogData_,
									   const Schema::Object::Name& cDatabaseName_,
									   bool redone_, bool bRollforward_);

			void		undoDropArea(Trans::Transaction& cTrans_,
									 const Schema::LogData& cLogData_,
									 const Schema::Object::Name& cDatabaseName_,
									 bool redone_, bool bRollforward_);

			// Admin::Mountも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			void		undoDropArea(Trans::Transaction& cTrans_, ObjectID::Value areaID_,
									 const ModVector<ModUnicodeString>& paths_,
									 const Schema::Object::Name& cDatabaseName_,
									 bool redone_);

			void		undoAlterArea(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::Object::Name& cDatabaseName_,
									  bool redone_, bool bRollforward_);

			// Admin::Mountも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			void		undoAlterArea(Trans::Transaction& cTrans_,
									  ObjectID::Value id_,
									  const ModVector<ModUnicodeString>& vecPrevPath_,
									  const ModVector<ModUnicodeString>& vecPostPath_,
									  const Schema::Object::Name& cDatabaseName_,
									  bool redone_, bool bMount_ = false);

			void		undoCreateTable(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::Object::Name& cDatabaseName_,
										bool redone, bool bRollforward_);

			void		undoDropTable(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::Object::Name& cDatabaseName_,
									  bool redone, bool bRollforward_);

			void		undoAlterTable(Trans::Transaction& cTrans_,
									   const LogData& cLogData_,
									   const Schema::Object::Name& cDatabaseName_,
									   bool redone_, bool bRollforward_);

			// Admin::Mountも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			bool		undoAlterTable(Trans::Transaction& cTrans_,
									   const ObjectID::Value tableID_,
									   const ModVector<ObjectID::Value>& vecPostArea_,
									   const Schema::Object::Name& cDatabaseName_,
									   bool redone_, bool bMount_ = false);

			void		undoRenameTable(Trans::Transaction& cTrans_,
										const LogData& cLogData_,
										const Schema::Object::Name& cDatabaseName_,
										bool redone_, bool bRollforward_);

			void		undoAddColumn(Trans::Transaction& cTrans_,
									  const LogData& cLogData_,
									  const Schema::Object::Name& cDatabaseName_,
									  bool redone_, bool bRollforward_);

			void		undoAlterColumn(Trans::Transaction& cTrans_,
										const LogData& cLogData_,
										const Schema::Object::Name& cDatabaseName_,
										bool redone_, bool bRollforward_);

			void		undoAddConstraint(Trans::Transaction& cTrans_,
										  const LogData& cLogData_,
										  const Schema::Object::Name& cDatabaseName_,
										  bool redone_, bool bRollforward_);

			void		undoDropConstraint(Trans::Transaction& cTrans_,
										   const LogData& cLogData_,
										   const Schema::Object::Name& cDatabaseName_,
										   bool redone_, bool bRollforward_);

			void		undoCreateIndex(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::Object::Name& cDatabaseName_,
										bool redone, bool bRollforward_);

			void		undoDropIndex(Trans::Transaction& cTrans_,
									  const Schema::LogData& cLogData_,
									  const Schema::Object::Name& cDatabaseName_,
									  bool redone, bool bRollforward_);

			void		undoAlterIndex(Trans::Transaction& cTrans_,
									   const Schema::LogData& cLogData_,
									   const Schema::Object::Name& cDatabaseName_,
									   bool redone_, bool bRollforward_);

			// Admin::Mountも使用するのでExportする
			SYD_SCHEMA_FUNCTION
			bool		undoAlterIndex(Trans::Transaction& cTrans_,
									   const ObjectID::Value indexID_,
									   const ModVector<ObjectID::Value>& vecPostArea_,
									   const Schema::Object::Name& cDatabaseName_,
									   bool redone_, bool bMount_ = false);

			void		undoRenameIndex(Trans::Transaction& cTrans_,
										const LogData& cLogData_,
										const Object::Name& cDatabaseName_,
										bool redone_, bool bRollforward_);

			void		undoCreatePrivilege(Trans::Transaction& cTrans_,
											const Schema::LogData& cLogData_,
											const Schema::Object::Name& cDatabaseName_,
											bool redone_, bool bRollforward_);

			void		undoDropPrivilege(Trans::Transaction& cTrans_,
										  const Schema::LogData& cLogData_,
										  const Schema::Object::Name& cDatabaseName_,
										  bool redone_, bool bRollforward_);

			void		undoAlterPrivilege(Trans::Transaction& cTrans_,
										   const Schema::LogData& cLogData_,
										   const Schema::Object::Name& cDatabaseName_,
										   bool redone_, bool bRollforward_);

			void		undoCreateCascade(Trans::Transaction& cTrans_,
										  const Schema::LogData& cLogData_,
										  const Schema::Object::Name& cDatabaseName_,
										  bool redone_, bool bRollforward_);

			void		undoDropCascade(Trans::Transaction& cTrans_,
										const Schema::LogData& cLogData_,
										const Schema::Object::Name& cDatabaseName_,
										bool redone_, bool bRollforward_);

			void		undoAlterCascade(Trans::Transaction& cTrans_,
										 const Schema::LogData& cLogData_,
										 const Schema::Object::Name& cDatabaseName_,
										 bool redone_, bool bRollforward_);

			void		undoCreatePartition(Trans::Transaction& cTrans_,
											const Schema::LogData& cLogData_,
											const Schema::Object::Name& cDatabaseName_,
											bool redone_, bool bRollforward_);

			void		undoDropPartition(Trans::Transaction& cTrans_,
										  const Schema::LogData& cLogData_,
										  const Schema::Object::Name& cDatabaseName_,
										  bool redone_, bool bRollforward_);

			void		undoAlterPartition(Trans::Transaction& cTrans_,
										   const Schema::LogData& cLogData_,
										   const Schema::Object::Name& cDatabaseName_,
										   bool redone_, bool bRollforward_);

			void		undoCreateFunction(Trans::Transaction& cTrans_,
										   const Schema::LogData& cLogData_,
										   const Schema::Object::Name& cDatabaseName_,
										   bool redone_, bool bRollforward_);

			void		undoDropFunction(Trans::Transaction& cTrans_,
										 const Schema::LogData& cLogData_,
										 const Schema::Object::Name& cDatabaseName_,
										 bool redone_, bool bRollforward_);

			// ファイルの移動をUNDOする(alterTable、renameTableの下請け)
			void		undoMovedFiles(Trans::Transaction& cTrans_, const LogData& cLogData_,
									   const Object::Name& cDatabaseName_, const ObjectID::Value& iTableID_,
									   const Object::Name& cPrevName_, const Object::Name& cPostName_,
									   const ModVector<ModUnicodeString>& vecDatabasePath_,
									   const ModVector<Object::ID::Value>& vecPrevAreaID_,
									   const ModVector<Object::ID::Value>& vecPostAreaID_,
									   int iLogIndexMovedFiles_, int iLogIndexPrevAreaPaths_, int iLogIndexPostAreaPaths_);

			// ファイルの作成をUNDOする(addColumnの下請け)
			void undoCreatedFiles(Trans::Transaction& cTrans_, const LogData& cLogData_,
								  const Object::Name& cDatabaseName_, const ObjectID::Value& iTableID_,
								  const Object::Name& cTableName_,
								  const ModVector<ModUnicodeString>& vecDatabasePath_,
								  const ModVector<Object::ID::Value>& vecAreaID_,
								  int iLogIndexCreatedFiles_,
								  int iLogIndexAreaPaths_,
								  int iLogIndexTempSuffix_ = -1);
		}
	}
}

_SYDNEY_SCHEMA_END
_SYDNEY_END

#endif	// __SYDNEY_SCHEMA_UNDO_H

//
// Copyright (c) 2001, 2002, 2005, 2006, 2007, 2009, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
