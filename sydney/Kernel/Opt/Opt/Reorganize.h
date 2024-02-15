// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Reorganize.h -- リカバリー用プランの作成および保持を行う
// 
// Copyright (c) 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_OPT_REORGANIZE_H
#define __SYDNEY_OPT_REORGANIZE_H

#include "Opt/Module.h"
#include "Opt/Declaration.h"

#include "Analysis/Declaration.h"
#include "Execution/Declaration.h"
#ifdef USE_OLDER_VERSION
#include "Plan/TypeDef.h"
#endif

_SYDNEY_BEGIN
namespace Schema
{
	class Database;
}
namespace Trans
{
	class Transaction;
}

_SYDNEY_OPT_BEGIN

//
//	CLASS
//		Opt::Reorganize --
//
//	NOTES
//		Provide plans used in reorganization
//
class Reorganize
{
public:
	static void generate(Schema::Database* pDatabase_,
						 Execution::Program* pProgram_,
						 const Opt::ImportArgument& cArgument_,
						 Trans::Transaction& cTransaction_);

protected:
private:
#ifdef USE_OLDER_VERSION
	static void generate1(Schema::Database* pDatabase_,
						  Execution::Program* pProgram_,
						  const Opt::ImportArgument& cArgument_,
						  Trans::Transaction& cTransaction_);
#endif
	static void generate2(Schema::Database* pDatabase_,
						  Execution::Program* pProgram_,
						  const Opt::ImportArgument& cArgument_,
						  Trans::Transaction& cTransaction_);
};

_SYDNEY_OPT_END
_SYDNEY_END

#endif // __SYDNEY_OPT_REORGANIZE_H

//
// Copyright (c) 2006, 2011, 2013, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
