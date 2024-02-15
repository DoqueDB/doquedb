// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Recovery.cpp -- リカバリー用プランの作成および保持を行う
// 
// Copyright (c) 2004, 2005, 2008, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
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
	const char moduleName[] = "Opt";
	const char srcFile[] = __FILE__;
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/Recovery.h"
#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/Generator.h"
#include "Opt/LogData.h"
#include "Opt/UndoLog.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/Analyzer.h"
#include "Analysis/Environment.h"
#endif

#include "Common/Assert.h"

#include "Exception/NotSupported.h"

#include "Execution/Program.h"
#include "Execution/Interface/IProgram.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

// FUNCTION public
//	Opt::Recovery::generate -- リカバリーのプランを生成する
//
// NOTES
//	Rollbackや自動リカバリーで実行される
//
// ARGUMENTS
//	Schema::Database* cDatabase_
//		データベース
//	Execution::Program* pProgram_
//		生成する対象のプログラム
//	const Opt::LogData* pLog_
//		リカバリーする対象の論理ログデータ
//	Trans::Transaction& cTransaction_
//		トランザクション記述子
//	Opt::Recovery::Type::Value eRecoveryType_
//		リカバリー種別
//
// RETURN
//	なし
//
// EXCEPTIONS

//static
void
Recovery::
generate(Schema::Database* pDatabase_,
		 Execution::Program* pProgram_,
		 const Opt::LogData* pLog_,
		 Trans::Transaction& cTransaction_,
		 Type::Value eRecoveryType_)
{
#ifdef USE_OLDER_VERSION
	if (Configuration::getOptimizerVersion().get() == Configuration::OptimizerVersion::Version2) {
		// try new optimizer
		try {
			generate2(pDatabase_,
					  pProgram_,
					  pLog_,
					  cTransaction_,
					  eRecoveryType_);
			return;
		} catch (Exception::NotSupported&) {
			if (Configuration::getUseOlderVersion().get()) {
				// continue to old version
				SydInfoMessage << "generate2 failed, use older version." << ModEndl;
			} else {
				throw;
			}
		}
	}

	generate1(pDatabase_,
			  pProgram_,
			  pLog_,
			  cTransaction_,
			  eRecoveryType_);
#else
	generate2(pDatabase_,
			  pProgram_,
			  pLog_,
			  cTransaction_,
			  eRecoveryType_);
#endif
}

// FUNCTION public
//	Opt::Recovery::undo -- Undoのプランを生成する
//
// NOTES
//	ロールバックでファイル単位の処理を取り消すために実行される
//
// ARGUMENTS
//	Schema::Database* cDatabase_
//		データベース
//	Execution::Program* pProgram_
//		生成する対象のプログラム
//	const Common::DataArrayData* pUndoLog_
//		リカバリーする対象のUndoログデータ
//	Trans::Transaction& cTransaction_
//		トランザクション記述子
//
// RETURN
//	なし
//
// EXCEPTIONS

