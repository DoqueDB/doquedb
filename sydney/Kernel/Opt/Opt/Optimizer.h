// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Optimizer.h -- オプティマイザー本体
// 
// Copyright (c) 2000, 2002, 2004, 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_OPTIMIZER_H
#define __SYDNEY_OPT_OPTIMIZER_H

#include "Opt/Module.h"
#include "Opt/Explain.h"

#include "Analysis/Declaration.h"
#include "Execution/Declaration.h"
#include "Opt/Declaration.h"
#include "Plan/Declaration.h"

_SYDNEY_BEGIN

namespace Common
{
class DataArrayData;
}

namespace Communication
{
class Connection;
}

namespace Statement
{
class Object;
}
namespace Trans
{
class Transaction;
	namespace Log
	{
		class Data;
	}
}

namespace Schema
{
class Database;
}

_SYDNEY_OPT_BEGIN

class Planner;

//
//	CLASS
//		Opt::Optimizer --
//
//	NOTES
//		オプティマイザー本体
//
class SYD_OPT_FUNCTION Optimizer
{
public:
	// 最適化してプログラムを生成する
	static void optimize(Schema::Database* pDatabase_,
						 Execution::Program* pProgram_,
						 Communication::Connection* pConnection_,
						 Statement::Object* pStatement_,
						 Common::DataArrayData* pParameter_,
						 Trans::Transaction* pTransaction_,
						 Explain::Option::Value iExplain_ = Explain::Option::None);
	// 最適化してプログラムを生成する(再構成用)
	static void import(Schema::Database* pDatabase_,
					   Execution::Program* pProgram_,
					   const Opt::ImportArgument& cArgument_,
					   Trans::Transaction& cTransaction_);

	// 最適化のみを行う
	static int prepare(Schema::Database* pDatabase_,
					   Communication::Connection* pConnection_,
					   Statement::Object* pStatement_,
					   Trans::Transaction* pTransaction_);
	static void prepare2(Planner& cPlanner_,
						 Schema::Database* pDatabase_,
						 Communication::Connection* pConnection_,
						 Statement::Object* pStatement_,
						 Trans::Transaction* pTransaction_);

	// 最適化したもののプログラムを生成する
	static void generate(Schema::Database* pDatabase_,
						 Execution::Program* pProgram_,
						 Communication::Connection* pConnection_,
						 int iPrepareID_,
						 Common::DataArrayData* pParameter_,
						 Trans::Transaction* pTransaction_,
						 Explain::Option::Value iExplain_ = Explain::Option::None);
	static void generate2(Schema::Database* pDatabase_,
						  Execution::Program* pProgram_,
						  Communication::Connection* pConnection_,
						  Planner& cPlanner_,
						  Common::DataArrayData* pParameter_,
						  Trans::Transaction* pTransaction_,
						  Explain::Option::Value iExplain_ = Explain::Option::None);

	// 関数からプログラムを生成する
	static void compile(Schema::Database* pDatabase_,
						Execution::Program* pProgram_,
						Statement::Object* pStatement_,
						Trans::Transaction* pTransaction_);

	// ロールバックのプログラムを生成する
	static void rollback(Schema::Database* pDatabase_,
						 Execution::Program* pProgram_,
						 const Trans::Log::Data* pLog_,
						 Trans::Transaction* pTransaction_);

	// ロールフォワードのプログラムを生成する
	static void rollforward(Schema::Database* pDatabase_,
							Execution::Program* pProgram_,
							const Trans::Log::Data* pLog_,
							Trans::Transaction* pTransaction_);

	// Undoのプログラムを生成する
	static void undo(Schema::Database* pDatabase_,
					 Execution::Program* pProgram_,
					 const Common::DataArrayData* pUndoLog_,
					 Trans::Transaction* pTransaction_);

	// 最適化を行ったSQL文のタイプを得る
	static int getPrepareStatementType(int iPrepareID_);
	// 最適化結果を削除する
	static void erasePrepareStatement(int iPrepareID_);

protected:
private:
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_OPTIMIZER_H

//
// Copyright (c) 2000, 2002, 2004, 2005, 2006, 2007, 2011, 2012, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
