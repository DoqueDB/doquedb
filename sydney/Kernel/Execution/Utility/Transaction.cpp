// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/Transaction.cpp --
// 
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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

namespace {
const char srcFile[] = __FILE__;
const char moduleName[] = "Execution::Utility";
}

#include "SyDefault.h"
#include "SyDynamicCast.h"
#include "SyReinterpretCast.h"

#include "Execution/Utility/Transaction.h"
#include "Execution/Action/Locker.h"
#include "Execution/Interface/IProgram.h"

#include "Common/DataInstance.h"
#include "Common/DecimalData.h"

#include "Exception/Unexpected.h"

#include "LogicalFile/AutoLogicalFile.h"
#include "LogicalFile/FileDriverManager.h"

#include "Schema/File.h"
#include "Schema/Table.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_EXECUTION_BEGIN
_SYDNEY_EXECUTION_UTILITY_BEGIN

// FUNCTION public
//	Utility::Transaction::getAdequateLock -- get adequate lock
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Lock::Name::Category::Value eLocked_
//	bool bReadOnly_
//	bool bBatchMode_
//	Action::LockerArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Transaction::
getAdequateLock(Trans::Transaction& cTrans_,
				Lock::Name::Category::Value eLocked_,
				bool bReadOnly_,
				bool bBatchMode_,
				Action::LockerArgument& cArgument_)
{
	return getAdequateLock(cTrans_,
						   eLocked_,
						   Lock::Name::Category::Tuple,
						   bReadOnly_,
						   bBatchMode_,
						   cArgument_);
}

// FUNCTION public
//	Utility::Transaction::getAdequateLock -- get adequate lock
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Lock::Name::Category::Value eLocked_
//	Lock::Name::Category::Value eManipulated_
//	bool bReadOnly_
//	bool bBatchMode_
//	Action::LockerArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Transaction::
getAdequateLock(Trans::Transaction& cTrans_,
				Lock::Name::Category::Value eLocked_,
				Lock::Name::Category::Value eManipulated_,
				bool bReadOnly_,
				bool bBatchMode_,
				Action::LockerArgument& cArgument_)
{
	if (cArgument_.m_bIsPrepare) {
		// if prepared, lockmode is determined in initialize
		return true;
	}

	// get by transaction table
	cTrans_.getAdequateLock(eLocked_, eManipulated_, bReadOnly_,
							cArgument_.m_eMode, cArgument_.m_eDuration,
							bBatchMode_);
	if (cArgument_.m_bIsUpdate && cArgument_.m_eMode == Lock::Mode::S) {
		cArgument_.m_eMode = Lock::Mode::U;
	}
	if (cArgument_.m_eDuration == Lock::Duration::Cursor
		&& (cArgument_.m_bIsUpdate || cArgument_.m_bIsCollection)) {
		cArgument_.m_eDuration = Lock::Duration::Statement;
	}
	return cArgument_.m_eMode != Lock::Mode::N;
}

// FUNCTION public
//	Utility::Transaction::attachLogicalFile -- attach logicalfile with latch specification
//
// NOTES
//
// ARGUMENTS
//	Trans::Transaction& cTrans_
//	Schema::File& cSchemaFile_
//	LogicalFile::AutoLogicalFile& cLogicalFile_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Transaction::
attachLogicalFile(Trans::Transaction& cTrans_,
				  Schema::File& cSchemaFile_,
				  LogicalFile::AutoLogicalFile& cLogicalFile_)
{
	cLogicalFile_.attach(*LogicalFile::FileDriverManager::getDriver(cSchemaFile_.getDriverID()),
						 cSchemaFile_.getFileID(),
						 Lock::FileName(
								cSchemaFile_.getDatabaseID(),
								cSchemaFile_.getTableID(),
								cSchemaFile_.getID()),
						 cTrans_,
						 cSchemaFile_.getTable(cTrans_)->isTemporary());
}

_SYDNEY_EXECUTION_UTILITY_END
_SYDNEY_EXECUTION_END
_SYDNEY_END

//
// Copyright (c) 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
