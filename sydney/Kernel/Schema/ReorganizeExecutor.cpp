// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// ReorganizeExecutor.cpp -- Base class for reorganization
// 
// Copyright (c) 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Schema";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"
#include "SyDynamicCast.h"

#include "Schema/Database.h"
#include "Schema/LogData.h"
#include "Schema/Manager.h"
#include "Schema/Message.h"
#include "Schema/ReorganizeExecutor.h"

#include "Common/Assert.h"
#include "Common/Configuration.h"
#include "Common/Message.h"

#include "Execution/Executor.h"
#include "Execution/Program.h"

#include "Opt/Optimizer.h"

#include "Trans/AutoLatch.h"
#include "Trans/LogFile.h"
#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_SCHEMA_USING

namespace
{

// 再構成処理で既存データの反映中に行われた更新操作を反映するテストのために
// 既存データの反映処理を遅延させる時間
Common::Configuration::ParameterInteger _cFileReflectDelay("Schema_FileReflectDelay", 0);

// Start Backupで同時に読み書き属性の変更が起こったときのテストを
// 行うために属性を変更した後に遅延させる時間
Common::Configuration::ParameterInteger _cAlterDatabaseDelay("Schema_AlterDatabaseDelay", 0);

// 再構成処理で同時に同名や同パスのオブジェクトを作成しようとしたときに
// 正しくエラーとできるかテストするために遅延させる時間
Common::Configuration::ParameterInteger _cObjectCheckDelay("Schema_ObjectCheckDelay", 0);

// 再構成処理でDROPに失敗した後にリトライする前に待つ時間
Common::Configuration::ParameterInteger _cReorganizationRetryWait("Schema_ReorganizationRetryWait", 5000);

// 特定の種類の再構成処理の後にチェックポイントを起こすか
Common::Configuration::ParameterBoolean _cCauseCheckpoint("Schema_CauseCheckpoint", false);

} // namespace

///////////////////////////////////////////////////////
// Manager::SystemTable::ReorganizeUtility::Executor
///////////////////////////////////////////////////////

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::storeSystemLog -- store log data to system log file
//
// NOTES
//
// ARGUMENTS
//	Schema::LogData& cLogData_
//
// RETURN
//	Trans::Log::LSN
//
// EXCEPTIONS

Trans::Log::LSN
Manager::SystemTable::ReorganizeExecutor::
storeSystemLog(Schema::LogData& cLogData_)
{
	// store in system log file
	return storeLog(cLogData_, Trans::Log::File::Category::System);
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::storeDatabaseLog -- store log data to database log file
//
// NOTES
//
// ARGUMENTS
//	Schema::LogData& cLogData_
//	Schema::Database* pDatabase_
//	
// RETURN
//	Trans::Log::LSN
//
// EXCEPTIONS

Trans::Log::LSN
Manager::SystemTable::ReorganizeExecutor::
storeDatabaseLog(Schema::LogData& cLogData_, Schema::Database* pDatabase_)
{
	// store in database log file
	getTransaction().setLog(*pDatabase_);

	return storeLog(cLogData_, Trans::Log::File::Category::Database, pDatabase_);
}

// FUNCTION private
//	Schema::Manager::SystemTable::ReorganizeExecutor::storeLog -- log operation
//
// NOTES
//
// ARGUMENTS
//	Schema::LogData& cLogData_
//	Trans::Log::File::Category::Value eLogCategory_
//	Schema::Database* pDatabase_ = 0
//	
// RETURN
//	Trans::Log::LSN
//
// EXCEPTIONS

Trans::Log::LSN
Manager::SystemTable::ReorganizeExecutor::
storeLog(Schema::LogData& cLogData_,
		 Trans::Log::File::Category::Value eLogCategory_,
		 Schema::Database* pDatabase_ /* = 0 */)
{
#ifdef DEBUG
#ifndef SYD_COVERAGE
	SydSchemaDebugMessage << "Reorganize storing log = " << cLogData_.toString() << ModEndl;
#endif
#endif
	Trans::Log::AutoFile cLogFile =
		(eLogCategory_ == Trans::Log::File::Category::System)
		? Manager::SystemTable::getLogFile()
		: pDatabase_->getLogFile();

	Trans::AutoLatch cAutoLatch(m_cTrans, cLogFile->getLockName());
	Trans::Log::LSN iResult =
		LogData::storeLog(m_cTrans, cLogData_, eLogCategory_);
	// flush log here so that undo process can get log information in case
	m_cTrans.flushLog(eLogCategory_);

	return iResult;
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::executeProgram -- execute a program using plan module
//
// NOTES
//
// ARGUMENTS
//	Opt::ImportArgument& cArgument_
//	Schema::Database* pDatabase_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeExecutor::
executeProgram(Opt::ImportArgument& cArgument_, Schema::Database* pDatabase_)
{
	// create program
	Execution::Program cProgram;
	Opt::Optimizer().import(pDatabase_, &cProgram, cArgument_, getTransaction());
	// execute the program
	Execution::Executor().execute(cProgram);
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::isCauseCheckpoint -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	bool
//
// EXCEPTIONS

bool
Manager::SystemTable::ReorganizeExecutor::
isCauseCheckpoint()
{
	return _cCauseCheckpoint.get();
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::objectCheckWait -- 
//
// NOTES
//	Used for testing by delaying execution
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeExecutor::
objectCheckWait()
{
	// following parameter should be re-read always
	_cObjectCheckDelay.clear();
	if (int iDelay = _cObjectCheckDelay) {
		SydInfoMessage << "ObjectCheckDelay: Wait for " << iDelay << "(ms)" << ModEndl;
		// sleep for testing
		ModOsDriver::Thread::sleep(iDelay);
		SydInfoMessage << "ObjectCheckDelay: Proceed." << ModEndl;
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::alterDatabaseWait -- 
//
// NOTES
//	Used for testing by delaying execution
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeExecutor::
alterDatabaseWait()
{
	// following parameter should be re-read always
	_cAlterDatabaseDelay.clear();
	if (int iDelay = _cAlterDatabaseDelay) {
		// sleep for testing
		ModOsDriver::Thread::sleep(iDelay);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::retryWait -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeExecutor::
retryWait()
{
	if (int iDelay = _cReorganizationRetryWait) {
		ModOsDriver::Thread::sleep(iDelay);
	}
}

// FUNCTION public
//	Schema::Manager::SystemTable::ReorganizeExecutor::fileReflectWait -- 
//
// NOTES
//
// ARGUMENTS
//	Nothing
//
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Manager::SystemTable::ReorganizeExecutor::
fileReflectWait()
{
	// following parameter should be re-read always
	_cFileReflectDelay.clear();
	if (int iDelay = _cFileReflectDelay) {
		// sleep for testing
		ModOsDriver::Thread::sleep(iDelay);
	}
}

//
// Copyright (c) 2006, 2007, 2011, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
