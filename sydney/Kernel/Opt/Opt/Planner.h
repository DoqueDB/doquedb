// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Planner.h -- プランの作成および保持を行う
// 
// Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_PLANNER_H
#define __SYDNEY_OPT_PLANNER_H

#include "Opt/Module.h"
#include "Opt/Explain.h"
#include "Opt/Declaration.h"

#include "Analysis/Declaration.h"
#include "Common/Object.h"
#include "Os/CriticalSection.h"
#ifdef USE_OLDER_VERSION
#include "Plan/ObjectSet.h"
#include "Plan/TypeDef.h"
#endif

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}
namespace Communication
{
	class Connection;
}
namespace Execution
{
	class Program;
}
namespace Schema
{
	class Database;
}
namespace Statement
{
	class Object;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_OPT_BEGIN

class SerialMemory;

//
//	CLASS
//		Opt::Planner --
//
//	NOTES
//		プランの生成と保持を行う
//
class Planner : public Common::Object
{
public:
	// コンストラクター
	SYD_OPT_FUNCTION
	Planner();
	// デストラクター
	SYD_OPT_FUNCTION
	~Planner();

	// 保持したプランのSQL文タイプを得る
	SYD_OPT_FUNCTION
	Statement::Object* getStatement();
	int getStatementType();

	// プランを生成する
	void generate(Schema::Database* pDatabase_,
				  Execution::Program* pProgram_,
				  Communication::Connection* pConnection_,
				  Statement::Object* pStatement_,
				  Common::DataArrayData* pParameter_,
				  Trans::Transaction& cTransaction_,
				  Explain::Option::Value iExplain_ = Explain::Option::None,
				  bool bPrepare_ = false);
	void generatePrepared(Schema::Database* pDatabase_,
						  Execution::Program* pProgram_,
						  Communication::Connection* pConnection_,
						  Common::DataArrayData* pParameter_,
						  Trans::Transaction& cTransaction_,
						  Explain::Option::Value iExplain_ = Explain::Option::None);

	// 関数からプログラムを生成する
	void compile(Schema::Database* pDatabase_,
				 Execution::Program* pProgram_,
				 Statement::Object* pStatement_,
				 Trans::Transaction& cTransaction_);

	// 初期化
	static void initialize();
	// 終了処理
	static void terminate();

	// 後の利用のために保持しておく
	static int keep(Planner* pPlan_);

	// 保持したプランを得る
	static Planner& get(int iID_);

	// 保持したプランを削除する
	static void erase(int iID_);

protected:
private:
#ifdef USE_OLDER_VERSION
	// explain plan
	void explain(Execution::Program* pProgram_,
				 Communication::Connection* pConnection_,
				 Explain::Option::Value iExplain_);
#endif

	// generate using new optimizer
	void generate2(const Analysis::Interface::IAnalyzer* pAnalyzer_,
				   Schema::Database* pDatabase_,
				   Execution::Program* pProgram_,
				   Communication::Connection* pConnection_,
				   Statement::Object* pStatement_,
				   Common::DataArrayData* pParameter_,
				   Trans::Transaction& cTransaction_,
				   Explain::Option::Value iExplain_,
				   bool bPrepare_);
	// generate pepared statement using new optimizer
	void generatePrepared2(Schema::Database* pDatabase_,
						   Execution::Program* pProgram_,
						   Communication::Connection* pConnection_,
						   Common::DataArrayData* pParameter_,
						   Trans::Transaction& cTransaction_,
						   Explain::Option::Value iExplain_);
	// explain plan for new optimizer
	void explain2(Opt::Environment* pEnvironment_,
				  Execution::Program* pProgram_,
				  Communication::Connection* pConnection_,
				  Explain::Option::Value iExplain_);

	Os::CriticalSection m_cLatch;
#ifdef USE_OLDER_VERSION
	Execution::Program* m_pProgram;
	Plan::TableInfoSet m_cUsingTableInfo;
#endif
	SerialMemory* m_pMemory;
	Statement::Object* m_pStatement;
	bool m_bRegenerate;
};

_SYDNEY_OPT_END
_SYDNEY_END

#ifdef USE_OLDER_VERSION
// TableInfoSetを使っているのでここでインクルードしておく
#include "Plan/TableInfo.h"
#endif

#endif // __SYDNEY_OPT_PLANNER_H

//
// Copyright (c) 2003, 2004, 2005, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
