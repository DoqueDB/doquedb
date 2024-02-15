// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Opt/Generator.cpp --
// 
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Opt";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Opt/Generator.h"
#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/Explain.h"

#include "Analysis/Interface/IAnalyzer.h"
#include "Analysis/Operation/Import.h"
#include "Analysis/Operation/Recovery.h"
#include "Analysis/Operation/UndoLog.h"

#include "Common/Assert.h"

#include "Communication/Connection.h"

#include "Exception/DynamicParameterNotMatch.h"
#include "Exception/PrepareFailed.h"
#include "Exception/NotSupported.h"

#include "Execution/Interface/IIterator.h"
#include "Execution/Interface/IProgram.h"

#include "Lock/Duration.h"
#include "Lock/Mode.h"

#include "Plan/AccessPlan/Source.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Interface/ICandidate.h"
#include "Plan/Interface/IRelation.h"
#include "Plan/Scalar/Value.h"
#include "Plan/Relation/RowInfo.h"

#include "Schema/Database.h"

#include "Trans/Transaction.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

namespace
{

// FUNCTION local
//	$$$::_lockDatabase -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_lockDatabase(Opt::Environment& cEnvironment_)
{
	Lock::Mode::Value eMode;
	Lock::Duration::Value eDuration;
	cEnvironment_.getTransaction().getAdequateLock(
			   Lock::Name::Category::Database,
			   !cEnvironment_.getProgram()->isUpdate(), /* true if readonly */
			   eMode, eDuration);

	if (eMode != Lock::Mode::N) {
		Schema::Object::ID::Value iDatabaseID = cEnvironment_.getDatabase()->getID();
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
			_OPT_EXECUTION_MESSAGE
				<< "lockDatabase: " << iDatabaseID
				<< " " << eMode << ":" << eDuration << ModEndl;
		}
#endif
		cEnvironment_.getTransaction().lock(
						Lock::DatabaseName(iDatabaseID),
						eMode, eDuration,
						Lock::Timeout::Unlimited);
	}
}

// FUNCTION local
//	$$$::_convertLockDatabase -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_convertLockDatabase(Opt::Environment& cEnvironment_)
{
	Lock::Mode::Value eFromMode;
	Lock::Duration::Value eFromDuration;
	Lock::Mode::Value eToMode;
	Lock::Duration::Value eToDuration;

	cEnvironment_.getTransaction().getAdequateLock(
												   Lock::Name::Category::Database,
												   !cEnvironment_.getProgram()->isUpdate(), /* true if readonly */
												   eFromMode, eFromDuration);
	cEnvironment_.getTransaction().getAdequateLock(
												   Lock::Name::Category::Database, // locked
												   Lock::Name::Category::Database, // manipulated
												   false, /* not read only */
												   eToMode, eToDuration,
												   true /* batch mode */);

	if (eFromMode != Lock::Mode::N
		&& eToMode != Lock::Mode::N
		&& (eFromMode != eToMode
			|| eFromDuration != eToDuration)) {
		Schema::Object::ID::Value iDatabaseID = cEnvironment_.getDatabase()->getID();
#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_EXECUTION(Opt::Configuration::TraceLevel::Lock)) {
			_OPT_EXECUTION_MESSAGE
				<< "convert lockDatabase: " << iDatabaseID
				<< " " << eFromMode << ":" << eFromDuration
				<< " -> " << eToMode << ":" << eToDuration
				<< ModEndl;
		}
#endif
		cEnvironment_.getTransaction().convertLock(
												   Lock::DatabaseName(iDatabaseID),
												   eFromMode, eFromDuration,
												   eToMode, eToDuration,
												   Lock::Timeout::Unlimited);
	}
}

