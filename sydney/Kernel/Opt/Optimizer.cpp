// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Optimizer.cpp -- オプティマイザー本体
// 
// Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
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
#include "Opt/Optimizer.h"

#include "Opt/LogData.h"
#include "Opt/Message.h"
#include "Opt/Planner.h"
#include "Opt/Recovery.h"
#include "Opt/Reorganize.h"

#include "Common/Assert.h"
#include "Common/DataArrayData.h"
#include "Common/Parameter.h"
#include "Common/SystemParameter.h"

#include "Exception/BadArgument.h"

#include "Execution/Program.h"

#include "Communication/Connection.h"

#include "Os/AutoCriticalSection.h"

#include "Schema/Database.h"

#include "Statement/Object.h"

#include "ModAutoPointer.h"

_SYDNEY_USING
_SYDNEY_OPT_USING

//
//	FUNCTION public
//	Opt::Optimizer::optimize -- 最適化してプログラムを生成する
//
//	NOTES
//	最適化してプログラムを生成する
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	Communication::Connection* pConnection_
//	Statement::Object* pStatement_
//	Common::DataArrayData* pParameter_
//	Trans::Transaction* pTransaction_
//	Explain::Option::Value iExplain_ = Explain::Option::None
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	???
//
void
Optimizer::optimize(Schema::Database* pDatabase_,
					Execution::Program* pProgram_,
					Communication::Connection* pConnection_,
					Statement::Object* pStatement_,
					Common::DataArrayData* pParameter_,
					Trans::Transaction* pTransaction_,
					Explain::Option::Value iExplain_ /* = Explain::Option::None */)
{
	; _SYDNEY_ASSERT(pDatabase_);
	; _SYDNEY_ASSERT(pProgram_);
	; _SYDNEY_ASSERT(pStatement_);
	; _SYDNEY_ASSERT(pTransaction_);

	// 作成したプランを表すオブジェクト
	Planner cPlanner;

	// プランを生成する
	cPlanner.generate(pDatabase_,
					  pProgram_,
					  pConnection_,
					  pStatement_,
					  pParameter_,
					  *pTransaction_,
					  iExplain_);
}

// FUNCTION public
//	Opt::Optimizer::import -- 最適化してプログラムを生成する(再構成用)
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
Optimizer::
import(Schema::Database* pDatabase_,
	   Execution::Program* pProgram_,
	   const Opt::ImportArgument& cArgument_,
	   Trans::Transaction& cTransaction_)
{
	; _SYDNEY_ASSERT(pDatabase_);
	; _SYDNEY_ASSERT(pProgram_);

	Reorganize::generate(pDatabase_,
						 pProgram_,
						 cArgument_,
						 cTransaction_);
}

//
//	FUNCTION public
//	Opt::Optimizer::prepare -- 最適化のみ実行する
//
//	NOTES
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//	Communication::Connection* pConnection_
//	Statement::Object* pStatement_
//	Tras::Transaction* pTransaction_
//
//	RETURN
//	int
//		 最適化結果ID
//
//	EXCEPTIONS
//	???
//
int
Optimizer::prepare(Schema::Database* pDatabase_,
				   Communication::Connection* pConnection_,
				   Statement::Object* pStatement_,
				   Trans::Transaction* pTransaction_)
{
	// 作成したプランを表すオブジェクト
	ModAutoPointer<Planner> pPlanner = new Planner;
	// 最適化結果を作る
	prepare2(*pPlanner, pDatabase_, pConnection_, pStatement_, pTransaction_);
	// 最適化結果をプランマップに追加する
	return Planner::keep(pPlanner.release());
}