//static
void
Recovery::
undo(Schema::Database* pDatabase_,
	 Execution::Program* pProgram_,
	 const Common::DataArrayData* pUndoLog_,
	 Trans::Transaction& cTransaction_)
{
#ifdef USE_OLDER_VERSION
	if (Configuration::getOptimizerVersion().get() == Configuration::OptimizerVersion::Version2) {
		// try new optimizer
		try {
			undo2(pDatabase_,
				  pProgram_,
				  pUndoLog_,
				  cTransaction_);
			return;
		} catch (Exception::NotSupported&) {
			if (Configuration::getUseOlderVersion().get()) {
				// continue to old version
				SydInfoMessage << "undo2 failed, use older version." << ModEndl;
			} else {
				throw;
			}
		}
	}

	undo1(pDatabase_,
		  pProgram_,
		  pUndoLog_,
		  cTransaction_);
#else
	undo2(pDatabase_,
		  pProgram_,
		  pUndoLog_,
		  cTransaction_);
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION private
//	Opt::Recovery::generate1 -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	const Opt::LogData* pLog_
//	Trans::Transaction& cTransaction_
//	Type::Value eRecoveryType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Recovery::
generate1(Schema::Database* pDatabase_,
		  Execution::Program* pProgram_,
		  const Opt::LogData* pLog_,
		  Trans::Transaction& cTransaction_,
		  Type::Value eRecoveryType_)
{
	Analysis::Environment cAnalysisEnv;
	pProgram_->setImplementation(Execution::Program::Version::V1);

	// LogDataからPlanを作成する
	{
		AUTOPOINTER<Analysis::Analyzer> pAnalyzer = pLog_->getAnalyzer(eRecoveryType_ == Type::RollBack ? true : false);
		pAnalyzer->analyze(cAnalysisEnv, 0, pDatabase_, 0, cTransaction_);
	}

	// 実行するプランとして登録する
	pProgram_->setPlan(cAnalysisEnv.getPlan(),
					   cAnalysisEnv.getPlaceHolderScalar(),
					   true, cTransaction_, pDatabase_);

	// Determine the plan here.
	if (!pProgram_->isEmpty())
		pProgram_->determine();
}
#endif

// FUNCTION private
//	Opt::Recovery::generate2 -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	const Opt::LogData* pLog_
//	Trans::Transaction& cTransaction_
//	Type::Value eRecoveryType_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Recovery::
generate2(Schema::Database* pDatabase_,
		  Execution::Program* pProgram_,
		  const Opt::LogData* pLog_,
		  Trans::Transaction& cTransaction_,
		  Type::Value eRecoveryType_)
{
	// create interface implementation object as V2 interface
	pProgram_->setImplementation(Execution::Program::Version::V2);
	pProgram_->getProgram()->setIsUpdate(true);

	bool bIsRollback = (eRecoveryType_ == Type::RollBack);

	// create Environment object for storing all the objects during optimization
	AUTOPOINTER<Environment> pOptEnv =
		Environment::create(EnvironmentArgument(
								 pDatabase_,
								 cTransaction_,
								 pProgram_,
								 0,
								 0,
								 false,
								 true /* recovery */,
								 bIsRollback));

	// analyzer for recovery
	const Analysis::Operation::Recovery* pAnalyzer = pLog_->getAnalyzer2(bIsRollback);
	if (Generator::generate(*pOptEnv,
							pAnalyzer,
							pLog_,
							bIsRollback) == false) {
		// can't create program
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

#ifdef USE_OLDER_VERSION
// FUNCTION private
//	Opt::Recovery::undo1 -- Undoのプランを生成する(v1)
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	const Common::DataArrayData* pUndoLog_
//	Trans::Transaction& cTransaction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Recovery::
undo1(Schema::Database* pDatabase_,
	  Execution::Program* pProgram_,
	  const Common::DataArrayData* pUndoLog_,
	  Trans::Transaction& cTransaction_)
{
	; _SYDNEY_ASSERT(pUndoLog_);

	if (int n = pUndoLog_->getCount() > 1) {

		pProgram_->setImplementation(Execution::Program::Version::V1);
		Analysis::Environment cAnalysisEnv;

		// UndoLogからプランを生成する
		{
			AUTOPOINTER<Analysis::Analyzer> pAnalyzer = Opt::UndoLog::getAnalyzer(pUndoLog_);
			pAnalyzer->analyze(cAnalysisEnv, 0, pDatabase_, 0, cTransaction_);
		}

		// 実行するプランとして登録する
		pProgram_->setPlan(cAnalysisEnv.getPlan(),
						   cAnalysisEnv.getPlaceHolderScalar(),
						   true, cTransaction_, pDatabase_);

		// Determine the plan here.
		if (!pProgram_->isEmpty())
			pProgram_->determine();
	}
}
#endif

// FUNCTION private
//	Opt::Recovery::undo2 -- Undoのプランを生成する(v2)
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	const Common::DataArrayData* pUndoLog_
//	Trans::Transaction& cTransaction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Recovery::
undo2(Schema::Database* pDatabase_,
	  Execution::Program* pProgram_,
	  const Common::DataArrayData* pUndoLog_,
	  Trans::Transaction& cTransaction_)
{
	; _SYDNEY_ASSERT(pUndoLog_);
	if (int n = pUndoLog_->getCount() > 1) {

		// create interface implementation object as V2 interface
		pProgram_->setImplementation(Execution::Program::Version::V2);
		pProgram_->getProgram()->setIsUpdate(true);

		// create Environment object for storing all the objects during optimization
		AUTOPOINTER<Environment> pOptEnv =
			Environment::create(EnvironmentArgument(
									 pDatabase_,
									 cTransaction_,
									 pProgram_,
									 0,
									 0,
									 false,
									 true /* recovery */,
									 true /* rollback */,
									 true /* undo */));

		const Analysis::Operation::UndoLog* pAnalyzer =
			Opt::UndoLog::getAnalyzer2(pUndoLog_);
		Generator::generate(*pOptEnv,
							pAnalyzer,
							pUndoLog_);
	}
}

//
// Copyright (c) 2004, 2005, 2008, 2009, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