// FUNCTION local
//	$$$::_setParameter -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
_setParameter(Opt::Environment& cEnvironment_)
{
	if (cEnvironment_.isPrepare() == false) {
		const VECTOR<Plan::Scalar::Value*>& vecVariable = cEnvironment_.getPlaceHolder();
		if (SIZE n = vecVariable.GETSIZE()) {
			const Common::DataArrayData* pParameter = cEnvironment_.getParameter();
			SIZE iDataCount = pParameter == 0 ? 0 : pParameter->getCount();
			if (iDataCount < n) {
				_SYDNEY_THROW2(Exception::DynamicParameterNotMatch,
							   iDataCount, n);
			}
			// set variable
			for (SIZE i = 0; i < n; ++i) {
				if (vecVariable[i]) {
					vecVariable[i]->setData(pParameter->getElement(i));
				}
			}
		}
	}
}

// FUNCTION local
//	$$$::_createAccessPlan -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	Plan::Interface::ICandidate*
//
// EXCEPTIONS

Plan::Interface::ICandidate*
_createAccessPlan(Opt::Environment& cEnvironment_,
				  Plan::Interface::IRelation* pRelation_)
{
	// mark result row info as retrieved
	Plan::Relation::RowInfo* pRowInfo = pRelation_->getRowInfo(cEnvironment_);
	if (pRowInfo) {
		pRowInfo->retrieveAll(cEnvironment_);
	}
	// create access plan with empty accessplan::source
	// empty means no previous conditions.
	Plan::AccessPlan::Source cSource;
	Plan::Interface::ICandidate* pResult =
		pRelation_->createAccessPlan(cEnvironment_, 
									 cSource);
	return pResult;
}

// FUNCTION local
//	$$$::_adoptCandidate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Execution::Interface::IProgram& cProgram_
//	Plan::Interface::ICandidate* pCandidate_
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS
void
_adoptCandidate(Opt::Environment& cEnvironment_,
				Execution::Interface::IProgram& cProgram_,
				Plan::Interface::ICandidate* pCandidate_,
				Plan::Interface::IRelation* pRelation_)
{
	// adopt candidate
	// -> main iterator is obtained
	Plan::Candidate::AdoptArgument cArgument;
	cArgument.m_pQuery = pCandidate_->generateSQL(cEnvironment_);
	Execution::Interface::IIterator* pIterator =
		pCandidate_->adopt(cEnvironment_, cProgram_, cArgument);

	if (pIterator != 0) {
		// generate program for top level
		pIterator = pCandidate_->generateTop(cEnvironment_,
											 cProgram_,
											 pRelation_,
											 pIterator,
											 cArgument);
		// set main iterator to program interface
		cProgram_.addExecuteIterator(pIterator);
	}
}
} // namespace

////////////////////////////////
// Opt::Generator::

// FUNCTION public
//	Opt::Generator::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Analysis::Interface::IAnalyzer* pAnalyzer_
//	Statement::Object* pStatement_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Generator::
generate(Opt::Environment& cEnvironment_,
		 const Analysis::Interface::IAnalyzer* pAnalyzer_,
		 Statement::Object* pStatement_)
{
	// lock database
	_lockDatabase(cEnvironment_);

	try {
		// convert statement into relation
		Plan::Interface::IRelation* pRelation = 0;
		{
			Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();
			if (cEnvironment_.hasCascade()) {
				pRelation = pAnalyzer_->getDistributeRelation(cEnvironment_, pStatement_);
			} else {
				pRelation = pAnalyzer_->getRelation(cEnvironment_, pStatement_);
			}
		}

		// set flag for batch mode
		if (cEnvironment_.getTransaction().isBatchMode()) {
			// set special mode for batch transaction
			cEnvironment_.getProgram()->setBatchMode(true);
			// convert lock database
			_convertLockDatabase(cEnvironment_);
		}

		return generateMain(cEnvironment_,
							pRelation);

	} catch (Exception::PrepareFailed&) {
		// preparation failed
		return false;
	}
}

