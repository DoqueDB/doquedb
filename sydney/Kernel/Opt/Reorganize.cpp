// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Reorganize.cpp -- 再構成用プランの作成および保持を行う
// 
// Copyright (c) 2006, 2008, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#include "Opt/Reorganize.h"
#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/Generator.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/Analyzer.h"
#include "Analysis/Environment.h"
#include "Analysis/Reorganize_Import.h"
#endif

#include "Analysis/Operation/Import.h"

#include "Exception/NotSupported.h"

#include "Execution/Program.h"
#include "Execution/Interface/IProgram.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

// FUNCTION public
//	Opt::Reorganize::generate -- Provide plans used in reorganization
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	const Opt::ImportArgument& cArgument_
//	Trans::Transaction& cTransaction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Reorganize::
generate(Schema::Database* pDatabase_,
		 Execution::Program* pProgram_,
		 const Opt::ImportArgument& cArgument_,
		 Trans::Transaction& cTransaction_)
{
#ifdef USE_OLDER_VERSION
	if (Configuration::getOptimizerVersion().get() == Configuration::OptimizerVersion::Version2) {
		// try new optimizer
		try {
			generate2(pDatabase_,
					  pProgram_,
					  cArgument_,
					  cTransaction_);
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
			  cArgument_,
			  cTransaction_);
#else
	generate2(pDatabase_,
			  pProgram_,
			  cArgument_,
			  cTransaction_);
#endif
}

#ifdef USE_OLDER_VERSION
// FUNCTION private
//	Opt::Reorganize::generate1 -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	const Opt::ImportArgument& cArgument_
//	Trans::Transaction& cTransaction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Reorganize::
generate1(Schema::Database* pDatabase_,
		  Execution::Program* pProgram_,
		  const Opt::ImportArgument& cArgument_,
		  Trans::Transaction& cTransaction_)
{
	Analysis::Environment cAnalysisEnv;
	Analysis::Reorganize_Import cAnalyzer(cArgument_);
	cAnalyzer.analyze(cAnalysisEnv, 0, pDatabase_, 0, cTransaction_);

	pProgram_->setImplementation(Execution::Program::Version::V1);

	const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan = cAnalysisEnv.getPlan();
	const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder = cAnalysisEnv.getPlaceHolderScalar();

	pProgram_->setPlan(vecPlan, vecPlaceHolder, false, cTransaction_, pDatabase_);
	// Determine the plan here.
	if (!pProgram_->isEmpty())
		pProgram_->determine();
}
#endif

// FUNCTION private
//	Opt::Reorganize::generate2 -- 
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	Opt::ImportArgument& cArgument_
//	Trans::Transaction& cTransaction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Reorganize::
generate2(Schema::Database* pDatabase_,
		  Execution::Program* pProgram_,
		  const Opt::ImportArgument& cArgument_,
		  Trans::Transaction& cTransaction_)
{
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
								 false));

	// analyzer for importing
	const Analysis::Operation::Import* pAnalyzer = Analysis::Operation::Import::create(cArgument_);
	bool bGenerated = Generator::generate(*pOptEnv,
										  pAnalyzer,
										  cArgument_);
	if (!bGenerated) {
		// analysis failed
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

//
// Copyright (c) 2006, 2008, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