//
//	FUNCTION public
//	Opt::Optimizer::prepare2 -- 最適化のみ実行する(セッション間の共有なし)
//
//	NOTES
//
//	ARGUMENTS
//	Opt::Planner& cPlanner_
//	Schema::Database* pDatabase_
//	Communication::Connection* pConnection_
//	Statement::Object* pStatement_
//	Tras::Transaction* pTransaction_
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	???
//
void
Optimizer::prepare2(Planner& cPlanner_,
					Schema::Database* pDatabase_,
					Communication::Connection* pConnection_,
					Statement::Object* pStatement_,
					Trans::Transaction* pTransaction_)
{
	; _SYDNEY_ASSERT(pDatabase_);
	; _SYDNEY_ASSERT(pStatement_);
	; _SYDNEY_ASSERT(pTransaction_);

	// プランを生成する
	cPlanner_.generate(pDatabase_,
					   0,
					   pConnection_,
					   pStatement_,
					   0,
					   *pTransaction_,
					   Explain::Option::None,
					   true /* prepare */);
}

//
//	FUNCTION public
//	Opt::Optimizer::generate -- 最適化結果からプログラムを作成する
//
//	NOTES
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	Communication::Connection* pConnection_
//	int iPrepareID_
//	Common::DataArrayData* pParameter_
//	Trans::Transaction* pTransaction_
//	Explain::Option::Value iExplain_ = Explain::Option::None
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Optimizer::generate(Schema::Database* pDatabase_,
					Execution::Program* pProgram_,
					Communication::Connection* pConnection_,
					int iPrepareID_,
					Common::DataArrayData* pParameter_,
					Trans::Transaction* pTransaction_,
					Explain::Option::Value iExplain_ /* = Explain::Option::None */)
{
	; _SYDNEY_ASSERT(pDatabase_);
	; _SYDNEY_ASSERT(pProgram_);
	; _SYDNEY_ASSERT(pTransaction_);

	// 最適化結果を検索する
	Planner& cPlanner = Planner::get(iPrepareID_);

	// プログラムを作成する
	generate2(pDatabase_, pProgram_, pConnection_,
			  cPlanner, pParameter_, pTransaction_, iExplain_);
}

//
//	FUNCTION public
//	Opt::Optimizer::generate2 -- 最適化結果からプログラムを作成する(セッション間の共有なし)
//
//	NOTES
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	Communication::Connection* pConnection_
//	Opt::Planner& cPlanner_
//	Common::DataArrayData* pParameter_
//	Trans::Transaction* pTransaction_
//	Explain::Option::Value iExplain_ = Explain::Option::None
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Optimizer::generate2(Schema::Database* pDatabase_,
					 Execution::Program* pProgram_,
					 Communication::Connection* pConnection_,
					 Planner& cPlanner_,
					 Common::DataArrayData* pParameter_,
					 Trans::Transaction* pTransaction_,
					 Explain::Option::Value iExplain_ /* = Explain::Option::None */)
{
	; _SYDNEY_ASSERT(pDatabase_);
	; _SYDNEY_ASSERT(pProgram_);
	; _SYDNEY_ASSERT(pTransaction_);

	// プログラムを作成する
	cPlanner_.generatePrepared(pDatabase_,
							   pProgram_,
							   pConnection_,
							   pParameter_,
							   *pTransaction_,
							   iExplain_);
}

// FUNCTION public
//	Opt::Optimizer::compile -- 関数からプログラムを生成する
//
// NOTES
//
// ARGUMENTS
//	Schema::Database* pDatabase_
//	Execution::Program* pProgram_
//	Statement::Object* pStatement_
//	Trans::Transaction* pTransaction_
//	
// RETURN
//	Nothing
//
// EXCEPTIONS

//static
void
Optimizer::
compile(Schema::Database* pDatabase_,
		Execution::Program* pProgram_,
		Statement::Object* pStatement_,
		Trans::Transaction* pTransaction_)
{
	Planner cPlanner;
	cPlanner.compile(pDatabase_,
					 pProgram_,
					 pStatement_,
					 *pTransaction_);
}