// FUNCTION public
//	Opt::Generator::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Analysis::Operation::Import* pAnalyzer_
//	const Opt::ImportArgument& cArgument_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Generator::
generate(Opt::Environment& cEnvironment_,
		 const Analysis::Operation::Import* pAnalyzer_,
		 const Opt::ImportArgument& cArgument_)
{
	Plan::Interface::IRelation* pRelation = 0;
	{
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();
		pRelation =
			pAnalyzer_->getRelation(cEnvironment_, cArgument_);
	}
	return generateMain(cEnvironment_,
						pRelation);
}

// FUNCTION public
//	Opt::Generator::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Analysis::Operation::Recovery* pAnalyzer_
//	const Opt::LogData* pLog_
//	bool bIsRollback_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Generator::
generate(Opt::Environment& cEnvironment_,
		 const Analysis::Operation::Recovery* pAnalyzer_,
		 const Opt::LogData* pLog_,
		 bool bIsRollback_)
{
	Plan::Interface::IRelation* pRelation = 0;
	{
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();
		pRelation =
			pAnalyzer_->getRelation(cEnvironment_, pLog_, bIsRollback_);
	}
	return pRelation ? generateMain(cEnvironment_,
									pRelation)
		: true; // just skip when generate returns 0
}

// FUNCTION public
//	Opt::Generator::generate -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	const Analysis::Operation::UndoLog* pAnalyzer_
//	const Common::DataArrayData* pUndoLog_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Generator::
generate(Opt::Environment& cEnvironment_,
		 const Analysis::Operation::UndoLog* pAnalyzer_,
		 const Common::DataArrayData* pUndoLog_)
{
	Plan::Interface::IRelation* pRelation = 0;
	{
		Opt::Environment::AutoPop cAutoPop = cEnvironment_.pushNameScope();
		pRelation =
			pAnalyzer_->getRelation(cEnvironment_, pUndoLog_);
	}
	return generateMain(cEnvironment_,
						pRelation);
}

// FUNCTION private
//	Opt::Generator::generateMain -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Plan::Interface::IRelation* pRelation_
//	
// RETURN
//	bool
//
// EXCEPTIONS

//static
bool
Generator::
generateMain(Opt::Environment& cEnvironment_,
			 Plan::Interface::IRelation* pRelation_)
{
	Execution::Interface::IProgram* pProgram = 0;
	if (pRelation_ == 0) {
		return false;
	} else {
		// set parameter data if exists
		_setParameter(cEnvironment_);

		// create access plan
		Plan::Interface::ICandidate* pCandidate =
			_createAccessPlan(cEnvironment_, pRelation_);

		if (pCandidate == 0) {
			// can't optimize
			return false;
		}

		// Program interface is obtained from environment
		pProgram = cEnvironment_.getProgram();

		// set database object
		// database object is used for locking
		pProgram->setDatabase(cEnvironment_.getDatabase());

		// adopt for main server
		_adoptCandidate(cEnvironment_, *pProgram, pCandidate, pRelation_);

#ifndef NO_TRACE
		if (_OPT_IS_OUTPUT_OPTIMIZATION(Opt::Configuration::TraceLevel::Normal)) {
			Opt::Explain cExplain(Opt::Explain::Option::Explain
								  | Opt::Explain::Option::File
								  | Opt::Explain::Option::Data,
								  0);
			cExplain.initialize();
			int n = pProgram->getExecuteIteratorCount();
			for (int i = 0; i < n; ++i) {
				pProgram->getExecuteIterator(i)->explain(&cEnvironment_, *pProgram, cExplain);
				_OPT_OPTIMIZATION_MESSAGE
					<< "Result Plan[" << i << "]:\n"
					<< cExplain.getString()
					<< ModEndl;
				cExplain.flush();
			}
			cExplain.terminate();
		}
#endif
	}

	; _SYDNEY_ASSERT(pProgram);



	if (!cEnvironment_.isPrepare()) {
		// if not prepared statement,
		// transaction and output connection can be set here.
		pProgram->setTransaction(cEnvironment_.getTransaction());
		pProgram->setOutputConnection(cEnvironment_.getConnection());
	}

	// succeeded
	return true;
}

//
// Copyright (c) 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
