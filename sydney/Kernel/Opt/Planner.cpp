// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Planner.cpp -- プランの作成および保持を行う
// 
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
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

#include "Opt/Planner.h"
#include "Opt/Argument.h"
#include "Opt/Configuration.h"
#include "Opt/Environment.h"
#include "Opt/Generator.h"
#include "Opt/SerialMemory.h"

#ifdef USE_OLDER_VERSION
#include "Analysis/Analyzer.h"
#include "Analysis/Environment.h"
#endif
#include "Analysis/Interface/IAnalyzer.h"

#include "Common/Assert.h"
#include "Common/ColumnMetaData.h"
#include "Common/InputArchive.h"
#include "Common/Message.h"
#include "Common/OutputArchive.h"
#include "Common/ResultSetMetaData.h"
#include "Common/SQLData.h"
#include "Common/UnicodeString.h"

#include "Communication/Connection.h"

#include "Exception/InvalidStatementIdentifier.h"
#include "Exception/NotSupported.h"
#include "Exception/Unexpected.h"

#include "Execution/Program.h"
#include "Execution/Interface/IProgram.h"

#include "Os/AutoCriticalSection.h"

#ifdef USE_OLDER_VERSION
#include "Plan/RelationInterface.h"
#include "Plan/ScalarInterface.h"
#include "Plan/TableInfo.h"
#include "Plan/Tuple.h"
#include "Plan/Variable.h"
#endif

#include "Plan/Interface/IRelation.h"

#include "Statement/Object.h"

#include "Trans/Transaction.h"

#include "ModMap.h"
#include "ModUnicodeString.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

namespace
{
	// 排他制御のためのラッチ
	Os::CriticalSection _latch;

	// Prepareにより保持するPlannerを保持するマップ
	typedef ModMap<int, Planner*, ModLess<int> > _Map;
	_Map _mapKeepPlan;

	// キャッシュにより保持するPlannerを保持するマップ

	// Prepareにより保持するPlannerを指すID
	int _iLastPrepareID = -1;
}

// FUNCTION public
//	Opt::Planner::Planner -- コンストラクター
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

Planner::
Planner()
	: m_cLatch(),
#ifdef USE_OLDER_VERSION
	  m_pProgram(0),
	  m_cUsingTableInfo(),
#endif
	  m_pStatement(0),
	  m_pMemory(0),
	  m_bRegenerate(false)
{
}

// FUNCTION public
//	Opt::Planner::~Planner -- デストラクター
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	なし
//
// EXCEPTIONS

//virtual
Planner::
~Planner()
{
	delete m_pStatement, m_pStatement = 0;
	delete m_pMemory, m_pMemory = 0;
#ifdef USE_OLDER_VERSION
	if (m_pProgram) {
		m_pProgram->terminate(true /* force */);
		delete m_pProgram, m_pProgram = 0;
	}
#endif
}


// FUNCTION public
//	Opt::Planner::getStatementType -- 保持したプランのSQL文タイプを得る
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	プランのSQL文タイプ
//
// EXCEPTIONS

Statement::Object*
Planner::
getStatement()
{
	return m_pStatement;
}

// FUNCTION public
//	Opt::Planner::getStatementType -- 保持したプランのSQL文タイプを得る
//
// NOTES
//
// ARGUMENTS
//	なし
//
// RETURN
//	プランのSQL文タイプ
//
// EXCEPTIONS

int
Planner::
getStatementType()
{
	return m_pStatement->getType();
}

// FUNCTION public
//	Opt::Planner::generate -- プランを生成する
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* cDatabase_
//		データベース
//	Execution::Program* pProgram_
//		生成する対象のプログラム
//	Communication::Connection* pConnection_
//		出力先のコネクション
//	Statement::Object* cStatement_
//		SQL文オブジェクト
//	Common::DataArrayData* pParameter_
//		パラメーター
//	Trans::Transaction& cTransaction_
//		トランザクション記述子
//	Explain::Option::Value iExplain_ = Explain::Option::None
//		explain plan tree
//	bool bPrepare_ = false
//		trueの場合、prepareのためにStatementをコピーしておく
//
// RETURN
//	なし
//
// EXCEPTIONS

