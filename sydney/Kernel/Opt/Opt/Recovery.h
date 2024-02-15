// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Recovery.h -- リカバリー用プランの作成および保持を行う
// 
// Copyright (c) 2004, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_RECOVERY_H
#define __SYDNEY_OPT_RECOVERY_H

#include "Opt/Module.h"
#ifdef USE_OLDER_VERSION
#include "Plan/TypeDef.h"
#endif

#include "Opt/Declaration.h"

#include "Execution/Declaration.h"

_SYDNEY_BEGIN

namespace Common
{
	class DataArrayData;
}
namespace Communication
{
	class Connection;
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

//
//	CLASS
//		Opt::Recovery --
//
//	NOTES
//		プランの生成と保持を行う
//
class Recovery
{
public:
	//
	//	ENUM
	//	Opt::Recovery::Type::Value
	//
	//	NOTES
	//	リカバリーの種別をあらわす
	//
	struct Type
	{
		enum Value
		{
			Undefined = 0,

			RollBack,
			RollForward
		};
	};

	// リカバリーのプランを生成する
	static void generate(Schema::Database* pDatabase_,
						 Execution::Program* pProgram_,
						 const Opt::LogData* pLog_,
						 Trans::Transaction& cTransaction_,
						 Type::Value eRecoveryType_);

	// Undoのプランを生成する
	static void undo(Schema::Database* pDatabase_,
					 Execution::Program* pProgram_,
					 const Common::DataArrayData* pUndoLog_,
					 Trans::Transaction& cTransaction_);

protected:
private:
#ifdef USE_OLDER_VERSION
	// リカバリーのプランを生成する(v1)
	static void generate1(Schema::Database* pDatabase_,
						  Execution::Program* pProgram_,
						  const Opt::LogData* pLog_,
						  Trans::Transaction& cTransaction_,
						  Type::Value eRecoveryType_);
#endif
	// リカバリーのプランを生成する(v2)
	static void generate2(Schema::Database* pDatabase_,
						  Execution::Program* pProgram_,
						  const Opt::LogData* pLog_,
						  Trans::Transaction& cTransaction_,
						  Type::Value eRecoveryType_);

#ifdef USE_OLDER_VERSION
	// Undoのプランを生成する(v1)
	static void undo1(Schema::Database* pDatabase_,
					  Execution::Program* pProgram_,
					  const Common::DataArrayData* pUndoLog_,
					  Trans::Transaction& cTransaction_);
#endif
	// Undoのプランを生成する(v2)
	static void undo2(Schema::Database* pDatabase_,
					  Execution::Program* pProgram_,
					  const Common::DataArrayData* pUndoLog_,
					  Trans::Transaction& cTransaction_);
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_RECOVERY_H

//
// Copyright (c) 2004, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
