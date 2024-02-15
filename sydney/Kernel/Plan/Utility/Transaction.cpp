// -*-Mode: C++; tab-width: 4; c-basic-offset: 4;-*-
// vi:set ts=4 sw=4:
//
// Utility/Transaction.cpp --
// 
// Copyright (c) 2011, 2015, 2023 Ricoh Company, Ltd.
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
const char moduleName[] = "Plan::Utility";
}

#include "SyDefault.h"
#include "SyReinterpretCast.h"

#include "Plan/Utility/Transaction.h"
#include "Plan/Candidate/Argument.h"
#include "Plan/Candidate/Table.h"
#include "Plan/Relation/Table.h"

#include "Execution/Action/Argument.h"
#include "Execution/Utility/Transaction.h"

#include "Opt/Environment.h"

#include "Trans/Transaction.h"

_SYDNEY_BEGIN
_SYDNEY_PLAN_BEGIN
_SYDNEY_PLAN_UTILITY_BEGIN

/////////////////
// Locker
/////////////////

// FUNCTION public
//	Utility::Transaction::Locker::createArgument -- 
//
// NOTES
//
// ARGUMENTS
//	Opt::Environment& cEnvironment_
//	Candidate::AdoptArgument& cArgument_
//	Schema::Table* pSchemaTable_
//	bool bForceCollection_
//	Execution::Action::LockerArgument& cResult_
//	
// RETURN
//	bool
//
// EXCEPTIONS

bool
Transaction::Locker::
createArgument(Opt::Environment& cEnvironment_,
			   Candidate::AdoptArgument& cArgument_,
			   Schema::Table* pSchemaTable_,
			   bool bForceCollection_,
			   Execution::Action::LockerArgument& cResult_)
{
	cResult_.setTable(pSchemaTable_);				 

	if (cResult_.isNeedLock()) {
		cResult_.m_bIsPrepare = cEnvironment_.isPrepare();
		cResult_.m_bIsUpdate = cEnvironment_.isUpdateTable(pSchemaTable_)
			&& !cArgument_.m_bForLock;
		cResult_.m_bIsCollection = bForceCollection_ || cArgument_.m_bCollecting;

		return Execution::Utility::Transaction::getAdequateLock(
									cEnvironment_.getTransaction(),
									Lock::Name::Category::Tuple,
									true /* read only */,
									cEnvironment_.getTransaction().isBatchMode(),
									cResult_);
	}
	return false;
}

_SYDNEY_PLAN_END
_SYDNEY_PLAN_UTILITY_END
_SYDNEY_END

//
// Copyright (c) 2011, 2015, 2023 Ricoh Company, Ltd.
// All rights reserved.
//