void
Planner::
generate(Schema::Database* pDatabase_,
		 Execution::Program* pProgram_,
		 Communication::Connection* pConnection_,
		 Statement::Object* pStatement_,
		 Common::DataArrayData* pParameter_,
		 Trans::Transaction& cTransaction_,
		 Explain::Option::Value iExplain_ /* = Explain::Option::None */,
		 bool bPrepare_ /* = false */)
{
#ifdef USE_OLDER_VERSION
	if (Configuration::getOptimizerVersion().get() == Configuration::OptimizerVersion::Version2)
#endif
	{
		// まず新バージョンでプログラムが作れるか試す
		const Analysis::Interface::IAnalyzer* analyzer2 = pStatement_->getAnalyzer2();
		if (analyzer2 != 0) {
			try {
				// new version
				if (bPrepare_) {
					Execution::Program cProgram;
					generate2(analyzer2,
							  pDatabase_,
							  &cProgram,
							  pConnection_,
							  pStatement_,
							  pParameter_,
							  cTransaction_,
							  iExplain_,
							  bPrepare_);
				} else {
					generate2(analyzer2,
							  pDatabase_,
							  pProgram_,
							  pConnection_,
							  pStatement_,
							  pParameter_,
							  cTransaction_,
							  iExplain_,
							  bPrepare_);
				}
				return;
			} catch (Exception::NotSupported&) {
#ifdef USE_OLDER_VERSION
				if (Configuration::getUseOlderVersion().get()) {
					// continue to old version
					SydInfoMessage << "generate2 failed, use older version." << ModEndl;

					if (bPrepare_ && m_pMemory) {
						delete m_pMemory, m_pMemory = 0;
					}
				} else
#endif
				{
					throw;
				}
			}
		}
	}

#ifdef USE_OLDER_VERSION
	// Opt::OptimizerVersionが0か、新バージョンでNotSupportedが返ったら旧バージョン
	Analysis::Environment cAnalysisEnv;

	cAnalysisEnv.setPreparing(bPrepare_);
	cAnalysisEnv.setParameter(pParameter_);

	// Statement::Object を解析し Plan を作成する
	const Analysis::Analyzer* analyzer = pStatement_->getAnalyzer();
	Communication::Connection* pConnection = pConnection_;

	bool bAnalyzed =
		analyzer->analyze(cAnalysisEnv, pStatement_, pDatabase_, pConnection, cTransaction_);

	const SHORTVECTOR<Plan::RelationInterfacePointer>& vecPlan = cAnalysisEnv.getPlan();
	const SHORTVECTOR<Plan::ScalarInterfacePointer>& vecPlaceHolder = cAnalysisEnv.getPlaceHolderScalar();
	bool bUpdate = pStatement_->isRecordLog();

	if (bPrepare_) {
		// PrepareとしてProgramを作る
		if (m_pProgram) {
			; _SYDNEY_ASSERT(m_pProgram->getImplementationVersion()
							 == Execution::Program::Version::V1);
			m_pProgram->terminate();
		} else {
			m_pProgram = new Execution::Program;
		}
		m_pProgram->setImplementation(Execution::Program::Version::V1);

		// Statementをコピーしておく
		m_pStatement = pStatement_->copy();
		if (bAnalyzed) {
			m_pProgram->setPlan(vecPlan, vecPlaceHolder, bUpdate, cTransaction_, pDatabase_, true /* preparing */);
			// 陳腐化を調べるためにTableInfoを記憶しておく
			m_cUsingTableInfo = cAnalysisEnv.getUsingTableInfo();

			// この時点でdetermine可能ならしておく
			if (!m_pProgram->isEmpty())
				m_pProgram->determine();
		} else {
			m_bRegenerate = true;
		}

	} else {
		; _SYDNEY_ASSERT(pProgram_);
		if (!bAnalyzed) {
			// Analyzation failed
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		pProgram_->setImplementation(Execution::Program::Version::V1);
		// プログラムに実行するプランとして設定する
		pProgram_->setPlan(vecPlan, vecPlaceHolder, bUpdate, cTransaction_, pDatabase_);
		// Connectionを設定する
		pProgram_->setConnection(pConnection);

		// Determine the plan here.
		if (!pProgram_->isEmpty()) {
			pProgram_->determine();

			if ((iExplain_ & Explain::Option::Explain) != 0) {
				explain(pProgram_, pConnection_, iExplain_);
			}
		}
	}
#else
	_SYDNEY_THROW0(Exception::NotSupported);
#endif
}

// FUNCTION private
//	Opt::Planner::generate2 -- generate using new optimizer
//
// NOTES
//
// ARGUMENTS
//	const Analysis::Interface::IAnalyzer* pAnalyzer_
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	Communication::Connection* pConnection_
//	Statement::Object* pStatement_
//	Common::DataArrayData* pParameter_
//	Trans::Transaction& cTransaction_
//	Explain::Option::Value iExplain_
//	bool bPrepare_
//	
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Planner::
generate2(const Analysis::Interface::IAnalyzer* pAnalyzer_,
		  Schema::Database* pDatabase_,
		  Execution::Program* pProgram_,
		  Communication::Connection* pConnection_,
		  Statement::Object* pStatement_,
		  Common::DataArrayData* pParameter_,
		  Trans::Transaction& cTransaction_,
		  Explain::Option::Value iExplain_,
		  bool bPrepare_)
{
	// create interface implementation object as V2 interface
	pProgram_->setImplementation(Execution::Program::Version::V2);
	pProgram_->getProgram()->setIsUpdate(pStatement_->isRecordLog());

	// create Environment object for storing all the objects during optimization
	AUTOPOINTER<Environment> pOptEnv =
		Environment::create(EnvironmentArgument(
								 pDatabase_,
								 cTransaction_,
								 pProgram_,
								 pConnection_,
								 pParameter_,
								 bPrepare_));

	// generate program from the relation
	bool bGenerated = Generator::generate(*pOptEnv,
										  pAnalyzer_,
										  pStatement_);

	if (bPrepare_) {
		// store copied Statement
		m_pStatement = pStatement_->copy();
		// if not generated, regenerate is needed in generatePrepared
		m_bRegenerate = !bGenerated;

		if (bGenerated) {
			// serialize program
			if (m_pMemory == 0) {
				m_pMemory = new SerialMemory;
			} else {
				m_pMemory->resetSerial();
			}
			// output to archive
			Common::OutputArchive cArchive(*m_pMemory);
			cArchive.writeObject(pProgram_->getProgram());
		}
	} else {
		if (!bGenerated) {
			// Analyzation failed
			_SYDNEY_THROW0(Exception::NotSupported);
		}
		if ((iExplain_ & Explain::Option::Explain) != 0) {
			explain2(pOptEnv, pProgram_, pConnection_, iExplain_);
		}
	}
}

// FUNCTION private
//	Opt::Planner::generatePrepared2 -- generate pepared statement using new optimizer
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	Communication::Connection* pConnection_
//	Common::DataArrayData* pParameter_
//	Trans::Transaction& cTransaction_
//	Explain::Option::Value iExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Planner::
generatePrepared2(Schema::Database* pDatabase_,
				  Execution::Program* pProgram_,
				  Communication::Connection* pConnection_,
				  Common::DataArrayData* pParameter_,
				  Trans::Transaction& cTransaction_,
				  Explain::Option::Value iExplain_)
{
	// create interface implementation object as V2 interface
	pProgram_->setImplementation(Execution::Program::Version::V2);

	// restore program from archive
	; _SYDNEY_ASSERT(m_pMemory);
	Common::InputArchive cArchive(*m_pMemory);
	AUTOPOINTER<Execution::Interface::IProgram> pProgram =
		_SYDNEY_DYNAMIC_CAST(Execution::Interface::IProgram*,
							 cArchive.readObject());
	if (pProgram.get() == 0) {
		_SYDNEY_THROW0(Exception::Unexpected);
	}

	// check schema information validity
	pProgram->setDatabase(pDatabase_);
	pProgram->setTransaction(cTransaction_);
	if (pProgram->isObsoleteSchema()) {
		// any schema information is obsolete -> generate again
		AUTOPOINTER<Statement::Object> pStatement = m_pStatement;
		m_pStatement = 0;
		generate(pDatabase_, pProgram_, pConnection_, pStatement.get(),
				 pParameter_, cTransaction_, iExplain_, true);
		// generate prepared again
		generatePrepared(pDatabase_, pProgram_, pConnection_, pParameter_,
						 cTransaction_, iExplain_);
		return;
	}

	pProgram->setParameter(pParameter_);
	pProgram->setOutputConnection(pConnection_);

	// set to result
	pProgram_->setProgram(pProgram.release());

	if ((iExplain_ & Explain::Option::Explain) != 0) {
		explain2(0, pProgram_, pConnection_, iExplain_);
	}
}

// FUNCTION public
//	Opt::Planner::generatePrepared -- prepareしたプランからプログラムを生成する
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* cDatabase_
//		データベース
//	Execution::Program* pProgram_
//		生成する対象のプログラム
//	Communication::Connection* pConnection_
//		出力先のコネクション
//	Common::DataArrayData* pParameter_
//		パラメーター
//	Trans::Transaction& cTransaction_
//		トランザクション記述子
//	Explain::Option::Value iExplain_ = Explain::Option::None
//		explain plan tree
//
// RETURN
//	なし
//
// EXCEPTIONS

void
Planner::
generatePrepared(Schema::Database* pDatabase_,
				 Execution::Program* pProgram_,
				 Communication::Connection* pConnection_,
				 Common::DataArrayData* pParameter_,
				 Trans::Transaction& cTransaction_,
				 Explain::Option::Value iExplain_ /* = Explain::Option::None */)
{
	; _SYDNEY_ASSERT(pProgram_);

	if (m_bRegenerate) {
		// generate as individual statements for each execution
		generate(pDatabase_, pProgram_, pConnection_, m_pStatement,
				 pParameter_, cTransaction_, iExplain_);
		return;
	}

	if (m_pMemory) {
		// use V2 optimizer
		generatePrepared2(pDatabase_,
						  pProgram_,
						  pConnection_,
						  pParameter_,
						  cTransaction_,
						  iExplain_);
		return;
	}

#ifdef USE_OLDER_VERSION
	; _SYDNEY_ASSERT(m_pProgram);

	// スキーマ情報がまだ生きているかチェックする
	if (SIZE n = m_cUsingTableInfo.getSize()) {
		Plan::TableInfoSet::Iterator iterator = m_cUsingTableInfo.begin();
		do {
			if (!(*iterator)->isValid(cTransaction_))
				break;
			// 生きている場合は一旦terminateしておく
			(*iterator)->terminate();
			++iterator;
		} while (--n > 0);

		// 陳腐化していたら保存していたStatementを用いてprepareしなおす
		if (n) {
			AUTOPOINTER<Statement::Object> pStatement = m_pStatement;
			m_pStatement = 0;
			generate(pDatabase_, pProgram_, pConnection_, pStatement.get(),
					 pParameter_, cTransaction_, iExplain_, true);
			; _SYDNEY_ASSERT(m_pStatement);
		}
	}

	pProgram_->setImplementation(Execution::Program::Version::V1);

	// プログラムに実行するプランとして設定する
	pProgram_->setPlan(*m_pProgram, cTransaction_);
	// '?' の値を代入する
	pProgram_->assignParameter(pParameter_);
	// Connectionを設定する
	pProgram_->setConnection(pConnection_);

	// Determine the plan here if not determined yet.
	if (!pProgram_->isEmpty()) {
		pProgram_->determine();
		if ((iExplain_ & Explain::Option::Explain) != 0) {
			explain(pProgram_, pConnection_, iExplain_);
		}
	}
#else
	_SYDNEY_THROW0(Exception::NotSupported);
#endif
}

// FUNCTION public
//	Opt::Planner::compile -- 関数からプログラムを生成する
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Interface::IProgram* pProgram_
//	Statement::Object* pStatement_
//	Trans::Transaction& cTransaction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Planner::
compile(Schema::Database* pDatabase_,
		Execution::Program* pProgram_,
		Statement::Object* pStatement_,
		Trans::Transaction& cTransaction_)
{
	const Analysis::Interface::IAnalyzer* pAnalyzer = pStatement_->getAnalyzer2();
	if (pAnalyzer == 0) {
		// can't compile
		_SYDNEY_THROW0(Exception::NotSupported);
	}

	// create Environment object for storing all the objects during optimization
	AUTOPOINTER<Environment> pOptEnv =
		Environment::create(EnvironmentArgument(
								 pDatabase_,
								 cTransaction_,
								 pProgram_,
								 0,
								 0,
								 true /* prepare */));

	if (false == Generator::generate(*pOptEnv,
									 pAnalyzer,
									 pStatement_)) {
		_SYDNEY_THROW0(Exception::NotSupported);
	}
}

// 初期化
//static
void
Planner::
initialize()
{
}

// 終了処理
//static
void
Planner::
terminate()
{
	Os::AutoCriticalSection l(_latch);

	if (_mapKeepPlan.getSize() > 0) {
		_Map::Iterator iterator = _mapKeepPlan.begin();
		const _Map::Iterator& last = _mapKeepPlan.end();
		do {
			delete (*iterator).second;
			_mapKeepPlan.erase(iterator++);
		} while (iterator != last);
	}
}

// FUNCTION public
//	Opt::Planner::keep -- 後の利用のために保持しておく
//
// NOTES
//
// ARGUMENTS
//	Planner* pPlan_
//		Plannerオブジェクト
//
// RETURN
//	保持したPlannerオブジェクトを後から取得するためのID
//
// EXCEPTIONS

//static
int
Planner::
keep(Planner* pPlan_)
{
	Os::AutoCriticalSection l(_latch);

	int id = ++_iLastPrepareID;
	_mapKeepPlan.insert(id, pPlan_);

	return id;
}

// FUNCTION public
//	Opt::Planner::get -- 保持したプランを得る
//
// NOTES
//
// ARGUMENTS
//	int iID_
//		keep時に返したID
//
// RETURN
//	保持したPlannerオブジェクト
//
// EXCEPTIONS

//static
Planner&
Planner::
get(int iID_)
{
	Os::AutoCriticalSection l(_latch);

	_Map::Iterator iterator = _mapKeepPlan.find(iID_);
	if (iterator == _mapKeepPlan.end()) {
		//セッションでチェックしているので、まずありえない
		_SYDNEY_THROW0(Exception::InvalidStatementIdentifier);
	}
	return *(*iterator).second;
}

// FUNCTION public
//	Opt::Planner::erase -- 保持したプランを削除する
//
// NOTES
//
// ARGUMENTS
//	int iID_
//		keep時に返したID
//
// RETURN
//	なし
//
// EXCEPTIONS

//static
void
Planner::
erase(int iID_)
{
	Os::AutoCriticalSection l(_latch);

	_Map::Iterator iterator = _mapKeepPlan.find(iID_);
	if (iterator != _mapKeepPlan.end()) {
		Planner* pPlan = (*iterator).second;
		if (pPlan->m_cLatch.trylock()) {
			// ロックできたので使われていないと考える
			pPlan->m_cLatch.unlock();
			_mapKeepPlan.erase(iterator);

			delete pPlan;
		}
	}
}

#ifdef USE_OLDER_VERSION
// FUNCTION private
//	Opt::Planner::explain -- explain plan
//
// NOTES
//
// ARGUMENTS
//	Execution::Program* pProgram_
//	Communication::Connection* pConnection_
//	Explain::Option::Value iExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Planner::
explain(Execution::Program* pProgram_,
		Communication::Connection* pConnection_,
		Explain::Option::Value iExplain_)
{
	if (SIZE n = pProgram_->getPlanSize()) {
		Explain cExplain(iExplain_, pConnection_);
		cExplain.initialize();
		SIZE i = 0;
		do {
			pProgram_->getPlan(i)->explain(cExplain);
			cExplain.flush();
		} while (++i < n);
		cExplain.terminate();
	}
}
#endif

// FUNCTION private
//	Opt::Planner::explain2 -- explain plan for new optimizer
//
// NOTES
//
// ARGUMENTS
//	Execution::Program* pProgram_
//	Communication::Connection* pConnection_
//	Explain::Option::Value iExplain_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

void
Planner::
explain2(Opt::Environment* pEnvironment_,
		 Execution::Program* pProgram_,
		 Communication::Connection* pConnection_,
		 Explain::Option::Value iExplain_)
{
	Explain cExplain(iExplain_, pConnection_);
	cExplain.initialize();
	pProgram_->getProgram()->explain(pEnvironment_, cExplain);
	cExplain.terminate();
}

//
// Copyright (c) 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
