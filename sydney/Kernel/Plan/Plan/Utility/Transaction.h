// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Transaction.h --
// 
// Copyright (c) 2011, 2023 Ricoh Company, Ltd.
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

#ifndef __SYDNEY_PLAN_UTILITY_TRANSACTION_H
#define __SYDNEY_PLAN_UTILITY_TRANSACTION_H

#include "Plan/Utility/Module.h"
#include "Plan/Declaration.h"

#include "Execution/Declaration.h"

_SYDNEY_BEGIN

namespace Schema
{
	class Table;
}

_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_UTILITY_BEGIN

namespace Transaction
{
	namespace Locker
	{
		bool createArgument(Opt::Environment& cEnvironment_,
							Candidate::AdoptArgument& cArgument_,
							Schema::Table* pSchemaTable_,
							bool bForceCollection_,
							Execution::Action::LockerArgument& cResult_);
	}
}

_SYDNEY_PLAN_END
_SYDNEY_PLAN_UTILITY_END
_SYDNEY_END

#endif // __SYDNEY_PLAN_UTILITY_TRANSACTION_H

//
//	Copyright (c) 2011, 2023 Ricoh Company, Ltd.
//	All rights reserved.
//