//
//	FUNCTION public
//	Opt::Optimizer::rollback -- ロールバックを行うプログラムを作成する
//
//	NOTES
//	ロールバックを行うプログラムを作成する
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//		データベースオブジェクト
//	Execution::Program* pProgram_
//		プログラムクラスへのポインタ
//	const Trans::Log::Data* pLog_
//		ログデータ
//	const Trans::Transaction* pTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Optimizer::rollback(Schema::Database* pDatabase_,
					Execution::Program* pProgram_,
					const Trans::Log::Data* pLog_,
					Trans::Transaction* pTransaction_)
{
	const Opt::LogData* pLogData =
		_SYDNEY_DYNAMIC_CAST(const Opt::LogData*, pLog_);
	; _SYDNEY_ASSERT(pLog_ == 0 || pLogData);

	if (Configuration::getParameterOutput().isOutput())
	{
		if (pLogData)
		{
			OptParameterMessage << "Rollback: " << *pLogData << ModEndl;
		}
		else
		{
			OptParameterMessage << "Rollback: " << "(null)" << ModEndl;
		}
	}

	Recovery::generate(pDatabase_, pProgram_, pLogData, *pTransaction_, Recovery::Type::RollBack);
}

//
//	FUNCTION public
//	Opt::Optimizer::rollforward -- ロールフォワードを行うプログラムを作成する
//
//	NOTES
//	ロールフォワードを行うプログラムを作成する
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//		データベースオブジェクト
//	Execution::Program* pProgram_
//		プログラムクラスへのポインタ
//	const Trans::Log::Data* pLog_
//		ログデータ
//	const Trans::Transaction* pTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Optimizer::rollforward(Schema::Database* pDatabase_,
					   Execution::Program* pProgram_,
					   const Trans::Log::Data* pLog_,
					   Trans::Transaction* pTransaction_)
{
	const Opt::LogData* pLogData =
		_SYDNEY_DYNAMIC_CAST(const Opt::LogData*, pLog_);
	; _SYDNEY_ASSERT(pLog_ == 0 || pLogData);

	if (Configuration::getParameterOutput().isOutput())
	{
		if (pLogData)
		{
			OptParameterMessage << "Rollforward: " << *pLogData << ModEndl;
		}
		else
		{
			OptParameterMessage << "Rollforward: " << "(null)" << ModEndl;
		}
	}

	Recovery::generate(pDatabase_, pProgram_, pLogData, *pTransaction_, Recovery::Type::RollForward);
}

//
//	FUNCTION public
//	Opt::Optimizer::undo -- Undoログからリカバリーするプログラムを作成する
//
//	NOTES
//	Undoログからリカバリーするプログラムを作成する。
//
//	ARGUMENTS
//	Schema::Database* pDatabase_
//		データベースオブジェクト
//	Exeuction::Program* pProgram_
//		プログラムクラスへのポインタ
//	const Common::DataArrayData* pUndoLog_
//		ファイル単位のUndoログ
//	const Trans::Transaction* pTransaction_
//		トランザクション
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//	その他
//		下位の例外はそのまま再送
//
void
Optimizer::undo(Schema::Database* pDatabase_,
				Execution::Program* pProgram_,
				const Common::DataArrayData* pUndoLog_,
				Trans::Transaction* pTransaction_)
{
	if (Configuration::getParameterOutput().isOutput())
	{
		if (pUndoLog_)
		{
			OptParameterMessage << "Undo: " << *pUndoLog_ << ModEndl;
		}
		else
		{
			OptParameterMessage << "Undo: " << "(null)" << ModEndl;
		}
	}
	Recovery::undo(pDatabase_, pProgram_, pUndoLog_, *pTransaction_);
}

//
//	FUNCTION public static
//	Opt::Optimizer::getPrepareStatementType -- 最適化を行ったSQL文のタイプをえる
//
//	NOTES
//
//	ARGUMENTS
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	int
//		SQL文のタイプ
//
//  EXCEPTIONS
//
int
Optimizer::getPrepareStatementType(int iPrepareID_)
{
	return Planner::get(iPrepareID_).getStatementType();
}

//
//	FUNCTION public static
//	Opt::Optimizer::erasePrepareStatement -- 最適化結果を削除する
//
//	NOTES
//
//	ARGUMENTS
//	int iPrepareID_
//		最適化結果ID
//
//	RETURN
//	なし
//
//	EXCEPTIONS
//
void
Optimizer::erasePrepareStatement(int iPrepareID_)
{
	Planner::erase(iPrepareID_);
}

//
//	Copyright (c) 2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2010, 2011, 2012, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
